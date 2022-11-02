#!/usr/bin/env python

# Run TOA3 program on all sorted_files_list in list
# Sky Coyote 18 March 2009
# Must be run from /Volumes/MISR_Nolin_Mac/2007/Code/ ???
# usage:
#

#from __future__ import print_function
import sys, os, os.path, signal

#class Entry:
	# class attributes

	# def __init__(self, path, orbit, block):

	# 	self.path = path
	# 	self.orbit = orbit
	# 	self.block = block
	# 	self.cfprocessing_hdf_file = None
	# 	self.caprocessing_hdf_file = None
	# 	self.anprocessing_hdf_file = None

    # def sigintHandler(a, b):
    # 	sys.exit(0)

# if __name__ == '__main__':
#   signal.signal(signal.SIGINT, sigintHandler)

    #l = []
    #f = open('../PSU/POB_2011_SantiamPass.txt', 'r')
    #f = open('../PSU/POB_2013_April_SeaIce.txt', 'r')
#     """
#     f = open('../PSU/POB_2016_Apr_SeaIce.txt', 'r')
#     for line in f:
# 	fields = line.strip().split()
# 	path = int(fields[0])
# 	mask = int(fields[1])
# 	orbit = int(fields[2])
# 	block_l = int(fields[3])
# 	block_u = int(fields[4])
# 	if mask:
# 	    for block in range(block_l, block_u + 1):
# 		l.append(Entry(path, orbit, block))
#     f.close()
#     """
    # for Apr2016
    #orders = ['0624733845',
	  #'0624733846',
	  #'0624733854',
	  #'0624733866',
	  #'0624733868',
	  #'0624733869',
	  #'0624733870',
	  #'0624733871',
	  #'0624733872',
	  #'0624733873',
	  #'0624733874',
	  #'0624733875',
	  #'0624733902',
	  #'0624733912',
	  #'0624733913',
	  #'0624733915',
    	  #['0624733916',
	  #'0624733917',
	  #'0624733918',
	  #'0624733940',
    	  #'0624733944',
	  #'0624733951',
	  #'0624733952',
	  #'0624733953']

    # for MI1B2E Jul2016
    # for MI1B2E Aug2001
#     """
#     ,,''''orders = ['0624779399',
# 	  '0624779400',
# 	  '0624779401',
# 	  '0624779402',
# 	  '0624779415',
# 	  '0624779416',
# 	  '0624779417',
# 	  '0624779418',
# 	  '0624779419',
# 	  '0624779420',
# 	  '0624779421',
# 	  '0624779422',
# 	  '0624779424',
# 	  '0624779427',
# 	  '0624779429',
# 	  '0624779432',
# 	  '0624779433',
# 	  '0624779434',
# 	  '0624779435',
# 	  '0624779436',
# 	  '0624779437',
# 	  '0624779438',
# 	  '0624779439',
# 	  '0624779440',
# 	  '0624779441']
#     """
    # for MI1B2E May2016 days 1-5
    #orders = ['0624864632',
#	      '0624864633',
#	      '0624864634',
#	      '0624864635']
    # for MILB2E August 2001


# NOTE: /home/mare/ == /Volumes/MISR_REPO/


root_path = '/Volumes/MISR_REPO/Nolin/2001/TOA3/'
month = 'August/'
dat_png_dir = root_path+month
print(f'-> root dir is= {dat_png_dir}')




import sys, os, os.path, signal



##################################################################

download_dir = '/Volumes/MISR_REPO/Nolin/2001/Ml1b2e/August/' # what is idir??? is it idir?

dl_orders_list = ['0627517785']# , '0627517787' , '0627517788']  # hdf files are here in each order dir

bands = ['Red'] # why only red?
minnaert = 0

# png and dat directories - is it destination dir? / toa dir?
TOA_dir_list = ['/Volumes/MISR_REPO/Nolin/2001/TOA3/August/Cf/',  # should I make them myself? what is this dir ?
	     								'/Volumes/MISR_REPO/Nolin/2001/TOA3/August/Ca/',
	     								'/Volumes/MISR_REPO/Nolin/2001/TOA3/August/Aa/']
	     #'/home/mare/Nolin/2009/TOA3/Apr_ascend/NIR/']

for order in dl_orders_list :  # we DL them in previous step; hdf and xml

	print(f'-> processing order dir = {order}')

	# only one path at a time
	local_DL_dir_fullpath = download_dir + order + '/'

	print( f'-> local DL dir= {local_DL_dir_fullpath}')

	#for local_order_dir_fullpath in local_DL_dir_fullpath: 	# what is d??? and local_DL_dir_fullpath ? issue: how get the dir itself? this way it only gets the characters in the path
		#print( type ( local_order_dir_fullpath))


	# make a list of all files in each DL dir
	sorted_files_list = sorted(os.listdir(local_DL_dir_fullpath))  # what are the files ? why sort?

	print('-> len of sorted file list is=')
	print(len(sorted_files_list))

	    # n = 0 # ???

	    # f == MISR_hdf_file if found anywhere

	  #### start working on each file

	for MISR_hdf_file in sorted_files_list :  # whow about xml and hdf file difference?
		#print(f'-> MISR file to process= {MISR_hdf_file}')

	###################### find the pattern in file ... #####################
	########## QA checking ???

		#if MISR_hdf_file.find('GRP_TERRAIN_GM') > -1 and MISR_hdf_file.endswith('.hdf'):

		# pick a MISR file and start processing it
		if MISR_hdf_file.find('GRP_ELLIPSOID_GM') > -1 and MISR_hdf_file.endswith('.hdf'): # find this pattern - use a different file pattern finder/catcher and get the necessary keywords from the file name. we should get the .hdf file

      ############################################# find the pattern for path -- where we use it? ######################
			index_of_path = MISR_hdf_file.index('_P') # finds the pattern
			path = int(MISR_hdf_file[ index_of_path + 2: index_of_path + 5])
			#print(f'-> path is= {path}')

        # find the pattern for orbit -- where we use it?
			index_of_orbit = MISR_hdf_file.index('_O')
			orbit = int(MISR_hdf_file[ index_of_orbit + 2: index_of_orbit + 8])
			#print(f'-> orbit is= {orbit}')

			# find the camera in the file name
			if MISR_hdf_file.find('_CF') > -1 :

				camera = 'cf'

			elif MISR_hdf_file.find('_CA') > -1 :

				camera = 'ca'

			elif MISR_hdf_file.find('_AN') > -1 :

				camera = 'an'

			else:

				camera = '??'

      ############################################# find the pattern for path -- where we use it? ######################

			#---> up to here
      # what is the process on the file ?
			processing_hdf_file = os.path.join( local_DL_dir_fullpath , MISR_hdf_file )
			# print(f'-> we are processing the file = {processing_hdf_file}')

			# ??????????????????????????????????????????
			# decide for block and band 

			#for e in l:  # ?????
			for block in range(1, 41) :  # block? range?
				#print(type(range(1,10)))

				for band in bands:  # why band range???
					#print(f'-> band is = {band}')

					# why check band ???
					if (band == 'NIR'): nband = 3 # ???
					elif (band == 'Red'): nband = 2  # what is nband?
					elif (band == 'Green'): nband = 1
					elif (band == 'Blue'): nband = 0
					#print(f'-> nband is= {nband}')

	    # ??????????????????????????????????????????

# 			# why skipping these orbit/block/orders and it goes to next block?

# 			if (orbit == 87985) and (block == 25) and (order == '0624779399'): continue
# 			if (orbit == 88220) and (block == 16) and (order == '0624779399'): continue
# 			if (orbit == 88390) and (block == 20) and (order == '0624779400'): continue
# 			if (orbit == 88269) and (block == 15) and (order == '0624779402'): continue
# 			if (orbit == 88143) and (block == 34) and (order == '0624779415'): continue
# 			if (orbit == 88153) and (block == 35) and (order == '0624779417'): continue
# 			if (orbit == 88030) and (block == 29) and (order == '0624779417'): continue
# 			if (orbit == 88030) and (block == 30) and (order == '0624779417'): continue
# 			if (orbit == 88091) and (block == 29) and (order == '0624779417'): continue
# 			if (orbit == 88096) and (block == 29) and (order == '0624779421'): continue
# 			if (orbit == 88245) and (block == 32) and (order == '0624779422'): continue
# 			if (orbit == 88116) and (block == 15) and (order == '0624779429'): continue
# 			if (orbit == 87979) and (block == 35) and (order == '0624779433'): continue
# 			if (orbit == 87979) and (block == 36) and (order == '0624779433'): continue
# 			if (orbit == 88342) and (block == 9) and (order == '0624779435'): continue
# 			#if ((e.cfprocessing_hdf_file != None) and (nband != 3)):

# #######################################################################################################

# Q- how define a good file name?



			if (camera == 'cf') and (nband != 3) :  # why? are we processing this?

				# naming .dat and .png files

				fname2 = '%stoa_p%03d_o%06d_b%03d_%s.dat' % (TOA_dir_list[0], path, orbit, block, 'cf') # [0] for cf
				#print(f'-> fname2 is= {fname2} and type= {type(fname2)}')


				fname3 = '%stoa_p%03d_o%06d_b%03d_%s.png' % (TOA_dir_list[0], path, orbit, block, 'cf')
				#print(f'-> fname3 is= {fname3}')

				#fname2 = '%smisr_p%03d_o%06d_b%03d_%s.dat' % (TOA_dir_list[0], e.path, e.orbit, e.block, 'cf')
				#fname3 = '%smisr_p%03d_o%06d_b%03d_%s.png' % (TOA_dir_list[0], e.path, e.orbit, e.block, 'cf')

    			## here .... what is cmd ?
                # is TOA3 a C-code? check the old script
               # check : https://www.geeksforgeeks.org/python-os-system-method/
               #------------------------------------------------- ???? 
				cmd = 'TOA3 "%s" %s %s %s \"%s\" \"%s\"' %(processing_hdf_file, block, nband, minnaert, fname2, fname3)
				# print(f'-> cmd is= { type(cmd)}')
                #------------------------------------------------
# # #######################################################################################################      

	      ### why check not ?
				#if (n >= 0):
				if not (os.path.exists(fname2) and os.path.exists(fname3)) :

					sys.stderr.write('%5d: %s\n' % (n, cmd))
					
					if os.system(cmd) != 0:
						sys.exit(1)

# # #######################################################################################################

# 			#if ((e.caprocessing_hdf_file != None) and (nband != 3)):
# 			if (camera == 'ca') and (nband != 3) : # why not checked?

# 				fname2 = '%stoa_p%03d_o%06d_b%03d_%s.dat' % (TOA_dir_list[1], path, orbit, block, 'ca')
# 				print(f'-> fname2= {fname2}')

# 				fname3 = '%stoa_p%03d_o%06d_b%03d_%s.png' % (TOA_dir_list[1], path, orbit, block, 'ca')
# 				print(f'-> fname3= {fname3}')

# 				#fname2 = '%smisr_p%03d_o%06d_b%03d_%s.dat' % (TOA_dir_list[1], e.path, e.orbit, e.block, 'ca')
# 				#fname3 = '%smisr_p%03d_o%06d_b%03d_%s.png' % (TOA_dir_list[1], e.path, e.orbit, e.block, 'ca')
# 				cmd = 'TOA3 \"%s\" %local_order_dir_fullpath %local_order_dir_fullpath %local_order_dir_fullpath \"%s\" \"%s\"' % (processing_hdf_file, block, nband, minnaert, fname2, fname3)
# 				print(f'-> cmd is= {cmd}')

# # #######################################################################################################

# 			    #if (n >= 0):
# 			if not (os.path.exists(fname2) and os.path.exists(fname3)) :
# 				sys.stderr.write('%5d: %s\n' % (n, cmd))
# 				if os.system(cmd) != 0:
# 					sys.exit(1)

# # #######################################################################################################

# 			#if (e.anprocessing_hdf_file != None):
# 			if (camera == 'an'):

# 			    if (nband == 3): idx = -1
# 			    elif (nband == 2): idx = 2
# 			    elif (nband == 1): idx = 1

# 			    #else: idx = 2
# 			    fname2 = '%stoa_p%03d_o%06d_b%03d_%s.dat' % (TOA_dir_list[idx], path, orbit, block, 'an')
# 			    fname3 = '%stoa_p%03d_o%06d_b%03d_%s.png' % (TOA_dir_list[idx], path, orbit, block, 'an')
# 			    #fname2 = '%smisr_p%03d_o%06d_b%03d_%s.dat' % (TOA_dir_list[band], e.path, e.orbit, e.block, 'an')
# 			    #fname3 = '%smisr_p%03d_o%06d_b%03d_%s.png' % (TOA_dir_list[band], e.path, e.orbit, e.block, 'an')
# 			    cmd = 'TOA3 \"%s\" %local_order_dir_fullpath %local_order_dir_fullpath %local_order_dir_fullpath \"%s\" \"%s\"' % (processing_hdf_file, block, nband, minnaert, fname2, fname3)
# 			    #if (n >= 0):
# 			if not (os.path.exists(fname2) and os.path.exists(fname3)) :
# 				sys.stderr.write('%5d: %s\n' % (n, cmd))
# 				if os.system(cmd) != 0:
# 					sys.exit(1)

# 			n += 1

# 		#sys.exit(0)











			#print(f'-> camera is= {camera}')
# 		    """ not mine
# 		    if camera == 'cf' or camera == 'ca' or camera == 'an':
# 			for e in l:
# 			    if e.path == path and e.orbit == orbit:
# 				if camera == 'cf':
# 				    e.cfprocessing_hdf_file = os.path.join(local_order_dir_fullpath, MISR_hdf_file)
# 				elif camera == 'ca':
# 				    e.caprocessing_hdf_file = os.path.join(local_order_dir_fullpath, MISR_hdf_file)
# 				elif camera == 'an':
# 				    e.anprocessing_hdf_file = os.path.join(local_order_dir_fullpath, MISR_hdf_file)

# 		    if camera == 'cf':
# 			cfprocessing_hdf_file = os.path.join(local_order_dir_fullpath, MISR_hdf_file)
# 		    elif camera == 'ca':
# 			caprocessing_hdf_file = os.path.join(local_order_dir_fullpath, MISR_hdf_file)
# 		    elif camera == 'an':
# 			anprocessing_hdf_file = os.path.join(local_order_dir_fullpath, MISR_hdf_file)
# 		    """ not mine



