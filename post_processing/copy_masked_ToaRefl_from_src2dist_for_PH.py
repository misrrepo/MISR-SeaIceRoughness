#!/usr/bin/env python
# coding: utf-8

# import numpy as np
import os, glob, shutil


'''
note: this code moves masked-toa-refl files from src to destination 
    chek home directories
'''

########################################
src_home = '/data/gpfs/assoc/misr_roughness/2016/april_2016/masked_toa_refl_april_2016_9cam4bands_day15_30_p1_233_b1_46'
dest_home = '/data/gpfs/assoc/misr_roughness/2016/april_2016/test_21april2016/masked_toa_refl'


camera_list = ['Cf', 'An', 'Ca']
path_list = ['088', '104', '120', '136', '152', '168', '184', '200'] # for 21 April 2016
########################################

for cam_dir in camera_list:

    # check destination camera directory
    dest_cam_dir = os.path.join(dest_home, cam_dir)

    if (not (os.path.isdir(dest_cam_dir)):
        print('dest. camera subdir does NOT exist! We will make that directory.')
        os.mkdir(dest_cam_dir)
        print(dest_cam_dir)
        print(os.path.isdir(dest_cam_dir))
    else:
        print('dest. cam subdir exists!')
        print(dest_cam_dir)
        print(os.path.isdir(dest_cam_dir))


    ### make list of all available file patterns

    for path in path_list:
        # make pattern 
        file_pattern = 'masked_toa_refl_P*'+path+'*'+'.dat'
        print('looking for pattern in src= %s' %file_pattern)

        # search for file pattern in src and make a list
        src_cam_dir = os.path.join(src_home, cam_dir)
        src_files_found_list = glob.glob(os.path.join(src_cam_dir, file_pattern))
        print(len(src_files_found_list))

        for src_file in src_files_found_list:
            dest_file = shutil.copy(src_file, dest_cam_dir)
            print("Destination file:", dest_file)



# print('-> finished moving roughness files for day= %d' %day)
# print(rough_subdir_fullpath)
# moved_files_list = glob.glob(rough_subdir_fullpath, 'roughness_toa_refl*.dat')
# print('-> moved files= %d' %len(moved_files_list))




