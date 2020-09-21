#!/bin/sh
# usage: ./start_docker.sh <GPU number>
docker build docker -t replica
docker run -u $(id -u):$(id -g) --rm --gpus device=$1 -it -v /data:/data -v /data2:/data2 -e NVIDIA_VISIBLE_DEVICES=$1 -v /usr/share/glvnd/egl_vendor.d:/usr/share/glvnd/egl_vendor.d -v /tmp/.X11-unix:/tmp/.X11-unix -e DISPLAY=$DISPLAY -h $HOSTNAME -v $HOME/.Xauthority:/data/$USER/.Xauthority -e USER=$USER -e HOME=/data/$USER -w $PWD replica bash

