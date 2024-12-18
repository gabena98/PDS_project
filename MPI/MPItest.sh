#!/bin/bash

# Define arrays for the values of ntasks, nodes, and dim
ntasks_values=(1)
nodes_values=(1)
dim_values=(4096)

#ntasks_values=(32 64 128 256)
#nodes_values=(6 8)
#dim_values=(1024 2048)

# Path to your MPI executable
mpi_executable="./MPIWavefront"

# Directory to store generated SLURM scripts and output files
output_dir="./MPItests"
mkdir -p $output_dir

# Number of repetitions for each configuration
num_repeats=5

# Loop through all combinations of ntasks, nodes, and dim
for nodes in "${nodes_values[@]}"; do
  for ntasks in "${ntasks_values[@]}"; do
    for dim in "${dim_values[@]}"; do
      for ((repeat=1; repeat<=num_repeats; repeat++)); do
        job_name="nodes${nodes}_ntasks${ntasks}_dim${dim}_repeat${repeat}"
        output_file="${output_dir}/${job_name}_output.txt"
        error_file="${output_dir}/${job_name}_error.txt"
        slurm_script="${output_dir}/${job_name}.slurm"

        # Create SLURM script
        cat <<EOL > $slurm_script
#!/bin/bash
#SBATCH -p normal       ### Using the 'normal' partition
#SBATCH -N $nodes       ### --nodes, Number of nodes
#SBATCH --ntasks=$ntasks   ### -n, Specify the number of tasks to run
#SBATCH --job-name=wavefront
#SBATCH -o $output_file
#SBATCH -e $error_file
#SBATCH -t 02:00:00     ### Maximum runtime (adjust if needed)

srun --mpi=pmix $mpi_executable $dim
EOL

        # Submit the SLURM script
        sbatch $slurm_script

      done
    done
  done
done