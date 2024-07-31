#!/bin/bash

# compile openmp
gcc -o omp-bf omp_bellmanford.c  -fopenmp -lpthread

# compile cude
