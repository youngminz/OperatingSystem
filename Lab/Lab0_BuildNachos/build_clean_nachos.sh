#!/bin/bash -ex

# Clean up
function cleanup {
    rm -r nachos-3.4.tar.gz nachos
}

trap cleanup EXIT

# Copy from local
cp ../../Files/nachos-3.4.tar.gz .
tar xzf nachos-3.4.tar.gz
mv nachos_dianti nachos
rm -r nachos/gnu-decstation-ultrix/arm

# Build docker
docker build --tag nachos:0.0.1 --file original.Dockerfile .
