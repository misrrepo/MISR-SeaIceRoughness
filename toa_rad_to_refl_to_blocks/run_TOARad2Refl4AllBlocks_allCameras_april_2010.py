#!/usr/bin/python3.6
'''
author: Ehsna Mosadegh (emosadegh@nevada.unr.edu)

version history: 30 oct 2021

about:
	reads each hdf (MISR imagery) file, transforms TOA-rad to TOA-refl, extract data for each block from each hdf file, and write out data to each block.
	each hdf file includes nominally 180 blocks in a file.

how to use:
	update dir path for input data to point to where ellipsoid.hdf files are

data flow:
	inputs: hdf Ellipsoid files- check file nam epattern; should be similar to: MISR_AM1_GRP_ELLIPSOID_GM_P156_O088029_CF_F03_0024.hdf --- NOT: MISR_AM1_GRP_ELLIPSOID_GM_P001_O086665_AN_F03_0024.b010-046.f8ba79f3f.hdf 
	outputs: toa_refl_p-o-b.cam.dat

to do: 
	- padd block num with 3 digs?
	- writes data to 3 differenct directories for each camera?? --> done

notes: 
	user provides labling tags
	this code and the next code needs img data from 3 cameras/hdf_files, so input shoud include at least those 3 files.
	output dir: writes out processed data inside same input dir

'''
########################################################################################################################

import glob, os, subprocess
import datetime as dt
from platform import python_version
import MisrToolkit as MTK

########################################################################################################################
#~ directory path setting (>>> set by USER <<<)

#===== input directory #=====
#~ input_storage_path: is where we stored hdf data for each project in sub-directories under this directories. subdirectories can be data for each month. hdf radiance files reflectance (GRP_ELLIPSOID) files, where we downloaded files

input_dir_fullpath = "/media/ehsan/seagate_6TB/AGU_2021/28892"
output_path = input_dir_fullpath 																# writes out processed data inside same input dir

exe_dir = '/home/ehsan/misr_lab/MISR-roughness/exe_dir'
exe_name = 'TOARad2Refl4AllBlocks_allCameras'

year = 2010
month = 4
day_range = [1,30]		# this code checks day-range and skips files w/days that are not in this range
block_range = [1,46] 	# [start, stop]; should match with block range in downloading step

#~ output file labling- rename them based on your project
month_label = 'april_2010_9cam4bands'
num_of_days = 'day1_30'
num_of_paths = 'p1_233'
num_of_blocks = 'b1_46'

#~ end of directory path setting (>>> set by USER <<<)
########################################################################################################################


		###############################################################################
		###																			###
		###																			###
		###		 		do not change anything below this line						###
		###																			###
		###																			###
		###############################################################################

#~ other settings - you might not need to change it
# band_list = ['Red']
# band_num = 2 # from [0,1,2,3]==[B,G,R,NIR]
###################################################
minnaert = 0	# correction, turns off minnert parameter, f=0 it will not run inside C-code

#~ some other settings- do not change
output_filelabel = "toa_refl" 	 		# toa_rad OR toa_refl; change setting inside C-code; 0==false/off &  1==true/on
# save_not_to_cloud = '.nosync'		
output_dir = output_filelabel+'_'+month_label+'_'+num_of_days+'_'+ num_of_paths+'_'+num_of_blocks            #+save_not_to_cloud # label of output subdirectory to name output dir; output processed files go here. this code with create this directory if it does not exist
exe_fullpath = os.path.join(exe_dir, exe_name)

#~ other settings - do not change
########################################################################################################################

def main():
	'''
	passes a pair of arguments to cmd to run TOA.c program
	'''
	hdf_files_fullpath_list, total_hdf_files, output_dir_fullpath = in_n_out_dir_setup(input_dir_fullpath, output_path, output_dir, minnaert, exe_fullpath)	# order of arg params: root_dir, input_dir, output_dir_name, band_list
	
	for hdf_counter, hdf_file_fullpath in enumerate(hdf_files_fullpath_list):
		# if day > day_range[1]
		#	continue to next iteration
		ret_day = check_day_from_orbit(hdf_file_fullpath);
		#~ check day to be in our range
		if (int(ret_day) > day_range[1]):
			print('days>%s, we will skip this file' % day_range[1]);
			continue;

		#-- 3 camera run-mode
		path, orbit, camera = parse_file_names(hdf_file_fullpath, total_hdf_files, hdf_counter)

		#-- check if camera is in list of 9 cameras
		if (camera not in ['df','cf','bf','af','an','aa','ba','ca','da']):
			print("Warning: camera not found in list, we continue to the next hdf file...")
			continue

		for nband in ['blue','green','red','NIR']: # we define what band we want
			print('nband= %s' %nband)
			#-- we only need/process all 9 cameras with red band and all bands for An camera
			if ((camera != 'an') and (nband != 'red')):
				print('note: cam-band combo not our favorite, will skip this HDF file- continue!')
				continue

			#-- define band_num
			if (nband == 'blue'): band_num = 0;
			elif (nband == 'green'): band_num = 1;
			elif (nband == 'red'): band_num = 2;
			elif (nband == 'nir'): band_num = 3;
			else: print('WARNING: band-number not assigned correctly!') 

			for block_num in range(block_range[0], block_range[1], 1):

				toa_file_fullpath = define_toa_file(path, orbit, block_num, camera, output_dir_fullpath, output_filelabel, hdf_counter, total_hdf_files, nband)
				#~ use a function to check if toa-file available on disc? if not, pass it to run_from_cmd()
				if (check_TOA_file_availability(toa_file_fullpath)):
					continue; # to next iteration == inside for loop

				# ~ now run TOA from UNIX to process hdf ellipsoid data 
				# print("returned False from past step. Go to cmd...")
				ret_from_C = run_from_cmd(exe_fullpath, hdf_file_fullpath, block_num, band_num, minnaert, toa_file_fullpath, hdf_counter, total_hdf_files, camera)  # note: to just check runtime setting comment out this line
				if (ret_from_C==1):
					print('ERROR FROM C CODE WHEN READING HDF BLOCK, BUT WE CONTINUE TO NEXT BLOCK')
					print('********************************************************************')
					continue
	return 0

########################################################################################################################

def check_day_from_orbit(hdf_file_fp):
	'''
	hdf files might be for whole month, but we only need days in range [1,16], so we will skip days>16
	'''
	orbit_num = hdf_file_fp.split('/')[-1].split('_')[6][1:]
	orbit_day = MTK.orbit_to_time_range(int(orbit_num))
	extracted_day = orbit_day[0].split('-')[-1][0:3]
	#~ get rid of 'T' in the end
	if (extracted_day.isalpha()==False):  # if found T in string
		refined_day = extracted_day[:-1]  # does not include last char == except last char
		print('hdf day check: %s' % refined_day)
	else:
		print('T not found in day-string')
		refined_day = extracted_day
		print(refined_day)
	return refined_day

########################################################################################################################

def check_TOA_file_availability(infile):

	if (os.path.isfile(infile)):
		print("EXIST-STATUS: TOA file exists on disc.: %s" % infile)
		return True
	else:
		print("EXIST-STATUS: TOA file NOT exist on disc.")
		return False

########################################################################################################################

def define_toa_file(path, orbit, block_num, camera, output_dir_name, file_label, hdf_counter, total_hdf_files, nband):

	block_num = str(block_num).rjust(3, '0') # added 3 to right-adjust for 3-zero digits for all range of blocks
	print('processing (block/HDF/totalHDF/cam): (%s/%s/%s/%s) (w/rjust performed) \n' % (block_num, hdf_counter, total_hdf_files, camera))


	# toa output file names to CMD command --> to do: make function for this section
	toa_file_pattern = (file_label+'_%s_%s_B%s_%s_%s.dat' %(path, orbit, block_num, camera, nband))    # will be written by TOA
		
	if (camera == 'df'):
		toa_file_fullpath = os.path.join(output_dir_name, 'Df', toa_file_pattern)
		print('output TOA file= %s' % (toa_file_fullpath))

	if (camera == 'cf'):
		toa_file_fullpath = os.path.join(output_dir_name, 'Cf', toa_file_pattern)
		print('output TOA file= %s' % (toa_file_fullpath))

	if (camera == 'bf'):
		toa_file_fullpath = os.path.join(output_dir_name, 'Bf', toa_file_pattern)
		print('output TOA file= %s' % (toa_file_fullpath))

	if (camera == 'af'):
		toa_file_fullpath = os.path.join(output_dir_name, 'Af', toa_file_pattern)
		print('output TOA file= %s' % (toa_file_fullpath))

	if (camera == 'an'):
		toa_file_fullpath = os.path.join(output_dir_name, 'An', toa_file_pattern)
		print('output TOA file= %s' % (toa_file_fullpath))

	if (camera == 'aa'):
		toa_file_fullpath = os.path.join(output_dir_name, 'Aa', toa_file_pattern)
		print('output TOA file= %s' % (toa_file_fullpath))

	if (camera == 'ba'):
		toa_file_fullpath = os.path.join(output_dir_name, 'Ba', toa_file_pattern)
		print('output TOA file= %s' % (toa_file_fullpath))

	if (camera == 'ca'):
		toa_file_fullpath = os.path.join(output_dir_name, 'Ca', toa_file_pattern)
		print('output TOA file= %s' % (toa_file_fullpath))

	if (camera == 'da'):
		toa_file_fullpath = os.path.join(output_dir_name, 'Da', toa_file_pattern)
		print('output TOA file= %s' % (toa_file_fullpath))

	return toa_file_fullpath

########################################################################################################################

def run_from_cmd(exe_fullpath, hdf_file_fullpath, block_num, band_num, minnaert, toa_file_fullpath, hdf_counter, total_hdf_files, camera):  # note: the value of argv[0] is the name of the program to execute. C kinda reserves the argv[0] index for this purpose
	
	# toa_image_file = '%stoa_%s_%s_b%s_%s.png' % (output_dir_name, path, orbit, block_num, camera)  #~ note: this is removed from the original C code, so we do not need image anymore
	# print(toa_image_file)

	#~ run the C-cmd program
	#cmd = 'TOA3 "%s" %s %s %s \"%s\" \"%s\"' %(hdf_file_fullpath, block_num, band_num, minnaert, toa_file_fullpath, toa_image_file) # old version
	# print(" ")
	# print('python= <exe-name> <Ellipsoid-HDF-file> <block> <band> <minnaert> <toa-file>')
	# print(exe_fullpath)
	# print(hdf_file_fullpath)
	# print('\n')
	cmd = (' "%s" "%s" %s %s %s \"%s\"' %(exe_fullpath, hdf_file_fullpath, block_num, band_num, minnaert, toa_file_fullpath))  # TOA writes data into toa_file_fullpath
	# print('to cmd= %s \n' % (cmd))	# run the cmd command.


	# run the cmd command
	return_value_of_cmd = subprocess.call(cmd, shell=True)
	# print('return value= %s' %return_value_of_cmd)
	print('finished processing (block/HDF/totalHDF/cam): (%s/%s/%s/%s) (w/rjust performed)' % (block_num, hdf_counter, total_hdf_files, camera))

	print("*********************************************************************************************\n") # this line indicates a signal from python that shows we go to next iteration inside python without any cmd ERROR

	# if (return_value_of_cmd != 0):
	# 	print('ERROR: return from call(): != zero; so either either of these: [1] path to the executable NOT set correctly in this path: %s, [2] there is an issue with input file name (check hdf file name), [3] maybe storage is full, [4] or some unknown issue from C-code. *** Exiting' %exe_fullpath)
	# 	raise SystemExit() 
	return return_value_of_cmd

########################################################################################################################

def in_n_out_dir_setup(input_dir_fullpath, output_path, output_dir, minnaert, exe_fullpath):
	'''
	reads dir paths and check if they exist, lists all HDF ellipsoid files and returns a list of files in fullpath mode
	'''
	output_dir_fullpath = os.path.join(output_path, output_dir)
	print('input dir : %s' % (input_dir_fullpath))
	print('output dir: %s' % (output_dir_fullpath))
	print('exe fullPath: %s' %exe_fullpath)

	#-- check if directories exist
	if not (os.path.isdir(input_dir_fullpath)): # needs fullpath
		print('input/download directory NOT exist! check the input path')
		raise SystemExit()

	if not (os.path.isdir(output_dir_fullpath)):
		print('output directory NOT exist!')
		print('so we try to create the output dir now ...\n')

		os.mkdir(output_dir_fullpath)
		if not (os.path.isdir(output_dir_fullpath)):
			print('we could not create outpur dir. You should create it manually.')
			raise SystemExit()
		else:
			print('output dir created successfully!\n')

	#-- we create dir for all 9 cameras
	camera_list = ['Da','Ca','Ba','Aa','An','Af','Bf','Cf','Df']
	for cam in camera_list:
		cam_dir = os.path.join(output_dir_fullpath, cam)
		if (not os.path.isdir(cam_dir)):
			os.mkdir(cam_dir)	 # note mkdir() doen't return anything!
			if (not os.path.isdir(cam_dir)):
				print("we could not create cam_dir")
				raise SystemExit()
			else:
				print('Successfully created cam dir for: %s' % cam)
		else:
			print('%s dir exist!' % cam)


######################################
	# #-- checking bands
	# for band in band_list:
	# 	if (band == 'Red'):
	# 		nband = band_num  # red band=2
	# 	else:
	# 		print('WARNING: band is NOT set correctly, we only work woth RED band.\n')
	# print('band= %s' %nband)
######################################
	print('minnaert= %d' % minnaert)

	# we need Ellipsoid radiance files
	misr_file_pattern = 'MISR_AM1_GRP_ELLIPSOID_GM*.hdf' # for ELLIPSOID data - check file names to have this pattern

	# get a list of available/downloaded Ellipsoid files, the list will be list of file_fullpath-s 
	hdf_files_fullpath_list = glob.glob(os.path.join(input_dir_fullpath, misr_file_pattern))
	total_hdf_files = len(hdf_files_fullpath_list)

	print('number of "GRP_ELLIPSOID***.hdf" files that will be processed= %s \n' % total_hdf_files)

	return hdf_files_fullpath_list, total_hdf_files, output_dir_fullpath

########################################################################################################################

def parse_file_names(hdf_file_fullpath, total_hdf_files, hdf_counter):
	print('=========================> NOW PROCESSING ELLIPSOID RADIATION HDF FILE: (%d/%d) <==========================\n' % (hdf_counter+1, total_hdf_files)) 
	print('input HDF: %s' %hdf_file_fullpath)

	# get the index of path and the path number
	path_index = hdf_file_fullpath.index('_P')
	path = hdf_file_fullpath[ path_index+1:path_index+5 ]


	# get the index of orbit and the orbit number
	orbit_index = hdf_file_fullpath.index('_O')
	orbit = hdf_file_fullpath[orbit_index+1:orbit_index+8]

	# find the camera in the file name

	#~ three needed cameras
	if hdf_file_fullpath.find('_DF') != -1 :
		camera = 'df'
	elif hdf_file_fullpath.find('_CF') != -1 :
		camera = 'cf'
	elif hdf_file_fullpath.find('_BF') != -1 :
		camera = 'bf'
	elif hdf_file_fullpath.find('_AF') != -1 :
		camera = 'af'
	elif hdf_file_fullpath.find('AN') != -1 :
		camera = 'an'
	elif hdf_file_fullpath.find('_AA') != -1 :
		camera = 'aa'
	elif hdf_file_fullpath.find('_BA') != -1 :
		camera = 'ba'
	elif hdf_file_fullpath.find('_CA') != -1 :
		camera = 'ca'
	elif hdf_file_fullpath.find('_DA') != -1 :
		camera = 'da'

	else:
		print('ERROR: needed camera NOT found! Check input hdf for camera label. Exiting...')
		raise SystemExit()

	print('path= %s' %path)
	print('orbit= %s' %orbit)
	print('camera= %s' %camera)

	return path, orbit, camera

########################################################################################################################

if __name__ == '__main__':
	
	start_time = dt.datetime.now()
	print('\n')
	print('######################################### TOA STARTED SUCCESSFULLY ########################')
	print('start time: %s' %start_time)
	print('python version: %s' % python_version())
	print(" ")
	main()
	end_time = dt.datetime.now()
	print('end time= %s' %end_time)
	print('runtime duration= %s' %(end_time-start_time))
	print(" ")
	print('####################################### TOA COMPLETED SUCCESSFULLY ########################')

########################################################################################################################
# we don't use it anymore, we need to ahve the full path to the files
# cwd = os.getcwd()
# print('-> we are at run-script dir= %s' %cwd)
# os.chdir( download_dir )
# cwd = os.getcwd()
# print('-> chaged directory to download dir= %s' %cwd)
########################################################################################################################
