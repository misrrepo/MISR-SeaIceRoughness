#!/usr/bin/python3
'''
	written by: Ehsan 19 Nov 2020
	usage: this program makes a list from available directories and moves files from inside the ATM-ordered zip directories out and to a target dir == current dir (to the save level)
		I used this program to bring ATM files from their subdirectories to a target directory
		set the target dir == source directory (to the save level- the directory where includes all ATM-downloaded directories

'''

import os, glob, shutil

src_dir_fullpath = "/Volumes/Ehsan-7757225325/2016/july_2016_old_review_later/atm_data_july_2016"

# target_dir_fullpath = "/home/ehsan/misr_lab/ATM_2010_2014_2019"
target_dir_fullpath = src_dir_fullpath



#~ define file pattern
# file_pattern = 'MISR_AM1_GRP_ELLIPSOID_GM_P*'+'.hdf'
file_pattern = 'ILATM2_*.csv'
# file_pattern = 'MISR_AM1_TC_CLASSIFIERS*.hdf'


print('source dir: %s' % src_dir_fullpath)
print('target dir: %s' % target_dir_fullpath)
print('file pattern: %s' % file_pattern)
print("\n")
dir_content_list = os.listdir(src_dir_fullpath)  # make list of all contents in a dir
print('subdirs found in src dir: %s' % len(dir_content_list))

for count, srcdir in enumerate(dir_content_list):
	src_dir = os.path.join(src_dir_fullpath, srcdir)
	if (os.path.isdir(src_dir)):  
		print('src dir: %s' % src_dir)
		src_files_fullpath_list = glob.glob(os.path.join(src_dir, file_pattern))

		if (len(src_files_fullpath_list)==0):
			print('src dir empty!')
			continue
		else:

			for src_file_fp in src_files_fullpath_list:
				print('src found: %s' % src_file_fp)
				# print(os.path.isfile(src_file_fp))
				##- check if src file has moved before and exists in distination dir
				if (os.path.isfile(os.path.join(target_dir_fullpath, src_file_fp.split('/')[-1]))==True):
					continue
				##- move files from src dir to target dir
				shutil.move(src_file_fp, target_dir_fullpath)
				##- check if file was moved successfully
				if (os.path.isfile(os.path.join(target_dir_fullpath, src_file_fp.split('/')[-1]))):
					print('file (%d/%d) moved successfully!' %(count+1, len(dir_content_list)))
				else:
					print('warning on moving file')

