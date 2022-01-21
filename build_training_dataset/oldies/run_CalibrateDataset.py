#!/usr/bin/python3
'''
by: Ehsan Mosadegh 26 November 2020

this script runs ATMModel.c 

'''
from subprocess import call
import os
import datetime as dt
from platform import python_version

########################################################################################################################
#-- setup paths
#-- inputs
atm_dir = '/home/ehsan/misr_lab/ATM_apr2016_5days'	 		# files in this directory should be all atm files listed there in one single directory 
masked_toa_refl_home = '/home/ehsan/misr_lab/orders/14528_apr2016/toa_refl_april2016_day1_30_p1to233_b1to46/masked_toa_refl_april2016_day1_30_p1to233_b1to46'				 # path to the home dir of 3 subdir of An/Ca/Cf
cloud_masked_dir = atm_dir 		#"/data/gpfs/assoc/misr_roughness/2016/july_2016/cloudmask_TC_CLASSIFIERS_F07_HC4_only"   # if not used, turn this option off in C.code
#-- output 
out_atmmodel_dir = '/home/ehsan/misr_lab/models/april_2016'
atmmodel_csvfile_label = "MisrAtmCalibratedModel_april_2016.csv"
#-- exe dir
exe_dir = "/home/ehsan/misr_lab/MISR-roughness/exe_dir"
exe_name = '?'
########################################################################################################################
def main():
	'''
	passes a pair of argumenst to cmd to run TOA.c program
	'''
	input_dir_list = [atm_dir, masked_toa_refl_home, cloud_masked_dir, out_atmmodel_dir, exe_dir]
	for in_dir in input_dir_list:
		ret_check = os.path.isdir(in_dir)
		if (ret_check==True):
			print('dir exists: %s' % in_dir)
		else:
			print('dir NOT found: %s' % in_dir)
			raise SystemExit()

	exe_fullpath = os.path.join(exe_dir, exe_name)
	out_atmmodel_fullpath = os.path.join(out_atmmodel_dir, atmmodel_csvfile_label)

	#~~ now run exe from UNIX to process hdf ellipsoid data 
	run_from_cmd(exe_fullpath, atm_dir, masked_toa_refl_home, cloud_masked_dir, out_atmmodel_fullpath)

	return 0
########################################################################################################################
def run_from_cmd(exe_fp, atm, maskedTOA, cm, atmmodel):
	#~~ run the C-cmd program
	print(" ");
	print('-> python: "Usage: <exe-name> <ATM-dir> <maskedTOA-dir> <cloudMask-dir> <atmmodelCSV-file>')
	cmd = (' %s %s %s %s %s' %(exe_fp, atm, maskedTOA, cm, atmmodel));
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
	print('######################## ATM-to-MISR COMPLETED SUCCESSFULLY ########################')
########################################################################################################################


