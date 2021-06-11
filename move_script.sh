#!/bin/bash

oldPath=$1
newPath=$2
numCubes=$3
$((numCubes--))

for num in $(seq 0 $numCubes)
do
    imgNum=$(printf "%02d" $num)
    echo $imgNum
    cubeDir=${newPath}/${imgNum}
    mkdir $cubeDir
    mkdir ${cubeDir}/cube_images
    mkdir ${cubeDir}/cube_images_json
    mkdir ${cubeDir}/cube_map
    mkdir ${cubeDir}/test
    mkdir ${cubeDir}/test_json

    for picNum in $(seq 1 6)
    do

        formatNum=$(printf "%02d" $picNum)
        mv ${oldPath}/cube_${imgNum}_${formatNum}_*.png $cubeDir/cube_images
        mv ${oldPath}/cube_${imgNum}_${formatNum}.json $cubeDir/cube_images_json
   
    done

    mv ${oldPath}/cube_${imgNum}_*_*.png $cubeDir/test
    mv ${oldPath}/cube_${imgNum}_*.json $cubeDir/test_json

done
