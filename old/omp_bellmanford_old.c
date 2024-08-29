#include <limits.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct node {
    int id;
    int cost;
    struct node *next;
} Node;

typedef struct graph {
    int V, E;
    Node **nodes;
} Graph;

int *BellmanFord(Graph *graph, int src);
void printArr(int dist[], int n);
Graph *initGraph(int V, int E);
void addEdge(Graph *graph, int src, int dest, int cost, int bidirectional);
void printInfoToFile(char *graph_file, double total_time, int threads);
Graph *createGraphFromFile(char *filename);
void printGraph(Graph *graph);
void printEdgesOfNode(Node *node);

int main(int argc, char *argv[]) {
    char *n_threads_s = argv[1];
    char *graph_file = argv[2];

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
    Graph *graph = createGraphFromFile(graph_file);
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

    // printing the distance array (i.e. the result)
    // if (dist_result == NULL) return -1;
    // printArr(dist_result, graph->V);

    // printf("\n");

    printf("Total execution time: %f seconds\n", total_time);
    printInfoToFile(graph_file, total_time, n_threads);

    return 0;
}

Graph *createGraphFromFile(char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file.\n");
        return NULL;
    }

    int V, E;
    fscanf(file, "%d %d", &V, &E);
    Graph *graph = initGraph(V, E);

    for (int i = 0; i < E; i++) {
        int u, v, weight;
        fscanf(file, "%d %d %d", &u, &v, &weight);
        addEdge(graph, u, v, weight, 1);
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
    int *dist = (int *)malloc(sizeof(int) * V);
    if (dist == NULL) {
        perror("Failed to allocate memory");
        return NULL;
    }

    // declaring variables here in order for the scoping to work
    int i, j, u, v, weight;
    Node *hd;

#pragma omp parallel for schedule(static)
    for (i = 0; i < V; i++) {
        dist[i] = INT_MAX;
    }

    dist[src] = 0;

    // compute distance array
    for (i = 1; i < V; i++) {
#pragma omp parallel for private(u, v, weight, j, hd) shared(dist) \
    schedule(static)
        for (j = 0; j < V; j++) {
            u = j;
            hd = graph->nodes[u]->next;
            while (hd != NULL) {
                v = hd->id;
                weight = hd->cost;

                if (dist[u] != INT_MAX && (dist[u] + weight) < dist[v]) {
#pragma omp atomic write
                    dist[v] = dist[u] + weight;
                }

                hd = hd->next;
            }
        }
    }

    int neg_check = 0;

// if graph still has a shorter path, then there's a negative cycle
#pragma omp parallel for private(u, v, weight, i, hd) schedule(static)
    for (i = 0; i < V; i++) {
        u = i;
        hd = graph->nodes[0]->next;

        while (hd != NULL) {
            int v = hd->id;
            int weight = hd->cost;

            // If negative cycle is detected, simply return
            if (dist[u] != INT_MAX && dist[u] + weight < dist[v]) {
                printf("Graph contains negative weight cycle\n");
#pragma omp atomic write
                neg_check = 1;
            }

            hd = hd->next;
        }
    }

    if (neg_check) return NULL;
    return dist;
}

void printArr(int dist[], int n) {
    printf("Vertex  |  Distance from Source\n");
    for (int i = 0; i < n; ++i) printf("%d \t\t %d\n", i, dist[i]);
}

//
// Graph handling functions
//
//

Graph *initGraph(int V, int E) {
    Graph *graph = (Graph *)malloc(sizeof(Graph));
    graph->V = V;
    graph->E = E;
    graph->nodes = (Node **)malloc(V * sizeof(Node *));

    for (int i = 0; i < V; i++) {
        graph->nodes[i] = (Node *)malloc(sizeof(Node));
        graph->nodes[i]->id = i;
        graph->nodes[i]->next = NULL;
    }

    return graph;
}

void addEdge(Graph *graph, int src, int dest, int cost, int bidirectional) {
    Node *hd = graph->nodes[src];
    while (hd->next != NULL) {
        hd = hd->next;
    }
    hd->next = (Node *)malloc(sizeof(Node));
    hd->next->id = dest;
    hd->next->cost = cost;
    hd->next->next = NULL;

    if (bidirectional) addEdge(graph, dest, src, cost, 0);
}

void printGraph(Graph *graph) {
    printf("Graph edges:\n");
    for (int i = 0; i < graph->V; ++i) {
        printf("info of %d: ", i);
        printEdgesOfNode(graph->nodes[i]);
    }
}

void printEdgesOfNode(Node *node) {
    Node *hd = node;
    printf("Node %d is attached to nodes", node->id);
    while (hd->next != NULL) {
        hd = hd->next;
        printf(" %d,", hd->id);
    }
    printf(" - END\n");
}