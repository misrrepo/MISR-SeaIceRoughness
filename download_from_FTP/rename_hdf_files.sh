#!/bin/bash
# Ehsan Mosadegh; January 2022
#
# usage: rename file labels of hdf files and remove unnecessary labels at the end of each file:
# MISR_AM1_GRP_ELLIPSOID_GM_P112_O088201_CA_F03_0024.b001-046.f83cc3096.hdf -- changes to --> MISR_AM1_GRP_ELLIPSOID_GM_P112_O088201_CA_F03_0024.hdf

echo "renaming HDF files...!"

hdf_dir=/data/gpfs/assoc/misr_roughness/2016/cloud_masks/july_10_25_2016_ascm



echo $hdf_dir

echo "before renaming..."
for hdf_file in $hdf_dir/MISR_AM1_TC_CLASSIFIERS*.hdf  # change this based on the fileName
do
	echo $hdf_file
	mv "$hdf_file"  "${hdf_file/.f*.hdf/.hdf}"
done

# check file names here
echo "after renaming..." 
for hdf_file in $hdf_dir/MISR_AM1_TC_CLASSIFIERS*.hdf  # change this based on the fileName
do
	echo $hdf_file
done
