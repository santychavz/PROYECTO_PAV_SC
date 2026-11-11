#!/usr/bin/env bash
set -e
mkdir -p output
g++ -std=c++17 -Wall -Wextra SIMPLETRON1.0.CPP -o output/simpletron_revision.exe
g++ -std=c++17 -Wall -Wextra CompiladorSimpletron/compiler.cpp -o output/compiler.exe
echo "Compilacion exitosa"
