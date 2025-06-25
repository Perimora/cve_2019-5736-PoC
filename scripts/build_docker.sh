#!/bin/sh

set -e

# rebuild exploit
make clean
make

# copy exploit binary into docker build directory
cp build/exploit docker/

# build image
docker build -t runc_exploit docker/

# clean up
rm docker/exploit
make clean
