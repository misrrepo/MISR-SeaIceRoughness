#!/bin/bash
##---------------------------------------------------------
#SBATCH --job-name=MosaicApr2016

#SBATCH --account=cpu-s5-misr_roughness-0
#SBATCH --partition=cpu-core-0			 			

#SBATCH --ntasks=1						# one task per node
#SBATCH --cpus-per-task=32
##SBATCH --mem-per-cpu=5000M        	# Allocate 3.5GB of memory per CPU
#SBATCH --mem=0    						# To request all the memory on a node
#SBATCH --hint=compute_bound			# to allocate all resources on a core (2threaded cpus)

#SBATCH --mail-type=ALL             		 		# Send mail on all state changes
#SBATCH --mail-user=emosadegh@nevada.unr.edu

#SBATCH --output=log_mosaic_april2016_gdalvrt.txt   		 # The output file name
#SBATCH --error=log_mosaic_april2016_gdalvrt.txt		 	# The error file nam

##---------------------------------------------------------

##hostname
date

## module load intel/compiler/64/2018/18.0.1
module list 


echo "mosaic job started here..."


python -u ./mosaic_gdalVRT_part2_build_mosaic_fromVRT_april2016.py


echo "mosaic job ended here..."

date

