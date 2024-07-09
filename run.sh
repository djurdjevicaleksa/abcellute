#!/bin/bash
set -xe

gcc -c ss.c -o ss.o
gcc -c main.c -o main.o
gcc main.o ss.o -o main

./main input.csv | tee output.csv