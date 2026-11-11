
# PROYECTO DE PROGRAMACIÓN AVANZADA
## Simulador Simpletron y Compilador Simple a SML

Proyecto desarrollado para la materia de Programación Avanzada de UPAEP.

El objetivo es implementar un simulador de la computadora Simpletron, incorporar mejoras a su funcionamiento y construir un compilador capaz de traducir programas escritos en lenguaje Simple a instrucciones SML.

## 1. Características del simulador

El proyecto cuenta con:

- Memoria de 1000 posiciones.
- Carga automática desde programa.simp.
- Carga manual cuando no existe un archivo válido.
- Ciclo de búsqueda, decodificación y ejecución.
- Registros internos de Simpletron.
- Las 12 instrucciones originales SML.
- Instrucciones adicionales.
- Detección de errores fatales.
- Validación de operaciones y direcciones.
- Vaciado de registros y memoria.
- Herramientas de pila y evaluación de expresiones.

### Registros

- accumulator
- instructionCounter
- instructionRegister
- operationCode
- operand

## 2. Instrucciones SML

| Código | Instrucción | Función |
|---|---|---|
| 10 | READ | Leer un número |
| 11 | WRITE | Mostrar un número |
| 12 | NEWLINE | Salto de línea |
| 13 | READSTRING | Leer una cadena ASCII |
| 14 | WRITESTRING | Mostrar una cadena ASCII |
| 20 | LOAD | Cargar al acumulador |
| 21 | STORE | Guardar el acumulador |
| 30 | ADD | Sumar |
| 31 | SUBTRACT | Restar |
| 32 | DIVIDE | Dividir |
| 33 | MULTIPLY | Multiplicar |
| 34 | MODULO | Calcular residuo |
| 35 | POWER | Calcular potencia |
| 40 | BRANCH | Salto incondicional |
| 41 | BRANCHNEG | Salto si es negativo |
| 42 | BRANCHZERO | Salto si es cero |
| 43 | HALT | Finalizar programa |

### Formato de instrucciones

La versión mejorada utiliza instrucciones de cinco dígitos:

- Dos dígitos para el código de operación.
- Tres dígitos para la dirección de memoria.

Ejemplo: 20500

- 20: LOAD
- 500: dirección de memoria

Este formato permite acceder a las 1000 posiciones de memoria.

## 3. Compilación del simulador

Se necesita un compilador compatible con C++17.

Desde Git Bash:

```bash
g++ -std=c++17 -Wall -Wextra SIMPLETRON1.0.CPP -o output/simpletron_revision.exe
```

Para ejecutar:

```bash
./output/simpletron_revision.exe
```

El simulador busca automáticamente programa.simp. Si no encuentra un archivo válido, permite introducir instrucciones manualmente.

## 4. Herramientas de expresiones

El programa incorpora herramientas basadas en pilas.

### Validación de agrupadores

```bash
./output/simpletron_revision.exe --pila
```

Permite revisar agrupadores y calcular la profundidad máxima de anidamiento.

### Evaluación postfija

```bash
./output/simpletron_revision.exe --postfijo
```

Ejemplo:

Entrada: 3 5 4 * +

Resultado: 23

### Conversión infija a postfija

```bash
./output/simpletron_revision.exe --infijo
```

Ejemplo:

Infija: 12 + 3 * (8 - 5)

Postfija: 12 3 8 5 - * +

Resultado: 21

Se implementaron reglas de precedencia, asociatividad, paréntesis, operadores aritméticos y validación de expresiones.

## 5. Compilador Simple a SML

El compilador se encuentra en:

CompiladorSimpletron/compiler.cpp

Traduce programas escritos en Simple a instrucciones ejecutables por Simpletron.

### Instrucciones del lenguaje Simple

- rem: comentarios.
- input: entrada de datos.
- let: asignación y expresiones.
- print: salida de datos.
- goto: saltos incondicionales.
- if/goto: condiciones y bifurcaciones.
- end: finalización.

### Arquitectura del compilador

Se implementa un proceso de dos pasadas.

Primera pasada:

- Lectura y análisis de instrucciones Simple.
- Identificación de símbolos.
- Construcción de la tabla de símbolos.
- Asignación de direcciones de memoria.
- Generación de instrucciones SML.
- Registro de referencias pendientes.

Segunda pasada:

- Resolución de saltos pendientes.
- Comprobación de referencias.
- Generación del archivo SML final.

La tabla de símbolos registra líneas, variables y constantes, junto con sus direcciones.

## 6. Compilación y ejecución del compilador

Compilar:

```bash
g++ -std=c++17 -Wall -Wextra CompiladorSimpletron/compiler.cpp -o output/compiler.exe
```

Ejecutar:

```bash
./output/compiler.exe
```

El compilador lee:

CompiladorSimpletron/programa.simple

Y genera:

- programa.sml: código SML.
- programa.simp: archivo para el simulador.

Para ejecutar el programa traducido:

```bash
./output/simpletron_revision.exe
```

## 7. Ejemplo de programa Simple

```text
10 input a
20 input b
30 let c = a + b
40 print c
50 end
```

Si se introducen 7 y 5, el resultado será:

```text
+12
```

El compilador traduce el programa a SML y Simpletron ejecuta las instrucciones generadas.

## 8. Pruebas realizadas

### Simulador

Se comprobaron:

- Lectura y escritura.
- Operaciones aritméticas.
- Memoria de 1000 posiciones.
- Carga automática y manual.
- División entre cero.
- Códigos de operación inválidos.
- Desbordamiento.
- Lectura y escritura ASCII.
- Módulo, potencia y salto de línea.
- Recuperación ante archivos inválidos.

### Expresiones

Se realizaron pruebas de:

- Agrupadores correctos e incorrectos.
- Profundidad de anidamiento.
- Evaluación postfija.
- Conversión infija a postfija.
- Precedencia y asociatividad.
- Potencias.
- Números negativos.
- Expresiones inválidas.

### Compilador

Se comprobaron:

- Generación de instrucciones SML.
- Construcción de la tabla de símbolos.
- Asignación de variables.
- Operaciones aritméticas.
- Expresiones con paréntesis.
- Condiciones if/goto.
- Referencias hacia adelante.
- Saltos hacia atrás.
- Ejecución de ciclos.
- Detección de saltos a líneas inexistentes.
- Integración con el simulador Simpletron.

## 9. Organización del proyecto

```text
PROYECTO PROGRA AV/
|
|-- SIMPLETRON1.0.CPP
|-- CompiladorSimpletron/
|   |-- compiler.cpp
|   |-- programa.simple
|
|-- Pruebas/
|-- output/
|-- .gitignore
|-- README
```

La carpeta output contiene ejecutables generados localmente.

Los archivos programa.simp y programa.sml de la raíz son resultados generados por el compilador y están excluidos mediante .gitignore.

## 10. Control de versiones

El proyecto utiliza Git y GitHub.

Ramas de desarrollo:

- master: versión básica.
- feature/mejoras-simpletron: ampliaciones del simulador.
- feature/pila-reduccion: algoritmos de pila.
- feature/correcciones-simpletron: correcciones del simulador.
- feature/expresiones-completas: procesamiento de expresiones.
- feature/compilador-simple: compilador Simple a SML.

Las funcionalidades se desarrollaron y probaron progresivamente en ramas independientes.

## 11. Repositorio

https://github.com/santychavz/PROYECTO_PAV_SC

Proyecto académico de Programación Avanzada.

