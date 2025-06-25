#!/bin/sh

# remove runc binary
sudo rm -f /usr/bin/runc

# remove docker and dependencies
sudo apt remove --purge -y docker-ce
sudo apt autoremove --purge -y

