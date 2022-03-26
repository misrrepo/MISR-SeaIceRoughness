#!/bin/bash
##---------------------------------------------------------
#SBATCH --job-name=arr2raster
##---------------------------------------------------------
## --- if to run on test partition for QualityAssurance

##SBATCH --account=cpu-s6-test-0 
##SBATCH --partition=cpu-s6-test-0

##---------------------------------------------------------
## --- run on MISR_roughness partition

#SBATCH --account=cpu-s5-misr_roughness-0
#SBATCH --partition=cpu-core-0			 			


##---------------------------------------------------------
## --- multi-core/threaded

## SBATCH --ntasks=1
## SBATCH --ntasks-per-core=1



#SBATCH --ntasks=1						# one task per node
#SBATCH --cpus-per-task=1
#SBATCH --mem-per-cpu=3500M        		# Allocate 3.5GB of memory per CPU
##SBATCH --time=14-00:00					# sets the max. run time; format: D-HH:MM
#SBATCH --hint=compute_bound			# to allocate all resources on a core (2threaded cpus)



#SBATCH --mail-type=ALL             		 		# Send mail on all state changes
#SBATCH --mail-user=emosadegh@pronghorn.rc.unr.edu

#SBATCH --output=log_array2raster.txt   		 # The output file name
#SBATCH --error=log_array2raster.txt		 	# The error file nam

##---------------------------------------------------------
## other settings 

##SBATCH --cores-per-socket=30  --> error
##SBATCH --hint=compute_bound
##SBATCH --nodes=1
##SBATCH --mem=1gb                              # Job memory request
##SBATCH --mem-per-cpu=4000M        		 	# Allocate 3.5GB of memory per CPU
##SBATCH --time=14-00:00				 		# sets the max. run time; format: D-HH:MM

##---------------------------------------------------------

##hostname
date

## module load intel/compiler/64/2018/18.0.1
module list 

## load virtual environment
# conda activate /data/gpfs/home/emosadegh/miniconda2/envs/virtenv_py36 # no need, all libraries are installed from source


echo “array2raster job started here...”


# python2.7 -u ./roughness_array2raster_georefrencing.py
python -u ./roughness_array2raster_georefrencing.py

echo “array2raster job ended here...”

date
