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

def plot_graph(data, label, output_folder, color):
    for graph_file, values in data.items():
        if values:  # Only plot if there is valid data (ignore files with only timeouts)
            # Sort the data by the number of threads
            values.sort(key=lambda x: x[0])
            threads, times = zip(*values)
            
            # Plotting
            plt.figure()
            plt.plot(threads, times, marker='o', color=color)
            plt.xlabel('Number of Threads')
            plt.ylabel('Time (s)')
            plt.title(f'OMP: Performance of Graph with {graph_configs[graph_file][0]} nodes')
            plt.grid(True)
            
            # Ensure directory exists
            os.makedirs(output_folder, exist_ok=True)
            # Save the plot
            plot_filename = os.path.join(output_folder, f'{label}_{graph_file.replace(".txt", "").replace("graphs/", "")}.png')
            plt.savefig(plot_filename)
            plt.close()
        else:
            print(f"No valid data to plot for {graph_file} in {label}.")

# plot graph for cuda
def plot_graph_cuda(graph_cfg, time_data, save_folder):
    x_nodes = []  # List to store number of nodes
    y_times = []  # List to store execution times

    for filename, (num_nodes, _) in graph_cfg.items():
        if filename in time_data:
            exec_time = time_data[filename]  # Get the execution time from dict2
            x_nodes.append(num_nodes)    # Append the number of nodes to the x-axis list
            y_times.append(exec_time)

    # Create the scatter plot
    plt.figure(figsize=(10, 6))
    plt.plot([str(x) for x in x_nodes], y_times, marker='o',  linestyle='-', color="limegreen")
    plt.title('CUDA: Time vs Nodes for Parallel Execution')
    plt.xlabel('Nodes')
    plt.ylabel('Time (seconds)')
    plt.grid(True)

    # Ensure the save directory exists
    os.makedirs(save_folder, exist_ok=True)
    
    # Full path for saving the file
    full_path = os.path.join(save_folder, "CUDA.png")

    plt.savefig(full_path)
    print(f"Graph saved to {full_path}")

graph_configs = get_graph_config("graphs")

# Directories and files
output_folder = 'results/plots'
omp_file = 'results/omp.csv'
cuda_file = 'results/cuda.csv'

# Read data from both CSV files
omp_data = read_data(omp_file, "OMP")
cuda_data = read_data(cuda_file)

# Plot and save OMP data
plot_graph(omp_data, 'OMP', output_folder, "blue")

# Plot and save CUDA data
plot_graph_cuda(graph_configs, cuda_data, output_folder)
