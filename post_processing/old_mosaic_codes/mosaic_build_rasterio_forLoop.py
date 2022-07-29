#!/usr/bin/env python
# coding: utf-8
# Ehsan Mosadegh; April 2022

'''
first move all polar files into a single directory and then run this code
'''


import rasterio
from rasterio.merge import merge
from rasterio.plot import show
import glob
import os
import datetime as dt

dt1 = dt.datetime.now()
####################################################################
# File and folder paths

# where all geolocated rasters are
dirpath = "/Users/ehsanmos/MLP_dataset/mosaics/test_21apr2016_inOrder_and_reversCams/all_polar_rasters_reversed"


# cameras = 'inOrder' 
cameras = 'reversedOrder'

####################################################################
print(os.path.isdir(dirpath))

# select a single path
path_num_list = [15, 31, 47, 63, 104, 120, 136, 152, 168, 184, 200, 216, 232] # for test- 21 April 2016


for path_num in path_num_list:

    print('path num: %s' %path_num)

    # final mosaic name
    if (cameras=='inOrder'):    
        mosaic_name = "mosaic_rasterio_21april2016_path_"+str(path_num)+"_test_AMLineFixed_CamsInOrder.tif"

    if (cameras=='reversedOrder'):
        mosaic_name = "mosaic_rasterio_21april2016_path_"+str(path_num)+"_test_AMLineFixed_reversCams.tif"


    # output directory where mosaic goes
    out_fp = os.path.join(dirpath, mosaic_name)

    # Make a search criteria to select the DEM files
    search_criteria = 'raster_path_'+str(path_num)+'_'+'*_reprojToEPSG_3995.tif'

    raster_dir_fp = os.path.join(dirpath, search_criteria)
    print(raster_dir_fp)


    # glob function can be used to list files from a directory with specific criteria
    rasters_fps = glob.glob(raster_dir_fp)

    # Files that were found:
    print('raster files: %s' %len(rasters_fps))



    # List for the source files
    src_files_to_mosaic = []

    # Iterate over raster files and add them to source -list in 'read mode'
    print('reading all rasters...')
    for fp in rasters_fps:
        src = rasterio.open(fp)
        src_files_to_mosaic.append(src)

    # src_files_to_mosaic


    # Merge function returns a single mosaic array and the transformation info
    print('merging all rasters...')
    mosaic, out_trans = merge(src_files_to_mosaic, method='max')

    # Plot the result
    # show(mosaic, cmap='terrain')



    # Copy the metadata
    out_meta = src.meta.copy()

    # Update the metadata
    out_meta.update({"driver": "GTiff",
                     "height": mosaic.shape[1],
                     "width": mosaic.shape[2],
                     "transform": out_trans})



    # Write the mosaic raster to disk
    print('writing out files...')
    with rasterio.open(out_fp, "w", **out_meta) as dest:
        dest.write(mosaic)
    print(out_fp)




dt2 = dt.datetime.now()
delta_time = dt2-dt1
print('time: %s' %delta_time)





