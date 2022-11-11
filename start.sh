#!/bin/bash
websockify 127.0.0.1:51421 127.0.0.1:51422 & ./MMOGameServer && fg
