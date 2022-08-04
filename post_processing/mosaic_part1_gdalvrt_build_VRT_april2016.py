#!/usr/bin/python

from osgeo import gdal
import os
import glob, os
import pandas as pd

#********************************************************
raster_dir = '/data/gpfs/assoc/misr_roughness/2016/april_2016/predict_roughness_k_zero_npts_10/all_polar_rasters/blocks_20_46'



output_VRT_filename = 'vrt_original_mosaic_gdal_april_2016_blocks_20_46.vrt'

#********************************************************
output_VRT_fp = os.path.join(raster_dir, output_VRT_filename)


print(os.path.isfile(output_VRT_fp))


file_pattern = 'raster_path_*3995.tif'
file_pattern_fp = os.path.join(raster_dir, file_pattern)
files_to_mosaic_list = glob.glob(file_pattern_fp)
print(len(files_to_mosaic_list))


print('building VRT dataset!')
# vrt_options = gdal.BuildVRTOptions(resampleAlg='average') #, addAlpha=True)
my_vrt_ptr = gdal.BuildVRT(output_VRT_fp, 
                            files_to_mosaic_list,
                            srcNodata = -99.0, # don't seperate with comma
                            VRTNodata = -99.0
                            )
                                    # , options=vrt_options)

print('closing VRT dataset!')
my_vrt_ptr = None 

print(os.path.isfile(output_VRT_fp))


print(output_VRT_fp)