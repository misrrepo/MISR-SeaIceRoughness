#!/usr/bin/python3
'''
by: Ehsan Mosadegh 26 November 2020

this script runs ATMModel.c [later renamed to "build_trainingData_Xcams"]
check values for the following parameters inside main(.) C code:

    int cloudMask_runMode = 1; // 0 == turn off cloud mask


    // this codes runs either of the following, not both at the same time
    int misr_cloudmask = 0;    // to run with MISR cloudmask data products
    int cmcombo_cloudmask = 1; // to run cmcombo mode


inputs:
	1) path to ATM directory
	2) path to masked_toa_refl files
	3) path to cloud-mask directory
	4) path to output directory where we save traningDataSet.csv 

'''
from subprocess import call
import os
import datetime as dt
from platform import python_version

########################################################################################################################
#-- setup paths

#> inputs

#> files in this directory should be all atm files listed there in one single directory 
# atm_dir = "/data/gpfs/assoc/misr_roughness/2016/april_2016/atm_data_april_2016/ATM_IceBridge_ILATM2_V002"
atm_dir = "/data/gpfs/assoc/misr_roughness/2016/july_2016/atm_data_july_2016"



#> path to directory that includes 9 camera sub-directories: An/Ca/Cf/Ba/Bf/Da/Df
# masked_toa_refl_home = "/data/gpfs/assoc/misr_roughness/2016/april_2016/masked_toa_refl_april_2016_9cam4bands_day15_30_p1_233_b1_46"
masked_toa_refl_home = "/data/gpfs/assoc/misr_roughness/2016/july_2016/masked_toa_refl_july_2016_9cam4bands_day10_25_p1_233_b1_46"



#> if you do not use cloudMask, turn its option off in C.code (cloudMask_runMode=1)
# e.g.: #"/data/gpfs/assoc/misr_roughness/2016/july_2016/cloudmask_TC_CLASSIFIERS_F07_HC4_only"
# cloud_masked_dir = "/data/gpfs/assoc/misr_roughness/2016/cloud_masks/cmcombo_stereo_angular_april2016"    
cloud_masked_dir = "/data/gpfs/assoc/misr_roughness/2016/cloud_masks/cmcombo_stereo_angular_july2016"



# output 
out_atmmodel_dir = "/data/gpfs/assoc/misr_roughness/2016/july_2016/atmmodel"
atmmodel_csvfile_label = "traindataset_july_2016_k_zero_9cams3ANbands_withcmcombo.csv"



# exe dir
exe_dir = "/data/gpfs/home/emosadegh/MISR-SeaIceRoughness/exe_dir"
exe_name = "build_trainingData_9cams"
########################################################################################################################
def main():
	'''
	passes a pair of arguments to cmd to run TOA.c program
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

	# now run exe from UNIX to process hdf ellipsoid data 
	run_from_cmd(exe_fullpath, atm_dir, masked_toa_refl_home, cloud_masked_dir, out_atmmodel_fullpath)

	return 0
########################################################################################################################
def run_from_cmd(exe_fp, atm, maskedTOA, cm, atmmodel):

	#~~ run the C-cmd program
	print(" ");
	print('python: "Usage: <exe-name> <ATM-dir> <maskedTOA-dir> <cloudMask-dir> <atmmodelCSV-file>')
	cmd = (' %s %s %s %s %s' %(exe_fp, atm, maskedTOA, cm, atmmodel));
	print('to cmd= %s \n' %cmd)	# run the cmd command.
	#~~ run the cmd command
	return_value_of_cmd = call(cmd, shell=True);
	#~~ print('return value= %s' %return_value_of_cmd)
	print("\n******************************************\n")	# this line represents a signal from python that shows we go to next iteration inside python without any cmd ERROR
	if (return_value_of_cmd != 0):
		print('ERROR: either %s program path NOT set in path, or issue from C-code. *** Exiting' %exe_fp)
		raise SystemExit();

	print(atmmodel)
	return 0;
########################################################################################################################
if __name__ == '__main__':
	
	start_time = dt.datetime.now()
	print('start time: %s' %start_time)
	print('python version: %s' % python_version())
	print(" ")
	main()
	end_time = dt.datetime.now()
	print('end time= %s' %end_time)
	print('runtime duration= %s' %(end_time-start_time))
	print(" ")
	print('######################## ATM-to-MISR COMPLETED SUCCESSFULLY ########################')
########################################################################################################################


