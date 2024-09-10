#include <limits.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

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
    char *n_threads_s = argv[1];
    char *graph_file = argv[2];
    char *debug_flag = argv[3];

    // doing a background check on these guys
    if (n_threads_s == NULL) {
        printf("ERROR: No threads inputted.\n");
        return -1;
    }

    if (graph_file == NULL) {
        printf("ERROR: No graph file inputted.\n");
        return -1;
    }

    // setting the number of threads
    int n_threads = atoi(n_threads_s);
    omp_set_num_threads(n_threads);

    // generating the graph
    Graph *graph = createGraphFromFile(graph_file, 1);
    if (graph == NULL) {
        printf("Error creating graph\n");
        return -1;
    };

    double time_start, time_end;

    time_start = omp_get_wtime();
    int *dist_result;
    dist_result = BellmanFord(graph, 0);
    time_end = omp_get_wtime();

    double total_time = time_end - time_start;

    if (debug_flag != NULL) {
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

            if (dist[u] != INT_MAX && (dist[u] + weight) < dist[v]) {
#pragma omp atomic write
                dist[v] = dist[u] + weight;
            }
        }
    }

    int neg_check = 0;

// if graph still has a shorter path, then there's a negative cycle
#pragma omp parallel for private(u, v, weight, i) schedule(static)
    for (i = 0; i < V; i++) {
        u = graph->edges[j].u;
        v = graph->edges[j].v;
        weight = graph->edges[j].cost;

        // If negative cycle is detected, simply return
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