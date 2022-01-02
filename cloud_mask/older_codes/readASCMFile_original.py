#!/usr/bin/env python

# Generate ASCM cloudmask from MISR MIL2TCCL.
# The MISR ASCM s upsampled from 128 x 512 to 512 x 2048.
# Created 12/13/2010 by gmar

from __future__ import print_function
from __future__ import division
import sys, os, os.path, signal
import struct

# idir = "/home/mare/Nolin/2012/MIL2TCCL/MarJun"
# #odir = "/home/mare/Nolin/2012/MIL2TCCL/JunJul/ASCM"
# odir = "/home/mare/Nolin/2012/MIL2TCCL/MarJun/ASCM_w17.6km_10%"

idir = "/Volumes/Ehsanm_DRI/research/MISR/cloud_mask/cloudmask_files_april_2013/"
odir = idir
end_block_not_included = 41
exe_name = "ReadASCMFile"

if not os.path.exists(odir):
	cmd = "mkdir \"" + odir + "\""
	sys.stderr.write('%s\n' % cmd)
	if os.system(cmd) != 0:
		sys.exit(1)

n = 0
files = [file for file in os.listdir(idir) if (os.path.splitext(file)[1] == '.hdf')]
for file in files:
	path = file.split('_')[4]
	orbit = file.split('_')[5]
	cmdir = odir + '/cloudmask_' + orbit + '_' + path # cloud mask dir for each orbit file
	cmd = 'mkdir \"' + cmdir + '\"'		# create dir for each orbit file
	if not os.path.exists(cmdir):
			sys.stderr.write('%s\n' % cmd)
			if os.system(cmd) != 0:
				sys.exit(1)

	f = open(idir + '/' + file, 'rb')  # open each hdf file
	items = file.split('.') # split file label based on .

	ifile = idir + '/' + file  # define each hdf file
	# for block in range(int(items[1][1:4]), int(items[1][5:8]) + 1):
	for block in range(1, end_block_not_included, 1):

		ofile = cmdir + '/cloudmask_' + path + '_' + orbit + '_B%03d.msk' % block  # define output file
		cmd = "./%s \"%s\" %d \"%s\"" % (exe_name, ifile, block, ofile)  # define comand to run code.c
		sys.stderr.write('%5d: %s\n' % (n + 1, cmd))
		if (os.system(cmd) != 0):
			sys.exit(1)
		n += 1
	f.close()
	#break;
