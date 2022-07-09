#!/bin/bash

#SBATCH --job-name=MISR2roughness

#SBATCH --account=cpu-s5-misr_roughness-0
#SBATCH --partition=cpu-core-0			 			# Submit job to the cpu partition
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=32 
#SBATCH --hint=compute_bound
#SBATCH --mem-per-cpu=3500M
#SBATCH --mail-type=ALL             		 		# Send mail on all state changes
#SBATCH --mail-user=emosadegh@nevada.unr.edu
#SBATCH --output=log_misr2rough_parallel_test21april2016_reversed.txt   		 	# The output file name
#SBATCH --error=log_misr2rough_parallel_test21april2016_reversed.txt		 		# The error file nam
#SBATCH --time=14-00:00				 				# Set max. run time; format: D-HH:MM
date
module list 
echo “MISR job started here...”
python  ./run_MISR2roughness_parallel_HPC.py
echo “MISR job ended here...”
date
