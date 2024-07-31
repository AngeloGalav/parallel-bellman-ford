#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

typedef struct node {
    int id;
    int cost;
    struct node* next;
} Node;

typedef struct graph
{
    int V, E;
    Node** nodes;
} Graph;

void BellmanFord(Graph *graph, int src);
void printArr(int dist[], int n);
Graph *initGraph(int V, int E);
void addEdge(Graph *graph, int src, int dest, int cost);
Graph *createTestGraph();
void printGraph(Graph *graph);
void printEdgesOfNode(Node *node);

int main(int argc, char *argv[])
{
    Graph* graphino = createTestGraph();
    BellmanFord(graphino, 0);

    printf("\n");

// Beginning of parallel region

    printf("Printing test message from OpenMP...\n");
#pragma omp parallel
    {

        printf("Hello World... from thread = %d\n",
               omp_get_thread_num());
    }
    // Ending of parallel region

}

void BellmanFord(Graph *graph, int src)
{
    int V = graph->V;
    int E = graph->E;
    int dist[V];

    // this definitely can be parallelized
    for (int i = 0; i < V; i++)
        dist[i] = INT_MAX;
    dist[src] = 0;

    // compute distance array
    for (int i = 1; i < V; i++)
    {
        for (int j = 0; j < V; j++)
        {
            int u = j;
            Node *hd = graph->nodes[j]->next;
            while (hd != NULL)
            {
                int v = hd->id;
                int weight = hd->cost;

                if (dist[u] != INT_MAX && (dist[u] + weight) < dist[v])
                    dist[v] = dist[u] + weight;

                hd = hd->next;
            }
        }
    }

    // if graph still has a shorter path, then there's a negative cycle
    for (int j = 0; j < V; j++)
    {
        int u = j;
        Node *hd = graph->nodes[0]->next;
        while (hd != NULL)
        {
            int v = hd->id;
            int weight = hd->cost;

            if (dist[u] != INT_MAX && dist[u] + weight < dist[v])
            {
                printf("Graph contains negative weight cycle\n");
                return; // If negative cycle is detected, simply
                // return
            }

            hd = hd->next;
        }
    }

    printArr(dist, V);

    return;
}

void printArr(int dist[], int n)
{
    printf("Vertex  |  Distance from Source\n");
    for (int i = 0; i < n; ++i)
        printf("%d \t\t %d\n", i, dist[i]);
}

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
    int E = 8;
    Graph *graph = initGraph(V, E);

    for (int i = 0; i < V; i++)
    {
        graph->nodes[i] = malloc(sizeof(Node));
        graph->nodes[i]->id = i;
    }

    addEdge(graph, 0, 1, -1);
    addEdge(graph, 0, 2, 4);
    addEdge(graph, 1, 2, 3);
    addEdge(graph, 1, 3, 2);
    addEdge(graph, 1, 4, 2);
    addEdge(graph, 3, 2, 5);
    addEdge(graph, 3, 1, 1);
    addEdge(graph, 4, 3, -3);

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