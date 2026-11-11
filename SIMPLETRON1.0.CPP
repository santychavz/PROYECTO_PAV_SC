#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>
#include <windows.h>

#define MEMORY_SIZE 1000
#define PROGRAM_FILE "programa.simp"
#define MAX_LINE 512
#define MAX_STRING 99
#define WORD_LIMIT 99999.0

/* Capacidad de la pila para revisar expresiones */
#define STACK_SIZE MAX_LINE

typedef struct
{
    char elementos[STACK_SIZE];
    int tope;
} Pila;

/* Esta pila guarda los valores de una expresion en postfijo */
typedef struct
{
    float elementos[STACK_SIZE];
    int tope;
} PilaNumerica;

/* Operaciones originales */
#define READ 10
#define WRITE 11
#define LOAD 20
#define STORE 21
#define ADD 30
#define SUBTRACT 31
#define DIVIDE 32
#define MULTIPLY 33
#define BRANCH 40
#define BRANCHNEG 41
#define BRANCHZERO 42
#define HALT 43

/* Operaciones nuevas */
#define NEWLINE 12
#define READSTRING 13
#define WRITESTRING 14
#define MODULO 34
#define POWER 35


void mostrarBienvenida(void);

int cargarPrograma(double memory[]);
int cargarDesdeArchivo(double memory[]);
int cargarInteractivo(double memory[]);

void ejecutarPrograma(double memory[]);

void vaciado(
    double memory[],
    double accumulator,
    int instructionCounter,
    int instructionRegister,
    int operationCode,
    int operand
);

void errorFatal(
    const char mensaje[],
    double memory[],
    double accumulator,
    int instructionCounter,
    int instructionRegister,
    int operationCode,
    int operand
);

int leerLinea(char buffer[], int tam);

int convertirNumero(
    const char texto[],
    double *valor
);

int esEntero(double valor);

int validarRangoPalabra(double valor);

void limpiarMemoria(double memory[]);

int decodificarInstruccion(
    int instruction,
    int *operationCode,
    int *operand
);

int opcodeValido(int operationCode);

int verificarResultado(double resultado);

int leerCadenaEnMemoria(
    double memory[],
    int base
);

int escribirCadenaDesdeMemoria(
    double memory[],
    int base
);

void imprimirPalabra(double valor);

FILE *abrirProgramaAutomatico(
    char rutaUsada[],
    int tamRuta
);

/* Funciones de la pila */
void inicializarPila(Pila *p);
int push(Pila *p, char elemento);
int pop(Pila *p, char *elemento);
int isEmpty(const Pila *p);
int stackTop(const Pila *p, char *elemento);
int profundidad(const char expresion[]);
int profundidadMaxima(const char expresion[], int *maxima);
int simbolosEquivalentes(char abre, char cierra);
void modoPila(void);

/* Funciones para evaluar expresiones en postfijo */
void inicializarPilaNumerica(PilaNumerica *p);
int pushNumero(PilaNumerica *p, float elemento);
int popNumero(PilaNumerica *p, float *elemento);
int isEmptyNumerica(const PilaNumerica *p);
int cantidadPilaNumerica(const PilaNumerica *p);
int esOperadorPostfijo(char simbolo);
int aplicarOperadorPostfijo(
    float opnd1,
    float opnd2,
    char operador,
    float *resultado
);
float evaluar(char expresion[]);
void modoPostfijo(void);

void pausarPrograma(void);


/* ===============================
   MAIN
   =============================== */

int main(int argc, char *argv[])
{
    /*
       Si uso --pila pruebo esta parte aparte.
       Sin argumentos Simpletron funciona igual que antes.
    */

    if (
        argc > 1 &&
        strcmp(argv[1], "--pila") == 0
    )
    {
        modoPila();
        pausarPrograma();

        return 0;
    }


    /*
       Con --postfijo pruebo la evaluacion
       sin cambiar la ejecucion normal de Simpletron.
    */

    if (
        argc > 1 &&
        strcmp(argv[1], "--postfijo") == 0
    )
    {
        modoPostfijo();
        pausarPrograma();

        return 0;
    }


    double memory[MEMORY_SIZE] = {0};

    int palabrasCargadas;

    mostrarBienvenida();

    palabrasCargadas =
        cargarPrograma(memory);

    if (palabrasCargadas <= 0)
    {
        printf(
            "\nNo se cargo ningun programa.\n"
        );

        pausarPrograma();

        return 0;
    }

    ejecutarPrograma(memory);

    pausarPrograma();

    return 0;
}


/* ===============================
   MENSAJE INICIAL
   =============================== */

void mostrarBienvenida(void)
{
    printf(
        "*** Bienvenido a Simpletron! ***\n"
    );

    printf(
        "** Primero se buscara el archivo programa.simp. **\n"
    );

    printf(
        "** Si no existe, se usara la carga manual. **\n"
    );

    printf(
        "** En modo manual escriba 9999 o 99999 para terminar. **\n\n"
    );
}


/* ===============================
   BUSCAR programa.simp
   =============================== */

FILE *abrirProgramaAutomatico(
    char rutaUsada[],
    int tamRuta
)
{
    FILE *archivo;

    char rutaExe[MAX_PATH];
    char carpetaExe[MAX_PATH];
    char rutaPrograma[MAX_PATH];

    char *ultimaBarra;

    DWORD longitudRuta;


    /*
       Obtengo la ruta real del ejecutable.
       Asi no dependo de la carpeta desde
       donde se abrio el programa.
    */

    longitudRuta =
        GetModuleFileNameA(
            NULL,
            rutaExe,
            MAX_PATH
        );


    if (
        longitudRuta == 0 ||
        longitudRuta >= MAX_PATH
    )
    {
        return NULL;
    }


    strcpy(
        carpetaExe,
        rutaExe
    );


    /*
       Quito el nombre del .exe para
       quedarme solo con su carpeta.
    */

    ultimaBarra =
        strrchr(
            carpetaExe,
            '\\'
        );


    if (ultimaBarra != NULL)
    {
        *ultimaBarra = '\0';
    }


    /*
       Primero busco programa.simp
       junto al ejecutable.
    */

    snprintf(
        rutaPrograma,
        MAX_PATH,
        "%s\\%s",
        carpetaExe,
        PROGRAM_FILE
    );


    archivo =
        fopen(
            rutaPrograma,
            "r"
        );


    if (archivo != NULL)
    {
        snprintf(
            rutaUsada,
            tamRuta,
            "%s",
            rutaPrograma
        );

        return archivo;
    }


    /*
       Mi ejecutable normalmente queda
       dentro de output, asi que tambien
       busco una carpeta arriba.
    */

    snprintf(
        rutaPrograma,
        MAX_PATH,
        "%s\\..\\%s",
        carpetaExe,
        PROGRAM_FILE
    );


    archivo =
        fopen(
            rutaPrograma,
            "r"
        );


    if (archivo != NULL)
    {
        snprintf(
            rutaUsada,
            tamRuta,
            "%s",
            rutaPrograma
        );

        return archivo;
    }


    /*
       Como ultimo intento lo busco en
       la carpeta de trabajo actual.
    */

    archivo =
        fopen(
            PROGRAM_FILE,
            "r"
        );


    if (archivo != NULL)
    {
        snprintf(
            rutaUsada,
            tamRuta,
            "%s",
            PROGRAM_FILE
        );

        return archivo;
    }


    return NULL;
}


/* ===============================
   ELEGIR TIPO DE CARGA
   =============================== */

int cargarPrograma(double memory[])
{
    int resultadoArchivo;

    resultadoArchivo =
        cargarDesdeArchivo(memory);


    if (resultadoArchivo >= 0)
    {
        return resultadoArchivo;
    }


    printf(
        "No se encontro programa.simp.\n"
    );

    printf(
        "Se usara la carga interactiva.\n\n"
    );


    return cargarInteractivo(memory);
}


/* ===============================
   CARGA AUTOMATICA
   =============================== */

int cargarDesdeArchivo(double memory[])
{
    FILE *archivo;

    char linea[MAX_LINE];

    char rutaUsada[MAX_PATH];

    int posicion = 0;

    int numeroLinea = 0;

    double palabra;


    archivo =
        abrirProgramaAutomatico(
            rutaUsada,
            sizeof(rutaUsada)
        );


    if (archivo == NULL)
    {
        return -1;
    }


    printf(
        "Archivo encontrado: %s\n",
        rutaUsada
    );

    printf(
        "Cargando programa...\n\n"
    );


    while (
        fgets(
            linea,
            sizeof(linea),
            archivo
        ) != NULL
    )
    {
        char *inicio =
            linea;

        char *comentario;


        numeroLinea++;


        /*
           Quito espacios del inicio.
        */

        while (
            isspace(
                (unsigned char)*inicio
            )
        )
        {
            inicio++;
        }


        /*
           Ignoro lineas vacias.
        */

        if (
            *inicio == '\0' ||
            *inicio == '\n' ||
            *inicio == '\r'
        )
        {
            continue;
        }


        /*
           Puedo usar # para dejar notas
           dentro de programa.simp.
        */

        if (*inicio == '#')
        {
            continue;
        }


        comentario =
            strchr(
                inicio,
                '#'
            );


        if (comentario != NULL)
        {
            *comentario =
                '\0';
        }


        if (
            !convertirNumero(
                inicio,
                &palabra
            )
        )
        {
            printf(
                "Error en programa.simp, linea %d: valor invalido.\n",
                numeroLinea
            );

            printf(
                "Se limpiara la memoria y se pasara a carga manual.\n\n"
            );


            fclose(archivo);

            limpiarMemoria(memory);


            return cargarInteractivo(
                memory
            );
        }


        /*
           Mantengo los dos codigos
           para terminar la carga.
        */

        if (
            esEntero(palabra) &&
            (
                (int)palabra == 9999 ||
                (int)palabra == 99999
            )
        )
        {
            break;
        }


        if (
            !validarRangoPalabra(
                palabra
            )
        )
        {
            printf(
                "Error en programa.simp, linea %d: valor fuera de rango.\n",
                numeroLinea
            );

            printf(
                "Se limpiara la memoria y se pasara a carga manual.\n\n"
            );


            fclose(archivo);

            limpiarMemoria(memory);


            return cargarInteractivo(
                memory
            );
        }


        if (
            posicion >=
            MEMORY_SIZE
        )
        {
            printf(
                "Error: programa.simp tiene mas de %d palabras.\n",
                MEMORY_SIZE
            );


            fclose(archivo);

            limpiarMemoria(memory);


            return cargarInteractivo(
                memory
            );
        }


        memory[posicion] =
            palabra;


        posicion++;
    }


    fclose(archivo);


    printf(
        "*** Programa cargado desde archivo: %d palabras ***\n",
        posicion
    );


    printf(
        "*** Comienza la ejecucion del programa ***\n\n"
    );


    return posicion;
}


/* ===============================
   CARGA MANUAL
   =============================== */

int cargarInteractivo(double memory[])
{
    char linea[MAX_LINE];

    double palabra;

    int posicion = 0;


    printf(
        "*** Carga manual ***\n"
    );

    printf(
        "** Introduzca una instruccion o palabra de datos por linea. **\n"
    );

    printf(
        "** Escriba 9999 o 99999 para terminar. **\n\n"
    );


    while (
        posicion <
        MEMORY_SIZE
    )
    {
        printf(
            "%03d ? ",
            posicion
        );

        fflush(stdout);


        if (
            !leerLinea(
                linea,
                sizeof(linea)
            )
        )
        {
            break;
        }


        if (
            !convertirNumero(
                linea,
                &palabra
            )
        )
        {
            printf(
                "Entrada invalida. Escriba un numero.\n"
            );

            continue;
        }


        if (
            esEntero(palabra) &&
            (
                (int)palabra == 9999 ||
                (int)palabra == 99999
            )
        )
        {
            break;
        }


        if (
            !validarRangoPalabra(
                palabra
            )
        )
        {
            printf(
                "Valor invalido. Debe estar entre -99999 y +99998.\n"
            );

            continue;
        }


        memory[posicion] =
            palabra;


        posicion++;
    }


    printf(
        "\n*** Carga del programa completa: %d palabras ***\n",
        posicion
    );


    printf(
        "*** Comienza la ejecucion del programa ***\n\n"
    );


    return posicion;
}


/* ===============================
   EJECUCION
   =============================== */

void ejecutarPrograma(double memory[])
{
    double accumulator =
        0.0;

    int instructionCounter =
        0;

    int instructionRegister =
        0;

    int operationCode =
        0;

    int operand =
        0;

    char linea[MAX_LINE];

    double dato;

    double resultado;


    while (1)
    {
        if (
            instructionCounter < 0 ||
            instructionCounter >= MEMORY_SIZE
        )
        {
            errorFatal(
                "Contador de instrucciones fuera de rango.",
                memory,
                accumulator,
                instructionCounter,
                instructionRegister,
                operationCode,
                operand
            );

            return;
        }


        /*
           Las instrucciones deben ser
           enteras aunque los datos puedan
           tener punto decimal.
        */

        if (
            !esEntero(
                memory[
                    instructionCounter
                ]
            )
        )
        {
            errorFatal(
                "Se intento ejecutar un valor decimal como instruccion.",
                memory,
                accumulator,
                instructionCounter,
                instructionRegister,
                operationCode,
                operand
            );

            return;
        }


        instructionRegister =
            (int)memory[
                instructionCounter
            ];


        if (
            !decodificarInstruccion(
                instructionRegister,
                &operationCode,
                &operand
            )
        )
        {
            errorFatal(
                "Formato de instruccion no valido.",
                memory,
                accumulator,
                instructionCounter,
                instructionRegister,
                operationCode,
                operand
            );

            return;
        }


        if (
            !opcodeValido(
                operationCode
            )
        )
        {
            errorFatal(
                "Codigo de operacion no valido.",
                memory,
                accumulator,
                instructionCounter,
                instructionRegister,
                operationCode,
                operand
            );

            return;
        }


        if (
            operand < 0 ||
            operand >= MEMORY_SIZE
        )
        {
            errorFatal(
                "Direccion de memoria fuera de rango.",
                memory,
                accumulator,
                instructionCounter,
                instructionRegister,
                operationCode,
                operand
            );

            return;
        }


        /*
           Normalmente avanzo una posicion.
           Los BRANCH pueden cambiarla.
        */

        instructionCounter++;


        switch (operationCode)
        {

            /* 10 READ */

            case READ:

                while (1)
                {
                    printf("? ");

                    fflush(stdout);


                    if (
                        !leerLinea(
                            linea,
                            sizeof(linea)
                        )
                    )
                    {
                        errorFatal(
                            "No se pudo leer el dato de entrada.",
                            memory,
                            accumulator,
                            instructionCounter - 1,
                            instructionRegister,
                            operationCode,
                            operand
                        );

                        return;
                    }


                    if (
                        !convertirNumero(
                            linea,
                            &dato
                        )
                    )
                    {
                        printf(
                            "Entrada invalida. Escriba un numero entero o decimal.\n"
                        );

                        continue;
                    }


                    if (
                        fabs(dato) >
                        WORD_LIMIT
                    )
                    {
                        printf(
                            "Valor fuera de rango. Use valores entre -99999 y +99999.\n"
                        );

                        continue;
                    }


                    memory[operand] =
                        dato;


                    break;
                }

                break;


            /* 11 WRITE */

            case WRITE:

                imprimirPalabra(
                    memory[operand]
                );

                printf("\n");

                break;


            /* 12 NEWLINE */

            case NEWLINE:

                printf("\n");

                break;


            /* 13 READSTRING */

            case READSTRING:

                if (
                    !leerCadenaEnMemoria(
                        memory,
                        operand
                    )
                )
                {
                    errorFatal(
                        "No se pudo guardar la cadena en memoria.",
                        memory,
                        accumulator,
                        instructionCounter - 1,
                        instructionRegister,
                        operationCode,
                        operand
                    );

                    return;
                }

                break;


            /* 14 WRITESTRING */

            case WRITESTRING:

                if (
                    !escribirCadenaDesdeMemoria(
                        memory,
                        operand
                    )
                )
                {
                    errorFatal(
                        "Cadena invalida o direccion fuera de rango.",
                        memory,
                        accumulator,
                        instructionCounter - 1,
                        instructionRegister,
                        operationCode,
                        operand
                    );

                    return;
                }

                break;


            /* 20 LOAD */

            case LOAD:

                accumulator =
                    memory[operand];

                break;


            /* 21 STORE */

            case STORE:

                memory[operand] =
                    accumulator;

                break;


            /* 30 ADD */

            case ADD:

                resultado =
                    accumulator +
                    memory[operand];


                if (
                    !verificarResultado(
                        resultado
                    )
                )
                {
                    errorFatal(
                        "Desbordamiento del acumulador al sumar.",
                        memory,
                        accumulator,
                        instructionCounter - 1,
                        instructionRegister,
                        operationCode,
                        operand
                    );

                    return;
                }


                accumulator =
                    resultado;

                break;


            /* 31 SUBTRACT */

            case SUBTRACT:

                resultado =
                    accumulator -
                    memory[operand];


                if (
                    !verificarResultado(
                        resultado
                    )
                )
                {
                    errorFatal(
                        "Desbordamiento del acumulador al restar.",
                        memory,
                        accumulator,
                        instructionCounter - 1,
                        instructionRegister,
                        operationCode,
                        operand
                    );

                    return;
                }


                accumulator =
                    resultado;

                break;


            /* 32 DIVIDE */

            case DIVIDE:

                if (
                    fabs(
                        memory[operand]
                    ) < 1e-12
                )
                {
                    errorFatal(
                        "Intento de division entre cero.",
                        memory,
                        accumulator,
                        instructionCounter - 1,
                        instructionRegister,
                        operationCode,
                        operand
                    );

                    return;
                }


                resultado =
                    accumulator /
                    memory[operand];


                if (
                    !verificarResultado(
                        resultado
                    )
                )
                {
                    errorFatal(
                        "Resultado fuera del rango permitido al dividir.",
                        memory,
                        accumulator,
                        instructionCounter - 1,
                        instructionRegister,
                        operationCode,
                        operand
                    );

                    return;
                }


                accumulator =
                    resultado;

                break;


            /* 33 MULTIPLY */

            case MULTIPLY:

                resultado =
                    accumulator *
                    memory[operand];


                if (
                    !verificarResultado(
                        resultado
                    )
                )
                {
                    errorFatal(
                        "Desbordamiento del acumulador al multiplicar.",
                        memory,
                        accumulator,
                        instructionCounter - 1,
                        instructionRegister,
                        operationCode,
                        operand
                    );

                    return;
                }


                accumulator =
                    resultado;

                break;


            /* 34 MODULO */

            case MODULO:

                if (
                    fabs(
                        memory[operand]
                    ) < 1e-12
                )
                {
                    errorFatal(
                        "Intento de modulo entre cero.",
                        memory,
                        accumulator,
                        instructionCounter - 1,
                        instructionRegister,
                        operationCode,
                        operand
                    );

                    return;
                }


                resultado =
                    fmod(
                        accumulator,
                        memory[operand]
                    );


                if (
                    !verificarResultado(
                        resultado
                    )
                )
                {
                    errorFatal(
                        "Resultado invalido en la operacion modulo.",
                        memory,
                        accumulator,
                        instructionCounter - 1,
                        instructionRegister,
                        operationCode,
                        operand
                    );

                    return;
                }


                accumulator =
                    resultado;

                break;


            /* 35 POWER */

            case POWER:

                errno = 0;


                resultado =
                    pow(
                        accumulator,
                        memory[operand]
                    );


                if (
                    errno != 0 ||
                    !verificarResultado(
                        resultado
                    )
                )
                {
                    errorFatal(
                        "Resultado invalido o desbordamiento en la potencia.",
                        memory,
                        accumulator,
                        instructionCounter - 1,
                        instructionRegister,
                        operationCode,
                        operand
                    );

                    return;
                }


                accumulator =
                    resultado;

                break;


            /* 40 BRANCH */

            case BRANCH:

                instructionCounter =
                    operand;

                break;


            /* 41 BRANCHNEG */

            case BRANCHNEG:

                if (
                    accumulator <
                    0.0
                )
                {
                    instructionCounter =
                        operand;
                }

                break;


            /* 42 BRANCHZERO */

            case BRANCHZERO:

                if (
                    fabs(
                        accumulator
                    ) < 1e-12
                )
                {
                    instructionCounter =
                        operand;
                }

                break;


            /* 43 HALT */

            case HALT:

                printf(
                    "\n*** Termino la ejecucion de Simpletron ***\n"
                );


                vaciado(
                    memory,
                    accumulator,
                    instructionCounter - 1,
                    instructionRegister,
                    operationCode,
                    operand
                );


                return;
        }
    }
}


/* ===============================
   ERROR FATAL
   =============================== */

void errorFatal(
    const char mensaje[],
    double memory[],
    double accumulator,
    int instructionCounter,
    int instructionRegister,
    int operationCode,
    int operand
)
{
    printf(
        "\n*** ERROR FATAL: %s ***\n",
        mensaje
    );


    printf(
        "*** La ejecucion de Simpletron termino anormalmente ***\n"
    );


    vaciado(
        memory,
        accumulator,
        instructionCounter,
        instructionRegister,
        operationCode,
        operand
    );
}


/* ===============================
   VACIADO
   =============================== */

void vaciado(
    double memory[],
    double accumulator,
    int instructionCounter,
    int instructionRegister,
    int operationCode,
    int operand
)
{
    int fila;

    int columna;


    printf(
        "\nREGISTROS:\n\n"
    );


    printf(
        "accumulator          %+12.3f\n",
        accumulator
    );


    printf(
        "instructionCounter        %03d\n",
        instructionCounter
    );


    printf(
        "instructionRegister      %+05d\n",
        instructionRegister
    );


    printf(
        "operationCode               %02d\n",
        operationCode
    );


    printf(
        "operand                    %03d\n",
        operand
    );


    printf(
        "\nMEMORIA:\n\n"
    );


    printf("     ");


    for (
        columna = 0;
        columna < 10;
        columna++
    )
    {
        printf(
            "%12d",
            columna
        );
    }


    printf("\n");


    for (
        fila = 0;
        fila < MEMORY_SIZE;
        fila += 10
    )
    {
        printf(
            "%03d  ",
            fila
        );


        for (
            columna = 0;
            columna < 10;
            columna++
        )
        {
            imprimirPalabra(
                memory[
                    fila + columna
                ]
            );
        }


        printf("\n");
    }
}


/* ===============================
   LEER LINEA
   =============================== */

int leerLinea(
    char buffer[],
    int tam
)
{
    size_t longitud;


    if (
        fgets(
            buffer,
            tam,
            stdin
        ) == NULL
    )
    {
        return 0;
    }


    longitud =
        strlen(buffer);


    if (
        longitud > 0 &&
        buffer[
            longitud - 1
        ] == '\n'
    )
    {
        buffer[
            longitud - 1
        ] = '\0';
    }

    else
    {
        int c;


        while (
            (c = getchar()) != '\n' &&
            c != EOF
        )
        {
        }
    }


    return 1;
}


/* ===============================
   TEXTO A NUMERO
   =============================== */

int convertirNumero(
    const char texto[],
    double *valor
)
{
    char *fin;

    double numero;


    errno = 0;


    numero =
        strtod(
            texto,
            &fin
        );


    if (
        texto == fin ||
        errno == ERANGE
    )
    {
        return 0;
    }


    while (
        isspace(
            (unsigned char)*fin
        )
    )
    {
        fin++;
    }


    if (*fin != '\0')
    {
        return 0;
    }


    if (!isfinite(numero))
    {
        return 0;
    }


    *valor =
        numero;


    return 1;
}


/* ===============================
   REVISAR ENTERO
   =============================== */

int esEntero(double valor)
{
    return
        fabs(
            valor -
            round(valor)
        ) < 1e-9;
}


/* ===============================
   VALIDAR RANGO
   =============================== */

int validarRangoPalabra(
    double valor
)
{
    /*
       99999 queda reservado
       para terminar la carga.
    */

    return
        valor >= -WORD_LIMIT &&
        valor <= (
            WORD_LIMIT - 1.0
        );
}


/* ===============================
   LIMPIAR MEMORIA
   =============================== */

void limpiarMemoria(
    double memory[]
)
{
    int i;


    for (
        i = 0;
        i < MEMORY_SIZE;
        i++
    )
    {
        memory[i] =
            0.0;
    }
}


/* ===============================
   DECODIFICAR
   =============================== */

int decodificarInstruccion(
    int instruction,
    int *operationCode,
    int *operand
)
{
    if (
        instruction < 0 ||
        instruction > 99999
    )
    {
        return 0;
    }


    /*
       Formato nuevo:
       OOAAA

       Ejemplo:
       20500

       20 = LOAD
       500 = direccion

       Tambien mantengo el formato
       anterior de cuatro digitos:

       2007

       20 = LOAD
       07 = direccion
    */

    if (
        instruction >=
        10000
    )
    {
        *operationCode =
            instruction /
            1000;


        *operand =
            instruction %
            1000;
    }

    else
    {
        *operationCode =
            instruction /
            100;


        *operand =
            instruction %
            100;
    }


    return 1;
}


/* ===============================
   OPCODES VALIDOS
   =============================== */

int opcodeValido(
    int operationCode
)
{
    switch (operationCode)
    {
        case READ:
        case WRITE:

        case NEWLINE:
        case READSTRING:
        case WRITESTRING:

        case LOAD:
        case STORE:

        case ADD:
        case SUBTRACT:
        case DIVIDE:
        case MULTIPLY:

        case MODULO:
        case POWER:

        case BRANCH:
        case BRANCHNEG:
        case BRANCHZERO:

        case HALT:

            return 1;


        default:

            return 0;
    }
}


/* ===============================
   VALIDAR RESULTADO
   =============================== */

int verificarResultado(
    double resultado
)
{
    if (
        !isfinite(
            resultado
        )
    )
    {
        return 0;
    }


    if (
        fabs(resultado) >
        WORD_LIMIT
    )
    {
        return 0;
    }


    return 1;
}


/* ===============================
   LEER CADENA: longitud(2) + ASCII(3)
   Primera palabra: LLAAA
   Palabras siguientes: 00AAA
   =============================== */

int leerCadenaEnMemoria(double memory[], int base)
{
    char texto[MAX_LINE];
    size_t longitud;
    size_t i;

    printf("? ");
    fflush(stdout);

    if (!leerLinea(texto, sizeof(texto)))
        return 0;

    longitud = strlen(texto);

    if (longitud > MAX_STRING)
    {
        printf("La cadena supera el limite de %d caracteres.\n", MAX_STRING);
        return 0;
    }

    /* Una cadena vacia utiliza solo memory[base]. */
    if (base < 0 || base >= MEMORY_SIZE ||
        (longitud > 0 && longitud > (size_t)(MEMORY_SIZE - base)))
    {
        printf("La cadena no cabe a partir de la direccion %03d.\n", base);
        return 0;
    }

    for (i = 0; i < longitud; ++i)
    {
        if ((unsigned char)texto[i] > 127)
        {
            printf("La entrada de cadenas usa ASCII (0-127).\n");
            return 0;
        }
    }

    /* Primeros dos digitos = longitud, ultimos tres = primer ASCII. */
    memory[base] = (double)((int)longitud * 1000 +
        (longitud > 0 ? (unsigned char)texto[0] : 0));

    /* Los caracteres restantes se almacenan como ASCII decimal. */
    for (i = 1; i < longitud; ++i)
        memory[base + (int)i] = (double)(unsigned char)texto[i];

    return 1;
}


/* ===============================
   ESCRIBIR CADENA: longitud(2) + ASCII(3)
   =============================== */

int escribirCadenaDesdeMemoria(double memory[], int base)
{
    int primeraPalabra;
    int longitud;
    int primerAscii;
    int i;

    if (base < 0 || base >= MEMORY_SIZE || !esEntero(memory[base]))
        return 0;

    primeraPalabra = (int)memory[base];
    if (primeraPalabra < 0 || primeraPalabra > 99999)
        return 0;

    longitud = primeraPalabra / 1000;
    primerAscii = primeraPalabra % 1000;

    if (longitud < 0 || longitud > MAX_STRING ||
        (longitud > 0 && longitud > MEMORY_SIZE - base))
        return 0;

    if (longitud == 0)
        return primerAscii == 0;

    if (primerAscii > 127)
        return 0;

    /* Validar la cadena completa antes de imprimirla. */
    for (i = 1; i < longitud; ++i)
    {
        if (!esEntero(memory[base + i]) ||
            memory[base + i] < 0 || memory[base + i] > 127)
            return 0;
    }

    putchar((char)primerAscii);
    for (i = 1; i < longitud; ++i)
        putchar((char)(int)memory[base + i]);

    return 1;
}


/* ===============================
   MOSTRAR PALABRA
   =============================== */

void imprimirPalabra(
    double valor
)
{
    if (
        esEntero(valor)
    )
    {
        printf(
            "%+12.0f",
            valor
        );
    }

    else
    {
        printf(
            "%+12.3f",
            valor
        );
    }
}


/* ===============================
   PILA
   =============================== */

void inicializarPila(Pila *p)
{
    /*
       -1 significa que todavia no hay
       ningun elemento dentro de la pila.
    */

    p->tope = -1;
}


int isEmpty(
    const Pila *p
)
{
    return p->tope == -1;
}


int push(
    Pila *p,
    char elemento
)
{
    /*
       Antes de agregar reviso que todavia
       exista espacio para evitar overflow.
    */

    if (
        p->tope >=
        STACK_SIZE - 1
    )
    {
        return 0;
    }


    p->tope++;

    p->elementos[
        p->tope
    ] = elemento;


    return 1;
}


int pop(
    Pila *p,
    char *elemento
)
{
    /*
       No puedo sacar un elemento si
       la pila ya esta vacia.
    */

    if (
        isEmpty(p)
    )
    {
        return 0;
    }


    *elemento =
        p->elementos[
            p->tope
        ];


    p->tope--;


    return 1;
}


int stackTop(
    const Pila *p,
    char *elemento
)
{
    /*
       Aqui solo consulto el tope.
       No modifico la pila.
    */

    if (
        isEmpty(p)
    )
    {
        return 0;
    }


    *elemento =
        p->elementos[
            p->tope
        ];


    return 1;
}


int simbolosEquivalentes(
    char abre,
    char cierra
)
{
    if (
        abre == '(' &&
        cierra == ')'
    )
    {
        return 1;
    }


    if (
        abre == '{' &&
        cierra == '}'
    )
    {
        return 1;
    }


    if (
        abre == '[' &&
        cierra == ']'
    )
    {
        return 1;
    }


    return 0;
}


int profundidad(
    const char expresion[]
)
{
    Pila p;

    int i;

    char symb;
    char temp;


    inicializarPila(
        &p
    );


    /*
       Recorro toda la expresion y solo me
       interesan los simbolos de agrupacion.
    */

    for (
        i = 0;
        expresion[i] != '\0';
        i++
    )
    {
        symb =
            expresion[i];


        if (
            symb == '(' ||
            symb == '{' ||
            symb == '['
        )
        {
            if (
                !push(
                    &p,
                    symb
                )
            )
            {
                /*
                   Si no cabe otro elemento
                   se presenta un overflow.
                */

                return 0;
            }
        }

        else if (
            symb == ')' ||
            symb == '}' ||
            symb == ']'
        )
        {
            /*
               Un cierre sin apertura provoca
               underflow y la expresion falla.
            */

            if (
                !pop(
                    &p,
                    &temp
                )
            )
            {
                return 0;
            }


            if (
                !simbolosEquivalentes(
                    temp,
                    symb
                )
            )
            {
                return 0;
            }
        }
    }


    /*
       Al final la pila debe quedar vacia.
       Si queda algo, falto cerrar un simbolo.
    */

    return isEmpty(
        &p
    );
}


int profundidadMaxima(
    const char expresion[],
    int *maxima
)
{
    Pila p;

    int i;
    int actual = 0;

    char symb;
    char temp;


    inicializarPila(
        &p
    );

    *maxima = 0;


    for (
        i = 0;
        expresion[i] != '\0';
        i++
    )
    {
        symb =
            expresion[i];


        if (
            symb == '(' ||
            symb == '{' ||
            symb == '['
        )
        {
            if (
                !push(
                    &p,
                    symb
                )
            )
            {
                return 0;
            }


            actual++;


            if (
                actual >
                *maxima
            )
            {
                *maxima =
                    actual;
            }
        }

        else if (
            symb == ')' ||
            symb == '}' ||
            symb == ']'
        )
        {
            if (
                !stackTop(
                    &p,
                    &temp
                )
            )
            {
                return 0;
            }


            if (
                !simbolosEquivalentes(
                    temp,
                    symb
                )
            )
            {
                return 0;
            }


            if (
                !pop(
                    &p,
                    &temp
                )
            )
            {
                return 0;
            }


            actual--;
        }
    }


    return isEmpty(
        &p
    );
}


/* ===============================
   MODO DE PRUEBA DE PILA
   =============================== */

void modoPila(void)
{
    char expresion[MAX_LINE];

    int maxima;


    printf(
        "*** Prueba de Pila ***\n"
    );

    printf(
        "Escriba una expresion: "
    );

    fflush(stdout);


    if (
        !leerLinea(
            expresion,
            sizeof(expresion)
        )
    )
    {
        printf(
            "No se pudo leer la expresion.\n"
        );

        return;
    }


    if (
        profundidad(
            expresion
        ) &&
        profundidadMaxima(
            expresion,
            &maxima
        )
    )
    {
        printf(
            "Expresion correcta.\n"
        );

        printf(
            "Profundidad maxima: %d\n",
            maxima
        );
    }

    else
    {
        printf(
            "Expresion incorrecta.\n"
        );
    }
}


/* ===============================
   PILA NUMERICA PARA POSTFIJO
   =============================== */

void inicializarPilaNumerica(
    PilaNumerica *p
)
{
    p->tope = -1;
}


int isEmptyNumerica(
    const PilaNumerica *p
)
{
    return p->tope == -1;
}


int cantidadPilaNumerica(
    const PilaNumerica *p
)
{
    return p->tope + 1;
}


int pushNumero(
    PilaNumerica *p,
    float elemento
)
{
    /*
       Antes de guardar reviso que todavia
       exista espacio dentro de la pila.
    */

    if (
        p->tope >=
        STACK_SIZE - 1
    )
    {
        return 0;
    }


    p->tope++;

    p->elementos[
        p->tope
    ] = elemento;


    return 1;
}


int popNumero(
    PilaNumerica *p,
    float *elemento
)
{
    /*
       No puedo sacar un operando si
       la pila numerica ya esta vacia.
    */

    if (
        isEmptyNumerica(p)
    )
    {
        return 0;
    }


    *elemento =
        p->elementos[
            p->tope
        ];


    p->tope--;


    return 1;
}


int esOperadorPostfijo(
    char simbolo
)
{
    return (
        simbolo == '+' ||
        simbolo == '-' ||
        simbolo == '*' ||
        simbolo == '/' ||
        simbolo == '%' ||
        simbolo == '^'
    );
}


int aplicarOperadorPostfijo(
    float opnd1,
    float opnd2,
    char operador,
    float *resultado
)
{
    /*
       Respeto el orden del algoritmo:
       primero sale opnd2 y despues opnd1.
    */

    switch (operador)
    {
        case '+':

            *resultado =
                opnd1 + opnd2;

            break;


        case '-':

            *resultado =
                opnd1 - opnd2;

            break;


        case '*':

            *resultado =
                opnd1 * opnd2;

            break;


        case '/':

            if (
                fabs((double)opnd2) < 1e-12
            )
            {
                printf(
                    "Error: division entre cero en la expresion.\n"
                );

                return 0;
            }


            *resultado =
                opnd1 / opnd2;

            break;


        case '%':

            if (
                fabs((double)opnd2) < 1e-12
            )
            {
                printf(
                    "Error: modulo entre cero en la expresion.\n"
                );

                return 0;
            }


            *resultado =
                (float)fmod(
                    (double)opnd1,
                    (double)opnd2
                );

            break;


        case '^':

            errno = 0;

            *resultado =
                (float)pow(
                    (double)opnd1,
                    (double)opnd2
                );


            if (
                errno != 0 ||
                !isfinite((double)*resultado)
            )
            {
                printf(
                    "Error: resultado invalido en la potencia.\n"
                );

                return 0;
            }

            break;


        default:

            return 0;
    }


    if (
        !isfinite((double)*resultado)
    )
    {
        printf(
            "Error: el resultado de la operacion no es valido.\n"
        );

        return 0;
    }


    return 1;
}


float evaluar(
    char expresion[]
)
{
    PilaNumerica pila;

    int i = 0;
    int hayEspacios = 0;

    char symb;

    float opnd1;
    float opnd2;
    float value;


    inicializarPilaNumerica(
        &pila
    );


    /*
       Si hay espacios puedo leer numeros
       completos como 12, -3 o 4.5.
    */

    if (
        strchr(
            expresion,
            ' '
        ) != NULL ||
        strchr(
            expresion,
            '\t'
        ) != NULL
    )
    {
        hayEspacios = 1;
    }


    while (
        expresion[i] != '\0'
    )
    {
        while (
            isspace(
                (unsigned char)expresion[i]
            )
        )
        {
            i++;
        }


        if (
            expresion[i] == '\0'
        )
        {
            break;
        }


        symb =
            expresion[i];


        /*
           Con espacios intento leer un numero
           completo antes de revisar operadores.
        */

        if (hayEspacios)
        {
            char *fin;

            double numero =
                strtod(
                    &expresion[i],
                    &fin
                );


            if (
                fin !=
                &expresion[i]
            )
            {
                if (
                    !pushNumero(
                        &pila,
                        (float)numero
                    )
                )
                {
                    printf(
                        "Error: overflow de la pila durante la evaluacion.\n"
                    );

                    return NAN;
                }


                i =
                    (int)(
                        fin - expresion
                    );

                continue;
            }
        }


        /*
           Sin espacios sigo el ejemplo clasico
           y cada digito representa un operando.
        */

        if (
            !hayEspacios &&
            isdigit(
                (unsigned char)symb
            )
        )
        {
            if (
                !pushNumero(
                    &pila,
                    (float)(symb - '0')
                )
            )
            {
                printf(
                    "Error: overflow de la pila durante la evaluacion.\n"
                );

                return NAN;
            }


            i++;

            continue;
        }


        if (
            esOperadorPostfijo(
                symb
            )
        )
        {
            /*
               El algoritmo indica sacar primero
               opnd2 y despues opnd1.
            */

            if (
                !popNumero(
                    &pila,
                    &opnd2
                ) ||
                !popNumero(
                    &pila,
                    &opnd1
                )
            )
            {
                printf(
                    "Error: faltan operandos para el operador %c.\n",
                    symb
                );

                return NAN;
            }


            if (
                !aplicarOperadorPostfijo(
                    opnd1,
                    opnd2,
                    symb,
                    &value
                )
            )
            {
                return NAN;
            }


            if (
                !pushNumero(
                    &pila,
                    value
                )
            )
            {
                printf(
                    "Error: overflow de la pila durante la evaluacion.\n"
                );

                return NAN;
            }


            i++;

            continue;
        }


        printf(
            "Error: simbolo no valido en la expresion: %c\n",
            symb
        );

        return NAN;
    }


    /*
       Una expresion correcta debe terminar
       con un solo resultado dentro de la pila.
    */

    if (
        cantidadPilaNumerica(
            &pila
        ) != 1
    )
    {
        printf(
            "Error: la expresion postfija no esta completa.\n"
        );

        return NAN;
    }


    if (
        !popNumero(
            &pila,
            &value
        )
    )
    {
        printf(
            "Error: no se pudo obtener el resultado final.\n"
        );

        return NAN;
    }


    return value;
}


/* ===============================
   MODO DE EVALUACION POSTFIJA
   =============================== */

void modoPostfijo(void)
{
    char expresion[MAX_LINE];

    float resultado;


    printf(
        "*** Evaluacion de expresiones en postfijo ***\n"
    );

    printf(
        "Puede usar el formato compacto: 354*+\n"
    );

    printf(
        "O separar numeros con espacios: 3 5 4 * +\n"
    );

    printf(
        "Expresion: "
    );

    fflush(stdout);


    if (
        !leerLinea(
            expresion,
            sizeof(expresion)
        )
    )
    {
        printf(
            "No se pudo leer la expresion.\n"
        );

        return;
    }


    resultado =
        evaluar(
            expresion
        );


    if (
        isnan((double)resultado)
    )
    {
        printf(
            "La expresion no pudo evaluarse.\n"
        );

        return;
    }


    if (
        fabs(
            (double)resultado -
            round((double)resultado)
        ) < 1e-9
    )
    {
        printf(
            "Resultado: %.0f\n",
            resultado
        );
    }

    else
    {
        printf(
            "Resultado: %.3f\n",
            resultado
        );
    }
}


/* ===============================
   PAUSA FINAL
   =============================== */

void pausarPrograma(void)
{
    char pausa[10];


    printf(
        "\nPresione Enter para cerrar..."
    );


    fgets(
        pausa,
        sizeof(pausa),
        stdin
    );
}
