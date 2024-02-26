#include "../include/graph.h"

Graph *initGraph(int V, int E)
{
    Graph* graph = (Graph*) malloc(sizeof(Graph));
    graph->V = V;
    graph->E = E;
    graph->nodes = (Node*) malloc(sizeof(V * sizeof(Node)));
    return graph;
}

void addNode(Node* nodeL) {
    nodeL->next = (Node*) malloc(sizeof(Node));
}

void addEdge(Graph *graph, int edgeIndex, int src, int dest, int cost)
{
    // graph->nodes[edgeIndex].src = src;
    // graph->nodes[edgeIndex].dest = dest;
    // graph->nodes[edgeIndex].cost = cost;
}

Graph *createTestGraph()
{
    int V = 5;
    int E = 7;
    Graph *graph = initGraph(V, E);

    addEdge(graph, 0, 0, 1, 1);
    addEdge(graph, 1, 0, 4, 2);
    addEdge(graph, 2, 1, 2, 3);
    addEdge(graph, 3, 1, 3, -1);
    addEdge(graph, 4, 1, 4, -2);
    addEdge(graph, 5, 2, 3, -3);
    addEdge(graph, 6, 3, 4, 0);

    return graph;
}

void printGraph(Graph *graph)
{
    printf("Graph edges:\n");
    for (int i = 0; i < graph->E; ++i)
    {
        // printf("edge %d, nodes %d -- %d,  cost %d\n", i, graph->edge[i].src,
            //    graph->edge[i].dest, graph->edge[i].cost);
    }
}