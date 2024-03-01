// OpenMP program to print Hello World
// using C language

// OpenMP header
#include <omp.h>
#include "../include/graph.h"
#include "../include/bellmanford.h"


int main(int argc, char *argv[])
{
    Graph* graphino = createTestGraph();
    // printGraph(graphino);
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