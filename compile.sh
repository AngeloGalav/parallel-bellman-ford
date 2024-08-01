#!/bin/bash

# compile openmp
gcc -o omp-bf openmp/omp_bellmanford.c -fopenmp -lpthread

# compile cuda
# nvcc -o cuda-bf cuda/cuda_bellmanford.cu
