#!/bin/bash

#./build/ReplicaSDK/ReplicaRenderer $1 /data2/replica_v1/apartment_0/mesh.ply /data2/replica_v1/apartment_0/textures /data2/replica_v1/apartment_0/glass.sur

#./build/ReplicaSDK/ReplicaRenderer $1 ./replica_dir/apartment_0/mesh.ply ./replica_dir/apartment_0/textures ./replica_dir/apartment_0/glass.sur

#USAGE: New Direcory, Atlas, Pose File, #Cubes in Pose File

./build/ReplicaSDK/ReplicaRenderer $3 ./replica_dir/$2/mesh.ply ./replica_dir/$2/textures ./replica_dir/$2/glass.sur

textFile=$3
cur_dir=$(pwd)

dirname=${textFile%".txt"}
new_dir=../pics/$1/$2

mkdir -p ../pics/$1
mkdir -p $new_dir

echo $dirname
echo $cur_dir
echo $new_dir

#mv *.png ./tempDir/
#mkdir -p $new_dir

./move_script.sh $cur_dir $new_dir $4
#rm ../pics/$dirname/*
#mkdir -p ../pics/$dirname
#mv *.png ../pics/$dirname
