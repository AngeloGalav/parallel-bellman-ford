import sys

def bellman_ford(vertices, edges, start):
    # Initialize distance to all vertices as infinite and distance to start as 0
    distance = [float('inf')] * vertices
    distance[start] = 0

    # Relax edges |V| - 1 times
    for _ in range(vertices - 1):
        for u, v, w in edges:
            if distance[u] != float('inf') and distance[u] + w < distance[v]:
                distance[v] = distance[u] + w

    # Check for negative-weight cycles
    for u, v, w in edges:
        if distance[u] != float('inf') and distance[u] + w < distance[v]:
            print("Graph contains negative weight cycle")
            return

    # Print all distances
    print("Vertex Distance from Source")
    for i in range(vertices):
        if distance[i] == float('inf'):
            print(f"{i}\tINF")
        else:
            print(f"{i}\t{distance[i]}")

def read_graph_from_file(filename):
    with open(filename, 'r') as file:
        first_line = file.readline().strip()
        vertices, _ = map(int, first_line.split())
        edges = []

        for line in file:
            u, v, w = map(int, line.strip().split())
            edges.append((u, v, w))
            edges.append((v, u, w))

    return vertices, edges

if __name__ == "__main__":
    filename = "graph_1.txt"
    start_vertex = 0  # Change this to the desired start vertex

    vertices, edges = read_graph_from_file(filename)
    bellman_ford(vertices, edges, start_vertex)
