#!/usr/bin/python

from osgeo import gdal
import os
import glob, os
import pandas as pd


raster_dir = '/data/gpfs/assoc/misr_roughness/2016/april_2016/predict_roughness_k_zero_npts_10/all_polar_rasters/blocks_20_46'



input_vrt_filename = 'vrt_average_mosaic_gdal_april_2016.vrt'

output_mosaic_filename = 'mosaic_gdal_april_2016.tif'


output_mosaic_fp = os.path.join(raster_dir, output_mosaic_filename)


#*****************************************************
## remembr to update & rename VRT file 
input_VRT_fp = os.path.join(raster_dir, input_vrt_filename)

if (os.path.isfile(input_VRT_fp)!=True):
    raise SystemExit
#*****************************************************

print('start building mosaic from VRT dataset...')

resamplingAlg = 'nearest'

gdal.SetConfigOption('GDAL_VRT_ENABLE_PYTHON', 'YES') # to be able to include python functions

mosaic_ds = gdal.Translate(output_mosaic_fp,
                            input_VRT_fp,
                            format = 'GTiff',
                            noData = -99.0,
                            resampleAlg = resamplingAlg,
                            outputType = gdal.GDT_Byte) # note: input dtype is float_64==double, maybe here change dtype to make it smaller img???? # outputType = gdal.GDT_Byte     


print('\noutput mosaic: ')
print(output_mosaic_fp)

# Properly close the datasets to flush to disk
print('\nclosing mosaic dataset!')

mosaic_ds = None
input_VRT_fp = None  # close opened input file
