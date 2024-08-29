#!/bin/bash
# Usage: ./benchmark.sh [-d] [-t TIMEOUT]
# Runs all program on all graphs with all thread configurations

# Function to print usage
usage() {
  echo "Usage: $0 [-d] [-t TIMEOUT]"
  echo "  -d          Saves output of programs to files"
  echo "  -t TIMEOUT  Set custom timeout in minutes (default is 3 minutes)"
}

# Parse command-line options
debug=0
timeout=3m

# Parse command-line options
while getopts ":dt:" opt; do
  case ${opt} in
    d )
      debug=1
      ;;
    t )
      timeout=${OPTARG}
      echo "timeout set to ${timeout}."
      ;;
    \? )
      usage
      exit 1
      ;;
    : )
      echo "Option -$OPTARG requires an argument."
      usage
      exit 1
      ;;
  esac
done

# check for folder presence
if [ ! -d "results" ]; then
    mkdir results
fi

if [ $debug -eq 1 ]; then
    mkdir results/sanity
fi

threads=(1 2 3 4 5 6 7 8 9 10 16)

graph_files=$(ls graphs/)

for n in "${threads[@]}"; do
  for graph_file in $graph_files; do
    if [ $debug -eq 1 ]; then
      output_path="results/sanity/omp_${graph_file}.txt"
      echo "Running omp-bf on $graph_file with $n threads, outputting to $output_path"
      echo "omp-bf with $n threads" >> "$output_path"
      timeout ${timeout} ./omp-bf "$n" "graphs/$graph_file" -d >> "$output_path" 2>&1
    else
      echo "Running omp-bf on $graph_file with $n threads"
      timeout ${timeout} ./omp-bf "$n" "graphs/$graph_file"
    fi

    if [ $? -eq 124 ]; then
      echo "Execution timed out for $graph_file with $n threads"
      echo "graphs/${graph_file},${n},TO" >> results/omp.csv
    fi
  done
done

# for graph_file in $graph_files; do
#   if [ $debug -eq 1 ]; then
#     output_path="results/sanity/cuda_${graph_file}.txt"
#     echo "Running cuda-bf on $graph_file, outputting to $output_path"
#     timeout ${timeout} ./cuda-bf "graphs/$graph_file" -d > "$output_path" 2>&1
#   else
#     echo "Running cuda-bf on $graph_file"
#     timeout ${timeout} ./cuda-bf "graphs/$graph_file"
#   fi
# done