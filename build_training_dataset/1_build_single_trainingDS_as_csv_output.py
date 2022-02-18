#!/usr/bin/python3.6
# coding: utf-8
'''
calibrate MISR dataset with ATM measurement data and returns the training dataset

input: dir path where is home to masked_toa_refl & ATM data
output: single datasets.csv for each ATM.csv; and then will merge all output single datasets together in the end and will return a final merged dataset for filtering 

to-do:
- do not check empty lists
- check all code again
'''
'''
notes:
	-999.0 represents no MISR toa file
	-998 is the first line
'''

import MisrToolkit as mtk
import pandas as pd
import numpy as np
import sys, glob, os, csv
from platform import python_version
import datetime as dt

# !pip uninstall jedi --yes

# check python version be 3.6 cuz MTK works with 3.6

# py_version = sys.version
# py_ver = py_version[0:3]
# if (py_ver != str(3.6)):

# another way to get verion. note: this f() returns py version as str dtype, we change it to float
py_ver = float(python_version()[0:3])
print(py_ver)
if (py_ver != 3.6):
	print('python version is NOT 3.6- MTK does not work')
	raise SystemExit()
else:
	print('python version: %s' %py_ver)


##- set up paths
#################### input dir should include 9 dir
# masked_toa_home = "/media/ehsan/Gdrive_18TB/all_masked_toa_refl_2010_2014_2019"
masked_toa_home = "/media/ehsan/6T_part1/2016/april_2016/14528_apr2016/project_april_2016_9cam3bands/masked_toa_refl_april_2016_9cam4bands_day1_30_p1_233_b1_46"


# atm_dir = "/home/ehsan/misr_lab/ATM_2010_2014_2019"
atm_dir = "/media/ehsan/6T_part1/2016/april_2016/14528_apr2016/project_april_2016_3cam/ATM_apr2016_5days"


#################### output
# trainingDS_dir = "/media/ehsan/Gdrive_18TB/training_DS_2010_2014_2019"
trainingDS_dir = "/media/ehsan/6T_part1/2016/april_2016/14528_apr2016/project_april_2016_3cam/training_data/april_2016"


# output_final_ds_label = "april_2016_9cam3bands_final_merged_dataset.csv"


#################### setup pattern- do not change
atm_file_pattern = 'ILATM2*'+'.csv'
single_csv_ds_label = "out9cam3bands"


# atm_file_sample = 'ILATM2_20160420_175457_smooth_nadir3seg_50pt.csv'
# atm_file_fp = os.path.join(atm_dir, atm_file_sample)
# check ATM file exists
# print(os.path.isfile(atm_file_fp))
# print(atm_file_fp)

# check runtime
t1 = dt.datetime.now()


#### create datadet table
# cam_labels = ['Da_r','Ca_r','Ba_r','Aa_r','An_r','An_g','An_b','An_nir','Af_r','Bf_r','Cf_r','Df_r','mean_ATM_roughness']

# add all found fields to pandas.DataFrame and make a dataset with column labels and filter for similar pixels
# column_names = ['path','orbit','block','line','sample','lat','lon', 'points_in_pixel', cam_labels[0],cam_labels[1],cam_labels[2],cam_labels[3],cam_labels[4],cam_labels[5],cam_labels[6],cam_labels[7],cam_labels[8],cam_labels[9],cam_labels[10],cam_labels[11],'mean_ATM_roughness']

ds_row_index = 0

column_names = ['path','orbit','block','line','sample','lat','lon','Da_r','Ca_r','Ba_r','Aa_r',\
				'An_r','An_g','An_b','An_nir','Af_r','Bf_r','Cf_r','Df_r','mean_ATM_roughness','ATM_start_time','ATM_end_time']

final_ds_list = []

## get a list of ATM csv files
atm_list = glob.glob(os.path.join(atm_dir, atm_file_pattern))
total_atm_files_found = len(atm_list)
print('total ATM.csv files: %d' %total_atm_files_found)


## create pandas DF and set index to DF based on pre-estimate: 100000 per each ATM file
# final_df = pd.DataFrame(columns=column_names) #, index=range(total_atm_files_found*1000))  # if nut set the index number correctly, returns segmentation fault- Q- how create a DF with variable size?

## this does not work
# final_ds.set_index(total_atm_files_found*10000)


# open each ATM file
for atm_cntr, ATMfile in enumerate(atm_list):

	print('\n********** processing ATM file: (%d/%d)' %(atm_cntr+1, len(atm_list)))
	print("try to use ATM time and locations to find MISR pixel...\n")
	print(ATMfile)

	atm_filelabel = ATMfile.split('/')[-1]
	atm_filelabel_date_list = atm_filelabel.split('_')[0:3]
	atm_filelabel_date = atm_filelabel_date_list[0]+"_"+atm_filelabel_date_list[1]+"_"+atm_filelabel_date_list[2]
	output_ds_label = single_csv_ds_label+'_'+atm_filelabel_date+'.csv'
	print("output csv: %s" %output_ds_label)


	output_file_fp = os.path.join(trainingDS_dir, output_ds_label)
	if (os.path.isfile(output_file_fp)):
		print("CSV file exists on disk, continue to next ATM file")
		continue


	# get date-time from ATM file label
	atm_label = ATMfile.split('/')[-1]
	yrmonday = atm_label.split('_')[1]
	atm_yr = yrmonday[0:4]
	atm_mon = yrmonday[4:6]
	atm_day = yrmonday[6:8]
	# print(atm_yr)
	# print(atm_mon)
	# print(atm_day)

	hrminsec = atm_label.split('_')[2]
	atm_hr = hrminsec[0:2]
	atm_min = hrminsec[2:4]
	atm_sec = hrminsec[4:6]

	ATM_start_time = atm_yr+'-'+atm_mon+'-'+atm_day+"T00:00:00Z" # YYYY-MM-DDThh:mm:ssZ
	ATM_end_time = atm_yr+'-'+atm_mon+'-'+atm_day+"T23:59:59Z" # YYYY-MM-DDThh:mm:ssZ

	ATM_start_time_hrminsec = atm_yr+'-'+atm_mon+'-'+atm_day+"T"+atm_hr+":"+atm_min+":"+atm_sec+"Z" # YYYY-MM-DDThh:mm:ssZ
	ATM_end_time_hrminsec = atm_yr+'-'+atm_mon+'-'+atm_day+"T"+atm_hr+":"+atm_min+":"+atm_sec+"Z" # YYYY-MM-DDThh:mm:ssZ

	print("ATM start: %s" %ATM_start_time)
	print("ATM end:   %s" %ATM_end_time)

	# get a list of orbits for date-time
	orbit_list_today = mtk.time_range_to_orbit_list(ATM_start_time, ATM_end_time)
	# print('orbits today: %s' %len(orbit_list_today))
	# print(orbit_list_today)



	# for each orbit we find path number and use that to find MISR pixel
	for misr_orbit_today in orbit_list_today:
		# print('\nprocessing orbit: %d' %misr_orbit_today)

		# orbit to path
		misr_pathNumber_today = mtk.orbit_to_path(misr_orbit_today)
		# print('path: %s' %misr_pathNumber_today)


		# open and read ATM file
		# atm_file_columnt = []

		input_atm_file = pd.read_csv(ATMfile, comment='#', sep=',') #skiprows=15, header=0)

		# input_atm_file.head(5)

		# find total atm_rows to make DF with Nan
		csv_total_rows = input_atm_file.shape[0]
		# print(csv_total_rows)


		# read each row in ATM-file and extract latLong
		for atm_row_num in range(csv_total_rows): 
			# print(type(atm_row_num))
			atm_lat = input_atm_file.iloc[atm_row_num, 1]	
			atm_lon = input_atm_file.iloc[atm_row_num, 2]	
			atm_roughness = input_atm_file.iloc[atm_row_num, 6]

			# print('atm lat %s' %atm_lat)
			# print('atm lon %s' %atm_lon)
			# print('roughness %s' %atm_roughness)


			# print('finidng list of paths for ATM location...')
			try:  # to ignore Exception for some path numbers
				paths_cover_ATM_location = mtk.latlon_to_path_list(atm_lat, atm_lon)
				# print('MISR paths cover ATM latlon:')
				# print(paths_cover_ATM_location)
			except Exception:
				print('Exception was raised- continue to next ATM row')
				continue


			if misr_pathNumber_today not in paths_cover_ATM_location:
				# print('MISR path %d in day %s not match with path that ATM lat-lon falls in, continue to next ATM latLon row!' %(misr_pathNumber_today, atm_day))
				continue    # to next location == latLon
			# else:
				print('MISR path %d covers ATM location- we will find Block-Line-Sample!' %misr_pathNumber_today)

	
			# find MISR pixel from that
			resolution_meters = 275
			
			misr_block, misr_pixel_x, misr_pixel_y = mtk.latlon_to_bls(misr_pathNumber_today, resolution_meters, atm_lat, atm_lon) # q- why crash? these paths cover that that ATM location
			
			# print('block: %s' %misr_block)
			# print('pixel x: %s' %misr_pixel_x)
			# print('pixel y: %s' %misr_pixel_y)


			path_num_str = str(misr_pathNumber_today).zfill(3)
			misr_orbit_str = str(misr_orbit_today).zfill(6) 
			misr_block_str = str(misr_block).zfill(3)


			# print('path-str: %s' %path_num_str)
			# print('orbit-str: %s' %misr_orbit_str)
			# print('block-str: %s' %misr_block_str)


			#### create fulll path to 9 cameras and 4 spectral bands

			# find MISR files associated with P-O-B: masked_toa_P_O_B
			# aft/back cameras
			aa_red_fp = os.path.join(masked_toa_home,'Aa', ('masked_toa_refl_P'+path_num_str+'_'+'O'+misr_orbit_str+'_'+'B'+misr_block_str+'_aa_red.dat'))
			ba_red_fp = os.path.join(masked_toa_home,'Ba', ('masked_toa_refl_P'+path_num_str+'_'+'O'+misr_orbit_str+'_'+'B'+misr_block_str+'_ba_red.dat'))
			ca_red_fp = os.path.join(masked_toa_home,'Ca', ('masked_toa_refl_P'+path_num_str+'_'+'O'+misr_orbit_str+'_'+'B'+misr_block_str+'_ca_red.dat'))
			da_red_fp = os.path.join(masked_toa_home,'Da', ('masked_toa_refl_P'+path_num_str+'_'+'O'+misr_orbit_str+'_'+'B'+misr_block_str+'_da_red.dat'))
		   
			# front cameras
			af_red_fp = os.path.join(masked_toa_home,'Af', ('masked_toa_refl_P'+path_num_str+'_'+'O'+misr_orbit_str+'_'+'B'+misr_block_str+'_af_red.dat'))
			bf_red_fp = os.path.join(masked_toa_home,'Bf', ('masked_toa_refl_P'+path_num_str+'_'+'O'+misr_orbit_str+'_'+'B'+misr_block_str+'_bf_red.dat'))
			cf_red_fp = os.path.join(masked_toa_home,'Cf', ('masked_toa_refl_P'+path_num_str+'_'+'O'+misr_orbit_str+'_'+'B'+misr_block_str+'_cf_red.dat'))
			df_red_fp = os.path.join(masked_toa_home,'Df', ('masked_toa_refl_P'+path_num_str+'_'+'O'+misr_orbit_str+'_'+'B'+misr_block_str+'_df_red.dat'))
			
			# nadir
			an_red_fp = os.path.join(masked_toa_home,'An', ('masked_toa_refl_P'+path_num_str+'_'+'O'+misr_orbit_str+'_'+'B'+misr_block_str+'_an_red.dat'))
			an_green_fp = os.path.join(masked_toa_home,'An', ('masked_toa_refl_P'+path_num_str+'_'+'O'+misr_orbit_str+'_'+'B'+misr_block_str+'_an_green.dat'))
			an_blue_fp = os.path.join(masked_toa_home,'An', ('masked_toa_refl_P'+path_num_str+'_'+'O'+misr_orbit_str+'_'+'B'+misr_block_str+'_an_blue.dat'))
			an_nir_fp = os.path.join(masked_toa_home,'An', ('masked_toa_refl_P'+path_num_str+'_'+'O'+misr_orbit_str+'_'+'B'+misr_block_str+'_an_NIR.dat')) # note here: should match the file names on drive ("an_NIR" is ok, not "an_nir")

			#### check if camera files availabel


			# create a list of camreas based on order from Da->Df- order if important 
			masked_toaFile_orderedList = [da_red_fp, ca_red_fp, ba_red_fp, aa_red_fp,
										  an_red_fp, an_green_fp, an_blue_fp, an_nir_fp, 
										  af_red_fp, bf_red_fp, cf_red_fp, df_red_fp]



			available_file_status = []
			# check file exists
			for masked_toa_file in masked_toaFile_orderedList:
				# print(masked_toa_file)
				available_status = os.path.isfile(masked_toa_file)
				# print(available_status)
				available_file_status.append(available_status)


			##-- check if all 12 cameras are available
			# if (all(available_file_status)==False): # check if any element is false==file not found
			#     # print('***some masked-files missing- continue*** \n')
			#     continue # to the next ATM row

			if (not any(available_file_status)):
				# print('did not find even a single camera value for ATM location/row- continue')
				continue

			# print("num. of masked toa files found:")
			# print(available_file_status)



			# open masked-toa-refl and extract pixel value
			pixel_values = []

			for maskedTOA_rawfile in masked_toaFile_orderedList:
				# print(maskedTOA_rawfile)
				is_file = os.path.isfile(maskedTOA_rawfile)
				# print(is_file)
				
				#-- if a camera is missing, assign fill value for cameras that are not found
				if (is_file==False):
					pixel_values.append(-999.0) 
				else:
					rough_2d_arr = np.fromfile(maskedTOA_rawfile, dtype=np.double)[0:1048576].reshape((512,2048))   # 'double==float64'     # is this roughness in cm?
					# print(rough_2d_arr.shape)
					# print(type(rough_2d_arr))
					##- maybe make a image from it?
					pixel_value = rough_2d_arr[int(misr_pixel_x), int(misr_pixel_y)]
					pixel_values.append(round(pixel_value,5))


			# print('path-str: %s' %path_num_str)
			# print('orbit-str: %s' %misr_orbit_str)
			# print('block-str: %s' %misr_block_str)

			# print("found these pixel values for an ATM location:")
			# print(pixel_values)

			########################################################
			# new way of making a list and then open it in dataframe

			final_ds_values = [misr_pathNumber_today, misr_orbit_today, misr_block,\
			 int(misr_pixel_x), int(misr_pixel_y), round(atm_lat,7), round(atm_lon,7), \
			 pixel_values[0],pixel_values[1],pixel_values[2],pixel_values[3],pixel_values[4],\
			 pixel_values[5],pixel_values[6],pixel_values[7],pixel_values[8],pixel_values[9],\
			 pixel_values[10],pixel_values[11],atm_roughness,\
			 ATM_start_time_hrminsec,ATM_end_time_hrminsec]
			

			zipped = zip(column_names, final_ds_values)
			a_dictionary = dict(zipped)
			# print(a_dictionary)
			final_ds_list.append(a_dictionary)
			
			ds_row_index +=1

	# ## write each csf file in he loop
	# print("1st row of data:")
	# print(final_ds_list[0])

	##################################################################

	# print("processed %d rows" %ds_row_index)
	# if (ds_row_index == 0):

	##################################################################


	if (len(final_ds_list) == 0):
		print(final_ds_list)
		print("ATM list is empty, we write -998 flag & continue to next ATM file")
		print("note: zero means we did not find any MISR pixel for locations in an ATM file.")

		
		final_ds_list = [-998,-998,-998,-998,-998,-998,-998,-998,-998,-998,-998,-998,-998,-998,-998,-998,-998,-998,-998,-998]
		
		with open(output_file_fp, 'w', encoding='UTF8', newline='') as fileObj:
			writer = csv.writer(fileObj)  # create a new instance of the DictWriter class by passing the file object
			writer.writerow(column_names)
			writer.writerow(final_ds_list)

		final_ds_list.clear()
		continue

	else:
		## dataframe method
		# final_df = pd.DataFrame(columns=column_names)
		# final_df = final_df.append(final_ds_list, ignore_index=True) # add list to existing dataframe; True: preserve the DataFrame indices; 

		# out_DS  = os.path.join(trainingDS_dir, output_ds_label)
		# final_df.to_csv(out_DS)
		# print(out_DS)
		# final_df=None
		# final_df.drop(columns=column_names, axis=1, inplace=True)

		## csv writer method
		# ## if each row is a list 
		# with open (output_file_fp, "w") as fileObj:
		# 	writer = csv.writer(fileObj)
		# 	writer.writerow(column_names)
		# 	writer.writerows(final_ds_list)

		## if ech row is a dictionary
		print('writing out single CSV file at a time...')
		
		with open(output_file_fp, 'w', encoding='UTF8', newline='') as fileObj:
			writer = csv.DictWriter(fileObj, fieldnames=column_names)  # create a new instance of the DictWriter class by passing the file object
			writer.writeheader()
			writer.writerows(final_ds_list)
				
		print(output_file_fp)
		print('successfully finished writing a single CSV file!')

		## empty the list
		print("before empty the list...")
		print(len(final_ds_list))
		final_ds_list.clear()
		print("after empty the list...")
		print(len(final_ds_list))

# check runtime
t2 = dt.datetime.now()
runtime = t2-t1
print("\nruntime: %s" %runtime)
print("\nFINISHED SUCCESSFULLY: we processed %d csv files. In next step we should use another code to merge all single csv files..." %atm_cntr)


#########################################################################################################################################################
# ## now merge all csv files into a single dataframe and write the output
# print("\n")
# print("now merging all CSV files...")
# csv_file_pattern = "april_2016_9cam_4bands_ILATM2*"+".csv"
# csv_files_list = glob.glob(os.path.join(trainingDS_dir, csv_file_pattern))
# print(len(csv_files_list))
# ## join files
# final_df = pd.concat(map(pd.read_csv, csv_files_list), ignore_index=False, axis=0)  # true: assigns a continuous index numbers for the merged DF; axis=0 along the rows
# out_DS  = os.path.join(trainingDS_dir, output_final_ds_label)
# final_df.to_csv(out_DS, index=False) # do not include index column in output csv
# print(out_DS)
# print('successfully finished writing final DataFrame!')

#########################################################################################################################################################

