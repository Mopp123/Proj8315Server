# MMOGameServer

## Dependancies

### Websockify
Current client uses websockets so we need to websockify incomming traffic.
This is because I don't want this to be web exclusive.

### Database
Using PostgreSQL and libpqxx(https://github.com/jtv/libpqxx) management system. NOTE: libpqxx requires libpq-dev to be installed as well!


## Building
To build just the raw game server executable run build.sh.
