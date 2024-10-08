#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <cuda_runtime.h>

#define BLOCK_SIZE 512 // threads per block
#define THREAD_ID (blockIdx.x * blockDim.x + threadIdx.x);
#define N_OF_BLOCKS(N) ((N + BLOCK_SIZE - 1) / BLOCK_SIZE) // total n of blocks

#define cudaSafeCall( err ) __cudaSafeCall( err, __FILE__, __LINE__ )
#define cudaCheckError()    __cudaCheckError( __FILE__, __LINE__ )

//  Different implementation of cuda_gettime depending on
//  the OS used by the user, as Windows is not POSIX compliant
#ifdef _WIN32
#include <windows.h>
#define MS_PER_SEC      1000ULL     // MS = milliseconds
#define US_PER_MS       1000ULL     // US = microseconds
#define HNS_PER_US      10ULL       // HNS = hundred-nanoseconds (e.g., 1 hns = 100 ns)
#define NS_PER_US       1000ULL

#define HNS_PER_SEC     (MS_PER_SEC * US_PER_MS * HNS_PER_US)
#define NS_PER_HNS      (100ULL)    // NS = nanoseconds
#define NS_PER_SEC      (MS_PER_SEC * US_PER_MS * NS_PER_US)

double cuda_gettime(void)
{
    struct timespec ts;
    static LARGE_INTEGER ticksPerSec;
    LARGE_INTEGER ticks;

    if (!ticksPerSec.QuadPart) {
        QueryPerformanceFrequency(&ticksPerSec);
        if (!ticksPerSec.QuadPart) {
            errno = ENOTSUP;
            return -1;
        }
    }

    QueryPerformanceCounter(&ticks);

    ts.tv_sec = (long)(ticks.QuadPart / ticksPerSec.QuadPart);
    ts.tv_nsec = (long)(((ticks.QuadPart % ticksPerSec.QuadPart) * NS_PER_SEC) / ticksPerSec.QuadPart);
    return ts.tv_sec + (double)ts.tv_nsec / 1e9;
}
#elif __unix__
double cuda_gettime( void )
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts );
    return ts.tv_sec + (double)ts.tv_nsec / 1e9;
}
#endif

typedef struct node {
    int id;
    int cost;
    struct node *next;
} Node;

typedef struct graph {
    int V, E;
    Node **nodes;
} Graph;

void copyLinkedList(Node** d_nodes, Node* h_nodeList, int index) {
    Node* d_current = d_nodes[index];
    Node* hd = h_nodeList;
    while (hd != NULL) {
        // Allocate memory for each node on the device
        Node* d_newNode;
        cudaMalloc((void**)&d_newNode, sizeof(Node));
        cudaMemcpy(d_newNode, hd, sizeof(Node), cudaMemcpyHostToDevice);

        // Link the nodes
        if (d_current == NULL) {
            d_nodes[index] = d_newNode;
        } else {
            Node* d_lastNode = d_current;
            while (d_lastNode->next != NULL) {
                d_lastNode = d_lastNode->next;
            }
            d_lastNode->next = d_newNode;
        }

        hd = hd->next;
    }
}

void freeGraphMemory(Graph* d_graph, const Graph& h_graph) {
    // Free each linked list
    for (int i = 0; i < h_graph.V; i++) {
        Node* node = d_graph->nodes[i];
        while (node != NULL) {
            Node* next = node->next;
            cudaFree(node);
            node = next;
        }
    }
    // Free nodes array
    cudaFree(d_graph->nodes);
}

void copyGraphToDevice(const Graph& h_graph, Graph* d_graph) {
    cudaMemcpy(d_graph, &h_graph, sizeof(Graph), cudaMemcpyHostToDevice);

    // Copy each linked list
    for (int i = 0; i < h_graph.V; i++) {
        Node* h_nodeList = h_graph.nodes[i];
        Node* d_nodeList = NULL;
        copyLinkedList(d_graph->nodes, h_nodeList, i);
    }
}

__global__ void initDistArray(int* dist, int V){
    int i = THREAD_ID;
    if (i < V) {
        dist[i] = INT_MAX;
    }

    if (i == 0) dist[i] = 0;
}

__global__ void checkNegative(int* dist, int V, Graph* graph, int &neg_check){
    int i = THREAD_ID;
    if (i < V) {
        int u = i;
        Node* hd = graph->nodes[0]->next;

        while (hd != NULL) {
            int v = hd->id;
            int weight = hd->cost;

            // If negative cycle is detected, simply return
            if (dist[u] != INT_MAX && dist[u] + weight < dist[v]) {
                printf("Graph contains negative weight cycle\n");
                neg_check = 1;
                return;
            }

            hd = hd->next;
        }
    }
}

__global__ void relaxationStep(int* dist, int V, Graph* graph){
    int i = THREAD_ID;
    if (i < V) {
        int u = i;
        Node* hd = graph->nodes[u]->next;
        while (hd != NULL) {
            int v = hd->id;
            int weight = hd->cost;
            if (dist[u] != INT_MAX && (dist[u] + weight) < dist[v]) {
                int upd = dist[u];
                __syncthreads();
                dist[v] = upd + weight;
            }

            hd = hd->next;
        }
    }
}


int *BellmanFord(Graph *graph, int src);
void printArr(int dist[], int n);
Graph *initGraph(int V, int E);
void addEdge(Graph *graph, int src, int dest, int cost, int bidirectional);
void printInfoToFile(char *graph_file, double total_time, int threads);
Graph *createGraphFromFile(char *filename);
void printGraph(Graph *graph);
void printEdgesOfNode(Node *node);

int main(int argc, char *argv[]) {
    // char *n_threads_s = argv[1];
    // char *graph_file = argv[2];

    // // doing a background check on these guys
    // // if (n_threads_s == NULL) {
    // //     printf("ERROR: No threads inputted.\n");
    // //     return -1;
    // // }

    // if (graph_file == NULL) {
    //     printf("ERROR: No graph file inputted.\n");
    //     return -1;
    // }

    // // setting the number of threads
    // int n_threads = atoi(n_threads_s);

    // generating the graph
    const char* filename = "graphs/graph_5.txt";
    char graph_filename[100];
    strcpy(graph_filename, filename);

    Graph *graph = createGraphFromFile(graph_filename);
    if (graph == NULL) {
        printf("Error creating graph\n");
        return -1;
    };

    double time_start, time_end;

    time_start = cuda_gettime();
    int *dist_result;
    dist_result = BellmanFord(graph, 0);
    cudaDeviceSynchronize();
    time_end = cuda_gettime();

    double total_time = time_end - time_start;

    // printing the distance array (i.e. the result)
    // printArr(dist_result, graph->V);

    printf("\n");

    printf("Total execution time: %f seconds\n", total_time);
    // printInfoToFile(graph_file, total_time, n_threads);

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
    snprintf(file_path, sizeof(file_path), "results/cuda.csv");

    FILE *file = fopen(file_path, "a");
    if (!file) {
        fprintf(stderr,
                "Failed to open file: %s, creating file on WD instead...\n",
                file_path);
        snprintf(file_path, sizeof(file_path), "cuda.csv");
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

    Graph* graph_gpu;
    cudaMalloc(&graph_gpu, sizeof(Graph));
    cudaMalloc(&(graph_gpu->nodes), (V * sizeof(Node *)));
    copyGraphToDevice(*graph, graph_gpu);

    int *dist;
    cudaMalloc(&dist, sizeof(int) * V);

    initDistArray<<<N_OF_BLOCKS(V), BLOCK_SIZE>>>(dist, V);
    cudaDeviceSynchronize();

    relaxationStep<<<N_OF_BLOCKS(V), BLOCK_SIZE>>>(dist, V, graph_gpu);
    cudaDeviceSynchronize();

    int* neg_check;
    cudaMalloc(&neg_check, sizeof(int));
    checkNegative<<<N_OF_BLOCKS(V), BLOCK_SIZE>>>(dist, V, graph_gpu, *neg_check);
    cudaDeviceSynchronize();

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
        graph->nodes[i] = (Node*)malloc(sizeof(Node));
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
    hd->next = (Node*) malloc(sizeof(Node));
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