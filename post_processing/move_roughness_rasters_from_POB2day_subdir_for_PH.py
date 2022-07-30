#!/usr/bin/env python
# coding: utf-8

# import numpy as np
import os, glob
import MisrToolkit as Mtk 
from MisrToolkit import * 
import shutil


'''
note:   this code moves misr roughness binary files based on POB to its specific day and 
        creates a folder for it. This way we can run roushness2raster code for 
        each subdir/date at the same time. First, setup dir path and dates here 
        and then run this notebook at a time for each date. Run with python3.6 (MisrToolkit)
'''


year = 2016
month = 7
day = 26


# fullpath to predicted roughness directory
main_roughness_dir_fullpath = "/data/gpfs/assoc/misr_roughness/2016/july_2016/predict_roughness_k_zero_npts_10"


# ####################### do not change anything else bellow ################################################


start_time = str(year)+'-'+str(month)+'-'+str(day)+'T00:00:00Z'
end_time = str(year)+'-'+str(month)+'-'+str(day)+'T23:59:59Z'


print(start_time)
print(end_time)


orbit_list = Mtk.time_range_to_orbit_list(start_time, end_time)
print(orbit_list)
print('found %d orbits!' %len(orbit_list))


# ## for each orbit in list, we will find roughness files for that specific orbit 

# ## check if directory for specific day exists, else we create the directory


rough_subdir_name = 'roughness_subdir_'+str(year)+'_'+str(month)+'_'+str(day)
print(rough_subdir_name)



rough_subdir_fullpath = os.path.join(main_roughness_dir_fullpath, rough_subdir_name)
print(rough_subdir_fullpath)



if (not (os.path.isdir(rough_subdir_fullpath))):
    print('roughness subdir does NOT exist! We will make that directory.')
    os.mkdir(rough_subdir_fullpath)
    print(rough_subdir_fullpath)
    print(os.path.isdir(rough_subdir_fullpath))
else:
    print('roughness subdir exists!')
    print(rough_subdir_fullpath)
    print(os.path.isdir(rough_subdir_fullpath))


# ## make list of all available roughness file patterns for specific day



for orbit in orbit_list:
    print('processing orbit= %d' %orbit)
    #~ make pattern 
    roughness_file_pattern = 'roughness_toa_refl_P*'+'*_O0'+str(orbit)+'*'+'.dat'
    print('looking for pattern= %s' %roughness_file_pattern)
    #~ search for file pattern and make a list
    roughness_files_found_list = glob.glob(os.path.join(main_roughness_dir_fullpath, roughness_file_pattern))
    print(len(roughness_files_found_list))
#     print(roughness_files_found_list)

    for rough_file_day in roughness_files_found_list:
        new_path = shutil.move(rough_file_day, rough_subdir_fullpath)



# print('-> finished moving roughness files for day= %d' %day)
# print(rough_subdir_fullpath)
# moved_files_list = glob.glob(rough_subdir_fullpath, 'roughness_toa_refl*.dat')
# print('-> moved files= %d' %len(moved_files_list))




