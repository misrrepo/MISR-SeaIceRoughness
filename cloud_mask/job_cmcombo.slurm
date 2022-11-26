#!/bin/bash
##---------------------------------------------------------
#SBATCH --job-name=cmcombo-july2016

#SBATCH --account=cpu-s5-misr_roughness-0
#SBATCH --partition=cpu-core-0			 			# Submits your job to the cpu partition
#SBATCH --ntasks=1
#SBATCH --ntasks-per-core=1
#SBATCH --mail-type=ALL             		 # Send mail on all state changes
#SBATCH --mail-user=emosadegh@nevada.unr.edu
#SBATCH --time=5-00:00				 		# sets the max. run time; format: D-HH:MM

##----------------------------------------------------------

#SBATCH --output=log_cmcombo_bin_july2016.txt   				 # The output file name
#SBATCH --error=log_cmcombo_bin_july2016.txt				 	# The error file nam


##---------------------------------------------------------
##			set run script
##---------------------------------------------------------
##hostname
date

## module load intel/compiler/64/2018/18.0.1
module list 


echo “cmcombo job started ...”

# MISR2Roughness_parallel
python ./cloudmask_to_cmcombo.py 


echo “cmcombo job ended ...”

date
