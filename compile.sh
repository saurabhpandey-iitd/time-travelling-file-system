#!/bin/bash

# === Compile the project ===
g++ -std=c++17 -O2 -Wall main.cpp -o file_system

# === Check if compilation failed ===
if [ $? -ne 0 ]; then
    echo "Compilation failed!"
    exit 1
fi

# === Run the program ===
./file_system
