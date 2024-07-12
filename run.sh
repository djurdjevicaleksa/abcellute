#!/bin/bash
set -xe

gcc -g -O0 main.c -o main

./main input.csv #| tee output.csv