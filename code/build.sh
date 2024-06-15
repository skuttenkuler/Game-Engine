#!/bin/bash
#sources
SOURCE_DIR="$(pwd)/."
OUTPUT_DIR="$(pwd)/../build"

mkdir -p "$OUTPUT_DIR"

#build
g++ "$SOURCE_DIR/main.cpp" -o "$OUTPUT_DIR/main.exe"
g++ -c "$SOURCE_DIR/main.cpp" -o "$OUTPUT_DIR/main.obj"
#execute
cd "$OUTPUT_DIR" || exit

./main
