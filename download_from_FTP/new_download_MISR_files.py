#!/usr/bin/python

import glob, os
from subprocess import call

# email_dir_fullpath = '/Users/ehsanmos/Documents/MISR/MISR-roughness/email_orders/april_2016'
email_dir_fullpath = '/data/gpfs/home/emosadegh/MISR-roughness/email_orders/april_2016'

file_pattern = 'hdf_*.txt'
file_pattern_fullpath = os.path.join(email_dir_fullpath, file_pattern)

orders_list = glob.glob(file_pattern_fullpath)
print('-> orders (hdf files) found= %s' %len(orders_list))

order_to_download_list = []

for order in orders_list:
	with open(order, 'r') as order_obj:
		found = 0
		for row in order_obj:
			# print('-> row= %s' %row)
			row_token = row.split(".")  # plit each row based on seperator
			# print(row_token)

			first = row_token[0].split(":")[0] 
			last = row_token[-1][0:2]   # we only need 1st 2 characters: indexes 0 and 1
			# print(len(first))
			# print(len(last))

			if (len(row_token) == 0):
				continue
			if (first == 'https' and last == 'gz'):
				# print('-> order found: %s' %row)
				order_to_download_list.append(row)
				found = found + 1

		if (found == 0):
			print('-> WARNING: order NOT found in file %s' %order)

#~ now download each tar.gz file found w/ wget utility
for https_path in order_to_download_list:
	command = "wget "+https_path
	print('===============================================================================')
	print('-> connecting to the server & downloading order...')
	print('-> command == %s' %command)

	ret = call(command, shell=True) 	#~ note: shell=true to execute the line as full command including arguments
	if (ret == 0):
		print('-> command finished with success!')
	else:
		print('-> ERROR: Command failed with return code!')
		raise SystemExit()
