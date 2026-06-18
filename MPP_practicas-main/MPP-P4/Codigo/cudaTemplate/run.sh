#!/bin/bash

make FILE=cudaTemplate

# Pruebas 2d
# make run FILE=cudaTemplate

# Pruebas 3d
make run3d FILE=cudaTemplate

make clean FILE=cudaTemplate