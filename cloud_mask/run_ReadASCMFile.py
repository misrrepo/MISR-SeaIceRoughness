#!/usr/bin/env python
'''
Generate ASCM cloudmask from MISR MIL2TCCL.
The MISR ASCM was upsampled from 128 x 512 to 512 x 2048.
created 18 Aug 2020 by Ehsan Mosadegh
original name: readASCMFile.py

notes: 
		we need TC_CLASSIFIERS_F07 after we download files
		we run for 46 blocks in here
		we extract TC_CLASSIFIERS_F07_HC4 field
		output dir will be inside inout dir

'''

import sys, os, os.path 

# in_dir = "/home/mare/Nolin/2012/MIL2TCCL/MarJun"
# nodir = "/home/mare/Nolin/2012/MIL2TCCL/JunJul/ASCM"
# odir = "/home/mare/Nolin/2012/MIL2TCCL/MarJun/ASCM_w17.6km_10%"
# in_dir = "/Volumes/easystore/from_home/Nolin_except_dataFolder/remainder_forExternalHD_3.1TB/2013/MIL2TCSP/Apr"
# in_dir = "/Volumes/Ehsanm_DRI/research/MISR/cloud_mask/cloudmask_apr2013_day1to16_p1to233/"

# set the input path
in_dir = '/data/gpfs/assoc/misr_roughness/2016/april_2016/cloudmasks'
exe_dir = '/data/gpfs/home/emosadegh/MISR-roughness/exe_dir'

################################## DO NOT CHANGE ##################################
#~ other settings
exe_name = "ReadASCMCloudMask"
exe_dir_fullpath = os.path.join(exe_dir, exe_name)

end_block_not_included = 47  # reads up to this number
# set a label for output dir
out_dir_label = 'cloudmask_TC_CLASSIFIERS_F07_HC4_only' # we build this dir inside our input dir

###################################################################################

out_dir_fullpath = os.path.join(in_dir, out_dir_label)
print("-> out dir full path: %s" % out_dir_fullpath)

if (not os.path.isdir(out_dir_fullpath)):
	print('-> output directory NOT exist. We make it.')
	# raise SystemExit()
	cmd = "mkdir " + out_dir_fullpath
	sys.stderr.write('%s\n' % cmd)
	if os.system(cmd) != 0:
		sys.exit(1)

# if not os.path.exists(odir):
# 	cmd = "mkdir \"" + odir + "\""
# 	sys.stderr.write('%s\n' % cmd)
# 	if os.system(cmd) != 0:
# 		sys.exit(1)


n = 0
files_list = [file for file in os.listdir(in_dir) if (os.path.splitext(file)[1] == '.hdf')]

for file_count, file in enumerate(files_list):

	print("\nMISR TC-CLASSIFIER.hdf (%d/%d): %s \n" % (file_count+1, len(files_list), file))
	path = file.split('_')[4]
	orbit = file.split('_')[5]

	# cmdir = odir + '/cloudmask_' + orbit + '_' + path
	# cmd = 'mkdir \"' + cmdir + '\"'
	# if not os.path.exists(cmdir):
	# 		sys.stderr.write('%s\n' % cmd)
	# 		if os.system(cmd) != 0:
	# 			sys.exit(1)

	f = open(in_dir + '/' + file, 'rb') # open each hdf file
	# items = file.split('_') # ? # based on old file name pattern
	# print("items: %s" % items[0])

	ifile = in_dir + '/' + file # define hdf file
	# for block in range(int(items[1][1:4]), int(items[1][5:8]) + 1):  # 
	for block in range(1, end_block_not_included, 1):  # define range for blocks

		ofile = out_dir_fullpath + '/' + 'cloudmask_' + path + '_' + orbit + '_B%03d.msk' % block		
		cmd = "%s \"%s\" %d \"%s\"" % (exe_dir_fullpath, ifile, block, ofile)
		sys.stderr.write('%5d: %s\n' % (n + 1, cmd)) # why n+1 ?
		if (os.system(cmd) != 0):
			sys.exit(1)
		n += 1

	print('cloudMask finished successfully!')

	f.close() # close each hdf file
	#break;
