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
dirpath = "/data/gpfs/assoc/misr_roughness/2016/april_2016/predict_roughness_k_zero_npts_10/all_polar_rasters"

output_mosaic_name = "final_mosaic_rasterio_15_30_april_2016_max.tif"

out_fp = os.path.join(dirpath, output_mosaic_name)

####################################################################
# Make a search criteria to select the DEM files
search_criteria = 'raster_path_*'+'*_reprojToEPSG_3995.tif'
raster_dir_fp = os.path.join(dirpath, search_criteria)
print(raster_dir_fp)


# In[3]:


# glob function can be used to list files from a directory with specific criteria
rasters_fps = glob.glob(raster_dir_fp)

# Files that were found:
print('raster files: %s' %len(rasters_fps))


# In[4]:


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





