#!/bin/bash
#SBATCH --job-name=MISR2roughness
##SBATCH --account=cpu-s6-test-0 
##SBATCH --partition=cpu-s6-test-0
#SBATCH --account=cpu-s5-misr_roughness-0
#SBATCH --partition=cpu-core-0			 			# Submit job to the cpu partition
#SBATCH --ntasks=32									# N independent processes (no communication)
#SBATCH --ntasks-per-node=32						# N processes to stay on the same node
#SBATCH --cpus-per-task=1
#SBATCH --mail-type=ALL             		 		# Send mail on all state changes
#SBATCH --mail-user=emosadegh@nevada.unr.edu
#SBATCH --output=log_misrToRoughness.txt   		 	# The output file name
#SBATCH --error=log_misrToRoughness.txt		 		# The error file nam
#SBATCH --time=14-00:00				 				# Set max. run time; format: D-HH:MM
date
module list 
echo “MISR job started here...”
python  
echo “MISR job ended here...”
date
