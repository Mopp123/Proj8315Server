#!/bin/bash

echo "Starting build..."
cd ./build

cmake -S ../ -B .
cmake --build .

mv compile_commands.json ../
