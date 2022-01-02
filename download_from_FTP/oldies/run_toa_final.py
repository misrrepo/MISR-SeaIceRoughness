#!/usr/bin/python3

import glob , os , sys

##################################################################

# update following 3 directories

MISR_root_dir = '/media/mare/MISR_REPO/MISR_root/' # '/Volumes/MISR_REPO/MISR_root/'
download_directory_name = 'misr_test_data/' # dir with downloaded hdf data
toa_dir_name = 'toa_dir/' # output of TOA run

##################################################################

download_dir = MISR_root_dir+download_directory_name
TOA_directory = MISR_root_dir+toa_dir_name

cwd = os.getcwd()
print('-> we are at run-script dir= %s' %cwd)
os.chdir( download_dir )
cwd = os.getcwd()
print('-> chaged directory to download dir= %s' %cwd)


bands = ['Red'] # why only red?
for band in bands :
	if (band == 'Red') :
		nband = 2  # what is nband?
	else:
		print('-> WARNING: band is NOT set correctly.')
print('-> band= %s' %nband)
# correction
minnaert = 0


misr_file_patern = 'MISR_AM1_GRP_ELLIPSOID_GM*.hdf' # for ELLIPSOID data - check file names 

list_of_misr_files = glob.glob( misr_file_patern )
print('-> no. of hdf files found= %s' %len(list_of_misr_files))

for each_dhf_file in list_of_misr_files :

	print('-> hdf file= %s' %each_dhf_file)
	# get the index of path and the path number
	path_index = each_dhf_file.index('_P')
	path = each_dhf_file[ path_index+1 : path_index+5 ]
	# print(f'-> path= {path}')

	# get the index of orbit and the orbit number
	orbit_index = each_dhf_file.index('_O')
	orbit = each_dhf_file[ orbit_index+1 : orbit_index+8]
	# print(f'-> orbit = {orbit}')

	# find the camera in the file name
	if each_dhf_file.find('_CF') != -1 :
		camera = 'cf'
	elif each_dhf_file.find('_CA') != -1 :
		camera = 'ca'
	elif each_dhf_file.find('_AN') != -1 :
		camera = 'an'
	else:
		print('-> WARNING: camera NOT found!')
		sys.exit(1)

	print('-> camera is= %s' %camera)

	for each_block in range(1,43) : # why loop over 42 blocks in one hdf file

		# name the putput file names to CMD command
		toa_data_file = '%stoa_%s_%s_b%s_%s.dat' %(TOA_directory, path, orbit, each_block, camera) # will be written by TOA
		#print(f'-> TOA writes hdf data to file= {toa_data_file}')

		# toa_image_file = '%stoa_%s_%s_b%s_%s.png' % (TOA_directory, path, orbit, each_block, camera)
		# print(toa_image_file)

		# run the C-cmd program
		#cmd = 'TOA3 "%s" %s %s %s \"%s\" \"%s\"' %(each_dhf_file, each_block, nband, minnaert, toa_data_file, toa_image_file) // old version 
		cmd = ' "TOA_Ehsan" "%s" %s %s %s \"%s\"' %(each_dhf_file, each_block, nband, minnaert, toa_data_file)  # TOA writes data into toa_data_file
		print('-> runScript to TOA= %s' %cmd)

		if os.system(cmd) != 0 :
			print('-> Error: TOA exe NOT found in path. Exiting...')
			sys.exit(1)

print('->>> TOA SUCCESSFULLY FINISHED!')


	# extract path, orbit, camera from each MISR hdf file --> done

	# label toa_data_file and toa_image_file by (toa_dir, path , orbit , block , camera ) --> done

	# cmd TOA3 processing_hdf_file, block, nband, minnaert, toa_data_file, toa_image_file
