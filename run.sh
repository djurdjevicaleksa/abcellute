#!/bin/bash
set -xe

make

./abcellute input.csv #| tee output.csv