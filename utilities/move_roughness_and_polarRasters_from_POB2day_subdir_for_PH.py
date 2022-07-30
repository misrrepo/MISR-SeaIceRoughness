#!/usr/bin/env python
# coding: utf-8

# import numpy as np
import os, glob
import MisrToolkit as Mtk 
from MisrToolkit import * 
import shutil


'''
note:   this code moves MISR roughness files (binary and polar rasters) based on POB to its specific day and 
        creates a folder for that day. Using this code we can run roushness2raster code for 
        each subdir/date at the same time. 

        First, setup dir path and datetime at the top of the code and then run this code at a time for each datetime. 

        Run with python3.6 (MisrToolkit) in Conda virtualEnvironment

        functions(.) are at the bottom of this code
'''

day = 16
month = 7
year = 2016



# for roughness and polar rasters: fullpath to predicted roughness directory
home_dir = "/data/gpfs/assoc/misr_roughness/2016/july_2016/predict_roughness_k_zero_npts_10/all_polar_files/blocks_1_19"



# the following 2 paths are for moving masked files
# where camera dir is- should exist on disk
src_dir = '/data/gpfs/assoc/misr_roughness/2016/april_2016/masked_toa_refl_april_2016_9cam4bands_day15_30_p1_233_b1_46/Cf'

# general path to home dir where code will create 3 camera subdirectories there- should exist on disk
dest_dir = '/data/gpfs/assoc/misr_roughness/2016/april_2016/tests/test_21april2016/masked_toa_refl'


#################### do not change anything else bellow ###############


############################## functions ##############################

def find_orbits_per_day(year, month, day):
    start_time = str(year)+'-'+str(month)+'-'+str(day)+'T00:00:00Z'
    end_time = str(year)+'-'+str(month)+'-'+str(day)+'T23:59:59Z'
    print(start_time)
    print(end_time)
    orbit_list = Mtk.time_range_to_orbit_list(start_time, end_time)
    print(orbit_list)
    print('found %d orbits!' %len(orbit_list))
    return orbit_list


def check_subdir_path(subdir_fullpath):
    # ## check if directory for specific day exists, else we create the directory
    # check if subdir exists
    if (not (os.path.isdir(subdir_fullpath))):
        print('subdir does NOT exist! We will make that directory.')
        os.mkdir(subdir_fullpath)
        print(subdir_fullpath)
        print(os.path.isdir(subdir_fullpath))
    else:
        print('subdir exists!')
        print(subdir_fullpath)
        print(os.path.isdir(subdir_fullpath))
    return 0


def move_roughness_files(orbit_list, subdir_fullpath):
    for orbit in orbit_list:
        print('processing orbit= %d' %orbit)
        # make pattern 
        file_pattern = 'roughness_toa_refl_P*'+'*_O0'+str(orbit)+'*'+'.dat'
        print(file_pattern)
    
        print('looking for pattern= %s' %file_pattern)
        # search for file pattern and make a list
        files_found_list = glob.glob(os.path.join(home_dir, file_pattern))
        print(len(files_found_list))

        # move roughness files to subdirectories
        for file_in_day in files_found_list:
            new_path = shutil.move(file_in_day, subdir_fullpath)
    return 0


def move_roughness_files_for_date(home_dir, year, month, day):
    subdir_name = 'roughness_subdir_'+str(year)+'_'+str(month)+'_'+str(day)
    print(subdir_name)


    subdir_fullpath = os.path.join(home_dir, subdir_name)
    print(subdir_fullpath)

    check_subdir_path(subdir_fullpath)

    list_of_orbits_per_day = find_orbits_per_day(year, month, day)
    move_roughness_files(list_of_orbits_per_day, subdir_fullpath)
    return 0


def move_polar_raters_for_date(home_dir, year, month, day):
    subdir_name = 'polar_raster_subdir_'+str(year)+'_'+str(month)+'_'+str(day)
    print(subdir_name)

    subdir_fullpath = os.path.join(home_dir, subdir_name)
    print(subdir_fullpath)

    check_subdir_path(subdir_fullpath)

    list_of_orbits_per_day = find_orbits_per_day(year, month, day)
    move_polar_rasters(list_of_orbits_per_day, subdir_fullpath)
    return 0


def move_polar_rasters(orbit_list, subdir_fullpath):

    path_list = []
    for iorbit in orbit_list:
        path_list.append(orbit_to_path(iorbit))
    print(path_list)

    for ipath in path_list:
        # make pattern 
        polar_raster_file_pattern = 'raster_path_'+str(ipath)+'_block_*'+'*_3995.tif'

        print('looking for pattern= %s' %polar_raster_file_pattern)
        # search for file pattern and make a list
        files_found_list = glob.glob(os.path.join(home_dir, polar_raster_file_pattern))
        print(len(files_found_list))

        # move roughness files to subdirectories
        for file_in_day in files_found_list:
            new_path = shutil.move(file_in_day, subdir_fullpath)
    return 0




############## review this code from here
def move_masked_toa_refl_files(orbit_list, dest_cam_dir):
    for orbit in orbit_list:
        print('processing orbit= %d' %orbit)
        # make pattern 
        file_pattern = 'masked_toa_refl_P*'+'*_O0'+str(orbit)+'*'+'_red.dat'  # only red band 
        print(file_pattern)
        print('looking for pattern= %s' %file_pattern)
        # search for file pattern and make a list
        files_found_list = glob.glob(os.path.join(src_dir, file_pattern))
        print(len(files_found_list))

        # move roughness files to subdirectories
        for src_files_fp in files_found_list:
            new_path = shutil.move(src_files_fp, dest_cam_dir)
    return 0



def move_masked_toa_refl_files_for_date(src_dir, dest_dir, year, month, day):
    cam_label = src_dir.split('/')[-1]

    subdir_name = 'masked_toa_refl_'+str(year)+'_'+str(month)+'_'+str(day)
    print(subdir_name)


    subdir_fullpath = os.path.join(dest_dir, subdir_name)
    print(subdir_fullpath)
    check_subdir_path(subdir_fullpath)
    # create camera dir inside subdir
    cam_dir_fullpath = os.path.join(subdir_fullpath, cam_label)
    # make the camera dir
    check_subdir_path(cam_dir_fullpath)

    list_of_orbits_per_day = find_orbits_per_day(year, month, day)
    move_masked_toa_refl_files(list_of_orbits_per_day, cam_dir_fullpath)
    return 0


############################### main ###############################
# use each of the following funcitons based on your need

# move_roughness_files_for_date(home_dir, year, month, day)
move_polar_raters_for_date(home_dir, year, month, day)
# move_masked_toa_refl_files_for_date(src_dir, dest_dir, year, month, day)

