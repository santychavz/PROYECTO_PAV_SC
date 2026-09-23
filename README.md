# Simpletron

Proyecto de simulacion de la computadora Simpletron desarrollado para la materia de Programacion Avanzada.

Esta rama contiene la version basica de Simpletron. El programa utiliza una memoria de 100 posiciones y permite cargar y ejecutar instrucciones escritas en SML (Simpletron Machine Language).

## Caracteristicas

- Memoria de 100 posiciones.
- Carga manual de instrucciones.
- El valor 9999 termina la carga del programa.
- Validacion de palabras entre -9999 y +9998 durante la carga.
- Ciclo de busqueda, decodificacion y ejecucion.
- Manejo de los registros internos de Simpletron.
- Implementacion de las 12 operaciones SML.
- Deteccion de errores fatales.
- Vaciado completo de registros y memoria al terminar la ejecucion.
- Vaciado de registros y memoria cuando ocurre un error fatal.

## Registros

El simulador utiliza los siguientes registros:

- accumulator
- instructionCounter
- instructionRegister
- operationCode
- operand

## Operaciones SML

| Codigo | Operacion | Funcion |
|---|---|---|
| 10 | READ | Lee un valor y lo guarda en memoria |
| 11 | WRITE | Muestra un valor almacenado en memoria |
| 20 | LOAD | Carga un valor de memoria al acumulador |
| 21 | STORE | Guarda el acumulador en memoria |
| 30 | ADD | Suma un valor al acumulador |
| 31 | SUBTRACT | Resta un valor al acumulador |
| 32 | DIVIDE | Divide el acumulador entre un valor de memoria |
| 33 | MULTIPLY | Multiplica el acumulador por un valor de memoria |
| 40 | BRANCH | Realiza un salto a otra posicion |
| 41 | BRANCHNEG | Salta si el acumulador es negativo |
| 42 | BRANCHZERO | Salta si el acumulador es cero |
| 43 | HALT | Termina la ejecucion |

## Formato de las instrucciones

Las instrucciones utilizan cuatro digitos.

Ejemplo:

    2090

Donde:

    20 = LOAD
    90 = posicion de memoria

El codigo de operacion se obtiene dividiendo la instruccion entre 100 y la direccion se obtiene utilizando el residuo de la division entre 100.

## Archivo principal

El archivo principal del proyecto es:

    SIMPLETRON1.0.CPP

## Compilacion

Para compilar el programa con g++:

    g++ "SIMPLETRON1.0.CPP" -o "SIMPLETRON1.0.exe"

Tambien puede compilarse desde Visual Studio Code utilizando el compilador configurado.

## Ejecucion

Despues de compilar se genera:

    SIMPLETRON1.0.exe

Puede abrirse directamente o ejecutarse desde la terminal con:

    .\SIMPLETRON1.0.exe

Al iniciar, Simpletron solicita una palabra por cada posicion de memoria:

    00 ?
    01 ?
    02 ?

Para terminar la carga del programa se introduce:

    9999

## Ejemplo de programa

El siguiente programa lee dos numeros, los suma y muestra el resultado:

    1090
    1091
    2090
    3091
    2192
    1192
    4300
    9999

Durante la ejecucion, si se introducen los valores:

    8
    2

el resultado es:

    +0010

Al terminar la ejecucion se muestran los registros y las 100 posiciones de memoria.

## Manejo de errores

El simulador detecta errores como:

- Division entre cero.
- Codigo de operacion invalido.
- Desbordamiento del acumulador.
- Contador de instrucciones fuera de rango.
- Estados o accesos invalidos.

Cuando ocurre un error fatal se muestra el mensaje correspondiente y se realiza un vaciado completo de los registros y de la memoria.

## Pruebas realizadas

Se comprobaron las siguientes operaciones:

- READ
- WRITE
- LOAD
- STORE
- ADD
- SUBTRACT
- DIVIDE
- MULTIPLY
- BRANCH
- BRANCHNEG
- BRANCHZERO
- HALT

Tambien se realizaron pruebas de:

- Division entre cero.
- Codigo de operacion invalido.
- Desbordamiento del acumulador.
- Validacion de valores fuera del rango permitido.

Los casos de prueba se encuentran en la carpeta pruebas.

## Ramas del proyecto

El repositorio contiene dos versiones del proyecto:

- master: version basica de Simpletron.
- feature: version con modificaciones y mejoras posteriores.