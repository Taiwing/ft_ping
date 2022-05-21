#/usr/bin/env bash

docker build -t $(basename $PWD)-docker .
docker run --network host -it $(basename $PWD)-docker:latest
