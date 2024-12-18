#!/bin/bash

# Directory where the SLURM jobs output files are stored
output_dir="./MPItests"

# Output directory for the statistics
stats_output_dir="./MPIstats"
mkdir -p $stats_output_dir

# Define arrays for the values of ntasks, nodes, and dim
ntasks_values=(1 2 4 8 16 32 64 128 256)
nodes_values=(1 2 4 6 8)
dim_values=(1024 2048 4096 8192)

#ntasks_values=(32 64 128 256)
#nodes_values=(6 8)
#dim_values=(1024 2048)

# Number of repetitions for each configuration
num_repeats=5

# Function to calculate mean and standard deviation
calculate_stats() {
  values=("$@")
  n=${#values[@]}
  mean=$(echo "${values[@]}" | awk '{sum=0; for(i=1;i<=NF;i++) sum+=$i; print sum/NF}')
  stddev=$(echo "${values[@]}" | awk -v mean=$mean '{sum=0; for(i=1;i<=NF;i++) sum+=($i-mean)*($i-mean); print sqrt(sum/NF)}')
  echo $mean $stddev
}

# Loop through all combinations of ntasks, nodes, and dim
for nodes in "${nodes_values[@]}"; do
  for ntasks in "${ntasks_values[@]}"; do
    for dim in "${dim_values[@]}"; do
      job_name_prefix="nodes${nodes}_ntasks${ntasks}_dim${dim}"
      output_times=()

      # Collect output values from each run
      for ((repeat=1; repeat<=num_repeats; repeat++)); do
        output_file="${output_dir}/${job_name_prefix}_repeat${repeat}_output.txt"
        if [ -f "$output_file" ]; then
          # Extract the time value from the first line of the output file
          time_value=$(grep -oP 'Time with .* processes: \K[\d.]+(?= seconds)' $output_file)
          if [ ! -z "$time_value" ]; then
            output_times+=($time_value)
          fi
        fi
      done

      # Calculate mean and standard deviation
      if [ ${#output_times[@]} -eq $num_repeats ]; then
        stats=$(calculate_stats "${output_times[@]}")
        mean=$(echo $stats | cut -d ' ' -f 1)
        stddev=$(echo $stats | cut -d ' ' -f 2)
        echo "ntasks=$ntasks nodes=$nodes dim=$dim mean=$mean stddev=$stddev"
        echo "ntasks=$ntasks nodes=$nodes dim=$dim mean=$mean stddev=$stddev" >> "${stats_output_dir}/stats.txt"
      else
        echo "Missing output for configuration: ntasks=$ntasks nodes=$nodes dim=$dim"
      fi
    done
  done
done