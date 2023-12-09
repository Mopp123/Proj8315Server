FROM ubuntu:22.04
RUN apt-get update
RUN apt-get install -y python3
RUN apt-get install -y websockify
RUN apt-get install -y build-essential
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y postgresql
WORKDIR /usr/src/gameserver
EXPOSE 51421
