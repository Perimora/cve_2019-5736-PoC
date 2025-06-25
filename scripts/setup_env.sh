#!/bin/sh

set -e  # shutdown on error

# check if docker_files directory exists
if [ ! -d docker_files ]; then
    echo "Directory containing docker files does not exist"
    echo "Downloading docker files to docker_files/"
    
    # create directory
    mkdir -p docker_files
    
    # download docker tarball
    wget -O docker_files/docker-18.09.1.tgz https://download.docker.com/linux/static/stable/x86_64/docker-18.09.1.tgz

    # unpack files
    tar -xvf docker_files/docker-18.09.1.tgz -C docker_files/
fi

# copy fresh runc binary onto path
sudo cp -f docker_files/docker/* /usr/bin/

echo "Successfully restored all Docker binaries to /usr/bin!"

# check versions
docker --version
runc --version
