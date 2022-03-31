#!/usr/bin/env python
# coding: utf-8
'''
notes:
	this f() builds georeferenced tif files from roughness arrays. 
	input: roughness array 
	output: georeferenced raster <.tif> file in both latlon and polar stereographic projections
	
	note: need to be run with python 3.6 if using precompiled MTK library
	note: on Pronghorn I built and installed MTK and gdal libraries from source myself, so no need to use virtual environment then!
'''

import numpy as np
import pandas as pd
import os, glob
from osgeo import gdal  # python3.6 on Mac ?
import MisrToolkit as Mtk # python2.7
from MisrToolkit import * # 
from subprocess import call
import datetime as dt
from matplotlib import pyplot as plt  #  ?   pyplot uses the actual RGB values as they are, more accurate than PIL
# import PIL 
# from PIL import Image  
import tifffile # to write images with dtype=float64 on disc as bigTiff

########################################################################################################################
# dir path setup by user
########################################################################################################################
#~ setup dir w/ roughness files there
rough_dir_fullpath =  "/data/gpfs/assoc/misr_roughness/2016/april_2016/predict_roughness_k_zero_npts_10/roughness_subdir_2016_4_16"


#~ tiff dir where arr2tiff goes to; for now se build it inside rouhness dir
# georefRaster_dir_name = 'rasters_noDataNeg99_TiffFileFloat64_max'


roughness_date = "2016_4"


# output_dir = "/media/ehsan/6TB_part2/roughness2raster_2016/july"
output_dir = rough_dir_fullpath


georefRaster_dir_name = roughness_date+'_rasters_noDataNeg99_TiffFileFloat64_max_geographicalMesh_withLatLonList'


########################################################################################################################
#~ global ID-fiers
########################################################################################################################
# misr_img_res = (512, 2048)
# img_nrows = misr_img_dims[0]
# img_ncols = misr_img_dims[1]

ascending_block_threshold = 5
misr_res_meter = 275
gcp_mode = "corners_n_inside"                       									# 'inside' OR "corners_n_inside"
reprojection = 'on'
skip_antimeridian = 'on' # if on == skip any image block that crosses A.M. line
# copy_Anne_region = False  # copy raters that fall into a region to a seperate dir 
########################################################################################################################

########################################################################################################################
''' this function reads in each raw img block, processes information, and embeds geolocation information in each 
	raster and writes out a raster/tiff img'''
def main():

	#~ new iter
	rough_files_fullpath_list, tot_found_rough_files = make_roughness_list_from_dir(rough_dir_fullpath)
	# ## build a dir where images from arr2img will go to

	# image_dir = img_dir_setup(rough_dir_fullpath, georefRaster_dir_name)
	image_dir = img_dir_setup(output_dir, georefRaster_dir_name)

	#~ reading roughness files in loop & process each at a time
	for file_count, rough_fname in enumerate(rough_files_fullpath_list):
		print("--------------------------------------------------------------------------------------------------------")
		print('processing new roughness array: (%d/ %d)' % (file_count+1, tot_found_rough_files))
		print(rough_fname)
		print('\n')
		rough_arr_2d, block_num, path_num = read_rough_file(rough_fname)
		
		## write img to disc w/using arr2img_writeToDisc() function
		path_label = 'path_'+str(path_num)
		block_label = 'block_'+str(block_num)

		ret = arr2img_writeToDisc(rough_arr_2d, path_label, block_label, image_dir)
		
		if (ret=='skipThisImg'):
			print('continue to next img.')
			continue
		else:
			out_img_fullpath = ret

		print('block: %s' %block_num)
		if (block_num < ascending_block_threshold): 	# we exclude blocks less than 20 (I changed to 10) to exclude blocks in ascending path
			print('block num < ascending threshold=%s, so we skip it!' %ascending_block_threshold)
			continue

		#~ we open the saved-on-disc images from previous step; gdal.Open(can read any img format=tif, jpg, png, but png has issues w/resamplinh alg later, so jpg + tif(but larger files) is better)
		in_ds = gdal.Open(out_img_fullpath)
		# print('-> inDS type: %s' % type(in_ds)) # returns a Dataset obj
		
		# gcp_list, total_gcps, antimaridina_crossing, gcp_numbers = create_gcp_list_for_imgBlockPixels_mostGCPs(path_num, block_num, misr_res_meter)
		gcp_list, total_gcps, gcp_numbers, antimaridina_crossing = create_gcp_list_for_imgBlockPixels_fixedGCPs_skipAMcrossing(path_num, block_num, misr_res_meter, image_dir)

		if (skip_antimeridian=='on'):
			if (antimaridina_crossing==True):
				print('note: image block crosses Anti-Maridian! we will skip it')
				continue

		translated_img_fullpath = apply_gcp(path_label, block_label, image_dir, in_ds, gcp_list)
		warpedFile_fullPath_noExt = warp_img(path_label, block_label, total_gcps, image_dir, translated_img_fullpath, gcp_numbers)
		
		if (reprojection == 'on'):
			reproject_to_polar(warpedFile_fullPath_noExt)

		#~ we print the raster directory at the end to be easier to use for next step: create-mosaic
		print('\n')
		print('raster direcotyr:')
		print(image_dir)

	return 0

########################################################################################################################
'''this func'''
def arr2img_writeToDisc(in_arr_2d, path_label, block_label, img_dir):

	# if (in_arr_2d.max() < 1):
	# 	print('-> PlotImgFunc(.): image is dark! We skip it.')
	# 	return 'skipThisImg'
	
	# else:

	# img_format = ".jpg"  # does this format support saving neg- values?
	# img_format = ".png"
	img_format = ".tif"  # this image format supports saving neg- values in image


	print('\n')
	print('image array min= %d' % np.nanmin(in_arr_2d))
	print('image array max= %d' % np.nanmax(in_arr_2d))

	#~ now replace negative pixelValues w/ zero ---> (why???)
	# in_arr_2d[in_arr_2d<0] = 0 # masks any element of np.array that has value < zero
	# print('-> img array min= %d' % in_arr_2d.min())
	# print('-> img array max= %d' % in_arr_2d.max())


	out_img_label = path_label+'_'+block_label+img_format
	# print(img_dir)
	# print(out_img_label)

	out_img_fullpath = os.path.join(img_dir, out_img_label)

	if (os.path.isfile(out_img_fullpath)):
		print('image EXISTS on disc, we will skip this path!')
		return 'skipThisImg' 

	else:
		print('\nimage is NOT on disc, so we will build this file!')
		print("saving output image as:")
		print(out_img_fullpath)

		# NOTE: plt.imsave() can not save neg. pixel values in an image!
		# plt.imsave(out_img_fullpath, in_arr_2d, cmap='gray', vmin=0, vmax=in_arr_2d.max())  # note: vmin=in_arr_2d.min() is wrong in this case (why???), cuz roughness array has many fill values with ranges in -99999, so vmin=0 to plot images in range [0,max)
		# plt.imsave(out_img_fullpath, in_arr_2d, cmap='gray', vmin=in_arr_2d.min(), vmax=in_arr_2d.max())  # note: vmin=in_arr_2d.min() might be right, cuz roughness array has many fill values with ranges in -99999, and we need these fillValues to plot background == [min,max]



		# #~ create PIL image from roughness array 2D w/ negative values
		# rough_img = Image.fromarray(in_arr_2d)
		# print('-> image mode: %s' % rough_img.mode)
		# #~ save roughness PIL image on disc; 
		# rough_img.save(out_img_fullpath)  # note: PIL is able to save neg. pixel values in .tif file
		# # rgb_rough_img.save(out_img_fullpath)


		#~ new way to write tif image w/negative values; 
		tifffile.imwrite(
						out_img_fullpath, 
						in_arr_2d,
						dtype='float64', 
						bigtiff=True,  # why bigTiff? we use bigTiff to be able to store large roughness values as float64 dtype in our rasters
						# compress='jpeg'
						)

		return out_img_fullpath

	#################################################
	# decided to include dark images in processing because they might be just land and we need them in our final plot
		
	# if (in_arr_2d.max() < 1):
	# 	print('-> PlotImgFunc(.): image is dark! We skip it.')
	# 	return 'skipThisImg'
	
	# else:

	# 	# img_format = ".jpg"  # does this format support saving neg- values?
	# 	# img_format = ".png"
	# 	img_format = ".tif"  # this image format supports saving neg- values in image


	# 	print('\n')
	# 	print('-> image array min= %d' % np.nanmin(in_arr_2d))
	# 	print('-> image array max= %d' % np.nanmax(in_arr_2d))

	# 	#~ now replace negative pixelValues w/ zero ---> (why???)
	# 	# in_arr_2d[in_arr_2d<0] = 0 # masks any element of np.array that has value < zero
	# 	# print('-> img array min= %d' % in_arr_2d.min())
	# 	# print('-> img array max= %d' % in_arr_2d.max())


	# 	out_img_label = path_label+'_'+block_label+img_format
	# 	# print(img_dir)
	# 	# print(out_img_label)

	# 	out_img_fullpath = os.path.join(img_dir, out_img_label)

	# 	if (os.path.isfile(out_img_fullpath)):
	# 		print('-> image EXISTS, we will skip this path!')
	# 		return 'skipThisImg' 

	# 	else:
	# 		print('\n-> image is NOT on disc, so we will go on with this path!')
	# 		print("-> saving output image as:")
	# 		print(out_img_fullpath)

	# 		# NOTE: plt.imsave() can not save neg. pixel values in an image!
	# 		# plt.imsave(out_img_fullpath, in_arr_2d, cmap='gray', vmin=0, vmax=in_arr_2d.max())  # note: vmin=in_arr_2d.min() is wrong in this case (why???), cuz roughness array has many fill values with ranges in -99999, so vmin=0 to plot images in range [0,max)
	# 		# plt.imsave(out_img_fullpath, in_arr_2d, cmap='gray', vmin=in_arr_2d.min(), vmax=in_arr_2d.max())  # note: vmin=in_arr_2d.min() might be right, cuz roughness array has many fill values with ranges in -99999, and we need these fillValues to plot background == [min,max]



	# 		# #~ create PIL image from roughness array 2D w/ negative values
	# 		# rough_img = Image.fromarray(in_arr_2d)
	# 		# print('-> image mode: %s' % rough_img.mode)
	# 		# #~ save roughness PIL image on disc; 
	# 		# rough_img.save(out_img_fullpath)  # note: PIL is able to save neg. pixel values in .tif file
	# 		# # rgb_rough_img.save(out_img_fullpath)


	# 		#~ new way to write tif image w/negative values; 
	# 		tifffile.imwrite(
	# 						out_img_fullpath, 
	# 						in_arr_2d,
	# 						dtype='float64', 
	# 						bigtiff=True,  # why bigTiff? we use bigTiff to be able to store large roughness values as float64 dtype in our rasters
	# 						# compress='jpeg'
	# 						)

	# 		return out_img_fullpath

########################################################################################################################
'''this f() builds image dir inside roughness dir'''
def img_dir_setup(arr2tiff_dir_path, georefRaster_dir_name):
	img_dir = os.path.join(arr2tiff_dir_path, georefRaster_dir_name) 
	if (os.path.isdir(img_dir)):
		print('image dir exists!')
		print(img_dir)
	else:
		print('image dir NOT exist. We will make it.')
		os.mkdir(img_dir)
		print(img_dir)
	return img_dir
########################################################################################################################
def reproject_to_polar(warpedFile_fullPath_noExt):
	target_epsg = 3995
	print('\nre-projecting to EPSG: %s' % target_epsg)
	file_extension = '.tif'
	input_raster = warpedFile_fullPath_noExt+file_extension
	output_raster_fullpath = warpedFile_fullPath_noExt+'_reprojToEPSG_'+str(target_epsg)+file_extension
	print(output_raster_fullpath)
	gdal.Warp(output_raster_fullpath, input_raster, dstSRS='EPSG:3995')
	print('\nfinal re-projected raster:')
	print(output_raster_fullpath)
	return 0
########################################################################################################################

def make_roughness_list_from_dir(rough_dir_fullpath):
	print(rough_dir_fullpath)

	#~ make a list of existing roughness files
	roughness_file_patern = 'roughness_toa_refl_P*'+'_O*'+'_B*'+'.dat'   # for ELLIPSOID data - check file names 
	print("looking for pattern: %s" %roughness_file_patern)

	#~ get a list of available/downloaded Ellipsoid files, the list will be list of file_fullpath-s 
	rough_files_fullpath_list = glob.glob(os.path.join(rough_dir_fullpath, roughness_file_patern))

	tot_found_files = len(rough_files_fullpath_list)
	print("files found: %d" %tot_found_files)
	#~ maybe split and sort?
	return rough_files_fullpath_list, tot_found_files
########################################################################################################################

def read_rough_file(rough_fname):
	#~ get only the roughness values from file, roughness file has roughness-lat-lon in it as a list & create 2D roughness array from list
	# rough_2d_arr = np.fromfile(rough_fname, dtype=np.double)[0:1048576].reshape((512,2048))			# 'double==float64'     # is this roughness in cm?
	rough_2d_arr = np.fromfile(rough_fname, dtype=np.double)[0:1048576].reshape((512,-1))			# 'double==float64'     # is this roughness in cm?
	#~ define NaN values
	rough_2d_arr[(rough_2d_arr < 0.0) & (rough_2d_arr != -999994.0)] = -99991.0 # NaN any negative pixels and keep positive pixels and land mask only

	#~ get some info about each array
#     print("-> input file elements: %d" % rough_latlon_arr.size)
#     print("-> input file dim: %d" % rough_latlon_arr.ndim)
#     print("-> input file shape: %s" % rough_latlon_arr.shape)
	print("roughnessArray new shape: (%d, %d)" % rough_2d_arr.shape)

	#~ find and extract path number
	path_num = rough_fname.split("/")[-1].split("_")[3][1:] # extract path chuncka and then etract just the number
	path_num = int(path_num)
	print("roughness-Path: %s" % (path_num))

	#~ find and extract block number
	block_num = rough_fname.split("/")[-1].split("_")[-1].split(".")[0]
	block_num = int(block_num[-2:])
	print("roughness-Block: %s" % (block_num))

	return rough_2d_arr, block_num, path_num
########################################################################################################################

def create_gcp_list_for_imgBlockPixels_fixedGCPs_skipAMcrossing(path_num, block_num, misr_res_meter, image_dir):

	'''index is useful here, so first we create a list from img coordinates from all img pixels
	then we use the index of each element in the pixel coord list as the index/count of each GCP'''

	print("transfer img frame -> latlon frame for path_num: %d, block_num: %s" %(path_num, block_num))
	#~ make 2 lists for img and geographic frames, then analyze long. to see if all are neg. or all are pos.; if all are the same then pass, else: change <-> to <+>
	img_pixelFrame_rowcol_list = []  # (row, column)
	img_worldFrame_latlon_list = []

	#~ based on suggestion from Anne, changed the range steps to every 128 pixels
	# for jrow in [0, 200, 400, 511]: # [127*0, 127*1, 127*2, 127*3, 127*4, ..., 511]
	# 	for icol in [0, 300, 600, 1000, 1300, 1700, 2047]: # [127*0, 127*1, 127*2, 127*3, 127*4, ..., 2047]
	distance_from_AM_line = 50; # degree
	row_list = []
	col_list = []
	
	##############################################################
	#- make a list of row numbers- old
	for num in range(20):
		row=num*128
		if(row<512):
			row_list.append(row)
	row_list.append(511)
	# print(row_list)

	#- make a list of col numbers
	for num in range(20):
		col=num*128
		if(col<2048):
			col_list.append(col)
	col_list.append(2047)
	# print(col_list)


	##############################################################
	# #- make a list of row numbers- includes all geographical mesh
	# print('calculating row-col for all geographical mesh!')
	# for row in range(512):
	# 	row_list.append(row)
	# # print(row_list)

	# #- make a list of col numbers
	# for col in range(2048):
	# 	col_list.append(col)
	# # print(col_list)
	
	##############################################################


	for jrow in row_list:
		for icol in col_list:
			#- process each row and col
			# print("\nprocessing coordinates for each pixel: (%d, %d)" %(jrow, icol))
	
			#~ use MTK function to map from img frame to geographic frame
			pixel_latlon_tuple = bls_to_latlon(path_num, misr_res_meter, block_num, jrow, icol) # struct=(lat, lon) depends on location of point in img frame, corner or center of a pixel
			# print(pixel_latlon_tuple)  # print every tuple of transfered latlon

			img_pixelFrame_rowcol_list.append([jrow, icol]) # rename to img_frame_...
			img_worldFrame_latlon_list.append(pixel_latlon_tuple) # rename to geographical(or world)_frame_...
			# print(pixel_latlon_tuple)


	print("total GCPs from img frame: %d" %len(img_pixelFrame_rowcol_list)) 
	gcp_numbers = len(img_pixelFrame_rowcol_list)

	# print(img_pixelFrame_rowcol_list)
	# print(img_worldFrame_latlon_list)  # print this to check all latlon list; tuple of (lat, lon)
	

	#- write out the latLon list
	# img_worldFrame_latlon_list.to_csv()
	import csv
	latLon_fileName = 'latLon_P'+str(path_num)+'_B'+str(block_num)+'.csv'
	latLon_file_name_fp = os.path.join(image_dir, latLon_fileName)
	with open (latLon_file_name_fp, 'w') as output_fp:
		write = csv.writer(output_fp)
		write.writerows(img_worldFrame_latlon_list)
		print('output latLon file:')
		print(latLon_file_name_fp)







	# df_latLon = pd.DataFrame(img_worldFrame_latlon_list)	
	# df_latLon.to_csv(latLon_file_name_fp, encoding='utf-8', index=False)
	# print('output latLon file:')
	# print(latLon_file_name_fp)


	min_long_in_img = min([ituple[1] for ituple in img_worldFrame_latlon_list]) # get lon from latlon tuple
	print('min long.: %s' % min_long_in_img)



	#~~ check all long. elements to have same sign, either +/-
	#~~ if any Lon in img_worldFrame_latlon_list is neg. we change that to pos. 
	if (any([pixel_LatLon_tuple[1] < 0  for  pixel_LatLon_tuple in img_worldFrame_latlon_list])):  ## all(iterable); iterable==list or anything that we can iterate on; Return True if all elements of the iterable are true
		''' we do this part if any OR all block pixels are on West hemisphere (have neg. long.) in img block, and we change them to pos. and the range will be [0, +360] '''
		print("found some neg. lon. in img_worldFrame_latlon_list!")
		if (all([pixel_LatLon_tuple[1] < 0  for  pixel_LatLon_tuple in img_worldFrame_latlon_list])):
			print('\nall pixels have negative lon. (West Hem.)!\n')
			#~ no conversion of lon. anymore
			gcp_list = []  # a list of ground control points
			for index, element in enumerate(img_pixelFrame_rowcol_list):    # index == each GCP ; element order == [row, col]
				#~ for each point we add GCPs to a list
				gcp_list.append(gdal.GCP())  # initialize GCP dataStruct for each coordinate point
				#~ X,Y in img frame
				gcp_list[index].GCPLine  = img_pixelFrame_rowcol_list[index][0]       # y == row == line
				gcp_list[index].GCPPixel = img_pixelFrame_rowcol_list[index][1]       # x == column == pixel
				#~ we add lat and lon to list
				gcp_list[index].GCPY = img_worldFrame_latlon_list[index][0]  # lat=northing 
				gcp_list[index].GCPX = img_worldFrame_latlon_list[index][1]
			
			#~# shows that block does NOT cross A.M.
			antimaridina_crossing = False

		elif ((180-abs(min_long_in_img))<distance_from_AM_line):
			#~ for any block that crosses AM line, we just label that block as AM crossing 
			print('\nimage crosses A.M. line! \n')
			gcp_list = []  # a list of ground control points

			for index, element in enumerate(img_pixelFrame_rowcol_list):    # index == each GCP ; element order == [row, col]
				#~ for each point we add GCPs to a list
				gcp_list.append(gdal.GCP())  # initialize GCP dataStruct for each coordinate point
				#~ X,Y in img frame
				gcp_list[index].GCPLine  = img_pixelFrame_rowcol_list[index][0]       # y == row == line
				gcp_list[index].GCPPixel = img_pixelFrame_rowcol_list[index][1]       # x == column == pixel	
				#~ 1st we add lat to list
				gcp_list[index].GCPY = img_worldFrame_latlon_list[index][0]  # lat=northing
				#~ 2nd check for long.; note= this section is as a result of any() meaning that we have 2 scenarios: some long. are neg./pos. or all are neg.
				pixel_long = img_worldFrame_latlon_list[index][1]
				# print("-> found neg. pixel long. in the crossing block: %f" % pixel_long)
				#~ update neg. long. to pos. long.

				if ( pixel_long < 0 ):  # if we find neg. long., we change it to pos. long. and will be in range [0, +360], especially case is all in neg. side of A.M.
					# print("-> neg. pixel long. in block: %f" % pixel_long)
					updated_pixel_lon = (360.0 + pixel_long )  # this long. is changed to pos. and will be in range [0, +360]
					# print("=> updated to range 0-360 to: %f" %updated_pixel_lon)
					#~ update neg. long. to pos. long.
					gcp_list[index].GCPX = updated_pixel_lon
				else:  # here we collect some pos. long. among neg. long. in the case block is crossing A.M. line
					# print("-> found pos. long. among neg. longs. in the crossing block: %f" %img_worldFrame_latlon_list[index][1]
					#~ no need to update Lon. since it is pos. Lon.
					gcp_list[index].GCPX = pixel_long

			#~# shows that block is crossing A.M. & in the code we will skip this img block
			antimaridina_crossing = True  

		else:
			print('\n--------> image crosses P.M. line! \n')
			#~ for PM crossing we do NOT convert any lon. in the image, meaning we combine negative and positive lon. on both sides of PM line
			gcp_list = []  # a list of ground control points
			for index, element in enumerate(img_pixelFrame_rowcol_list):    # index == each GCP ; element order == [row, col]
				#~ for each point we add GCPs to a list
				gcp_list.append(gdal.GCP())  # initialize GCP dataStruct for each coordinate point
				#~ X,Y in img frame
				gcp_list[index].GCPLine  = img_pixelFrame_rowcol_list[index][0]       # y == row == lat
				gcp_list[index].GCPPixel = img_pixelFrame_rowcol_list[index][1]       # x == column == pixel
				#~ we add lat and lon to list
				gcp_list[index].GCPY = img_worldFrame_latlon_list[index][0]  # lat=northing 
				gcp_list[index].GCPX = img_worldFrame_latlon_list[index][1]
			
			#~~ shows that block does NOT cross A.M.
			antimaridina_crossing = False
			#~~ make lit of latitudes
			# if (min( [pixel_LatLon_tuple[0] for pixel_LatLon_tuple in img_worldFrame_latlon_list] ) >= 75.0): # list comprehension to find the min(lat) in block
			# block_lat_list = [pixel_LatLon_tuple[0] for  pixel_LatLon_tuple in img_worldFrame_latlon_list]
			# min_lat_in_block = min(block_lat_list)
			# print('-> min-lat in block: %s' %min_lat_in_block)

	else:
		print("\nall pixels have positive lon. (East Hem.)!\n")
		gcp_list = []  # a list of ground control points
		for index, element in enumerate(img_pixelFrame_rowcol_list):    # index == each GCP ; element order == [row, col]
			#~ for each point we add GCPs to a list
			gcp_list.append(gdal.GCP())  # initialize GCP dataStruct for each coordinate point
			#~ X,Y in img frame
			gcp_list[index].GCPLine  = img_pixelFrame_rowcol_list[index][0]       # y == row
			gcp_list[index].GCPPixel = img_pixelFrame_rowcol_list[index][1]       # x == column == pixel
			#~ add lat&lon
			gcp_list[index].GCPY = img_worldFrame_latlon_list[index][0]  # lat=northing 
			gcp_list[index].GCPX = img_worldFrame_latlon_list[index][1]  # lon=easting
		#~~ shows that block does NOT cross A.M.
		antimaridina_crossing = False


	return gcp_list, len(img_pixelFrame_rowcol_list), gcp_numbers, antimaridina_crossing

########################################################################################################################

# def create_gcp_list_for_imgBlockPixels_mostGCPs(path_num, block_num, misr_res_meter):
# 	#~ create list of GCPs 
# 	gcp_step = 100
# 	print('\n-> GCP steps: %d \n' % gcp_step)
# 	'''index is useful here, so first we create a list from img coordinates from all img pixels
# 	then we use the index of each element in the pixel coord list as the index/count of each GCP'''

# 	print("-> transfer img frame -to- latlon frame for path_num: %d, block_num: %s" %(path_num, block_num))
# 	#~ make 2 lists for img and geographic frames, then analyze long. to see if all are neg. or all are pos.; if all are the same then pass, else: change <-> to <+>
# 	img_pixelFrame_rowcol_list = []  # (row, column)
# 	img_worldFrame_latlon_list = []

# 	for jrow in range(0, 512, gcp_step):
# 		for icol in range(0, 2048, gcp_step):

# 			#~ process each col of each row
# 			# print("\n-> processing img coords: (%d, %d)" %(jrow, icol))
	
# 			#~ use MTK function to map from img frame to geographic frame
# 			pixel_latlon_tuple = bls_to_latlon(path_num, misr_res_meter, block_num, jrow, icol) # struct=(lat, lon)?
# 			# print(pixel_latlon_tuple)  # print every tuple of transfered latlon

# 			img_pixelFrame_rowcol_list.append([jrow, icol])
# 			img_worldFrame_latlon_list.append(pixel_latlon_tuple)

# 	print("-> total GCPs from img frame: %d" %len(img_pixelFrame_rowcol_list))    
# 	# print(img_pixelFrame_rowcol_list)
# 	# print(img_worldFrame_latlon_list)  # print this to check all latlon list
# 	gcp_numbers = len(img_worldFrame_latlon_list)

# 	min_long_in_img = min([i[1] for i in img_worldFrame_latlon_list])
# 	print('-> min long.: %s' % min_long_in_img)

# 	#~ check all elements of long. to have same sign, either +/-
# 	# all(iterable); iterable==list or anything that we can iterate on; Return True if all elements of the iterable are true
# 	if (any( [pixel_LatLon_tuple[1] < 0  for  pixel_LatLon_tuple in img_worldFrame_latlon_list] )):   # if any Lon in img_worldFrame_latlon_list is neg. we change that to pos.
# 		''' we do this part if any OR all block pixels are on West hemisphere (have neg. long.) in img block, and we change them to pos. and the range will be [0, +360] '''

# 		print("-> found some neg. lon. in img_worldFrame_latlon_list!")

# 		if (all([pixel_LatLon_tuple[1] < 0  for  pixel_LatLon_tuple in img_worldFrame_latlon_list])):

# 			print('-> West Hem. all negative lon! we change all to range: [0-360]')
# 			#~ convert W-> E: all lon. on W.H. side, change to range [0, 360] 
# 			gcp_list = []  # a list of ground control points

# 			for index, element in enumerate(img_pixelFrame_rowcol_list):    # index == each GCP ; element order == [row, col]

# 				#~ for each point we add GCPs to a list
# 				gcp_list.append(gdal.GCP())  # initialize GCP dataStruct for each coordinate point

# 				#~ X,Y in img frame
# 				gcp_list[index].GCPLine  = img_pixelFrame_rowcol_list[index][0]       # y == row
# 				gcp_list[index].GCPPixel = img_pixelFrame_rowcol_list[index][1]       # x == column == pixel
				
# 				#~ do this section for blocks that pass AntiMeridian, and we change all neg. long. to pos. long. that will span [0, 360] range 
				
# 				#~ 1st we add lat to list
# 				gcp_list[index].GCPY = img_worldFrame_latlon_list[index][0]  # lat=northing 

# 				#~ 2nd check for long. note; this section is as a result of any() meaning that we have 2 scenarios: some long. are neg./pos. or all are neg.
# 				pixel_long = img_worldFrame_latlon_list[index][1]
				
# 				# print("-> found neg. pixel long. in the crossing block: %f" % pixel_long)

# 				updated_pixel_lon = (360.0 + pixel_long )  # this long. is changed to pos. and will be in range [0, +360]
# 				# print("=> update: neg.long. updated to: %f" %updated_pixel_lon)

# 				#~ update neg. long. to pos. long.
# 				gcp_list[index].GCPX = updated_pixel_lon
# 				antimaridina_crossing = False


# 		elif ((180-abs(min_long_in_img))<50):
# 			#~ for any block that crosses AM line, we change negative lon. to positive lon. to map to range [0, 360] in that image
# 			print('-> image crosses A.M. line!')
# 			gcp_list = []  # a list of ground control points

# 			for index, element in enumerate(img_pixelFrame_rowcol_list):    # index == each GCP ; element order == [row, col]

# 				#~ for each point we add GCPs to a list
# 				gcp_list.append(gdal.GCP())  # initialize GCP dataStruct for each coordinate point

# 				#~ X,Y in img frame
# 				gcp_list[index].GCPLine  = img_pixelFrame_rowcol_list[index][0]       # y == row
# 				gcp_list[index].GCPPixel = img_pixelFrame_rowcol_list[index][1]       # x == column == pixel
								
# 				#~ 1st we add lat to list
# 				gcp_list[index].GCPY = img_worldFrame_latlon_list[index][0]  # lat=northing 

# 				#~ 2nd check for long. note; this section is as a result of any() meaning that we have 2 scenarios: some long. are neg./pos. or all are neg.
# 				pixel_long = img_worldFrame_latlon_list[index][1]
# 				# print("-> found neg. pixel long. in the crossing block: %f" % pixel_long)
# 				#~ update neg. long. to pos. long.

# 				if ( pixel_long < 0 ):  # if we find neg. long., we change it to pos. long. and will be in range [0, +360], especially case is all in neg. side of A.M.
# 					print("-> neg. pixel long. in block: %f" % pixel_long)

# 					updated_pixel_lon = (360.0 + pixel_long )  # this long. is changed to pos. and will be in range [0, +360]
# 					print("=> updated to range 0-360 to: %f" %updated_pixel_lon)

# 					#~ update neg. long. to pos. long.
# 					gcp_list[index].GCPX = updated_pixel_lon

# 				else:  # here we collect some pos. long. among neg. long. in the case block is crossing A.M. line
					
# 					# print("-> found pos. long. among neg. longs. in the crossing block: %f" %img_worldFrame_latlon_list[index][1])

# 					#~ no need to update Lon. since it is pos. Lon.
# 					gcp_list[index].GCPX = pixel_long
# 					antimaridina_crossing = True

# 		else:
# 			print('-> image crosses P.M. line!')
# 			#~ for PM crossing we do NOT convert any lon. in the image, meaning we combine negative and positive lon. on both sides of PM line
# 			gcp_list = []  # a list of ground control points

# 			for index, element in enumerate(img_pixelFrame_rowcol_list):    # index == each GCP ; element order == [row, col]

# 				#~ for each point we add GCPs to a list
# 				gcp_list.append(gdal.GCP())  # initialize GCP dataStruct for each coordinate point

# 				#~ X,Y in img frame
# 				gcp_list[index].GCPLine  = img_pixelFrame_rowcol_list[index][0]       # y == row
# 				gcp_list[index].GCPPixel = img_pixelFrame_rowcol_list[index][1]       # x == column == pixel
				
# 				#~ we add lat and lon to list
# 				gcp_list[index].GCPY = img_worldFrame_latlon_list[index][0]  # lat=northing 
# 				gcp_list[index].GCPX = img_worldFrame_latlon_list[index][1]
# 				antimaridina_crossing = True
# 				#~ what if we change negative lon. to range [0-360]?

# 	else:
# 		print("-> East Hem.: all positive lon.!")

# 		gcp_list = []  # a list of ground control points

# 		for index, element in enumerate(img_pixelFrame_rowcol_list):    # index == each GCP ; element order == [row, col]

# 			#~ for each point we add GCPs to a list
# 			gcp_list.append(gdal.GCP())  # initialize GCP dataStruct for each coordinate point
# 			# print('-> processing list element: %d' % index)
# 			#~ X,Y in img frame
# 			gcp_list[index].GCPLine  = img_pixelFrame_rowcol_list[index][0]       # y == row
# 			gcp_list[index].GCPPixel = img_pixelFrame_rowcol_list[index][1]       # x == column == pixel

# 			#~ add lat&lon
# 			gcp_list[index].GCPY = img_worldFrame_latlon_list[index][0]  # lat=northing 
# 			gcp_list[index].GCPX = img_worldFrame_latlon_list[index][1]  # lon=easting
# 			antimaridina_crossing = False
# 			# print(len(gcp_list))
# 			# print(type(gcp_list))

# 		# print(gcp_list)

# 	return gcp_list, len(img_pixelFrame_rowcol_list), antimaridina_crossing, gcp_numbers

########################################################################################################################

def apply_gcp(path_label, block_label, image_dir, in_ds, gcp_list):
	#~ gdal.translate to convert data set formats (how input data from memory???)
	#~ write/save image array into raster ??????

	translated_img = 'translated_'+path_label+'_'+block_label+'.tif'
	out_translated_img_fullpath = os.path.join(image_dir, translated_img)

	#~ define Translate Options --> we don't need to create a TranslateOptions object, we only need do define anything as an keyword_argument for the .Translate()
	#~ writes translated output to a .tif file and returns a gdal.Dataset object; after writing it, translate_ds will be empty --> how do it VRT???
	translate_ds = gdal.Translate(
									out_translated_img_fullpath,
									in_ds,  
									format = 'GTiff',
									outputType = gdal.GDT_Float64, 	# note: inout dtype is float_64==double, maybe here change dtype to make it smaller img????
									GCPs = gcp_list,
									noData = -99.9

								) 

	# print(type(ds_obj))

	#~ Properly close the datasets to flush to disk
	translate_ds = None
	in_ds = None

	return out_translated_img_fullpath
########################################################################################################################

def warp_img(path_label, block_label, total_gcps, image_dir, translated_img_fullpath, gcp_numbers):
	'''
	#~ note: commandLine implementation of gdalwarp, since < --config CENTER_LONG > is not an option for the gdal.Warp() lib. function
	#~ note: CENTER_LONG +180 maps from [0,-/+180] to [0,360] all will be positive lon.
	'''
	out_dtype = 'Float64'
	warped_img_tag = 'raster_'+path_label+'_'+block_label+'_'+str(total_gcps)+'GCPs_noGdalRes_'+'dType'+out_dtype+'_'+str(gcp_numbers)+'gcps'+'_latlon'
	warped_img_extension = '.tif'
	warped_img = warped_img_tag+warped_img_extension
	output_file_warped = os.path.join(image_dir, warped_img)
	print('\noutput_file_warped:')
	print(output_file_warped)
	# print(os.path.isfile(translated_img_fullpath))
	# print(os.path.isfile(output_file_warped))
	warp_ds = gdal.Warp(
							output_file_warped,
							translated_img_fullpath,
							format = 'GTiff',
							srcSRS = "EPSG:4326",
							dstSRS = "EPSG:4326",
							outputType = gdal.GDT_Float64,
							resampleAlg = 'max', 		            # resampling method; should correlate w/ img type (tif, jpg, or png)? looks like works better w/jpg (how about .tif?)
							srcNodata = -99.0,  # define noData values=-99 in input files; igniores x in src dataset			# for .tif: bilinear= bad; cubicspline= bad; nearest= bad; cubic= bad; average= bad; max= is better
							dstNodata = -99.0  	# in the case of NaN?
						)
	#~~ close datasets in memory
	warp_ds = None
	translated_img_fullpath = None
	#~ we return this file to use it later to reproject it to Polar
	warpedFile_fullPath_noExt = os.path.join(image_dir, warped_img_tag)

	return warpedFile_fullPath_noExt

########################################################################################################################
######################################################## old methoid w/ cmdLine utility		
	# print('-> to cmdLine...')

	# cmd = 'gdalwarp\
	# 	-ot Float64 \
	# 	-of "GTiff" \
	# 	-r bilinear \
	# 	-s_srs "EPSG:4326" \
	# 	-t_srs "EPSG:4326" \
	# 	-overwrite \
	# 	-tps \
	# 	-co TILED=YES \
	# 	--config CENTER_LONG +180 \
	# 	-srcnodata 0 \
	# 	-dstnodata 0 ' + translated_img_fullpath + ' ' + output_file_warped  
	
	#~ notes: 
	# some toher projection????
	# -tr 0.001 0.001 --> target res: this makes output tif file large, so we use default resolution
	# output raster dtype= Float64, not double or float64

	# print('-> bottleneck here?! \n')
	# #~ note: shell=true to execute the line as full command including arguments
	# return_code = call(cmd, shell=True)  # subprocess.call() is better/safer that os.system() ==> finally used os.system() cuz call returned error! correct return code?      
	# # stat = os.system(input_str_args)  

	# print("\n-> gdalWarp subprocess call return stat: %d" %return_code)
	# print('-> output warped file: %s' % output_file_warped)
	# print("\n-> OK: GdalWarp FINISHED SUCCESSFULLY! \n")

	# return warpedFile_fullPath_noExt
########################################################################################################################

if __name__ == '__main__':
	
	start_time = dt.datetime.now()
	print('start time: %s' %start_time)
	print('\n\n')
	main()
	end_time = dt.datetime.now()
	print('\n\n')
	print('end time= %s' %end_time)
	print('runtime= %s' %(end_time-start_time))
	print(" ")
	print('######################## TOA COMPLETED SUCCESSFULLY ########################')

########################################################################################################################







