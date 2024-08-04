# Parallel stuff

The objective is to implement the Bellman-Ford algorithm using both the OpenMP and CUDA parallel frameworks.

From [Wikipedia](https://en.wikipedia.org/wiki/Bellman%E2%80%93Ford_algorithm):

> The Bellman–Ford algorithm is an algorithm that computes shortest paths from a single source vertex to all of the other vertices in a weighted digraph.
> It is slower than Dijkstra's algorithm for the same problem, but more versatile, as it is capable of handling graphs in which some of the edge weights are negative numbers.

## Graph maker tool

Pending.

## Usage
For OMP
Usage:
```
./omp-bf [threads]
```

For Graph maker:
```
usage: graph_maker.py [-h] [-v NODES] [-e EDGES] [-neg | --negative | --no-negative]
```

## Compiling

### Linux


Pending.

### Windows

Pending.

## TODO:

- [ ] OpenMP stuff
    - [ ] Handle time recording
    - [ ] omp num of threads
- [ ] Cuda stuff
