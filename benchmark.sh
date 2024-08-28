#!/bin/bash
# Runs all program on all graphs with all thread configurations

# # compile openmp
# gcc -o omp-bf openmp/omp_bellmanford.c -fopenmp -Wall

# # compile cuda
# # nvcc -o cuda-bf cuda/cuda_bellmanford.cu

# check for folder presence
if [ ! -d "results" ]; then
    mkdir results
fi

threads=(1 2 3 4 5 6 7 8 9 10 16)

graph_files=$(ls graphs/)

for n in "${threads[@]}"; do
  for graph_file in $graph_files; do
    echo "Running omp-bf on $graph_file with $n threads"
    timeout 3m ./omp-bf "$n" "graphs/$graph_file"
    if [ $? -eq 124 ]; then
      echo "Execution timed out for $graph_file with $n threads"
      echo "graphs/${graph_file},${n},TO" >> results/omp.csv
    fi
  done
done

# for n in "${threads[@]}"; do
#   for graph_file in $graph_files; do
#     echo "Running omp_bf on $graph_file with $n threads"
#     timeout 5m ./cuda-bf "$n" "graphs/$graph_file"
#   done
# done

# TODO: package everything into a .tar file
