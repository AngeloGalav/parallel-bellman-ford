#include <cuda_runtime.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BLOCK_SIZE 512  // threads per block
#define THREAD_ID (blockIdx.x * blockDim.x + threadIdx.x);
#define N_OF_BLOCKS(N) ((N + BLOCK_SIZE - 1) / BLOCK_SIZE)  // total n of blocks

//  Different implementation of cuda_gettime depending on
//  the OS used by the user, as Windows is not POSIX compliant
#ifdef _WIN32
#include <windows.h>
#define MS_PER_SEC 1000ULL  // MS = milliseconds
#define US_PER_MS 1000ULL   // US = microseconds
#define HNS_PER_US 10ULL    // HNS = hundred-nanoseconds (e.g., 1 hns = 100 ns)
#define NS_PER_US 1000ULL

#define HNS_PER_SEC (MS_PER_SEC * US_PER_MS * HNS_PER_US)
#define NS_PER_HNS (100ULL)  // NS = nanoseconds
#define NS_PER_SEC (MS_PER_SEC * US_PER_MS * NS_PER_US)

double cuda_gettime(void) {
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
    ts.tv_nsec = (long)(((ticks.QuadPart % ticksPerSec.QuadPart) * NS_PER_SEC) /
                        ticksPerSec.QuadPart);
    return ts.tv_sec + (double)ts.tv_nsec / 1e9;
}
#elif __unix__
double cuda_gettime(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + (double)ts.tv_nsec / 1e9;
}
#endif

typedef struct edge {
    int u;
    int v;
    int cost;
} Edge;

typedef struct graph {
    int V, E;
    Edge *edges;
} Graph;

int *BellmanFord(Graph *graph, int src, double *time);
void printArr(int dist[], int n);
Graph *initGraph(int V, int E);
void addEdge(Graph *graph, int src, int dest, int cost, int bidirectional);
void printInfoToFile(char *graph_file, double total_time);
Graph *createGraphFromFile(char *filename, int bidirectional);

// single threaded functions in cuda
__global__ void initDistArray(int *dist, int V, int src);
__global__ void relaxationStep(int *dist, int E, Edge *edges);
__global__ void checkNegative(int *dist, int E, Edge *edges, int *neg_check);
// single threaded functions in cuda
__global__ void oneThreadInitDistArray(int *dist, int V, int src);
__global__ void oneThreadRelaxationStep(int *dist, int E, Edge *edges);
__global__ void oneThreadCheckNegative(int *dist, int E, Edge *edges, int *neg_check);

int mode; // 1 is parallel, 0 is serial

int main(int argc, char *argv[]) {
    char *graph_file = argv[1];
    char *exec_mode = argv[2];
    char *debug_flag = argv[3];

    if (exec_mode != NULL) {
        mode = atoi(exec_mode);
    } else mode = 1;

    if (graph_file == NULL) {
        printf("ERROR: No graph file inputted.\n");
        return -1;
    }

    // building the graph
    const char *filename = graph_file;
    char graph_filename[100];
    strcpy(graph_filename, filename);

    Graph *graph = createGraphFromFile(graph_filename, 1);
    if (graph == NULL) {
        printf("Error creating graph\n");
        return -1;
    };

    // distance array on GPU
    int *dist_gpu;
    // time "without" memory overhead
    double *total_time = (double *)malloc(sizeof(double));
    double time_start, time_end;

    time_start = cuda_gettime();
    dist_gpu = BellmanFord(graph, 0, total_time);
    cudaDeviceSynchronize();
    time_end = cuda_gettime();

    // time with memory overhead
    double total_time_ov = time_end - time_start;
    if (dist_gpu == NULL) return -1;

    // create cpu copy of distance  array
    int *dist_cpu = (int *)malloc(sizeof(int) * graph->V);
    cudaMemcpy(dist_cpu, dist_gpu, sizeof(int) * graph->V,
               cudaMemcpyDeviceToHost);

    // printing the distance array (i.e. the result)
    if (debug_flag != NULL) {
        printArr(dist_cpu, graph->V);
    }
    cudaFree(dist_gpu);
    free(dist_cpu);

    printf("Total execution time: %f seconds\n", *total_time);
    printf("With memory transfer overhead: %f seconds\n", total_time_ov);
    printInfoToFile(graph_file, *total_time);
    printf("\n");

    return 0;
}

int *BellmanFord(Graph *graph, int src, double *total_time) {
    int V = graph->V;
    int E = graph->E;

    // allocate gpu memory
    Edge *edges_gpu;
    cudaMalloc(&edges_gpu, (E * sizeof(Edge)));
    cudaMemcpy(edges_gpu, graph->edges, E * sizeof(Edge),
               cudaMemcpyHostToDevice);

    int *dist;
    cudaMalloc(&dist, sizeof(int) * V);

    int *neg_check_gpu;
    int neg_check;
    cudaMalloc(&neg_check_gpu, sizeof(int));

    double time_start, time_end;

    // introduced time inside bellman-ford function for more precise
    // execution timing
    time_start = cuda_gettime();

    // parallel code
    if (mode) {
        initDistArray<<<N_OF_BLOCKS(V), BLOCK_SIZE>>>(dist, V, src);
        cudaDeviceSynchronize();

        // relaxation step must be done V times
        for (int i = 1; i < V; i++) {
            relaxationStep<<<N_OF_BLOCKS(E), BLOCK_SIZE>>>(dist, E, edges_gpu);
        }
        cudaDeviceSynchronize();

        checkNegative<<<N_OF_BLOCKS(E), BLOCK_SIZE>>>(dist, E, edges_gpu, neg_check_gpu);
        cudaDeviceSynchronize();
    }
    else // serial mode
    {
        oneThreadInitDistArray<<<1, 1>>>(dist, V, src);
        cudaDeviceSynchronize();

        for (int i = 1; i < V; i++) {
            oneThreadRelaxationStep<<<1, 1>>>(dist, E, edges_gpu);
        }
        cudaDeviceSynchronize();

        oneThreadCheckNegative<<<1, 1>>>(dist, E, edges_gpu, neg_check_gpu);
        cudaDeviceSynchronize();
    }
    cudaMemcpy(&neg_check, neg_check_gpu, sizeof(int), cudaMemcpyDeviceToHost);
    time_end = cuda_gettime();

    *total_time = time_end - time_start;

    cudaFree(edges_gpu);
    cudaFree(neg_check_gpu);
    if (neg_check) return NULL;
    return dist;
}

// parallel functions
__global__ void initDistArray(int *dist, int V, int src) {
    int i = THREAD_ID;
    if (i < V) {
        dist[i] = INT_MAX;
    }

    if (i == src) dist[i] = 0;
}

__global__ void checkNegative(int *dist, int E, Edge *edges, int *neg_check) {
    int i = THREAD_ID;
    if (i < E) {
        int u = edges[i].u;
        int v = edges[i].v;
        int weight = edges[i].cost;

        if (dist[u] != INT_MAX && dist[u] + weight < dist[v]) {
            *neg_check = 1;
        }
    }
}

__global__ void relaxationStep(int *dist, int E, Edge *edges) {
    int i = THREAD_ID;
    if (i < E) {
        int u = edges[i].u;
        int v = edges[i].v;
        int weight = edges[i].cost;

        if (dist[u] != INT_MAX && (dist[u] + weight) < dist[v]) {
            __syncthreads();
            dist[v] = dist[u] + weight;
        }
    }
}

// single thread functions
__global__ void oneThreadInitDistArray(int *dist, int V, int src) {
    for (int i = 0; i < V; i++) {
        dist[i] = INT_MAX;
    }

    dist[src] = 0;
}

__global__ void oneThreadRelaxationStep(int *dist, int E, Edge *edges) {
    for (int i = 0; i < E; i++){
        int u = edges[i].u;
        int v = edges[i].v;
        int weight = edges[i].cost;

        if (dist[u] != INT_MAX && (dist[u] + weight) < dist[v]) {
            dist[v] = dist[u] + weight;
        }
    }
}

__global__ void oneThreadCheckNegative(int *dist, int E, Edge *edges, int *neg_check) {
    for (int i = 0; i < E; i++){
        int u = edges[i].u;
        int v = edges[i].v;
        int weight = edges[i].cost;

        if (dist[u] != INT_MAX && dist[u] + weight < dist[v]) {
            *neg_check = 1;
        }
    }
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

void printInfoToFile(char *graph_file, double total_time) {
    // Define the file path
    char file_path[256];
    if (mode) snprintf(file_path, sizeof(file_path), "results/cuda.csv");
    else snprintf(file_path, sizeof(file_path), "results/cuda_serial.csv");

    FILE *file = fopen(file_path, "a");
    if (!file) {
        fprintf(stderr,
                "Failed to open file: %s, creating file on WD instead...\n",
                file_path);

        if (mode) snprintf(file_path, sizeof(file_path), "cuda.csv");
        else snprintf(file_path, sizeof(file_path), "results/cuda_serial.csv");

        file = fopen(file_path, "a");
        if (!file) {
            fprintf(stderr, "Failed to open file again, aborting.");
            return;
        }
        printf("File created successfully!\n");
    }

    fprintf(file, "%s,%.6f\n", graph_file, total_time);
    fclose(file);
}

void printArr(int dist[], int n) {
    printf("Vertex  |  Distance from Source\n");
    for (int i = 0; i < n; ++i) printf("%d \t\t %d\n", i, dist[i]);
}
