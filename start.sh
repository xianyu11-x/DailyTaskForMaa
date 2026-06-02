#!/bin/bash

mkdir -p ./coredumps
chmod 1777 ./coredumps

./cronUpdate.sh install

docker-compose up -d

./cronUpdate.sh
