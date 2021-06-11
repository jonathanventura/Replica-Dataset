#!/bin/bash

atlas_file=$1
new_dir=$2
pose_file=$3
num_cubes=$4

while read line; do
    ./run_renderer.sh $new_dir $line $pose_file $num_cubes
done < $atlas_file
