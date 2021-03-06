#!/usr/bin/python
'''
this script moves unique path-borbit MISR blocks from src to dest directory
'''

# libraries used
import os
import shutil
import pandas as pd

######################################################
# PO of files that we will move
cam = 'AN'

unique_pob_file = 'uniques_pob_april2016_withATMinThem_from_atmmodel.csv'
home_dir_unique = '/data/gpfs/home/emosadegh/MISR-SeaIceRoughness/utilities'


hdf_home = '/data/gpfs/assoc/misr_roughness/2016/april_2016/hdf_ellipsoid'
hdf_dest_dir_name = 'move_to_Mac_blocksInAtmmodel' 		# this will be inside hdf_home

######################################################
if (os.path.isdir(os.path.join(hdf_home, hdf_dest_dir_name))!=True):
	print('destination folder not available')
	raise RuntimeError()


input_unique_df = pd.read_csv(os.path.join(home_dir_unique, unique_pob_file))
print(input_unique_df.columns)

for row_indx in range(input_unique_df.shape[0]):
	path_int = input_unique_df['#path'].iloc[row_indx]
	orbit_int = input_unique_df['orbit'].iloc[row_indx]
	print(path_int, orbit_int)


	# zero-pad path to 3 digits
	if (path_int < 100):
		path_str = str(path_int)
		# path_str.zfill(3)
		path_str = "0"+path_str
		print('updated to: %s' %path_str)
	else:
		path_str = str(path_int) 




	hdf_file_pattern = 'MISR_AM1_GRP_ELLIPSOID_GM_P'+path_str+'_O0'+str(orbit_int)+'_'+cam+'_F03_0024.hdf'

	hdf_src_fp = os.path.join(hdf_home, hdf_file_pattern)
	print("src: %s" %hdf_src_fp)

	check_status = os.path.isfile(hdf_src_fp)
	if (check_status!=True):
		print('HDFfile not found!')
		continue


	else:

		# hdf_dest
		hdf_destination = os.path.join(hdf_home, hdf_dest_dir_name)
		hdf_dest_fp = os.path.join(hdf_destination, hdf_src_fp.split('/')[-1])

		# move file using shutil.move(src, dest)
		shutil.move(hdf_src_fp, hdf_dest_fp)
		if (os.path.isfile(hdf_dest_fp)==True):
			print('moved!')




