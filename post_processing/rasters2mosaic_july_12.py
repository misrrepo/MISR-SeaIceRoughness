#!/usr/bin/python2.7

#~ by: Ehsan Mosadegh, 25 Sep 2020

'''
input: directory to raster.tif files (output of roughness_array2raster_georefrencing.py)
output: intermediate VRT file, and then final mosaic.tif file in the same directory where raster.tif files are. Then, we will plot that mosaic later in QGIS.

note: before running this script, we have to build raster.tif files first (output of roughness_array2raster_georefrencing.py), 
		then move rasters to subdir for each day (move_roughness_rasters_from_POB2day_subdir.ipynb),
		and then use this script to create mosaics from files inside each day-dir

note: change the date also: day/month/year
'''
import glob, os
from osgeo import gdal
from subprocess import call
import datetime as dt


#~ we find this file pattern
raster_file_pattern = 'raster_path_*'+'*_reprojToEPSG_3995.tif'
# raster_file_pattern = 'raster_path_*'+'latlon.tif'
# raster_file_pattern = 'raster_path_*'+'85gcps.tif'

########################################################################################################################
def main():

	print('-> start main(): ')
	# path to date dir with raster.tif files there 
	raster_dir_fullpath = "/data/gpfs/assoc/misr_roughness/2016/july_2016/predict_roughness_k_zero_npts_10/roughness_subdir_2016_7_12/2016_7_rasters_noDataNeg99_TiffFileFloat64_max_geographicalMesh_withLatLonList"


	# day label
	day='12'
	month='july'
	year='2016'



	# form date-tag from 3 parameters
	date_tag = day+'-'+month+'-'+year

	# naming labels
	resamplingAlg = 'nearest'
	output_VRT_dataset_name = 'virtualDataset_float64_'+resamplingAlg+'_'+date_tag+'.vrt'
	output_mosaic_name = 'mosaic_fromVRT_float64_'+resamplingAlg+'_'+date_tag+'.tif'

	########################################################################################################################
	VRT_fullpath = os.path.join(raster_dir_fullpath, output_VRT_dataset_name)

	print('-> raster dir: %s' %raster_dir_fullpath)
	print('-> VRT: %s' %VRT_fullpath)
	print('-> resampling Algorithm: %s' % resamplingAlg)
	print('-> looking for pattern: %s' % raster_file_pattern)

	#~ build a list from input raster files
	files_to_mosaic = glob.glob(os.path.join(raster_dir_fullpath, raster_file_pattern))
	print('-> files found: %d' % len(files_to_mosaic))
	
	########################################################################################################################
	#~ method 1- gdal_merge.py cmdLine
	#~ problems: slow cuz of beiing cmdLine + a long string of inoput files into memory

	# files_string = " ".join(files_to_mosaic)
	# command = "gdal_merge.py -o " + VRT_fullpath + " -v -of GTiff -ot Float64 " + files_string
	# ret = call(command, shell=True)
	# print(ret)

	########################################################################################################################
	#~ mwthod 2- VRT and gdal_translate
	print('-> building VRT dataset!')
	# vrt_options = gdal.BuildVRTOptions(resampleAlg='linear') #, addAlpha=True)
	my_vrt_ptr = gdal.BuildVRT(
								VRT_fullpath, 
								files_to_mosaic,
								srcNodata = -99.0,
								VRTNodata = -99.0
							) 					#, options=vrt_options)
	
	print('-> closing VRT dataset!')
	my_vrt_ptr = None 


	########################################################################################################################
	# gdal_translate -of GTiff -co "COMPRESS=JPEG" -co "PHOTOMETRIC=YCBCR" -co "TILED=YES" virtualDS_paths_41_139_233_Float64.vrt mosaic.tif

	#~ name of mosaic
	out_mosaic_fullpath = os.path.join(raster_dir_fullpath, output_mosaic_name)


	#~ define Translate Options --> we don't need to create a TranslateOptions object, we only need do define anything as an keyword_argument for the .Translate()
	#~ writes translated output to a .tif file and returns a gdal.Dataset object; after writing it, mosaic_ds will be empty --> how do it VRT???
	print('-> building mosaic from VRT dataset!')
	mosaic_ds = gdal.Translate( 								# verbose?
									out_mosaic_fullpath,
									VRT_fullpath,
									format = 'GTiff',
									noData = -99.0,
									resampleAlg = resamplingAlg,
									outputType = gdal.GDT_Float64 # note: input dtype is float_64==double, maybe here change dtype to make it smaller img???? # outputType = gdal.GDT_Byte 	
								) 

	print('-> output mosaic: ')
	print(out_mosaic_fullpath)

	#~ Properly close the datasets to flush to disk
	print('\n-> closing mosaic dataset!')

	mosaic_ds = None
	VRT_fullpath = None  # close opened input file

	return 0
########################################################################################################################
if __name__ == '__main__':
	
	start_time = dt.datetime.now()
	print('-> start time: %s' %start_time)
	print(" ")

	main()
	
	end_time = dt.datetime.now()
	print('\n-> end time= %s' %end_time)
	print('-> runtime duration= %s' %(end_time-start_time))
	print(" ")
	print('######################## MOSAIC COMPLETED SUCCESSFULLY ########################')
########################################################################################################################

