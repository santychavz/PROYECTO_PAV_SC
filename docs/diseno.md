# Documentacion de diseno - Proyecto Simpletron

## 1. Arquitectura general
El proyecto esta formado por tres componentes:
- Simulador Simpletron.
- Herramientas de pilas y expresiones.
- Compilador Simple a SML.

## 2. Simulador
Utiliza 1000 posiciones de memoria y cinco registros:
accumulator, instructionCounter, instructionRegister,
operationCode y operand.

Ejecuta el ciclo de busqueda, decodificacion y ejecucion.
Admite carga manual o automatica desde programa.simp.

## 3. Organizacion de memoria
Las instrucciones se almacenan desde las primeras posiciones.
Las variables y constantes se asignan desde las posiciones
superiores, evitando superponer codigo y datos.

## 4. Expresiones
Se utilizan pilas para validar agrupadores y convertir
expresiones infijas a postfijas, respetando precedencia
y asociatividad de operadores.

## 5. Compilador de dos pasadas
Primera pasada:
- Analizar las instrucciones del lenguaje Simple.
- Registrar variables, constantes y numeros de linea.
- Construir la tabla de simbolos.
- Generar instrucciones SML preliminares.

Segunda pasada:
- Resolver referencias pendientes.
- Completar los saltos.
- Generar el archivo SML definitivo.

## 6. Decisiones de diseno
- Separar el compilador del simulador.
- Utilizar C++17.
- Conservar versiones mediante ramas Git.
- Validar errores antes de ejecutar programas.
- Generar programas compatibles con Simpletron.

## 7. Validacion
Se probaron programas de secuencia, seleccion y repeticion.
Tambien se verificaron operaciones, memoria, errores fatales,
expresiones aritmeticas y saltos condicionales.
