#!/usr/bin/python2.7

import os, glob, gdal, datetime, shutil

'''
note: this code builds directory inside raster directory ad copies all found files into that destination dir.
'''
#################################################################################################

raster_dir = '/Volumes/Ehsan7757420250/2013/roughness_2013_apr1to16_p1to233_b1to40/roughness_subdir_2013_4_16/rasters_noDataNeg99_TiffFileFloat64_max'
dest_path = raster_dir
dest_dir = 'Anne_region'
#################################################################################################
def main():
	dest_dir_fp = os.path.join(dest_path, dest_dir)
	check_in_dir(raster_dir)
	check_out_dir(dest_dir_fp)

	pattern = 'raster_path*'+'85gcps.tif'
	# pattern = 'raster_path*'+'latlon.tif'

	file_pattern_fullpath = os.path.join(raster_dir, pattern)

	file_pattern_list = glob.glob(file_pattern_fullpath)
	print('-> files found: %s' %len(file_pattern_list))

	copied_files = 0
	for src_file in file_pattern_list:
		#~~ open raster dataset and check info
		dataset = gdal.Open(src_file)
		geotransform = dataset.GetGeoTransform()
		# print(geotransform) # note: units in meters
		# print('order of geoTransform matrix: topleftX, pixelW, 0, topleftY, 0, pixelH')	
		xMin = geotransform[0]
		xMax = geotransform[0] + dataset.RasterXSize*geotransform[1]        # i changed it to * to get xMax value
		yMin = geotransform[3] + dataset.RasterYSize*geotransform[5]
		yMax = geotransform[3]

		if ((xMin >= -25) and (xMax <= +25)):
			if (yMin >= 70):
				print('-> this block is in Anne region! copying it.')
				shutil.copy(src_file, dest_dir_fp)
				copied_files+=1

				print('corners: tlY, blY, blX, brX')
				print(yMax)
				print(yMin)
				print(xMin)
				print(xMax)
				print('\n')
		dataset = None
	print('-> finished successfully! Copied %s files.' %copied_files)
	print('%s\n' %dest_dir_fp)
	return 0
#################################################################################################
def check_in_dir(in_dir):
	if (os.path.isdir(in_dir)):
	    print('-> input directory found: %s' %in_dir)
	    return 0
	else:
	    print('-> input directory NOT exist: %s' %in_dir)
	    raise SystemExit()
#################################################################################################
def check_out_dir(out_dir):
	if (os.path.isdir(out_dir)):
	    print('-> output directory found: %s' %out_dir)
	    return 0
	else:
	    print('-> output directory NOT exist: %s' %out_dir)
	    print('-> making output dir...')
	    os.mkdir(out_dir)
	    if (os.path.isdir(out_dir)==True):
	    	print('-> output dir was mad successfully!')
	    else:
	    	print('-> problem w/making output dir.')
	    	raise SystemExit()
#################################################################################################
if __name__ == '__main__':
	start=datetime.datetime.now();
	main();
	end=datetime.datetime.now();
	run_time=end-start;
	print('-> runtime was: %s' %run_time);
#################################################################################################