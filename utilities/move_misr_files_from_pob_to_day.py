#!/usr/bin/env python

# import numpy as np
import os, glob
import MisrToolkit as Mtk
from MisrToolkit import * 
import shutil

'''
usage: 
	this program builds folder for a single day of MISR (hdf)/roughness files 
	and then moves files (hdf or roughness) that belong to each day/date to its folder.
	useful to use on Pronghorn

steps:
step-1: setup src dir= directory that includes either HDF or roughness files
step-2: setup time period of files: year and month of HDF or roughness files that you want to move
step-3: select type/file name pattern from the list

	1st, set the number of days that you have data available for.

note:
	we use a python installation that can run MTK library to do this task. (or a virtual env. that we can run MTK) 
	MTK lib will find associated day/time from orbit data on file labels.

'''
#***************************************************************************
#-- step-1
src_dir_fp =  '/data/gpfs/assoc/misr_roughness/2016/new_hdf_files_9cams/month2_2016_9cams'

#***************************************************************************
#-- step-2 
year = 2016
month = 7				# april=4, july=7
num_of_days = 30  		# this is the end-date of processing/ should include the whole time period

#***************************************************************************
#-- step-3 
process_mode_num = 1  # index from 0

process_mode_list = ['move_roughness_files_to_subdir', \
					'move_MISR_AM1_GRP_ELLIPSOID_GM_to_sibdir', \
					'check_hdf_order_dates', \
					'move_wrong_hdf_files_to_subdir'] # the last one is turned off (why?)

process_mode = process_mode_list[process_mode_num]

############################################################################
#-- proicessing section --->>> do not change

if (process_mode == 'move_roughness_files_to_subdir'):
	#~ loop through all 16 days and move files for each day to its roughness subdir
	for iday in range(num_of_days):

		day = iday+1
		print('\nprocessing day= %d \n' %day)
		start_time = str(year)+'-'+str(month)+'-'+str(day)+'T00:00:00Z'
		end_time = str(year)+'-'+str(month)+'-'+str(day)+'T23:59:59Z'
		print(start_time)
		print(end_time)

		orbit_list = Mtk.time_range_to_orbit_list(start_time, end_time)
		print(orbit_list)
		print('\nfound %d orbits!' %len(orbit_list))

		rough_subdir_name = 'roughness_subdir_'+str(year)+'_'+str(month)+'_'+str(day)
		print(rough_subdir_name)

		rough_subdir_fullpath = os.path.join(src_dir_fp, rough_subdir_name)
		print(rough_subdir_fullpath)

		#~ check if directory for specific day exists, else we create the directory
		if (not (os.path.isdir(rough_subdir_fullpath))):
			print('roughness subdir does NOT exist! We make that directory.')
			os.mkdir(rough_subdir_fullpath)
			print(rough_subdir_fullpath)
			print(os.path.isdir(rough_subdir_fullpath))
		else:
			print('roughness subdir exists!')
			print(rough_subdir_fullpath)
			print(os.path.isdir(rough_subdir_fullpath))



		## make list of all available roughness file patterns for specific day
		for orbit in orbit_list:
			print('processing orbit= %d' %orbit)
			#~ file pattern that we will use 
			file_pattern = 'roughness_toa_refl_P*'+'*_O0'+str(orbit)+'*'+'.dat'
			print('looking for pattern= %s' %file_pattern)
			#~ search for file pattern and make a list
			roughness_files_found_list = glob.glob(os.path.join(src_dir_fp, file_pattern))
			print('roughness files found: %s' % len(roughness_files_found_list))
		#     print(roughness_files_found_list)

			for rough_file_day in roughness_files_found_list:
				print('moving data to subdir for: %s ' %rough_file_day)
				new_path = shutil.move(rough_file_day, rough_subdir_fullpath)

##----------------------------------------------------------------------------------------------------------------------
## what it does:
'''
this part moves each HDF file to its subdirectory == date
'''


if (process_mode == 'move_MISR_AM1_GRP_ELLIPSOID_GM_to_sibdir'):
	#~ loop through all 16 days and move files for each day to its roughness subdir
	for iday in range(0, num_of_days, 1):

		day = iday+1
		print('\n\nprocessing day= %d \n' %day)
		start_time = str(year)+'-'+str(month)+'-'+str(day)+'T00:00:00Z'
		end_time = str(year)+'-'+str(month)+'-'+str(day)+'T23:59:59Z'
		print(start_time)
		print(end_time)

		orbit_list = Mtk.time_range_to_orbit_list(start_time, end_time)
		print(orbit_list)
		print('\nfound %d orbits!' %len(orbit_list))

		misr_subdir_name = 'GRP_ELLIPSOID_GM_subdir_'+str(year)+'_'+str(month)+'_'+str(day)
		print(misr_subdir_name)

		hdf_subdir_fullpath = os.path.join(src_dir_fp, misr_subdir_name)
		print(hdf_subdir_fullpath)

		#~ check if directory for specific day exists, else we create the directory
		if (not (os.path.isdir(hdf_subdir_fullpath))):
			print('GRP_ELLIPSOID_GM subdir does NOT exist! We make that directory.')
			os.mkdir(hdf_subdir_fullpath)
			print(hdf_subdir_fullpath)
			print(os.path.isdir(hdf_subdir_fullpath))
		else:
			print('GRP_ELLIPSOID_GM subdir exists!')
			print(hdf_subdir_fullpath)
			print(os.path.isdir(hdf_subdir_fullpath))



		## make list of all available roughness file patterns for specific day
		for orbit in orbit_list:
			print('processing for orbit= %d' %orbit)
			#-- file pattern that we will use to move files
			file_pattern = 'MISR_AM1_GRP_ELLIPSOID_GM_P*'+'*_O0'+str(orbit)+'*'+'.hdf'
			print('looking for pattern= %s' %file_pattern)
			#-- search for file pattern and make a list
			hdf_files_found_list = glob.glob(os.path.join(src_dir_fp, file_pattern))
			print('hdf files found: %s' % len(hdf_files_found_list))
		#     print(hdf_files_found_list)

			for hdf_file_day in hdf_files_found_list:
				print('moving hdf file to its subdir for: %s ' %hdf_file_day)
				new_path = shutil.move(hdf_file_day, hdf_subdir_fullpath)

##----------------------------------------------------------------------------------------------------------------------
#-- check period of hdf files in a dir- if it is not in the period, files will be moved to a subdir
''' 
this part checks each hdf file to be in our desired date (for example: April 2016)
'''

if (process_mode == 'check_hdf_order_dates'):
	print('process mode: %s' %process_mode)

	print('checking directory: %s' %src_dir_fp)

	order_dir = src_dir_fp
	file_pattern = 'MISR_AM1_GRP_ELLIPSOID_GM_P**.hdf'

	hdf_file_list = glob.glob(os.path.join(order_dir, file_pattern))
	print('total hdf files: %s' %len(hdf_file_list))

	my_year = year
	my_month = month
	parking = 'HDF_files_not_in_range_'+str(my_year)+'_'+str(my_month)
	print('parking dir: %s' %parking)

	hdf_wrong_file_count = 0
	for ifile in range(0, len(hdf_file_list), 1):

		hdf_file_fp = hdf_file_list[ifile]
		# extract orbit 
		hdf_file = hdf_file_fp.split('/')[-1]
		# print(hdf_file)

		hdf_orbit = hdf_file.split('_')[6] # if O and 0 at the begining, delete them 
		# print(hdf_orbit)
		if hdf_orbit.startswith('O'):
		    hdf_orbit = hdf_orbit[1:]
		if hdf_orbit.startswith('0'):
		    hdf_orbit = hdf_orbit[1:]
		    # print(hdf_orbit)

		hdf_time_range = Mtk.orbit_to_time_range(int(hdf_orbit))
		# print('hdf time-range: (%s, %s)' %hdf_time_range)

		hdf_file_year = hdf_time_range[0].split('-')[0]
		# print('hdf-year: %s' %hdf_file_year)
		hdf_file_month = hdf_time_range[0].split('-')[1]
		# print('hdf-month: %s' %hdf_file_month)

		if (hdf_file_year != str(my_year)) or (hdf_file_month != str(my_month).zfill(2)):  # both conditions should be true to go inside; and==both; or==either

			print('WRONG year: found %s or month: %s, moving hdf file to parking' %(hdf_file_year, hdf_file_month))
			# mkdir if dir not found
			parking_dir = os.path.join(order_dir, parking)
			if (not os.path.isdir(parking_dir)):
				os.mkdir(parking_dir)
			# move hdf file to parking
			shutil.move(os.path.join(order_dir, hdf_file), os.path.join(parking_dir, hdf_file))
			hdf_wrong_file_count = hdf_wrong_file_count+1

	print('finished; wrong/miss-matched date hdf files found: %s' %hdf_wrong_file_count)

##----------------------------------------------------------------------------------------------------------------------
#-- this part moves HDF files that are NOT in our time period to a subdirectoy inside it---- need to develop later

# if (process_mode == 'move_wrong_hdf_files_to_subdir'):
# 	#~ loop through all days and move hdf files that are not in our period to its subdir
# 	for iday in range(0, num_of_days, 1):

# 		day = iday+1
# 		print('\n-> processing day= %d \n' %day)
# 		start_time = str(year)+'-'+str(month)+'-'+str(day)+'T00:00:00Z'
# 		end_time = str(year)+'-'+str(month)+'-'+str(day)+'T23:59:59Z'
# 		print(start_time)
# 		print(end_time)

# 		orbit_list = Mtk.time_range_to_orbit_list(start_time, end_time)
# 		print(orbit_list)
# 		print('\n-> found %d orbits!' %len(orbit_list))




# 		misr_subdir_name = 'GRP_ELLIPSOID_GM_subdir_'+str(year)+'_'+str(month)+'_'+str(day)
# 		print(misr_subdir_name)

# 		hdf_subdir_fullpath = os.path.join(src_dir_fp, misr_subdir_name)
# 		print(hdf_subdir_fullpath)

# 		#~ check if directory for specific day exists, else we create the directory
# 		if (not (os.path.isdir(hdf_subdir_fullpath))):
# 			print('-> GRP_ELLIPSOID_GM subdir does NOT exist! We make that directory.')
# 			os.mkdir(hdf_subdir_fullpath)
# 			print(hdf_subdir_fullpath)
# 			print(os.path.isdir(hdf_subdir_fullpath))
# 		else:
# 			print('-> GRP_ELLIPSOID_GM subdir exists!')
# 			print(hdf_subdir_fullpath)
# 			print(os.path.isdir(hdf_subdir_fullpath))



# 		## make list of all available roughness file patterns for specific day
# 		for orbit in orbit_list:
# 			print('-> processing for orbit= %d' %orbit)
# 			#-- file pattern that we will use to move files
# 			file_pattern = 'MISR_AM1_GRP_ELLIPSOID_GM_P*'+'*_O0'+str(orbit)+'*'+'.hdf'
# 			print('-> looking for pattern= %s' %file_pattern)
# 			#-- search for file pattern and make a list
# 			hdf_files_found_list = glob.glob(os.path.join(src_dir_fp, file_pattern))
# 			print('-> hdf files found: %s' % len(hdf_files_found_list))
# 		#     print(hdf_files_found_list)

# 			for hdf_file_day in hdf_files_found_list:
# 				print('-> moving hdf file to its subdir for: %s ' %hdf_file_day)
# 				new_path = shutil.move(hdf_file_day, hdf_subdir_fullpath)

