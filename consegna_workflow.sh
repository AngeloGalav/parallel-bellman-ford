#!/bin/bash

graph_dir="tools"

# compile ecerything first
./compile.sh

# run each model on each file
if ! [ -x "./omp-bf" ]; then
  echo "Error: omp-bf executable not found."
  exit 1
fi

for file in "$graph_dir"/*.txt; do
  if [ -f "$file" ]; then
    echo "Running OMP model on file: $file"
    ./omp-bf 1 "$file"
  fi
done


if ! [ -x "./cuda-bf" ]; then
  echo "Error: cuda-bf executable not found."
  exit 1
fi

for file in "$graph_dir"/*.txt; do
  if [ -f "$file" ]; then
    echo "Running CUDA model on file: $file"
    ./cuda-bf "$file"
  fi
done

# TODO: package everything into a .tar file


