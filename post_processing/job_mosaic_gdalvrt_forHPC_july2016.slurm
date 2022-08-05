#!/bin/bash
##---------------------------------------------------------
#SBATCH --account=cpu-s5-misr_roughness-0
#SBATCH --partition=cpu-core-0			 			
##---------------------------------- new script --------------------------
#SBATCH --job-name=MosaicJuly2016         # create a short name for your job
#SBATCH --nodes=1                # node count
#SBATCH --ntasks=1               # total number of tasks across all nodes
#SBATCH --cpus-per-task=1        # cpu-cores per task (>1 if multi-threaded tasks)
#SBATCH --mem=0
##SBATCH --mem-per-cpu=2G         # memory per cpu-core (4G is default)
##---------------------------------- old script --------------------------
##SBATCH --ntasks=1						# one task per node
##SBATCH --cpus-per-task=32
##SBATCH --mem-per-cpu=5000M        	# Allocate 3.5GB of memory per CPU
##SBATCH --mem=MaxMemPerNode    			# To request all the memory on a node
##SBATCH --mem-per-cpu=MaxMemPerCPU
##SBATCH --mem=0
##---------------------------------- other --------------------------
#SBATCH --hint=compute_bound			# to allocate all resources on a core (2threaded cpus)
#SBATCH --time=13-24:00:00   				# total run time limit (HH:MM:SS)
#SBATCH --mail-type=ALL             		 		# Send mail on all state changes
#SBATCH --mail-user=emosadegh@nevada.unr.edu
#SBATCH --output=log_mosaic_july2016_gdalvrt.txt   		 # The output file name
#SBATCH --error=log_mosaic_july2016_gdalvrt.txt		 	# The error file nam
##---------------------------------------------------------

##hostname
date

## module load intel/compiler/64/2018/18.0.1
module list 


echo "mosaic job started here..."


python -u ./mosaic_part2_gdalvrt_build_mosaic_fromVRT_july2016.py


echo "mosaic job ended here..."

date

