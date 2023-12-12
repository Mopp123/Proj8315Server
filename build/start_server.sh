#!/bin/bash

#su postgres -c "./init_db.sh"

hostname=$( hostname -i )
echo "Starting game server (hostname: $hostname)"

websockify $hostname:51421 $hostname:51422 & ./MMOGameServer && fg
