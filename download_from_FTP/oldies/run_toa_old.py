#!/usr/bin/python3

import glob , os , sys



##################################################################
MISR_root_dir = '/media/mare/MISR_REPO/MISR_root/'
download_directory_name = 'misr_test1/'
toa_dir_name = 'toa_images/'
download_dir = MISR_root_dir+download_directory_name
TOA_directory = MISR_root_dir+toa_dir_name

cwd = os.getcwd()
print('-> we are at script dir= %s' %cwd)
os.chdir( download_dir )
cwd = os.getcwd()
print('-> chaged directoey and now we are at DL dir= %s' %cwd)


bands = ['Red'] # why only red?
for band in bands :
	if (band == 'Red') :
		nband = 2  # what is nband?
	else:
		print('-> WARNING: band is NOT set correctly.')
print(nband)
# correction
minnaert = 0


misr_file_patern = 'MISR_AM1_GRP_ELLIPSOID_GM*.hdf' # for ELLIPSOID data

list_of_misr_files = glob.glob( misr_file_patern )
print('-> len of DL list= %s' %len(list_of_misr_files))

for each_misr_file in list_of_misr_files :

	print(each_misr_file)
	# get the index of path and the path number
	path_index = each_misr_file.index('_P')
	path = each_misr_file[ path_index+1 : path_index+5 ]
	# print(f'-> path= {path}')

	# get the index of orbit and the orbit number
	orbit_index = each_misr_file.index('_O')
	orbit = each_misr_file[ orbit_index+1 : orbit_index+8]
	# print(f'-> orbit = {orbit}')

	# find the camera in the file name
	if each_misr_file.find('_CF') != -1 :
		camera = 'cf'
	elif each_misr_file.find('_CA') != -1 :
		camera = 'ca'
	elif each_misr_file.find('_AN') != -1 :
		camera = 'an'
	else:
		print('-> WARNING: camera NOT found!')
		sys.exit(1)

	print('-> camera is= %s' %camera)

	for each_block in range(1,43) :

		# name the putput file names to CMD command
		fname2 = '%stoa_%s_%s_b%s_%s.dat' % (TOA_directory, path, orbit, each_block, camera)
		print(fname2)
		fname3 = '%stoa_%s_%s_b%s_%s.png' % (TOA_directory, path, orbit, each_block, camera)
		print(fname3)

		# run the C-cmd program
		cmd = 'TOA3 "%s" %s %s %s \"%s\" \"%s\"' %(each_misr_file, each_block, nband, minnaert, fname2, fname3)
		# print(cmd)
		print(cmd)
		if os.system(cmd) != 0 :
			print('-> Error: cmd NOT found. Exiting...')
			sys.exit(1)



	# extract path, orbit, camera from each MISR hdf file --> done

	# label fname2 and fname3 by (toa_dir, path , orbit , block , camera ) --> done

	# cmd TOA3 processing_hdf_file, block, nband, minnaert, fname2, fname3
