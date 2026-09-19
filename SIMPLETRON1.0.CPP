#include <stdio.h>

#define MEMORY_SIZE 100

/* Codigos de operacion */
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


/* Prototipos */
void mostrarBienvenida(void);

void cargarPrograma(int memory[]);

void ejecutarPrograma(int memory[]);

void vaciado(
    int memory[],
    int accumulator,
    int instructionCounter,
    int instructionRegister,
    int operationCode,
    int operand
);

void errorFatal(
    const char mensaje[],
    int memory[],
    int accumulator,
    int instructionCounter,
    int instructionRegister,
    int operationCode,
    int operand
);

int leerEntero(int *valor);

void limpiarEntrada(void);


/* =========================
   PROGRAMA PRINCIPAL
   ========================= */

int main(void)
{
    int memory[MEMORY_SIZE] = {0};

    mostrarBienvenida();

    cargarPrograma(memory);

    ejecutarPrograma(memory);

    return 0;
}


/* =========================
   BIENVENIDA
   ========================= */

void mostrarBienvenida(void)
{
    printf("*** Bienvenido a Simpletron! ***\n");
    printf("** Introduzca su programa una instruccion ***\n");
    printf("** (o palabra de datos) a la vez en la linea ***\n");
    printf("** de texto de entrada. Yo indicare el numero ***\n");
    printf("** de posicion y una interrogacion (?). Usted ***\n");
    printf("** tecleara entonces la palabra para esa ***\n");
    printf("** posicion. Escriba 9999 para terminar ***\n");
    printf("** de introducir su programa. ***\n\n");
}


/* =========================
   CARGAR PROGRAMA
   ========================= */

void cargarPrograma(int memory[])
{
    int posicion = 0;
    int palabra;

    while (posicion < MEMORY_SIZE)
    {
        printf("%02d ? ", posicion);

        fflush(stdout);

        if (!leerEntero(&palabra))
        {
            printf(
                "Entrada invalida. Introduzca un numero entero.\n"
            );

            continue;
        }


        /* 9999 termina la carga */
        if (palabra == 9999)
        {
            break;
        }


        /* Validar rango */
        if (palabra < -9999 || palabra > 9998)
        {
            printf(
                "Valor invalido. Debe estar entre -9999 y +9998.\n"
            );

            continue;
        }


        /* Guardar exactamente en la posicion mostrada */
        memory[posicion] = palabra;

        posicion++;
    }


    printf("\n*** Carga del programa completa ***\n");

    printf("*** Comienza la ejecucion del programa ***\n\n");
}


/* =========================
   EJECUTAR PROGRAMA
   ========================= */

void ejecutarPrograma(int memory[])
{
    int accumulator = 0;

    int instructionCounter = 0;

    int instructionRegister = 0;

    int operationCode = 0;

    int operand = 0;

    int dato;

    long resultado;


    while (1)
    {
        /* Verificar posicion de instruccion */
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


        /* Obtener instruccion */
        instructionRegister =
            memory[instructionCounter];


        /* Separar codigo de operacion y direccion */

        operationCode =
            instructionRegister / 100;

        operand =
            instructionRegister % 100;


        /*
           Se prepara la siguiente posicion.
           Las instrucciones BRANCH pueden
           modificarla despues.
        */

        instructionCounter++;


        switch (operationCode)
        {

            /* =========================
               10 - READ
               ========================= */

            case READ:

                do
                {
                    printf("? ");

                    fflush(stdout);

                    if (!leerEntero(&dato))
                    {
                        printf(
                            "Entrada invalida. Introduzca un numero entero.\n"
                        );

                        dato = 10000;

                        continue;
                    }


                    if (
                        dato < -9999 ||
                        dato > 9999
                    )
                    {
                        printf(
                            "Valor invalido. "
                            "Debe estar entre -9999 y +9999.\n"
                        );
                    }

                }
                while (
                    dato < -9999 ||
                    dato > 9999
                );


                /*
                   READ guarda el valor en la
                   direccion indicada.

                   Ejemplo:
                   1097 -> memory[97]
                */

                memory[operand] = dato;

                break;


            /* =========================
               11 - WRITE
               ========================= */

            case WRITE:

                printf(
                    "%+05d\n",
                    memory[operand]
                );

                break;


            /* =========================
               20 - LOAD
               ========================= */

            case LOAD:

                /*
                   Ejemplo:
                   2007 -> accumulator = memory[07]
                */

                accumulator =
                    memory[operand];

                break;


            /* =========================
               21 - STORE
               ========================= */

            case STORE:

                /*
                   Guarda accumulator en
                   la direccion indicada.
                */

                memory[operand] =
                    accumulator;

                break;


            /* =========================
               30 - ADD
               ========================= */

            case ADD:

                resultado =
                    (long) accumulator +
                    memory[operand];


                if (
                    resultado > 9999 ||
                    resultado < -9999
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
                    (int) resultado;

                break;


            /* =========================
               31 - SUBTRACT
               ========================= */

            case SUBTRACT:

                resultado =
                    (long) accumulator -
                    memory[operand];


                if (
                    resultado > 9999 ||
                    resultado < -9999
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
                    (int) resultado;

                break;


            /* =========================
               32 - DIVIDE
               ========================= */

            case DIVIDE:

                if (memory[operand] == 0)
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


                accumulator =
                    accumulator /
                    memory[operand];

                break;


            /* =========================
               33 - MULTIPLY
               ========================= */

            case MULTIPLY:

                resultado =
                    (long) accumulator *
                    memory[operand];


                if (
                    resultado > 9999 ||
                    resultado < -9999
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
                    (int) resultado;

                break;


            /* =========================
               40 - BRANCH
               ========================= */

            case BRANCH:

                instructionCounter =
                    operand;

                break;


            /* =========================
               41 - BRANCHNEG
               ========================= */

            case BRANCHNEG:

                if (accumulator < 0)
                {
                    instructionCounter =
                        operand;
                }

                break;


            /* =========================
               42 - BRANCHZERO
               ========================= */

            case BRANCHZERO:

                if (accumulator == 0)
                {
                    instructionCounter =
                        operand;
                }

                break;


            /* =========================
               43 - HALT
               ========================= */

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


            /* =========================
               CODIGO INVALIDO
               ========================= */

            default:

                errorFatal(
                    "Codigo de operacion no valido.",
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


/* =========================
   ERROR FATAL
   ========================= */

void errorFatal(
    const char mensaje[],
    int memory[],
    int accumulator,
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
        "*** La ejecucion de Simpletron "
        "termino anormalmente ***\n"
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


/* =========================
   VACIADO DE MEMORIA
   ========================= */

void vaciado(
    int memory[],
    int accumulator,
    int instructionCounter,
    int instructionRegister,
    int operationCode,
    int operand
)
{
    int fila;
    int columna;


    printf("\nREGISTROS:\n\n");


    printf(
        "accumulator             %+05d\n",
        accumulator
    );


    printf(
        "instructionCounter        %02d\n",
        instructionCounter
    );


    printf(
        "instructionRegister      %+05d\n",
        instructionRegister
    );


    printf(
        "operationCode              %02d\n",
        operationCode
    );


    printf(
        "operand                    %02d\n",
        operand
    );


    printf("\nMEMORIA:\n\n");


    /*
       Encabezado de columnas

       Ejemplo:

             0      1      2 ... 9
    */

    printf("   ");

    for (columna = 0; columna < 10; columna++)
    {
        printf(
            "%6d",
            columna
        );
    }

    printf("\n");


    /*
       Filas:

       00 = posiciones 00 a 09
       10 = posiciones 10 a 19
       ...
       90 = posiciones 90 a 99
    */

    for (
        fila = 0;
        fila < MEMORY_SIZE;
        fila += 10
    )
    {
        printf(
            "%02d ",
            fila
        );


        for (
            columna = 0;
            columna < 10;
            columna++
        )
        {
            printf(
                "%+05d ",
                memory[fila + columna]
            );
        }


        printf("\n");
    }
}


/* =========================
   LEER ENTERO
   ========================= */

int leerEntero(int *valor)
{
    if (scanf("%d", valor) == 1)
    {
        return 1;
    }


    limpiarEntrada();

    return 0;
}


/* =========================
   LIMPIAR ENTRADA
   ========================= */

void limpiarEntrada(void)
{
    int caracter;


    while (
        (caracter = getchar()) != '\n' &&
        caracter != EOF
    )
    {
    }
}