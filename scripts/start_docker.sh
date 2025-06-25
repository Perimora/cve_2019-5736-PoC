#!/bin/sh

# remove old container
docker rm exp 2> /dev/null

# start exploit container
docker run -it --name exp --rm runc_exploit
