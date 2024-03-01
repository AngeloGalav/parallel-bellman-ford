#include "../include/bellmanford.h"

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