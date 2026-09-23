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

int cargarPrograma(int memory[]);

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

void pausarPrograma(void);


/* =========================
   PROGRAMA PRINCIPAL
   ========================= */

int main(void)
{
    int memory[MEMORY_SIZE] = {0};

    mostrarBienvenida();


    /*
       Solo empiezo a ejecutar si la carga
       termino correctamente con 9999.
    */

    if (!cargarPrograma(memory))
    {
        pausarPrograma();

        return 1;
    }


    ejecutarPrograma(memory);


    /*
       Dejo la consola abierta para poder
       revisar los resultados antes de cerrar.
    */

    pausarPrograma();


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

int cargarPrograma(int memory[])
{
    int posicion = 0;

    int palabra;


    while (posicion < MEMORY_SIZE)
    {
        printf(
            "%02d ? ",
            posicion
        );


        fflush(stdout);


        if (!leerEntero(&palabra))
        {
            /*
               Si ya no hay entrada no tiene
               sentido comenzar la ejecucion.
            */

            if (feof(stdin))
            {
                printf(
                    "\nError: la entrada termino antes de recibir 9999.\n"
                );

                return 0;
            }


            printf(
                "Entrada invalida. Introduzca un numero entero.\n"
            );


            continue;
        }


        /*
           9999 es el centinela.
           No se guarda dentro de memory.
        */

        if (palabra == 9999)
        {
            printf(
                "\n*** Carga del programa completa ***\n"
            );


            printf(
                "*** Comienza la ejecucion del programa ***\n\n"
            );


            return 1;
        }


        /*
           Las palabras que se cargan deben
           estar dentro del rango permitido.
        */

        if (
            palabra < -9999 ||
            palabra > 9998
        )
        {
            printf(
                "Valor invalido. Debe estar entre -9999 y +9998.\n"
            );


            continue;
        }


        /*
           Guardo la palabra en la posicion
           que aparece antes del signo ?.
        */

        memory[posicion] =
            palabra;


        posicion++;
    }


    /*
       Si llegue hasta aqui se llenaron las
       100 posiciones sin recibir 9999.
    */

    printf(
        "\nError: la memoria se lleno antes de recibir el centinela 9999.\n"
    );


    return 0;
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
        /*
           Reviso el contador antes de buscar
           una instruccion dentro de memory.
        */

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
           Busco la instruccion que toca ejecutar.
        */

        instructionRegister =
            memory[instructionCounter];


        /*
           Separo los dos primeros digitos
           del opcode y los ultimos dos
           de la direccion.
        */

        operationCode =
            instructionRegister / 100;


        operand =
            instructionRegister % 100;


        /*
           Reviso que la direccion exista
           dentro de las 100 posiciones.
        */

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
           Los BRANCH pueden cambiarla despues.
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
                        if (feof(stdin))
                        {
                            errorFatal(
                                "La entrada termino durante una operacion READ.",
                                memory,
                                accumulator,
                                instructionCounter - 1,
                                instructionRegister,
                                operationCode,
                                operand
                            );


                            return;
                        }


                        printf(
                            "Entrada invalida. Introduzca un numero entero.\n"
                        );


                        dato =
                            10000;


                        continue;
                    }


                    /*
                       En READ el 9999 si puede
                       ser utilizado como dato.
                    */

                    if (
                        dato < -9999 ||
                        dato > 9999
                    )
                    {
                        printf(
                            "Valor invalido. Debe estar entre -9999 y +9999.\n"
                        );
                    }

                }
                while (
                    dato < -9999 ||
                    dato > 9999
                );


                memory[operand] =
                    dato;


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

                accumulator =
                    memory[operand];


                break;


            /* =========================
               21 - STORE
               ========================= */

            case STORE:

                memory[operand] =
                    accumulator;


                break;


            /* =========================
               30 - ADD
               ========================= */

            case ADD:

                /*
                   Uso long para revisar el resultado
                   antes de guardarlo en accumulator.
                */

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

                /*
                   Primero reviso que no se
                   intente dividir entre cero.
                */

                if (
                    memory[operand] == 0
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
                    (long) accumulator /
                    memory[operand];


                if (
                    resultado > 9999 ||
                    resultado < -9999
                )
                {
                    errorFatal(
                        "Desbordamiento del acumulador al dividir.",
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

                if (
                    accumulator < 0
                )
                {
                    instructionCounter =
                        operand;
                }


                break;


            /* =========================
               42 - BRANCHZERO
               ========================= */

            case BRANCHZERO:

                if (
                    accumulator == 0
                )
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
        "*** La ejecucion de Simpletron termino anormalmente ***\n"
    );


    /*
       Ante un error fatal muestro el estado
       completo que tenia Simpletron.
    */

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


    printf(
        "\nREGISTROS:\n\n"
    );


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


    printf(
        "\nMEMORIA:\n\n"
    );


    /*
       Encabezados de las columnas.
    */

    printf("   ");


    for (
        columna = 0;
        columna < 10;
        columna++
    )
    {
        printf(
            "%6d",
            columna
        );
    }


    printf("\n");


    /*
       Cada fila representa diez posiciones:
       00-09, 10-19, ... 90-99.
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
                memory[
                    fila + columna
                ]
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
    char linea[100];

    char extra;


    /*
       Leo una linea completa para evitar
       aceptar entradas como 123abc.
    */

    if (
        fgets(
            linea,
            sizeof(linea),
            stdin
        ) == NULL
    )
    {
        return 0;
    }


    /*
       Si solo encuentro un numero entero
       considero la entrada valida.
    */

    if (
        sscanf(
            linea,
            " %d %c",
            valor,
            &extra
        ) == 1
    )
    {
        return 1;
    }


    return 0;
}


/* =========================
   PAUSA FINAL
   ========================= */

void pausarPrograma(void)
{
    char pausa[10];


    printf(
        "\nPresione Enter para cerrar..."
    );


    /*
       El programa se queda aqui hasta
       que presione Enter.
    */

    fgets(
        pausa,
        sizeof(pausa),
        stdin
    );
}