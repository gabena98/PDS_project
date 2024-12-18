#!/bin/bash

# First set of input data
dim=(8192)

# Second set of input data
workers=(1 )

# Output directory for the statistics
stats_output_dir="./strong_parfor_stats_internal"
mkdir -p $stats_output_dir

# Function to calculate mean and standard deviation
calculate_stats() {
  values=("$@")
  n=${#values[@]}
  mean=$(echo "${values[@]}" | awk '{sum=0; for(i=1;i<=NF;i++) sum+=$i; print sum/NF}')
  stddev=$(echo "${values[@]}" | awk -v mean=$mean '{sum=0; for(i=1;i<=NF;i++) sum+=($i-mean)*($i-mean); print sqrt(sum/NF)}')
  echo $mean $stddev
}

# Loop through each value in the first input data set
for first_input in "${dim[@]}"; do
  # Loop through each value in the second input data set
  for second_input in "${workers[@]}"; do
    echo "Executing ParForFF with inputs: N = $first_input, workers = $second_input"
    
    elapsed_times=()
    
    # Run the program 5 times for the current configuration
    for i in {1..5}; do
      output=$(srun -N 1 ParForFF $first_input $second_input)
      elapsed_time=$(echo "$output" | grep 'elapsed time' | awk '{print $5}' | tr -d 's')
      elapsed_times+=("$elapsed_time")
    done
    
    # Compute mean and standard deviation
    read mean stddev <<< $(calculate_stats "${elapsed_times[@]}")
    
    # Print the mean and standard deviation
    echo "Mean elapsed time: $mean s, Standard Deviation: $stddev s for N = $first_input, workers = $second_input"

    # Write the results to a file in the results directory
    result_file="$stats_output_dir/results_N${first_input}_workers${second_input}.txt"
    echo "N = $first_input, workers = $second_input" > "$result_file"
    echo "Mean elapsed time: $mean s" >> "$result_file"
    echo "Standard Deviation: $stddev s" >> "$result_file"
  done
done