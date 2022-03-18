#!/usr/bin/python3.6
'''
by: Ehsan Mosadegh 20 August 2020
this script runs LandMask.c 
'''
from subprocess import call
import os
import datetime as dt
from platform import python_version

########################################################################################################################
#~ input dir: <toa_refl> should include 3 different directories in here (An/Ca/Cf)
toa_dir_fullpath = "/home/ehsan/misr_lab/test_not_cloudy_p104_o86919/toa_refl_april2016_3cams_day1_30_p1to233_b1to46"

#~ landmask dir
lsmask_dir_fullpath = "/home/ehsan/misr_lab/test_not_cloudy_p104_o86919/landseamask_blocks_1to46"									

exe_dir = "/home/ehsan/misr_lab/MISR-SeaIceRoughness/exe_dir"
exe_name = "LandMask_3cams" # should be set in $PATH

#~ output dir for masked_toa_refl file
output_path = toa_dir_fullpath

exe_dir_fullpath = os.path.join(exe_dir, exe_name)
########################################################################################################################
def main():
	#~ check if input dir exists
	print("-> input dir: %s" % toa_dir_fullpath)
	if (not os.path.isdir(toa_dir_fullpath)):
		print("-> ERROR: input dir NOT exist: %s" % toa_dir_fullpath)
		raise SystemExit()


	#~ 1st we make output dir 
	output_dir = "masked_" + toa_dir_fullpath.split('/')[-1]
	output_dir_fullpath = os.path.join(output_path, output_dir)	# output dir; dat and png files; go to 3 different directories	//"/home3/mare/Nolin/2016/Surface3_LandMasked/Jul/"; 
	if (not os.path.isdir(output_dir_fullpath)):
		print("-> Warning: output dir NOT exist, we will make it.")
		os.mkdir(output_dir_fullpath) 		# doesnt return anything
		if (not os.path.isdir(output_dir_fullpath)):
			print("-> ERROR: we could NOT make output dir.")
			raise SystemExit()
		else:
			print("-> output dir: %s" % output_dir_fullpath)

	lsmask_files_fullpath = os.path.join(lsmask_dir_fullpath, 'lsmask_pP_bB.dat');
	print('-> landMask dir: %s' % lsmask_files_fullpath)

	#~ now, we make 3 dirs for 3 cameras
	camera_list = ['Ca', 'An', 'Cf']
	for cam in camera_list:
		cam_dir = os.path.join(output_dir_fullpath, cam)
		if (os.path.isdir(cam_dir)==False):
			os.mkdir(cam_dir)	 # note mkdir() doen't return anything!
			if (not os.path.isdir(cam_dir)):
				print("-> we could not create cam_dir")
				raise SystemExit()
			else:
				print('-> Successfully created cam dir for: %s' % cam)
		else:
			print('-> %s dir exist!' % cam)


	cmd = ('"%s" "%s" "%s" "%s"' % (exe_dir_fullpath, toa_dir_fullpath, lsmask_files_fullpath, output_dir_fullpath) )  # cmd = should be a string
	# print(cmd)
	call(cmd, shell=True)

	return 0;

########################################################################################################################

if __name__ == '__main__':
	
	start_time = dt.datetime.now()
	print('-> start time: %s' %start_time)
	print('-> python version: %s' % python_version())
	print(" ")
	main()
	end_time = dt.datetime.now()
	print('-> end time= %s' %end_time)
	print('-> runtime duration= %s' %(end_time-start_time))
	print(" ")
	print('######################## TOA COMPLETED SUCCESSFULLY ########################')

########################################################################################################################









