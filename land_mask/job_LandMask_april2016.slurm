#!/bin/bash

#SBATCH --job-name=lmask-april2016

##SBATCH --account=cpu-s6-test-0 
##SBATCH --partition=cpu-s6-test-0

#SBATCH --account=cpu-s5-misr_roughness-0
#SBATCH --partition=cpu-core-0			 # Submit job to the cpu partition

#SBATCH --ntasks=1						# one task per node
#SBATCH --cpus-per-task=1
#SBATCH --mem-per-cpu=3500M        		 # Allocate 3.5GB of memory per CPU
#SBATCH --time=14-00:00					# sets the max. run time; format: D-HH:MM
#SBATCH --hint=compute_bound			# to allocate all resources on a core (2threaded cpus)
#SBATCH --mail-user=emosadegh@nevada.unr.edu
#SBATCH --mail-type=ALL             		 # Send mail on all state changes

#SBATCH --output=log_lmask_april2016_9cams.txt   		 # The output file name
#SBATCH --error=log_lmask_april2016_9cams.txt		 	# The error file nam

date
module list 
echo “MISR job started ...”

## path to python script to run
python ./run_LandMask_allCameras_april_2016.py

echo “MISR job ended ...”
date
