#!/bin/bash -ex

# Clean up
function cleanup {
    rm -r nachos
}

trap cleanup EXIT

# Copy from local
cp -r ../../Nachos nachos

# Build docker
docker build --tag nachos:${1:-0.1} --tag nachos:latest --file original.Dockerfile .
