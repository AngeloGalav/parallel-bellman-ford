// OpenMP program to print Hello World
// using C language

// OpenMP header
#include <omp.h>
#include "../include/graph.h"

int main(int argc, char *argv[])
{
    // struct Node* nodesList[20];
    // int V = 5; // Number of vertices in the graph
    // int E = 10; // Number of edges in the graph
    // struct Graph* graph = initGraph(V, E);

    // // // Adding edges
    // addEdge(graph, 0, 0, 1, 1);
    // addEdge(graph, 1, 0, 4, 2);

    // addNode(nodesList[0]);

    // // Print the graph
    // printGraph(graph);


// Beginning of parallel region
#pragma omp parallel
    {

        printf("Hello World... from thread = %d\n",
               omp_get_thread_num());
    }
    // Ending of parallel region
}