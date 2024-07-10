#!/bin/bash
set -xe

gcc -g main.c -o main

./main input.csv #| tee output.csv