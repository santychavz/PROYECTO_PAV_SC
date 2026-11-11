/*
 * COMPILADOR SIMPLE -> SML  (C++17)
 * Proyecto Simpletron - Programacion Avanzada
 *
 * Dos pasadas:
 *  1) Tabla de simbolos (lista enlazada), analisis de instrucciones,
 *     generacion SML preliminar y registro de referencias pendientes.
 *  2) Resolucion de saltos futuros y escritura de la memoria SML.
 *
 * Formato del simulador mejorado: OOAAA, 1000 posiciones, palabras
 * enteras de -99998 a +99998. Se generan programa.sml y programa.simp.
 *
 * Compilar desde la raiz del proyecto:
 *   g++ -std=c++17 -Wall -Wextra -pedantic CompiladorSimpletron/compiler.cpp -o output/compiler.exe
 * Ejecutar:
 *   ./output/compiler.exe CompiladorSimpletron/programa.simple
 */

#include <array>
#include <cctype>
#include <climits>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {
constexpr int MEM = 1000;
constexpr int MAX_WORD = 99998;
constexpr int READ = 10, WRITE = 11, LOAD = 20, STORE = 21;
constexpr int ADD = 30, SUBTRACT = 31, DIVIDE = 32, MULTIPLY = 33;
constexpr int MODULO = 34, POWER = 35;
constexpr int BRANCH = 40, BRANCHNEG = 41, BRANCHZERO = 42, HALT = 43;

struct Symbol {
    std::string symbol;
    char type; // L = numero de linea, V = variable, C = constante, T = temporal
    int location;
};

struct Token {
    std::string text;
};

bool isNumber(const std::string &s) {
    if (s.empty()) return false;
    for (char c : s) if (!std::isdigit(static_cast<unsigned char>(c))) return false;
    return true;
}

int number(const std::string &s, const std::string &what) {
    if (!isNumber(s)) throw std::runtime_error("Se esperaba un entero para " + what + ": '" + s + "'");
    try {
        long long n = std::stoll(s);
        if (n > INT_MAX) throw std::runtime_error("Entero demasiado grande para " + what);
        return static_cast<int>(n);
    } catch (const std::out_of_range &) {
        throw std::runtime_error("Entero demasiado grande para " + what);
    } catch (const std::invalid_argument &) {
        throw std::runtime_error("Entero invalido para " + what);
    }
}

bool isVariable(const std::string &s) {
    return s.size() == 1 && s[0] >= 'a' && s[0] <= 'z';
}

std::vector<Token> tokenize(const std::string &s) {
    std::vector<Token> out;
    for (size_t i = 0; i < s.size();) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        if (std::isspace(c)) { ++i; continue; }
        if (std::isdigit(c) || std::isalpha(c)) {
            size_t j = i + 1;
            while (j < s.size() && std::isalnum(static_cast<unsigned char>(s[j]))) ++j;
            out.push_back({s.substr(i, j - i)});
            i = j;
        } else if (i + 1 < s.size() &&
                   (s.substr(i, 2) == "<=" || s.substr(i, 2) == ">=" ||
                    s.substr(i, 2) == "==" || s.substr(i, 2) == "!=")) {
            out.push_back({s.substr(i, 2)}); i += 2;
        } else if (std::string("+-*/%^()[]{}=<>!").find(s[i]) != std::string::npos) {
            out.push_back({s.substr(i, 1)}); ++i;
        } else {
            throw std::runtime_error("Caracter no permitido: '" + s.substr(i, 1) + "'");
        }
    }
    return out;
}

bool opening(const std::string &s) { return s == "(" || s == "[" || s == "{"; }
bool closing(const std::string &s) { return s == ")" || s == "]" || s == "}"; }
bool matching(const std::string &a, const std::string &b) {
    return (a == "(" && b == ")") || (a == "[" && b == "]") || (a == "{" && b == "}");
}
bool arithmetic(const std::string &s) {
    return s == "+" || s == "-" || s == "*" || s == "/" || s == "%" || s == "^" ||
           s == "neg" || s == "pos";
}
int precedence(const std::string &s) {
    if (s == "^" ) return 4;
    if (s == "neg" || s == "pos") return 3;
    if (s == "*" || s == "/" || s == "%") return 2;
    if (s == "+" || s == "-") return 1;
    return 0;
}
bool rightAssociative(const std::string &s) {
    return s == "^" || s == "neg" || s == "pos";
}

// Convierte expresiones aritmeticas en un vector postfijo (Shunting-yard).
std::vector<std::string> infixToPostfix(const std::vector<Token> &tokens) {
    if (tokens.empty()) throw std::runtime_error("Expresion aritmetica vacia");
    std::vector<std::string> output, operators;
    bool needOperand = true;
    for (const Token &tok : tokens) {
        std::string s = tok.text;
        if (isVariable(s) || isNumber(s)) {
            if (!needOperand) throw std::runtime_error("Falta operador antes de '" + s + "'");
            output.push_back(s); needOperand = false;
        } else if (opening(s)) {
            if (!needOperand) throw std::runtime_error("Falta operador antes del agrupador");
            operators.push_back(s);
        } else if (closing(s)) {
            if (needOperand) throw std::runtime_error("Agrupador vacio o expresion incompleta");
            while (!operators.empty() && !opening(operators.back())) {
                output.push_back(operators.back()); operators.pop_back();
            }
            if (operators.empty() || !matching(operators.back(), s))
                throw std::runtime_error("Parentesis, corchetes o llaves sin pareja");
            operators.pop_back(); needOperand = false;
        } else if (arithmetic(s)) {
            if (needOperand) {
                if (s != "+" && s != "-")
                    throw std::runtime_error("Falta operando antes de '" + s + "'");
                // Unario se apila directamente: admite casos como 2^-3.
                operators.push_back(s == "-" ? "neg" : "pos");
            } else {
                while (!operators.empty() && arithmetic(operators.back()) &&
                       ((rightAssociative(s) && precedence(s) < precedence(operators.back())) ||
                        (!rightAssociative(s) && precedence(s) <= precedence(operators.back())))) {
                    output.push_back(operators.back()); operators.pop_back();
                }
                operators.push_back(s); needOperand = true;
            }
        } else {
            throw std::runtime_error("Token no valido dentro de expresion: '" + s + "'");
        }
    }
    if (needOperand) throw std::runtime_error("La expresion termina con un operador");
    while (!operators.empty()) {
        if (opening(operators.back())) throw std::runtime_error("Agrupador sin cerrar");
        output.push_back(operators.back()); operators.pop_back();
    }
    return output;
}

class Compiler {
public:
    void compile(const std::string &sourcePath, const std::string &smlPath, const std::string &simPath) {
        firstPass(sourcePath);
        secondPass();
        writeFile(smlPath);
        if (smlPath != simPath) writeFile(simPath);
        std::cout << "Compilacion correcta.\n";
        std::cout << "Instrucciones generadas: " << nextInstruction << "\n";
        std::cout << "Simbolos registrados: " << symbols.size() << "\n";
        std::cout << "Archivo SML: " << smlPath << "\n";
        std::cout << "Archivo para el simulador: " << simPath << "\n";
        printSymbols();
    }

private:
    std::array<int, MEM> memory{};
    std::array<int, MEM> flags{};
    std::list<Symbol> symbols; // Tabla de simbolos en lista ligada.
    int nextInstruction = 0;
    int nextData = MEM - 1;
    int previousLine = -1;
    int sourceLine = 0;
    int temporaries = 0;
    bool foundEnd = false;

    [[noreturn]] void fail(const std::string &reason) const {
        throw std::runtime_error("Linea fuente " + std::to_string(sourceLine) + ": " + reason);
    }

    Symbol *find(const std::string &s, char type) {
        for (Symbol &e : symbols) if (e.symbol == s && e.type == type) return &e;
        return nullptr;
    }

    int emit(int op, int address = 0) {
        if (address < 0 || address >= MEM) fail("Direccion SML fuera de memoria");
        if (nextInstruction > nextData) fail("Colision entre codigo y datos en memoria");
        int location = nextInstruction++;
        memory[location] = op * MEM + address;
        return location;
    }

    int dataSlot(const std::string &name, char type, int value = 0) {
        if (Symbol *e = find(name, type)) return e->location;
        if (nextData < nextInstruction) fail("Memoria insuficiente para variables/constantes");
        if (value < -MAX_WORD || value > MAX_WORD) fail("Constante fuera del rango SML");
        int addr = nextData--;
        memory[addr] = value;
        symbols.push_back({name, type, addr});
        return addr;
    }

    int constant(int value) {
        if (value < -MAX_WORD || value > MAX_WORD) fail("Constante fuera de rango (+/-99998)");
        // El simulador actual interpreta 9999 como centinela durante la carga.
        if (value == 9999) fail("La constante 9999 coincide con el centinela del simulador");
        return dataSlot(std::to_string(value), 'C', value);
    }

    int addressOf(const std::string &token) {
        if (isVariable(token)) return dataSlot(token, 'V');
        if (isNumber(token)) return constant(number(token, "constante"));
        fail("Operando invalido: '" + token + "'");
    }

    int temp() {
        return dataSlot("@" + std::to_string(temporaries++), 'T');
    }

    int evaluateToAddress(const std::vector<Token> &expression) {
        std::vector<std::string> postfix;
        try { postfix = infixToPostfix(expression); }
        catch (const std::exception &e) { fail(e.what()); }
        std::vector<int> stack;
        for (const std::string &t : postfix) {
            if (isVariable(t) || isNumber(t)) {
                stack.push_back(addressOf(t));
            } else if (t == "pos" || t == "neg") {
                if (stack.empty()) fail("Pila insuficiente para operador unario");
                int rhs = stack.back(); stack.pop_back();
                if (t == "pos") { stack.push_back(rhs); continue; }
                int dst = temp();
                emit(LOAD, constant(0)); emit(SUBTRACT, rhs); emit(STORE, dst);
                stack.push_back(dst);
            } else {
                if (stack.size() < 2) fail("Pila insuficiente para operador binario");
                int rhs = stack.back(); stack.pop_back();
                int lhs = stack.back(); stack.pop_back();
                int dst = temp();
                int op = 0;
                if (t == "+") op = ADD;
                else if (t == "-") op = SUBTRACT;
                else if (t == "*") op = MULTIPLY;
                else if (t == "/") op = DIVIDE;
                else if (t == "%") op = MODULO;
                else if (t == "^") op = POWER;
                else fail("Operador desconocido: " + t);
                emit(LOAD, lhs); emit(op, rhs); emit(STORE, dst);
                stack.push_back(dst);
            }
        }
        if (stack.size() != 1) fail("Error en la evaluacion de la expresion");
        return stack.back();
    }

    void branchTo(int op, int line) {
        if (line < 0) fail("Linea de destino negativa");
        if (Symbol *symbol = find(std::to_string(line), 'L')) {
            emit(op, symbol->location);
        } else {
            int instr = emit(op, 0);
            flags[instr] = line; // Referencia hacia adelante pendiente.
        }
    }

    static bool relational(const std::string &s) {
        return s == "<" || s == "<=" || s == ">" || s == ">=" || s == "==" || s == "!=";
    }

    void compileIf(const std::vector<Token> &tokens) {
        int relationIndex = -1, gotoIndex = -1;
        for (size_t i = 0; i < tokens.size(); ++i) {
            if (relational(tokens[i].text)) {
                if (relationIndex >= 0) fail("Condicion con varios operadores relacionales");
                relationIndex = static_cast<int>(i);
            }
            if (tokens[i].text == "goto") gotoIndex = static_cast<int>(i);
        }
        if (relationIndex <= 0 || gotoIndex <= relationIndex + 1 ||
            gotoIndex != static_cast<int>(tokens.size()) - 2)
            fail("Formato: if expresion comparador expresion goto numero");
        int line = number(tokens.back().text, "destino goto");
        if (line <= 0) fail("El destino goto debe ser positivo");
        std::string relation = tokens[relationIndex].text;
        std::vector<Token> left(tokens.begin(), tokens.begin() + relationIndex);
        std::vector<Token> right(tokens.begin() + relationIndex + 1, tokens.begin() + gotoIndex);
        int lhs = evaluateToAddress(left);
        int rhs = evaluateToAddress(right);

        // Para > y >= invertimos la resta, usando BRANCHNEG/BRANCHZERO.
        if (relation == ">" || relation == ">=") std::swap(lhs, rhs);
        emit(LOAD, lhs);
        emit(SUBTRACT, rhs);
        if (relation == "<" || relation == ">") {
            branchTo(BRANCHNEG, line);
        } else if (relation == "<=" || relation == ">=") {
            branchTo(BRANCHNEG, line);
            branchTo(BRANCHZERO, line);
        } else if (relation == "==") {
            branchTo(BRANCHZERO, line);
        } else { // !=: si la resta da cero, saltar sobre BRANCH.
            int skip = emit(BRANCHZERO, 0);
            branchTo(BRANCH, line);
            if (nextInstruction >= MEM) fail("No hay direccion para completar !=");
            memory[skip] = BRANCHZERO * MEM + nextInstruction;
        }
    }

    void compileStatement(const std::vector<Token> &tokens) {
        if (tokens.empty()) fail("Falta comando Simple");
        std::string command = tokens[0].text;
        if (foundEnd) fail("Hay instrucciones despues de end");
        if (command == "input" || command == "print") {
            if (tokens.size() != 2 || !isVariable(tokens[1].text))
                fail("Formato: " + command + " variable_de_una_letra");
            emit(command == "input" ? READ : WRITE, dataSlot(tokens[1].text, 'V'));
        } else if (command == "goto") {
            if (tokens.size() != 2) fail("Formato: goto numero_de_linea");
            int target = number(tokens[1].text, "goto");
            if (target <= 0) fail("El destino goto debe ser positivo");
            branchTo(BRANCH, target);
        } else if (command == "let") {
            if (tokens.size() < 4 || !isVariable(tokens[1].text) || tokens[2].text != "=")
                fail("Formato: let variable = expresion");
            int target = dataSlot(tokens[1].text, 'V');
            std::vector<Token> expr(tokens.begin() + 3, tokens.end());
            int value = evaluateToAddress(expr);
            emit(LOAD, value);
            emit(STORE, target);
        } else if (command == "if") {
            std::vector<Token> condition(tokens.begin() + 1, tokens.end());
            compileIf(condition);
        } else if (command == "end") {
            if (tokens.size() != 1) fail("end no acepta argumentos");
            emit(HALT);
            foundEnd = true;
        } else {
            fail("Comando Simple desconocido: '" + command + "'");
        }
    }

    void firstPass(const std::string &path) {
        flags.fill(-1);
        std::ifstream in(path);
        if (!in) throw std::runtime_error("No se pudo abrir el archivo fuente: " + path);
        std::string line;
        while (std::getline(in, line)) {
            ++sourceLine;
            size_t start = line.find_first_not_of(" \t\r\n");
            if (start == std::string::npos) continue;
            size_t space = line.find_first_of(" \t", start);
            if (space == std::string::npos) fail("Falta el comando despues del numero de linea");
            std::string lineNumber = line.substr(start, space - start);
            int numericLine = number(lineNumber, "numero de linea");
            if (numericLine <= previousLine || numericLine <= 0)
                fail("Numeros de linea deben ser positivos y estrictamente ascendentes");
            previousLine = numericLine;
            std::string remainder = line.substr(space + 1);
            size_t begin = remainder.find_first_not_of(" \t");
            if (begin == std::string::npos) fail("Falta comando Simple");
            remainder = remainder.substr(begin);
            size_t end = remainder.find_first_of(" \t\r\n");
            std::string command = remainder.substr(0, end);
            if (foundEnd) fail("Hay instrucciones despues de end");
            symbols.push_back({std::to_string(numericLine), 'L', nextInstruction});
            if (command == "rem") continue; // Comentarios admiten mayusculas y cualquier texto.
            std::vector<Token> tokens;
            try { tokens = tokenize(remainder); }
            catch (const std::exception &e) { fail(e.what()); }
            compileStatement(tokens);
        }
        if (!foundEnd) throw std::runtime_error("Falta el comando end en el programa Simple");
    }

    void secondPass() {
        for (int i = 0; i < nextInstruction; ++i) {
            if (flags[i] == -1) continue;
            Symbol *target = find(std::to_string(flags[i]), 'L');
            if (!target) throw std::runtime_error("Referencia hacia adelante no resuelta a la linea " + std::to_string(flags[i]));
            if (target->location >= MEM) throw std::runtime_error("Salto a linea sin instruccion ejecutable");
            int operation = memory[i] / MEM;
            memory[i] = operation * MEM + target->location;
            flags[i] = -1;
        }
    }

    void writeFile(const std::string &path) const {
        std::ofstream out(path);
        if (!out) throw std::runtime_error("No se pudo crear el archivo SML: " + path);
        // Memoria de 1000 posiciones: codigo al principio, datos desde 999 hacia atras.
        // El simulador lee las palabras consecutivas hasta EOF.
        for (int i = 0; i < MEM; ++i) {
            int value = memory[i];
            if (value == 9999 || value == 99999)
                throw std::runtime_error("La palabra de memoria " + std::to_string(i) +
                                         " coincide con un centinela de carga");
            out << (value < 0 ? '-' : '+') << std::setfill('0') << std::setw(5)
                << std::abs(value) << '\n';
        }
        if (!out) throw std::runtime_error("Error al escribir el archivo SML: " + path);
    }

    void printSymbols() const {
        std::cout << "\nTABLA DE SIMBOLOS (tipo / simbolo / direccion):\n";
        for (const Symbol &s : symbols) {
            if (s.type != 'T')
                std::cout << "  " << s.type << "  " << s.symbol << "  " << s.location << '\n';
        }
    }
};
} // namespace

int main(int argc, char *argv[]) {
    if (argc > 4) {
        std::cerr << "Uso: compiler.exe [fuente.simple] [salida.sml] [salida.simp]\n";
        return 1;
    }
    const std::string input = argc > 1 ? argv[1] : "CompiladorSimpletron/programa.simple";
    const std::string sml = argc > 2 ? argv[2] : "programa.sml";
    const std::string simp = argc > 3 ? argv[3] : "programa.simp";
    try {
        Compiler compiler;
        compiler.compile(input, sml, simp);
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "ERROR DE COMPILACION: " << e.what() << '\n';
        return 1;
    }
}
