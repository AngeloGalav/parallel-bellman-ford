#include <limits.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct edge {
    int u;
    int v;
    int cost;
} Edge;

typedef struct graph {
    int V, E;
    Edge *edges;
} Graph;

int *BellmanFord(Graph *graph, int src);
void printArr(int dist[], int n);
Graph *initGraph(int V, int E);
void printInfoToFile(char *graph_file, double total_time, int threads);
Graph *createGraphFromFile(char *filename, int bidirectional);

int main(int argc, char *argv[]) {

    // console args handling
    int bidirectional = 0, debug_flag = 0, n_threads = 1;
    char *graph_file = NULL;
    int i = 1;

    while (i < argc) {
        if (strcmp(argv[i], "-b") == 0) {
            bidirectional = 1;
        } else if (strcmp(argv[i], "-d") == 0) {
            debug_flag = 1;
        } else if (i == 1) {
            n_threads = atoi(argv[i]);
        } else if (i == 2) {
            graph_file = argv[i];
        }
        i++;
    }

    // setting the number of threads
    omp_set_num_threads(n_threads);

    // generating the graph
    Graph *graph = createGraphFromFile(graph_file, bidirectional);
    if (graph == NULL) {
        printf("Error creating graph\n");
        return -1;
    };

    double time_start, time_end;

    int *dist_result;
    time_start = omp_get_wtime();
    dist_result = BellmanFord(graph, 0);
    time_end = omp_get_wtime();

    double total_time = time_end - time_start;

    if (!dist_result) printf("Graph had negative edges\n");

    if (debug_flag && dist_result != NULL) {
        // printing the distance array (i.e. the result)
        printArr(dist_result, graph->V);
    }

    printf("Total execution time: %f seconds\n", total_time);
    printInfoToFile(graph_file, total_time, n_threads);
    printf("\n");

    // cleanup
    free(graph->edges);
    free(graph);

    return 0;
}

Graph *createGraphFromFile(char *filename, int bidirectional) {
    // read file and create graph
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file.\n");
        return NULL;
    }

    int V, E;
    fscanf(file, "%d %d", &V, &E);

    // graph memory allocation
    Graph *graph = (Graph *)malloc(sizeof(Graph));
    graph->V = V;
    graph->E = E;
    if (bidirectional) graph->E = 2 * E;
    graph->edges = (Edge *)malloc(graph->E * sizeof(Edge));

    // edge definition
    for (int i = 0; i < E; i++) {
        fscanf(file, "%d %d %d", &graph->edges[i].u, &graph->edges[i].v,
               &graph->edges[i].cost);
    }

    if (bidirectional) {
        for (int i = 0; i < E; i++) {
            graph->edges[i + E].u = graph->edges[i].v;
            graph->edges[i + E].v = graph->edges[i].u;
            graph->edges[i + E].cost = graph->edges[i].cost;
        }
    }

    fclose(file);
    return graph;
}

void printInfoToFile(char *graph_file, double total_time, int threads) {
    // Define the file path
    char file_path[256];
    snprintf(file_path, sizeof(file_path), "results/omp.csv");

    FILE *file = fopen(file_path, "a");
    if (!file) {
        fprintf(stderr,
                "Failed to open file: %s, creating file on WD instead...\n",
                file_path);
        snprintf(file_path, sizeof(file_path), "omp.csv");
        file = fopen(file_path, "a");
        if (!file) {
            fprintf(stderr, "Failed to open file again, aborting.");
            return;
        }
        printf("File created successfully!\n");
    }

    fprintf(file, "%s,%d,%.6f\n", graph_file, threads, total_time);
    fclose(file);
}

int *BellmanFord(Graph *graph, int src) {
    int V = graph->V;
    int E = graph->E;
    int *dist = (int *)malloc(sizeof(int) * V);
    if (dist == NULL) {
        perror("Failed to allocate memory");
        return NULL;
    }

    // declaring variables here in order for the scoping to work
    int i, u, v, weight;
    int j = 0;
#pragma omp parallel for schedule(static)
    for (i = 0; i < V; i++) {
        dist[i] = INT_MAX;
    }

    dist[src] = 0;

    // compute distance array
    for (i = 1; i < V; i++) {
#pragma omp parallel for private(u, v, weight, j) shared(dist) schedule(static)
        for (j = 0; j < E; j++) {
            u = graph->edges[j].u;
            v = graph->edges[j].v;
            weight = graph->edges[j].cost;

            if (dist[u] != INT_MAX && (dist[u] + weight < dist[v])) {
#pragma omp critical
                {
                    // having a second check here allows to execute the operation
                    // only if dist[v] is actually LESS, even if another thread
                    // already modified it.
                    if (dist[u] + weight < dist[v]) {
                        dist[v] = dist[u] + weight;
                    }
                }
            }
        }
    }

    int neg_check = 0;

// if graph still has a shorter path, then there's a negative cycle
#pragma omp parallel for private(u, v, weight, i) shared(neg_check) schedule(static)
    for (i = 0; i < E; i++) {
        u = graph->edges[i].u;
        v = graph->edges[i].v;
        weight = graph->edges[i].cost;

        // a single write operation is needed for this, as it is basically 
        // an "OR" operation (i.e. it is always zero unless a thread sets
        // it to 1).
        if (dist[u] != INT_MAX && dist[u] + weight < dist[v]) {
#pragma omp atomic write
            neg_check = 1;
        }
    }

    if (neg_check) return NULL;
    return dist;
}

void printArr(int dist[], int n) {
    printf("Vertex  |  Distance from Source\n");
    for (int i = 0; i < n; ++i) printf("%d \t\t %d\n", i, dist[i]);
}