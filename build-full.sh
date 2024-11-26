#!/bin/bash

echo "Starting full build"
echo "Building executable..."
cd ./build

cmake -S ../ -B .
cmake --build .

mv compile_commands.json ../

cd ..

echo "Building docker image..."
docker build . -t proj8315-gameserver

echo "creating container..."
docker compose -f docker-compose.yml create
