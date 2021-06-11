#!/bin/bash

old_path=$1
new_path=$2
num_imgs=$3

dir_count=0
dir_name=${new_path}/${dir_count}

for num in $(seq 0 $num_imgs)
do
    img_num=$(($num % 6))
    if [ $img_num = 0 ]
    then
        dir_name=${new_path}/${dir_count}
        mkdir $dir_name
        $((dir_count++))
    fi
    img_name=$(printf "%06d" $num)
    echo ${dir_name}
    mv ${old_path}/depth${img_name}.png ${dir_name}
    mv ${old_path}/frame${img_name}.png ${dir_name}
done
