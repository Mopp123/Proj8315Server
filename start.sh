#!/bin/bash

echo "Starting container..."
docker compose -f docker-compose.yml start

echo "Initializing database..."
docker exec --user postgres proj8315-server ./init_db.sh

echo "Starting server..."
docker exec -d proj8315-server ./start_server.sh
