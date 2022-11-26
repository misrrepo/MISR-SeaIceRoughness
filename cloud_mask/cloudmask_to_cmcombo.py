#!/usr/bin/env python
# coding: utf-8

# In[1]:


'''
Ehsan Mosadegh
21 Nov 2022

reads cloudmask.msk files and combine pixel values;
this code writes both csv(stats) file and binary file into a same directory (cmcombo_dir)

'''


# In[2]:


import glob, os
import numpy as np
import pandas as pd
import sys


# In[3]:


'''select run mode'''

run_cmcombo_csvstats = 0

run_cmcombo_binaryFile = 1


# In[4]:


# '''on my Mac'''

# stereo_dir = "/Users/ehsanmos/MLP_dataset/cloud_mask_data/test_cloudmask_consensus/stereo"
# angular_dir = "/Users/ehsanmos/MLP_dataset/cloud_mask_data/test_cloudmask_consensus/angular"
# cmcombo_dir = "/Users/ehsanmos/MLP_dataset/cloud_mask_data/test_cloudmask_consensus/cloudmask_combo"


# In[5]:


'''on PH'''

'''April-2016'''
# stereo_dir = "/data/gpfs/assoc/misr_roughness/2016/cloud_masks/april_2016_sdcm/cloudmask_TC_CLOUD_SDCM"
# angular_dir = "/data/gpfs/assoc/misr_roughness/2016/cloud_masks/april_15_30_2016_ascm/cloudmask_ASCM"
# cmcombo_dir = "/data/gpfs/assoc/misr_roughness/2016/cloud_masks/cmcombo_stereo_angular_april2016"


'''July-2016'''
stereo_dir = "/data/gpfs/assoc/misr_roughness/2016/cloud_masks/july_2016_sdcm/cloudmask_TC_CLOUD_SDCM"
angular_dir = "/data/gpfs/assoc/misr_roughness/2016/cloud_masks/july_10_25_2016_ascm/cloudmask_ASCM"
cmcombo_dir = "/data/gpfs/assoc/misr_roughness/2016/cloud_masks/cmcombo_stereo_angular_july2016"

# In[6]:


# make a list of files in stereo dir
stereo_filelist_fp = glob.glob(os.path.join(stereo_dir, "cloudmask_P*.msk"))
# print(len(stereo_filelist_fp))

# make a list just from cloudmask file names to make the comparison easier
stereo_filelist = []
for stereo_item in stereo_filelist_fp:
    stereo_filelist.append(stereo_item.split("/")[-1])
    
print(len(stereo_filelist))


# In[7]:


# make a list of files in angular dir
angular_filelist_fp = glob.glob(os.path.join(angular_dir, "cloudmask_P*.msk"))
# print(len(angular_filelist_fp))

# make a list just from cloudmask file names to make the comparison easier
angular_filelist = []
for angular_item in angular_filelist_fp:
    angular_filelist.append(angular_item.split("/")[-1])
    
print(len(angular_filelist))


# In[8]:


type(stereo_filelist[0])


# In[9]:


def write_cmcombo_binaryFile(cm_path, cm_orbit, cm_block, stereo_shp, stereo_f, angular_f):
    '''for each block of cloudmask.msk'''
    # check if cmcombo.csv is on disc, continue
    bin_fname = 'cmcombo'+'_'+cm_path+'_'+cm_orbit+'_'+cm_block+'.msk'
    
    if (os.path.isfile(os.path.join(cmcombo_dir, bin_fname))):
        print('cloudmask.msk is available on disc, continue to the next cloudmask.msk');
        #return continue #not needed; the print() works as the continue
        
    
    else:
        cmcombo_lst = [] # for each cloudmaskcombo

        # extract each pixel value from cloudmask.msk files
        for i in range(stereo_shp):  # make a combofile
            # extract each single element from both files
            stereo_elem = stereo_f[i]
            angular_elem = angular_f[i]
    
            #**** what is data type inside atmmodel.c ????
    
            # add both to a cmcombo number
            cmcombo = str(stereo_elem)+str(angular_elem)
            
            cmcombo = int(cmcombo)
            #print(type(cmcombo))
            cmcombo_lst.append(cmcombo)  # add as string

#         print('writing binary msk file %s to storage ...' %(cm_num+1))
#         print(cmcombo_lst[2000:3000])

        bin_fname_fp = os.path.join(cmcombo_dir, bin_fname)
        
        cmcombo_arr = np.array(cmcombo_lst)
#         print(cmcombo_arr.shape)
#         print(bin_fname_fp)
        cmcombo_arr.astype('uint8').tofile(bin_fname_fp) # maybe change dtype to uint_8 in C here? ref: https://os.mbed.com/handbook/C-Data-Types
        
#         sys.exit() # to stop and debug code at this line

    ''' 
    steps:
    
    1- how awrite int as binary? --> use numpy; done
    2- change dtype to uint8_t (8-bit unsigned char for C) equiv. to np.uint8 (8-bit unsigned int)
    
    ref: 
    https://numpy.org/doc/stable/reference/arrays.scalars.html#numpy.ubyte
    https://os.mbed.com/handbook/C-Data-Types
    '''
        
    return 0


# In[10]:


def write_cmcombo_stats_csvFile(cm_path, cm_orbit, cm_block, cmcombo_dir, stereo_shp, stereo_f, angular_f):
    
    '''for each block of cloudmask.msk'''
    # check if cmcombo.csv is on disc, continue
    csv_fname = 'cmcombo'+'_'+cm_path+'_'+cm_orbit+'_'+cm_block+'.csv'
    if (os.path.isfile(os.path.join(cmcombo_dir, csv_fname))):
        print('csv file is available on disc, continue to the next cloudmask.msk');
        #return continue #not needed; the print() works as the continue
        

    else:
        cmcombo_lst = [] # for each cloudmaskcombo

        # extract each pixel value from cloudmask.msk files
        for i in range(stereo_shp):  # make a combofile
    #         print('i is:%d'%i)
            # extract each single element from both files
            stereo_elem = stereo_f[i]
    #         print(type(stereo_elem))
            # same for angular
            angular_elem = angular_f[i]
            # check dtype
    #         print(type(angular_elem))

            # add both to a cmcombo number
            cmcombo = str(stereo_elem)+str(angular_elem)
    #         print('cmcombo: %s' %cmcombo)

            # append to cmcombo[] + other info: POB
            cmPOB = [cm_path, cm_orbit, cm_block, str(cmcombo)]
    #         print(cmPOB)
            cmcombo_lst.append(cmPOB)  # add as string

        # for later: write the cmcombo to an array and write as binary cmcombo; we will writ a single binary cmcombo for each pair of cloudmasks; we need these files for building future trainign dataset by Atmmodel.c; writefile can be a function for future use
    #     write_cloudmask_binary()

        # add cmcombo+POB info to global_cmcombo? here? for Anne
    #     print('adding cmcombo-list to global list') # makes it slow and not useful; better to write as csv to a single file on disc here
    #     global_cmcombo.append(cmcombo_lst)
        cmcombo_df = pd.DataFrame(cmcombo_lst, columns=['CMpath','CMorbit','CMblock','CMcombo'], dtype=str)
    # #     print(cmcombo_df.dtypes)
    #     cmcombo_df = cmcombo_df.astype(str)
    #     print(cmcombo_df.dtypes)

        print('writing stats file %s to csv...' %(cm_num+1))

    #     csv_fname = 'cmcombo'+'_'+cm_path+'_'+cm_orbit+'_'+cm_block+'.csv'
        csv_fname_fp = os.path.join(cmcombo_dir, csv_fname)
        cmcombo_df.to_csv(csv_fname_fp, index=False) # how write as string?? datatype??? change to string for easy comparison in future
        print(csv_fname_fp)

    return 0
    


# In[11]:


# check stereo file exists in angular file list?
# global_cmcombo = []

for cm_num, cm_fname in enumerate(stereo_filelist):
#     print(stereo_item2)
    if (cm_fname not in angular_filelist): # check if stereo cloudmask is not in angular filelist, continuw to next stereo
        print('cloudmask not found; continue')
        continue;
        
        
    # open and read stereo cloudmask
    stereo_fp = os.path.join(stereo_dir, cm_fname)
#     print(stereo_fp)
    stereo_f = np.fromfile(stereo_fp, dtype=np.ubyte) # this should be siilr to dtype from C
#     print(type(stereo_f))
    stereo_shp = stereo_f.shape[0]
#     print(type(stereo_shp))
    
    
    # open and read angular cloudmask
#     print('anguler files')
    angular_fp = os.path.join(angular_dir, cm_fname)
#     print(angular_fp)
    angular_f = np.fromfile(angular_fp, dtype=np.ubyte) # this should be siilr to dtype from C
#     print(type(angular_f))
    angular_shp = angular_f.shape[0]
    
    
    # check len(both files) if not similar then error msg
    if (stereo_shp != angular_shp):
        print('shape of cm-files not equal; continue');
        continue;
    
    
#     cmcombo_lst = [] # for each cloudmaskcombo
    # extract POB from filelabel
    print(cm_fname)
    cm_path = cm_fname.split("_")[1]
#     print(type(cm_path))
    cm_orbit = cm_fname.split("_")[2]
#     print(type(cm_orbit))
    cm_block = cm_fname.split("_")[3].split(".")[0]
#     print(type(cm_block))
    
    if (run_cmcombo_csvstats):
        write_cmcombo_stats_csvFile(cm_path, cm_orbit, cm_block, cmcombo_dir, stereo_shp, stereo_f, angular_f)

    if (run_cmcombo_binaryFile):
        write_cmcombo_binaryFile(cm_path, cm_orbit, cm_block, stereo_shp, stereo_f, angular_f)
    
    
print('*** successfully processed %s cmfiles ***' %(cm_num+1))
    
    

    
    
    
    
    
    
    


# In[12]:


# global_cmcombo_arr = np.array(global_cmcombo) # change to np.aaray


# In[13]:


# global_cmcombo_arr.shape


# In[14]:


# final_df = pd.DataFrame(global_cmcombo, columns=['a','b','c','d'])


# In[15]:


# final_df

