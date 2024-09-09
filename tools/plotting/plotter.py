import matplotlib.pyplot as plt
import os
from collections import defaultdict

def get_graph_config(folder_path):
    graph_configs = {}
    for filename in os.listdir(folder_path):
        file_path = os.path.join(folder_path, filename)
        if os.path.isfile(file_path):
            with open(file_path, 'r') as file:
                first_line = file.readline().strip()
                parts = first_line.split()
                num1 = int(parts[0])
                num2 = int(parts[1])
                graph_configs["graphs/"+filename] = (num1, num2)
    return graph_configs


# Function to read data from a CSV file and return a dictionary
def read_data(filename, name="CUDA"):
    data = defaultdict(list)
    try:
        with open(filename, 'r') as file:
            for line in file:
                if line.strip():  # Check if line is not empty
                    if (name != "CUDA") :
                        graph_file, threads, time = line.strip().split(',')
                        threads = int(threads)
                        if time != "TO":  # Handle the "TO" timeout case
                            time = float(time)
                            data[graph_file].append((threads, time))
                    else :
                        # TODO: add mode!
                        graph_file, time = line.strip().split(',')
                        if time != "TO":  # Handle the "TO" timeout case
                            time = float(time)
                            data[graph_file].append(time)
    except FileNotFoundError:
        print(f"Error: The file '{filename}' does not exist.")
    except Exception as e:
        print(f"An error occurred: {e}")
    return data

import math
def plot_graph(data, label, output_folder, color):
    # Number of graphs to plot
    num_graphs = len(data)
    
    num_cols = 3
    num_rows = math.ceil(num_graphs / num_cols)
    
    fig, axes = plt.subplots(num_rows, num_cols, figsize=(15, 5 * num_rows))  # Adjust the figure size as needed
    axes = axes.flatten()  # Flatten the 2D array of axes into 1D for easier indexing
    
    for i, (graph_file, values) in enumerate(data.items()):
        if values:  # Only plot if there is valid data (ignore files with only timeouts)
            # Sort the data by the number of threads
            values.sort(key=lambda x: x[0])
            threads, times = zip(*values)
            threads = list(threads)
            
            # Plotting on the corresponding subplot
            ax = axes[i]
            ax.plot(threads, times, marker='o', color=color)
            ax.set_xscale('log')
            ax.set_xticks(threads)
            ax.set_xticklabels([str(x) for x in threads])
            ax.tick_params(axis='x', which='minor', length=0) 
            ax.set_xlabel('Number of Threads')
            ax.set_ylabel('Time (s)')
            ax.set_title(f'OMP: {graph_file.replace(".txt", "").replace("graphs/", "")} ({graph_configs[graph_file][0]} nodes)')
            ax.grid(True)
        else:
            print(f"No valid data to plot for {graph_file} in {label}.")
            axes[i].axis('off')  # Turn off the subplot if there is no valid data
    
    # Hide any extra subplots if we have fewer graphs than subplots
    for i in range(num_graphs, num_rows * num_cols):
        axes[i].axis('off')
    
    fig.suptitle(f'OMP: Performance Comparison', fontsize=16)
    # Adjust layout to prevent overlapping
    plt.tight_layout()
    
    # Ensure the output folder exists
    os.makedirs(output_folder, exist_ok=True)
    
    # Save the figure with all subplots
    plot_filename = os.path.join(output_folder, f'{label}.png')
    plt.savefig(plot_filename)
    plt.close()

# plot graph for cuda
def plot_graph_cuda(graph_cfg, parallel_time_data, serial_time_data, save_folder):
    x_nodes = []  # List to store number of nodes
    y_parallel_times = []  # List to store parallel execution times
    y_serial_times = []  # List to store serial execution times
    y_serial_times_for_comparison = []  # List for serial times in the comparison plot

    for filename, (num_nodes, _) in sorted(graph_cfg.items()):
        if filename in parallel_time_data:
            parallel_exec_time = parallel_time_data[filename]  # Get the parallel execution time
            x_nodes.append(num_nodes)  # Append the number of nodes to the x-axis list
            y_parallel_times.append(parallel_exec_time)

            # Check for serial data and append accordingly
            if filename in serial_time_data:
                serial_exec_time = serial_time_data[filename]
                y_serial_times.append(serial_exec_time)
                y_serial_times_for_comparison.append(serial_exec_time)
            else:
                y_serial_times.append(None)  # No serial data, append None
                y_serial_times_for_comparison.append(None)  # No comparison data

    # Create the figure and subplots
    fig, axs = plt.subplots(1, 3, figsize=(18, 6))  # 1 row, 3 columns subplot

    # Plot for Parallel Execution
    axs[0].plot([str(x) for x in x_nodes], y_parallel_times, marker='o', linestyle='-', color="limegreen")
    axs[0].set_title('Parallel Execution Times')
    axs[0].set_xlabel('Nodes')
    axs[0].set_ylabel('Time (seconds)')
    axs[0].grid(True)

    # Plot for Serial Execution (only plots non-None values)
    axs[1].plot([str(x) for x, y in zip(x_nodes, y_serial_times) if y is not None], 
                [y for y in y_serial_times if y is not None], marker='o', linestyle='-', color="green")
    axs[1].set_title('Serial Execution Times')
    axs[1].set_xlabel('Nodes')
    axs[1].set_ylabel('Time (seconds)')
    axs[1].grid(True)

    # Comparison Plot
    axs[2].plot([str(x) for x in x_nodes], y_parallel_times, marker='o', linestyle='-', color="limegreen", label='Parallel')
    axs[2].plot([str(x) for x, y in zip(x_nodes, y_serial_times_for_comparison) if y is not None], 
                [y for y in y_serial_times_for_comparison if y is not None], marker='o', linestyle='--', color="green", label='Serial')
    axs[2].set_title('Comparison of Execution Times')
    axs[2].set_xlabel('Nodes')
    axs[2].set_ylabel('Time (seconds)')
    axs[2].grid(True)
    axs[2].legend()

    fig.suptitle(f'CUDA: Performance Comparison', fontsize=16)
    # Ensure the save directory exists
    os.makedirs(save_folder, exist_ok=True)
    
    # Full path for saving the file
    full_path = os.path.join(save_folder, "CUDA.png")

    plt.savefig(full_path)
    plt.close()

graph_configs = get_graph_config("graphs")

# Directories and files
output_folder = 'results/plots'
omp_file = 'results/omp.csv'
cuda_file = 'results/cuda.csv'
cuda_file_serial = 'results/cuda_serial.csv'


# Read data from both CSV files
omp_data = read_data(omp_file, "OMP")
cuda_data = read_data(cuda_file)
cuda_data_serial = read_data(cuda_file_serial)


# Plot and save OMP data
plot_graph(omp_data, 'OMP', output_folder, "royalblue")

# Plot and save CUDA data
plot_graph_cuda(graph_configs, cuda_data, cuda_data_serial, output_folder)
