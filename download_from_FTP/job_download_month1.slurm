#!/bin/bash

#SBATCH --job-name=HDF-DL-month1

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

#SBATCH --output=log_multi_download_month1.txt   		 # The output file name
#SBATCH --error=log_multi_download_month1.txt		 	# The error file nam

source /data/gpfs/home/emosadegh/.bashrc

date
module list 
echo “MISR job started ...”
conda activate /data/gpfs/home/emosadegh/miniconda2/envs/virtenv_py36
echo "python version:"
python -V

## path to python script to run
python /data/gpfs/home/emosadegh/MISR-SeaIceRoughness/download_from_FTP/multi_download_NASA_modified_month1.py

echo “MISR job ended ...”
conda deactivate
date
