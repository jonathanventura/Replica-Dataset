#!/usr/bin/env bash
# Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved

cd 3rdparty/Pangolin
mkdir build
cd build
cmake ..
make -j


cd ../../fmt
mkdir build
cd build
cmake ..

cd ../../../
mkdir build
cd build
cmake ..
make -j
