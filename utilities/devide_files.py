#!/usr/bin/python

import glob, shutil, os

#~ to devide files from whole to n-subdir to process parallel
src_dir = '/data/gpfs/assoc/misr_roughness/2016/july_2016'
dest_dir = src_dir

file_pattern = 'MISR_AM1_GRP_ELLIPSOID_GM_P*';
dest_subdir_list = ['part1', 'part2', 'part3', 'part4'];

src_files_fp_list = glob.glob(os.path.join(src_dir, file_pattern));
print('num. of files found: %s' % len(src_files_fp_list));


edgeCntr_list = range(0,len(src_files_fp_list),1000)
edgeCntr_list.append(len(src_files_fp_list))

for edgeCntr in range(len(edgeCntr_list)-1):
	#~ select a subdir 
	dest_subdir_fp = os.path.join(dest_dir, dest_subdir_list[edgeCntr])

	for fileCntr in range(edgeCntr_list[edgeCntr], edgeCntr_list[edgeCntr+1]):
		#~ check is file exists in destination
		if (os.path.isfile(os.path.join(dest_subdir_fp, src_files_fp_list[fileCntr].split('/')[-1]))==True):
				continue
		else:
			#~ move files from src dir to target dir
			print('moving files to dir: %s' % dest_subdir_fp)
			shutil.move(src_files_fp_list[fileCntr], dest_subdir_fp)
			#~ check if file was moved successfully
			if (os.path.isfile(os.path.join(dest_subdir_fp, src_files_fp_list[fileCntr].split('/')[-1]))):
				print('file %s moved successfully!' % (fileCntr))
			else:
				print('warning on moving file')

