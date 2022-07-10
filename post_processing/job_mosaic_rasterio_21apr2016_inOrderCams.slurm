#!/bin/bash
##---------------------------------------------------------
#SBATCH --job-name=MosaicApr2016
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


#SBATCH --ntasks=1						# one task per node
#SBATCH --cpus-per-task=32
#SBATCH --mem-per-cpu=3500M        		# Allocate 3.5GB of memory per CPU
##SBATCH --time=14-00:00				# sets the max. run time; format: D-HH:MM
#SBATCH --hint=compute_bound			# to allocate all resources on a core (2threaded cpus)



#SBATCH --mail-type=ALL             		 		# Send mail on all state changes
#SBATCH --mail-user=emosadegh@nevada.unr.edu

#SBATCH --output=log_mosaic_test_21apr_2016_inOrderCams.txt   		 # The output file name
#SBATCH --error=log_mosaic_test_21apr_2016_inOrderCams.txt		 	# The error file nam

##---------------------------------------------------------

##hostname
date

## module load intel/compiler/64/2018/18.0.1
module list 


echo "mosaic job started here..."


python -u ./mosaic_build_rasterio_forHPC_21apr2016_inOrderCams.py


echo "mosaic job ended here..."

date

