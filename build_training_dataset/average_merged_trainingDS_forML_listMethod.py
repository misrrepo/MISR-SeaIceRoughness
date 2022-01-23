#!/usr/bin/env python
"""
usage:

"""
import pandas as pd
import numpy as np
import csv, os
import datetime as dt

t1=dt.datetime.now()



# home_dir = "/media/ehsan/6T_part1/14528_apr2016/project_april_2016_9cam3bands/training_dataset/test_runtime_averaging"
home_dir = "/Users/ehsanmos/Documents/RnD/MISR_lab/ML_research/training_dataset"

input_csv_file = "merged_april_2016_9cam3bands_smallDSfortest.csv"


output_csv_name = "april_2016_9cam3bands_finalDS_forML_list_method.csv"
input_csv = os.path.join(home_dir, input_csv_file)

with open (input_csv, 'r') as inCSV:
	reader = csv.reader(inCSV)
	list_a = list(reader)


len(list_a)
# type(data_list)

np.array(list_a).shape


data_arr = np.array(list_a)


for i in range(2):
	print(data_arr[i,:])


len(list_a[0])


# ## clean data from any NaN or zero or fillValues





# ## process final dataset

temp_list_float = []
list_b = []


# for datarow in list_a:
for rowA_cntr_1 in range(1,len(list_a),1):
	print("\n")
	print("list-A row num.: (%d)" %rowA_cntr_1)

	## find path, orbit, block,...for each row
	new_rowA_1 = list_a[rowA_cntr_1]
	
	# print("new row from list-A")
	# print(new_rowA_1)

	path_int = int(new_rowA_1[0])
	# print("path: %d" %path_int)

	orbit_int = int(new_rowA_1[1])
	# print("orbit: %d" %orbit_int)

	block_int = int(new_rowA_1[2])
	# print("blokc: %d" %block_int)

	## we have float here for 4 & 5
	line_int = int(float(new_rowA_1[3]))
	# print("line: %d" %line_int)
	
	sample_int = int(float(new_rowA_1[4]))
	# print("sample: %s" %sample_int)
	

	## 1st check listB if it includes data
	if (len(list_b) == 0): ## we have 1st entry, we search listA for similar data
		## we add the 1st row to temp-list
		## do list comp. to convert str to int dtype for later that we need to compute mean()
		# temp_list_float.append([float(i) for i in new_rowA_1])

		print("list-B: empty, we search list-A for similar rows")
		# for row_a in list_a:
		for rowA_cntr_2 in range(1, len(list_a), 1):

			new_rowA_2 = list_a[rowA_cntr_2]

			## search list-A for similar rows and add them to temp-list as 2nd rows- percision?
			if (int(new_rowA_2[0])==path_int and int(new_rowA_2[1])==orbit_int and int(new_rowA_2[2])==block_int and int(float(new_rowA_2[3]))==line_int and int(float(new_rowA_2[4]))==sample_int):
			# if (int(new_rowA_2[0])==path_int and int(new_rowA_2[1])==orbit_int and int(new_rowA_2[2])==block_int and int(new_rowA_2[3])==line_int and int(new_rowA_2[4])==sample_int):
				temp_list_float.append([float(i) for i in new_rowA_2])

		
		print("len(temp-list): %d" %len(temp_list_float))
		## change dtype for first 5 columns
		myarr1= np.array(temp_list_float)
		myarr1[:,0:5] = myarr1[:,0:5].astype(int)
		temp_list_float = myarr1.tolist()

		## using np.concatenate method
		# myarr1_pob = myarr1[:,0:5].astype(int)
		# print(myarr1_pob)
		# myarr1_deleted = np.delete(myarr1, [0,5], axis=1) # axis=1 along columns
		# print(myarr1_deleted)
		# new_arr = np.concatenate((myarr1_pob,myarr1_deleted), axis=1) # axis=1 along columns
		# temp_list_float = new_arr.tolist()
		# print(temp_list_float)

		## compute parametrs for temp_list_float
		print("compute mean for all rows in temp-list & add to list-B...")
		temp_arr = np.array(temp_list_float)
		temp_arr_mean = np.mean(temp_arr, axis=0)  # 0 -> over columns
		temp_list_float_mean = temp_arr_mean.tolist()
		## append temp_list_float to listB
		list_b.append(temp_list_float_mean)
		print("len(list-B): %d" %len(list_b))

		## empty list from previous data
		temp_list_float.clear()
		
		continue
	

	
	if (len(list_b) != 0):

		found_similar_row = 'false'
		## here means listB not empty
		### what we do with new_row?
		## check if new-row exists in listB

		print("list-B NOT empty...")
		## check listB if we can find a similar rowA
		for row_b in list_b:
			
			# print("check againt this row in list-B:")
			# print(row_b)
			# print([j for j in row_b])

			## for search, we check int(line, sample) againt int(), others will be str
			# print("checking list-B for similar row...")
			# if (int(row_b[0])==path_int and int(row_b[1])==orbit_int and int(row_b[2])==block_int and int(row_b[3])==line_int and int(row_b[4])==sample_int):
			if (int(row_b[0])==path_int and int(row_b[1])==orbit_int and int(row_b[2])==block_int and int(float(row_b[3]))==line_int and int(float(row_b[4]))==sample_int):
				
				print("found similar row in list-B; break")
				found_similar_row = 'true'
				break ## will go to next rowA
			
			# else:
			# 	print("did not match- will check next row in list-B")





	if (found_similar_row=='true'):

		print("continue to list-A")
		continue ## to next iteration for rowA

	else:

		## did not find in list-B, will add the new row to temp-list and search for similar rows in listA
		print("did not find similar row in list-B")
		print("will search list-A for similar rows...")
		for rowA_cntr_3 in range(1, len(list_a), 1):

			new_rowA_3 = list_a[rowA_cntr_3]

			## look for other similar rows
			# if (int(new_rowA_3[0])==path_int and int(new_rowA_3[1])==orbit_int and int(new_rowA_3[2])==block_int and int(new_rowA_3[3])==line_int and int(new_rowA_3[4])==sample_int):
			if (int(new_rowA_3[0])==path_int and int(new_rowA_3[1])==orbit_int and int(new_rowA_3[2])==block_int and int(float(new_rowA_3[3]))==line_int and int(float(new_rowA_3[4]))==sample_int):
				
				# print("found a similar row in list-A; appending")
				temp_list_float.append([float(i) for i in new_rowA_3])


		print("len(temp-list): %d" %len(temp_list_float))
		## change dtype for first 5 columns
		myarr1= np.array(temp_list_float)
		myarr1[:,0:5] = myarr1[:,0:5].astype(int)
		temp_list_float = myarr1.tolist()

		print("compute mean for all rows in temp-list & add to list-B...")
		## compute parametrs for temp_list_float
		temp_arr = np.array(temp_list_float)
		temp_arr_mean = np.mean(temp_arr, axis=0)  # 0 -> over columns
		temp_list_float_mean = temp_arr_mean.tolist()
		## append temp_list_float to listB
		list_b.append(temp_list_float_mean)
		print("len(list-B): %d" %len(list_b))

		## empty list from previous data
		temp_list_float.clear()


##########################################################################
## write the output

print("\n")
len("clen of listB: %d " %len(list_b))
header = list_a[0]

output_csv_file_fp = os.path.join(home_dir, output_csv_name)

with open (output_csv_file_fp, "w") as output_file:
	## create writer for output-file
	writer = csv.writer(output_file)

	# write the header
	writer.writerow(header)

	# write multiple rows
	writer.writerows(list_b)

print("final output file:")
print(output_csv_file_fp)

# check shape final csv
df2 = pd.read_csv(output_csv_file_fp)
print("df2 final shape: (%s, %s)" %df2.shape)

t2=dt.datetime.now()
runtime=t2-t1
print("runtime: %s" %runtime)
##########################################################################







