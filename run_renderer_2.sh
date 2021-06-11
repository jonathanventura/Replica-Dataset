#!/bin/bash

#./build/ReplicaSDK/ReplicaRenderer $1 /data2/replica_v1/apartment_0/mesh.ply /data2/replica_v1/apartment_0/textures /data2/replica_v1/apartment_0/glass.sur

./build/ReplicaSDK/ReplicaRenderer $1 ./replica_dir/apartment_0/mesh.ply ./replica_dir/apartment_0/textures ./replica_dir/apartment_0/glass.sur

textFile=$1
cur_dir=$(pwd)

dirname=${textFile%".txt"}
new_dir=../pics/${dirname}

echo $dirname
echo $cur_dir
echo $new_dir

#mv *.png ./tempDir/
mkdir -p $new_dir

./moveScript.sh $cur_dir $new_dir $2

#rm ../pics/$dirname/*
#mkdir -p ../pics/$dirname
#mv *.png ../pics/$dirname
