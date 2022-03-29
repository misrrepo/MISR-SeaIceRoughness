#!/bin/bash
#SBATCH --job-name=atmModel
##SBATCH --account=cpu-s6-test-0 			
##SBATCH --partition=cpu-s6-test-0
#SBATCH --account=cpu-s5-misr_roughness-0	# submit to my CPU account
#SBATCH --partition=cpu-core-0				# Submit job to CPU partition
#SBATCH --ntasks=1							# Set one task per node
#SBATCH --cpus-per-task=1					# Select one core
#SBATCH --mem-per-cpu=3500M        			# Allocate 3.5GB of memory per core
#SBATCH --time=14-00:00						# Set max. run time; format: D-HH:MM
#SBATCH --hint=compute_bound				# Allocate all resources on a core (2threaded cpus)
#SBATCH --mail-user=emosadegh@nevada.unr.edu
#SBATCH --mail-type=ALL             		# Send mail on all stateChanges
#SBATCH --output=log_atmmodel.txt   		# The output file name
#SBATCH --error=log_atmmodel.txt		 	# The error file name
date
module list 
echo “MISR job started ...”
python  /data/gpfs/home/emosadegh/MISR-SeaIceRoughness/build_training_dataset/run_ATMModel.py

echo “MISR job ended ...”
date
