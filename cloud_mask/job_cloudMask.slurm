#!/bin/bash
##---------------------------------------------------------
#SBATCH --job-name=cloudmask-rccm

#SBATCH --account=cpu-s5-misr_roughness-0
#SBATCH --partition=cpu-core-0			 			# Submit job to the cpu partition
#SBATCH --ntasks=1
#SBATCH --ntasks-per-core=1
#SBATCH --mail-type=ALL             		 # Send mail on all state changes
#SBATCH --mail-user=emosadegh@nevada.unr.edu
#SBATCH --output=log_cloudmask_rccm_april.txt   				 # The output file name
#SBATCH --error=log_cloudmask_rccm_april.txt				 	# The error file nam
##SBATCH --time=2-00:00				 		# sets the max. run time; format: D-HH:MM

##---------------------------------------------------------
##			set run script
##---------------------------------------------------------
##hostname
date

## module load intel/compiler/64/2018/18.0.1
module list 


echo “MISR job started ...”

# MISR2Roughness_parallel
python ./run_ReadMISRCloudMask.py 


echo “MISR job ended ...”

date
