# Proj8315 Server

## Building
Currently only Linux build scripts are available. Tested with Ubuntu 22.04.5.

First you need to build libpqxx. Follow the instructions at https://github.com/jtv/libpqxx
*I was using the https://github.com/jtv/libpqxx/blob/master/BUILDING-configure.md
```
    ./configure --disable-shared
    make
    sudo make install
```

To build everything(the server executable, docker image and container) run `build-full.sh`
To build executable only run `build.sh`

## Running
This requires websockify since current client is web app.
We have docker setup here to handle that nicely.
After building run `start.sh` to start the docker container, initialize database and start the
server application.

## Dependancies

### Database
Using PostgreSQL and libpqxx(https://github.com/jtv/libpqxx) management system. NOTE: libpqxx requires libpq-dev to be installed as well!
