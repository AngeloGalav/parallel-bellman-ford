# Implementation of the Bellman-Ford SSSP algorithm in CUDA and OpenMP

From [Wikipedia](https://en.wikipedia.org/wiki/Bellman%E2%80%93Ford_algorithm):

> The Bellman–Ford algorithm is an algorithm that computes shortest paths from a single source vertex to all of the other vertices in a weighted digraph.
> It is slower than Dijkstra's algorithm for the same problem, but more versatile, as it is capable of handling graphs in which some of the edge weights are negative numbers.

## Usage
- OpenMP
```
./omp-bf [threads] [graph_file]
```
- CUDA
```
./cuda-bf [graph_file]
```

## Compiling
On both Linux and Windows, you can use the following commands to compile the project:
- OpeMP
```
gcc -o omp-bf openmp/omp_bellmanford.c -fopenmp -Wall
```
- CUDA
```
nvcc -o cuda-bf cuda/cuda_bellmanford.cu
```
## Graph Maker Tool
Graph Maker:
```
graph_maker.py [-h] [-v NODES] [-e EDGES] [-neg | --negative | --no-negative]
```

## TODO:
- [ ] Rewrite graphs
- [ ] Launch benchmark on remote machine
- [ ] Write plt scripts
- [ ] Write report
