#!/bin/bash
# Ehsan Mosadegh; January 2022
#
# usage: rename file labels of hdf files and remove unnecessary labels at the end of each file:
# MISR_AM1_GRP_ELLIPSOID_GM_P112_O088201_CA_F03_0024.b001-046.f83cc3096.hdf -- changes to --> MISR_AM1_GRP_ELLIPSOID_GM_P112_O088201_CA_F03_0024.hdf

echo "renaming roughness files...!"

home_dir=/data/gpfs/assoc/misr_roughness/2016/april_2016/test_21april2016/predict_roughness_k_zero_npts_10


echo $home_dir

echo "before renaming..."
for rough_file in $home_dir/roughness_toa_refl_P*.dat
do
	echo $rough_file
	mv "$rough_file"  "${rough_file/_an_red.dat/.dat}"
done

# check file names here
echo "after renaming..." 
for rough_file in $home_dir/roughness_toa_refl_P*.dat
do
	echo $rough_file
done
