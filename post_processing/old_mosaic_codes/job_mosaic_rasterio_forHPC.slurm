#!/bin/bash
##---------------------------------------------------------
#SBATCH --job-name=MosaicApr2016

#SBATCH --account=cpu-s5-misr_roughness-0
#SBATCH --partition=cpu-core-0			 			

#SBATCH --ntasks=1						# one task per node
#SBATCH --cpus-per-task=32
#SBATCH --mem-per-cpu=3500M        		# Allocate 3.5GB of memory per CPU
#SBATCH --hint=compute_bound			# to allocate all resources on a core (2threaded cpus)

#SBATCH --mail-type=ALL             		 		# Send mail on all state changes
#SBATCH --mail-user=emosadegh@nevada.unr.edu

#SBATCH --output=log_mosaic_july2016_inOrderCams_blocks20_46.txt   		 # The output file name
#SBATCH --error=log_mosaic_july2016_inOrderCams_blocks20_46.txt		 	# The error file nam

##---------------------------------------------------------

##hostname
date

## module load intel/compiler/64/2018/18.0.1
module list 


echo "mosaic job started here..."


python -u ./mosaic_build_rasterio_forHPC.py


echo "mosaic job ended here..."

date

