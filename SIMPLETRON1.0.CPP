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

void pausarPrograma(void);


/* ===============================
   MAIN
   =============================== */

int main(void)
{
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
   LEER CADENA
   =============================== */

int leerCadenaEnMemoria(
    double memory[],
    int base
)
{
    char texto[MAX_LINE];

    size_t longitud;

    size_t i;


    printf("? ");

    fflush(stdout);


    if (
        !leerLinea(
            texto,
            sizeof(texto)
        )
    )
    {
        return 0;
    }


    longitud =
        strlen(texto);


    if (
        longitud >
        MAX_STRING
    )
    {
        printf(
            "La cadena supera el limite de %d caracteres.\n",
            MAX_STRING
        );

        return 0;
    }


    if (
        base < 0 ||
        base +
        (int)longitud >=
        MEMORY_SIZE
    )
    {
        printf(
            "La cadena no cabe a partir de la direccion %03d.\n",
            base
        );

        return 0;
    }


    /*
       Uso ASCII normal, por eso
       acepto valores de 0 a 127.
    */

    for (
        i = 0;
        i < longitud;
        i++
    )
    {
        unsigned char c =
            (unsigned char)
            texto[i];


        if (c > 127)
        {
            printf(
                "La entrada de cadenas usa ASCII. Use caracteres ASCII.\n"
            );

            return 0;
        }
    }


    /*
       La primera palabra guarda
       la longitud.

       Ejemplo:
       Mundo tiene 5 caracteres.

       memory[base] = 05000
    */

    memory[base] =
        (double)(
            (int)longitud *
            1000
        );


    /*
       Despues guardo el numero
       de caracter y su codigo ASCII.

       Ejemplo:
       M = 077

       Queda:
       01 077
    */

    for (
        i = 0;
        i < longitud;
        i++
    )
    {
        memory[
            base +
            (int)i +
            1
        ] =
            (double)(
                (
                    (int)i + 1
                ) *
                1000 +
                (unsigned char)
                texto[i]
            );
    }


    return 1;
}


/* ===============================
   ESCRIBIR CADENA
   =============================== */

int escribirCadenaDesdeMemoria(
    double memory[],
    int base
)
{
    int primeraPalabra;

    int longitud;

    int i;


    if (
        base < 0 ||
        base >= MEMORY_SIZE
    )
    {
        return 0;
    }


    if (
        !esEntero(
            memory[base]
        )
    )
    {
        return 0;
    }


    primeraPalabra =
        (int)memory[base];


    longitud =
        primeraPalabra /
        1000;


    if (
        longitud < 0 ||
        longitud > MAX_STRING
    )
    {
        return 0;
    }


    if (
        base +
        longitud >=
        MEMORY_SIZE
    )
    {
        return 0;
    }


    for (
        i = 1;
        i <= longitud;
        i++
    )
    {
        int palabra;

        int indice;

        int ascii;


        if (
            !esEntero(
                memory[
                    base + i
                ]
            )
        )
        {
            return 0;
        }


        palabra =
            (int)memory[
                base + i
            ];


        indice =
            palabra /
            1000;


        ascii =
            palabra %
            1000;


        if (
            indice != i ||
            ascii < 0 ||
            ascii > 127
        )
        {
            return 0;
        }


        putchar(
            (char)ascii
        );
    }


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
