#!/usr/bin/python2.7

import numpy as np
import os, glob
from osgeo import gdal 		# built w/python2.7 - installed in: /usr/local/Cellar/gdal
import matplotlib
from matplotlib import pyplot as plt
import PIL
from PIL import Image
from datetime import datetime
import matplotlib.cm as cm
import matplotlib as mpl
from matplotlib.colors import ListedColormap


print(matplotlib.__version__)
print(gdal.VersionInfo())

########################################################################################################################
raster_dir = "/media/ehsan/6T_part1/14528_apr2016/toa_refl_april_2016_9cam4bands_day1_30_p1_233_b1_46/toa_files_in_range_2016_4_25/rasters_noDataNeg99_TiffFileFloat64_max"

#~~ file pattern that we will look for
raster_pattern='mosaic_*'+'.tif'
# raster_pattern='raster_path*'+'.tif'

#~~ dir where we will save plots
save_plot_dir = raster_dir
########################################################################################################################
raster_pattern_fullpath=os.path.join(raster_dir, raster_pattern)
#~~ make list of rasters in dir
rasters_fp_list=glob.glob(raster_pattern_fullpath)
#~~ check if directories exist
if (os.path.isdir(raster_dir)==False):
	print('raster dir. NOT found!')
	raise SystemExit()
elif (os.path.isdir(save_plot_dir)==False):
	print('save plot dir. NOT found!')
	raise SystemExit()
elif (len(rasters_fp_list)==0):
	print('no raster found!')
	raise SystemExit()
else:
	#~~ now everything is fine and we process each raster and finally plot it and save it on a directory
	print('rasters found: (%s)' %len(rasters_fp_list))
	print('raster.tif (mosaic or simple raster) files found...')
	for iraster in rasters_fp_list:
		print(iraster)
	
	for raster_fp in rasters_fp_list:
		#~~ split name to get date-tag
		print('\nparsing: %s' %raster_fp)
		raster_date_tag=raster_fp.split('/')[-1].split('_')[-1].split('.')[0]  # this is setup for mosaic files
		saved_img_name = 'allPaths_'+raster_date_tag+'.jpg'
		print('saved img label will be: %s' % saved_img_name)

		#~~ raster2array: read data from raster in to array
		dataset = gdal.Open(raster_fp)
		# print('-> dataset type: %s' %type(dataset))
		#~~ 
		raster = dataset.GetRasterBand(1)
		#~~ build metadata
		metadata = {}
		# print(type(metadata))

		metadata['array_rows'] = dataset.RasterYSize
		metadata['array_cols'] = dataset.RasterXSize

		print('rows: %s' % metadata['array_rows'])
		print('columns: %s' % metadata['array_cols'])

		geotransform = dataset.GetGeoTransform()
		print(geotransform) # note: units in meters
		print('order of geoTransform matrix: topleftX, pixelW, 0, topleftY, 0, pixelH')

		xMin = geotransform[0]
		xMax = geotransform[0] + dataset.RasterXSize*geotransform[1]        # i changed it to * to get xMax value
		yMin = geotransform[3] + dataset.RasterYSize*geotransform[5]
		yMax = geotransform[3]
		print('\n')
		print(xMin)
		print(xMax)
		print(yMin)
		print(yMax)
		print('\n')
		# metadata['extent'] = (xMin,xMax,yMin,yMax)
		plot_extent = (xMin,xMax,yMin,yMax)
		# plot_extent = (-3200000,3200000,-3200000,3200000)
		print(plot_extent)


		# array_shape = raster.ReadAsArray(0,0, metadata['array_cols'], metadata['array_rows']).astype('byte').shape # check this f(.)
		array_shape = raster.ReadAsArray(0,0, metadata['array_cols'], metadata['array_rows']).astype('float64').shape # check this f(.)

		print(array_shape)









		

		# rough_arr = np.zeros((array_shape[0], array_shape[1], dataset.RasterCount), dtype='byte') # check f(.) arguments
		rough_arr = np.zeros((array_shape[0], array_shape[1], dataset.RasterCount), dtype='float64') # check f(.) arguments

		print(rough_arr.dtype)
		print(rough_arr.min())
		print(rough_arr.max())

		# rough_arr = raster.ReadAsArray(0, 0, metadata['array_cols'], metadata['array_rows']).astype('byte')
		rough_arr = raster.ReadAsArray(0, 0, metadata['array_cols'], metadata['array_rows']).astype('float64')

		# print(type(rough_arr))
		# print(rough_arr.dtype)

		print('array shape: (%s, %s)' % rough_arr.shape)
		print('roughness min: %s' % rough_arr.min())
		print('roughness max: %s' % rough_arr.max())

		################################################################################################################
		#~ filter roughness array
		#~ make a copy of roughness array to use for filter
		roughness = np.copy(rough_arr)

		print(np.nanmin(roughness))
		print(np.nanmax(roughness))

		roughness[rough_arr <= 0.0] = np.nan  # mask neg. values and keep +values; to extract positive rough values; 

		print('min roughness: %s' % np.nanmin(roughness))
		print('max roughness: %s' % np.nanmax(roughness))
		print(np.count_nonzero(roughness[roughness > 0]))
		# print(type(roughness))

		#~ filter for land mask
		# land_mask = rough_arr  # make copy of rough_arr; not good cuz if we modify rough_arr ==> land_mask will change
		land = np.copy(rough_arr)  # right way to copy array 
		print('min: %s' % np.nanmin(land))
		print('max: %s' % np.nanmax(land))

		land[rough_arr != -999994.0] = np.nan  # keep -999994 and masks everything.

		print('landMask min: %s' % np.nanmin(land))
		print('landMask max: %s' % np.nanmax(land))
		print(np.count_nonzero(land[land == -999994.0]))
		# plt.imshow(land, cmap='gray')

		#~ filter for noData (rest of data)
		nodata = np.copy(rough_arr)  # right way to copy array 
		print('min: %s' % np.nanmin(nodata))
		print('max: %s' % np.nanmax(nodata))

		nodata[rough_arr != -99.0] = np.nan  # -99.0 is signal/flag for noData

		print('noData min: %s' % np.nanmin(nodata))
		print('noData max: %s' % np.nanmax(nodata))

		# plt.imshow(nodata, origin='lower', cmap='gray')

		#~ np.where with 2 conditions for noData???
		# nodata_mask = np.where((rough_arr >= 0.0) & (rough_arr==-999994.0), np.nan, -9.)
		# print(nodata_mask)


		''' colormap is used to map data to colors == colorcoded data'''

		#~~ seaice roughness colormap
		blues_for_seaice = cm.get_cmap('Blues', 100)
		seaice_color = ListedColormap(blues_for_seaice(np.linspace(0.1, 1, 100)))
		SIR_colormap = plt.cm.get_cmap(seaice_color, 10)
		SIR_colormap.set_over('red')  # to set anything onder zero==black
		# SIR_colormap.set_under('black')

		#~~ landcolormap
		land_cmap = cm.get_cmap('gray', 100)
		# print(land_cmap)
		land_color = ListedColormap(land_cmap(np.linspace(0.2, 0.25, 100)))  # should be in range: [0, 1]
		# print(land_color)  # this is the new sequential colormap and we will use the colorcode of our defined/extracted range from the full colormap

		#~~ noData colormap
		grays_for_nodata = cm.get_cmap('gray', 100)
		nodata_color = ListedColormap(grays_for_nodata(np.linspace(0.0, 0.01, 100)))

		#~ inspired by: https://matplotlib.org/3.2.1/tutorials/colors/colormap-manipulation.html
		################################################################################################################
		#~~ build a figure and an axis
		# (fig, ax) = plt.subplots(figsize=(8, 6), dpi=600, sharex=True, sharey=True)  # returns one figure==canvas and we can have multiple subplots==axes; has more features than plt.subplot(); figure != plot = image; e.g: fig, (ax1, ax2) = plt.subplots(nrows=2, figsize=(6, 5.4))
		(fig, ax) = plt.subplots();  # returns one figure==canvas and we can have multiple subplots==axes; has more features than plt.subplot(); figure != plot = image; e.g: fig, (ax1, ax2) = plt.subplots(nrows=2, figsize=(6, 5.4))

		# print(type(fig))
		# print(type(ax))
		'''
		origin='lower'
		normalization=None == not linear scale data into [lowest=0,highest=1] & we pick the data range to plot [0,100]
		'''
		#~~ try set figsize
		# fig.set_size_inches(8,6, forward=True)  # + dpi later in plt.savefig()
		# plt.subplots_adjust(0.5,0.5,1,1) 
		fig.set_figheight(6)
		fig.set_figwidth(6)


		#~~ transpose data to solve flipping issue that imshow() caused: https://github.com/bokeh/bokeh/issues/1666
		roughness=roughness.T
		land=land.T
		nodata=nodata.T

		img1 = ax.imshow(roughness, cmap=SIR_colormap , origin='lower', interpolation='none', extent=plot_extent, norm=None, vmin=0.0, vmax=200, aspect='auto');
		img2 = ax.imshow(land, cmap= land_color, origin='lower', interpolation='none', extent=plot_extent, aspect='auto');
		img3 = ax.imshow(nodata, cmap=nodata_color, origin='lower', interpolation='none', extent=plot_extent, aspect='auto');

		#~ colorbar
		cmap_title = 'Sea Ice Roughness (cm)'
		cbar = fig.colorbar(img1, extend='max', aspect=40, fraction=0.012, pad=0.02);  # extend shows the range that goes beyond max limit on plot
		cbar.set_label(cmap_title, rotation=90, labelpad=10, fontsize=5);
		cbar.ax.tick_params(labelsize=5)
		cbar.outline.set_visible(False)

		# plt.title('Sea Ice Roughness over the Arctic'); 
		ax.set_title('MISR-Estimated Sea Ice Roughness for ' + raster_date_tag, fontsize=8);

		#~ axis
		# ax = plt.gca();  # get-current-axis
		ax.ticklabel_format(useOffset=False, style='plain', size=10); # do not use scientific notation 
		rotatexlabels = plt.setp(ax.get_xticklabels(), rotation=90); # rotate x tick labels 90 degrees

		#~ axis limits
		xstart, xend = ax.get_xlim()
		xstepsize = abs(xend-xstart)/6
		ax.xaxis.set_ticks(np.arange(xstart, xend, xstepsize))

		# plt.xlim(xmin=0)


		ystart, yend = ax.get_ylim()
		ystepsize = abs(yend-ystart)/6
		ax.yaxis.set_ticks(np.arange(ystart, yend, ystepsize))

		#~ axis font size
		plt.xticks(fontsize=3)
		plt.yticks(fontsize=3)

		#~ grid
		ax.grid(True, which='both', axis='both', color='white', linewidth=0.05)

		#~ x and y labels
		plt.xlabel('Longitude (m)', fontsize=5)
		plt.ylabel('Latitude (m)', fontsize=5)


		#~ path to save directory
		saved_img_fullpath = os.path.join(save_plot_dir, saved_img_name)
		print('\n')
		print('START saving image on disc!\n')
		plt.savefig(saved_img_fullpath, bbox_inches='tight', pad_inches=0.2, dpi=600)  # dpi=dots-per-inch
		print(saved_img_fullpath)
		print('\nFINISHED saving image on disc!')
		print('\n')
		print('********************************************************************************************************')
		# plt.show()     # should come after plt.savefig()

		plt.close()    # close the figure window; plt.close(fig) if we created by:  fig, ax = plt.subplots()

		#~~ we close our opened GDAL dataset properly after we are finished
		dataset = None
		

		# inspired by: 
		# https://www.neonscience.org/resources/learning-hub/tutorials/mask-raster-py
		# https://www.neonscience.org/resources/learning-hub/tutorials/neon-hsi-aop-functions-tiles-py
		################################################################################################################
print('code finished successfully!')













