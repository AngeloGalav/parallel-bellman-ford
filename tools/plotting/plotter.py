import matplotlib.pyplot as plt
import os
from collections import defaultdict

# Function to read data from a CSV file and return a dictionary
def read_data(filename):
    data = defaultdict(list)
    try:
        with open(filename, 'r') as file:
            for line in file:
                if line.strip():  # Check if line is not empty
                    graph_file, threads, time = line.strip().split(',')
                    threads = int(threads)
                    if time != "TO":  # Handle the "TO" timeout case
                        time = float(time)
                        data[graph_file].append((threads, time))
    except FileNotFoundError:
        print(f"Error: The file '{filename}' does not exist.")
    except Exception as e:
        print(f"An error occurred: {e}")
    return data

# Function to plot the data for each graph file
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
            plt.title(f'{label} Performance - {graph_file}')
            plt.grid(True)
            
            # Ensure directory exists
            os.makedirs(output_folder, exist_ok=True)
            # Save the plot
            plot_filename = os.path.join(output_folder, f'{label}_{graph_file.replace(".txt", "").replace("graphs/", "")}.png')
            plt.savefig(plot_filename)
            plt.close()
        else:
            print(f"No valid data to plot for {graph_file} in {label}.")

# Directories and files
output_folder = 'results/plots'
omp_file = 'results/omp.csv'
cuda_file = 'results/cuda.csv'

# Read data from both CSV files
omp_data = read_data(omp_file)
# cuda_data = read_data(cuda_file)

# Plot and save OMP data
plot_graph(omp_data, 'OMP', output_folder, "blue")

# Plot and save CUDA data
# plot_graph(cuda_data, 'CUDA', output_folder, "green")
