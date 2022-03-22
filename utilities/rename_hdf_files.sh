#!/bin/bash
echo "hi Ehsan!"
hdf_dir=/Volumes/Ehsan-7757225325/2016/april_2016/hdf_files_April2016_from_PH


echo $hdf_dir

echo "before renaming..."
for hdf_file in $hdf_dir/MISR_AM1_GRP_ELLIPSOID_GM*.hdf
do
	echo $hdf_file
	mv "$hdf_file"  "${hdf_file/.b*-0*.f*.hdf/.hdf}"
done

echo "after renaming..." 
for hdf_file in $hdf_dir/MISR_AM1_GRP_ELLIPSOID_GM*.hdf
do
	echo $hdf_file
done