#!/bin/bash
#SBATCH --job-name=parallel-bellman-ford
#SBATCH --mail-type=ALL
#SBATCH --mail-user=angelo.galavotti@studio.unibo.it
#SBATCH --time=01:00:00
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=4
#SBATCH --output=output_%j.txt
#SBATCH --error=error_%j.txt
#SBATCH --gres=gpu:1


# compile openmp
gcc -o omp-bf omp_bellmanford.c -fopenmp -Wall

# compile cuda
nvcc -o cuda-bf cuda_bellmanford.cu

# complete a benchmark run on everything
./benchmark.sh -d -t 5m

# pack results into a tarball
tar -czvf fresh_results.tar.gz results
