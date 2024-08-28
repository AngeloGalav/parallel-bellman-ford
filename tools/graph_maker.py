'''
This script creates a random graph using the Erdos-Reny method.
'''
import argparse
import os
import random
import networkx as nx

parser = argparse.ArgumentParser(description="Generate and save a weighted graph.")
parser.add_argument("-v", "--nodes", type=int, help="Number of nodes in the graph", required=False)
parser.add_argument("-e", "--edges", type=int, help="Number of edges in the graph", required=False)
parser.set_defaults(negative=False)
parser.add_argument('-neg', "--negative", default=False, action=argparse.BooleanOptionalAction, help="Graph has negative edges")

args = parser.parse_args()

# weight range
range_w = (0, 300)
if args.negative : range_w = (-300, 300)

if args.nodes is None or args.edges is None:
    V = 10
    E = 25
else:
    V = args.nodes
    E = args.edges

def create_graph_file(V, E) :
    G = nx.gnm_random_graph(V, E, seed=None)

    # define random weights
    for (u,v,w) in G.edges(data=True):
        w['weight'] = random.randint(*range_w)

    filename = 'graph_0.txt'
    base_filename = 'graph.txt'
    counter = 0

    while os.path.exists(filename):
        filename = f"{base_filename.rsplit('.', 1)[0]}_{counter}.txt"
        counter += 1

    with open(filename, 'w') as file:
        file.write(f"{V} {E}\n")
        for u, v, data in G.edges(data=True):
            file.write(f"{u} {v} {data['weight']}\n")

create_graph_file(V, E)
