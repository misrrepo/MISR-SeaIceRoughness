#!/usr/bin/python3
'''
by: Ehsan Mosadegh 26 November 2020

	this script runs MISR2roughness.c parallel code


'''
from subprocess import call
import os
import datetime as dt
from platform import python_version

########################################################################################################################
#-- setup paths
#-- inputs
masked_toa_An_dir = '/home/ehsan/misr_lab/orders/14528_apr2016/toa_refl_april2016_day1_30_p1to233_b1to46/masked_toa_refl_april2016_day1_30_p1to233_b1to46/An/' 	# note: should set to masked/AN/ dir= should have An/ at the end of path
atmmodel_dir = '/home/ehsan/misr_lab/april_2016/models'
atmmodel_csvfile_label = 'MisrAtmCalibratedModel_april_2016.csv'
#-- output
predicted_roughness_dir = '/home/ehsan/misr_lab/roughness_2016_April_1to16_p1to233_b1to46'
#-- executable
exe_dir = '/home/ehsan/misr_lab/MISR-roughness/exe_dir'
exe_name = 'MISR2Roughness_parallel_checkFileExist'
########################################################################################################################
def main():
	'''
	passes a pair of argumenst to cmd to run TOA.c program
	'''
	input_dir_list = [masked_toa_An_dir, atmmodel_dir, predicted_roughness_dir, exe_dir]
	for in_dir in input_dir_list:
		ret_check = os.path.isdir(in_dir)
		if (ret_check==True):
			print('dir exists: %s' % in_dir)
		else:
			print('dir NOT found: %s' % in_dir)
			raise SystemExit()

	exe_fullpath = os.path.join(exe_dir, exe_name)
	atmmodel_fullpath = os.path.join(atmmodel_dir, atmmodel_csvfile_label)

	#~~ now run exe from UNIX to process hdf ellipsoid data 
	run_from_cmd(exe_fullpath, masked_toa_An_dir, atmmodel_fullpath, predicted_roughness_dir)

	return 0
########################################################################################################################
def run_from_cmd(exe_fp, maskedTOAan, atmmodel, roughDir):
	#~~ run the C-cmd program
	print(" ");
	print('-> python: "Usage: <exe-name> <maskedTOA-AN-dir> <atmmodelCSV-file> <predictedRoughness-dir>')
	cmd = (' %s %s %s %s' %(exe_fp, maskedTOAan, atmmodel, roughDir));
	print('-> to cmd= %s \n' %cmd)	# run the cmd command.
	#~~ run the cmd command
	return_value_of_cmd = call(cmd, shell=True);
	#~~ print('-> return value= %s' %return_value_of_cmd)
	print("\n******************************************\n")	# this line represents a signal from python that shows we go to next iteration inside python without any cmd ERROR
	if (return_value_of_cmd != 0):
		print('-> ERROR: either %s program path NOT set in path, or issue from C-code. *** Exiting' %exe_fp)
		raise SystemExit();
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
	print('######################## MISR-to-roughness COMPLETED SUCCESSFULLY ########################')
########################################################################################################################


