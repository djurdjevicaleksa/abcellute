#!/bin/bash
set -xe

gcc -g -O0 main.c -o main -lm

./main input.csv #| tee output.csv