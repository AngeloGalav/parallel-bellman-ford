#ifndef BELLMANFORD_H
#define BELLMANFORD_H

#include <limits.h>
#include "graph.h"

void BellmanFord(Graph* graph, int src);
void printArr(int dist[], int n);

#endif