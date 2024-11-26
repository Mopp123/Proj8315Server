# Proj8315 Server

## Building
To build everything(the server executable, docker image and container) run `build-full.sh`
To build executable only run `build.sh`

## Running
This requires websockify since current client is web app.
We have docker setup here to make that easier...
After building run `start.sh`

## Dependancies

### Websockify
Current client uses websockets so we need to websockify incomming traffic.
This is because I don't want this to be web exclusive.

### Database
Using PostgreSQL and libpqxx(https://github.com/jtv/libpqxx) management system. NOTE: libpqxx requires libpq-dev to be installed as well!
