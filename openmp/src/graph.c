#include "../include/graph.h"

Graph *initGraph(int V, int E)
{
    Graph *graph = (Graph *)malloc(sizeof(Graph));
    graph->V = V;
    graph->E = E;
    graph->nodes = (Node **)malloc(V * sizeof(Node *));
    return graph;
}

void addEdge(Graph *graph, int src, int dest, int cost)
{
    Node *hd = graph->nodes[src];
    while (hd->next != NULL)
    {
        hd = hd->next;
    }
    hd->next = malloc(sizeof(Node));
    hd->next->id = dest;
    hd->next->cost = cost;
}

Graph *createTestGraph()
{
    int V = 5;
    int E = 7;
    Graph *graph = initGraph(V, E);
    for (int i = 0; i < V; i++)
    {
        graph->nodes[i] = malloc(sizeof(Node));
        graph->nodes[i]->id = i;
    }

    addEdge(graph, 1, 3, 1);
    addEdge(graph, 1, 2, 1);
    addEdge(graph, 1, 4, 1);
    addEdge(graph, 1, 0, 1);
    addEdge(graph, 4, 0, 1);
    addEdge(graph, 2, 3, -1);
    addEdge(graph, 2, 5, -1);

    return graph;
}

void printGraph(Graph *graph)
{
    printf("Graph edges:\n");
    for (int i = 0; i < graph->V; ++i)
    {
        printf("info of %d: ", i);
        printEdgesOfNode(graph->nodes[i]);
    }
}

void printEdgesOfNode(Node *node)
{
    Node *hd = node;
    printf("Node %d is attached to nodes", node->id);
    while (hd->next != NULL)
    {
        hd = hd->next;
        printf(" %d,", hd->id);
    }
    printf(" - END\n");
}