#include <stdio.h>
#include <stdlib.h>

typedef struct node {
    int id;
    int cost;
    struct node* next;
} Node;

typedef struct graph
{
    int V, E;
    Node* nodes;
} Graph;

Graph *initGraph(int V, int E);
Graph *createTestGraph();
void addEdge(Graph* graph, int edgeIndex, int src, int dest, int cost);
void printGraph(Graph *graph);
