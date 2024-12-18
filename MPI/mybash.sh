#!/bin/bash
#SBATCH -p normal       ### Using the 'normal' partition
#SBATCH -N 4          ### --nodes, Number of nodes
#SBATCH --ntasks=64  ### -n, Specify the number of tasks to run
#SBATCH --job-name=MPIwavefront
#SBATCH -o output_%j.log
#SBATCH -e error_%j.err
#SBATCH -t 02:00:00          ### Maximum runtime (adjust if needed)

srun --mpi=pmix MPIWavefront 2048


