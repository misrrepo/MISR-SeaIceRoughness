#!/usr/bin/python

from osgeo import gdal
import os
import glob, os
import pandas as pd
import datetime as dt

# to record runtime 
t1 = dt.datetime.now()

# vrt directory
raster_dir = '/data/gpfs/assoc/misr_roughness/2016/april_2016/predict_roughness_k_zero_npts_10/all_polar_rasters/blocks_20_46'


input_vrt_filename = 'vrt_average_mosaic_gdal_april_2016_blocks_20_46.vrt'

output_mosaic_filename = 'mosaic_gdal_april_2016_blocks_20_46_int16.tif'

#*****************************************************
## remembr to update & rename VRT file 

output_mosaic_fp = os.path.join(raster_dir, output_mosaic_filename)
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
                            noData = -99.0 -99991.0,    # also consider -99991.0 which is 2 black sides of MISR block
                            resampleAlg = resamplingAlg,
                            outputType = gdal.GDT_Int16) # note: input dtype is float_64==double, maybe here change dtype to make it smaller img???? # outputType = gdal.GDT_Byte     


print('\noutput mosaic: ')
print(output_mosaic_fp)

# Properly close the datasets to flush to disk
print('\nclosing mosaic dataset!')

mosaic_ds = None
input_VRT_fp = None  # close opened input file


# record time
t2 = dt.datetime.now()
delta_time = t2-t1
print(delta_time)