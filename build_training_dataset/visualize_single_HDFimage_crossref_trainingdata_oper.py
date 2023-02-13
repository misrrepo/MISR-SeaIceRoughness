#!/usr/bin/env python
# coding: utf-8

# ## note: open w/ python3.6 in anaconda virtEnv

# In[1]:


import MisrToolkit as Mtk
from MisrToolkit import *
import os, glob
import pandas as pd
import numpy as np
from matplotlib import image as pltimg, pyplot as plt  #  pyplot uses the actual RGB values as they are, more accurate than PIL


######################################################################################################
################# functions start #################


# def find_unique_pob(atmmodel_file_fp, list_of_columns):
    
#     print('input file found: %s' %(os.path.isfile(atmmodel_file_fp)))
#     in_csv_df = pd.read_csv(atmmodel_file_fp)
#     print(in_csv_df.columns)
#     pob_unique = in_csv_df.drop_duplicates(subset=list_of_columns, keep='first', ignore_index=False)
#     pob_unique.info()
    
#     return pob_unique




# # from matplotlib import image as pltimg, pyplot as plt  #  pyplot uses the actual RGB values as they are, more accurate than PIL

# def plot_and_write(in_arr, img_dir, grid_name):
#     write_mode = 1
#     img_label = 'path_'+path_num+'_'+'blocks_'+str(start_block)+'_to_'+str(end_block)+'_'+cam+'_'+grid_name+cloudymode
#     img_format = ".jpg"
#     %matplotlib inline 

#     plt.gray() # This will show the images in grayscale as default

#     plt.figure(figsize=(20,20))  # set the figure size

#     plt.imshow(in_arr, cmap='viridis')
# #     plt.imshow(in_arr, cmap='gray')

#     plt.colorbar(shrink=0.2)
#     plt.show()
    
#     if (write_mode):
#         print('save mode: on, we write images to disc')
        
#         out_img = img_label+img_format
#         out_img_fullpath = os.path.join(img_dir, out_img)
# #         print("output img is: %s" %out_img_fullpath)
#         print("output img is:")
#         print(out_img_fullpath)
        
#         pltimg.imsave(out_img_fullpath, in_arr)
#         #     plt.savefig(out_img)
    
#     return 0




# from matplotlib import image as pltimg, pyplot as plt  #  pyplot uses the actual RGB values as they are, more accurate than PIL

def visualize_drawline_save(in_arr, img_dir, out_img, row_num, col_num):
    
    write_mode = 1
    
    # img_label = 'path_'+str(path)+'_'+'blocks_'+str(block1)+'_to_'+str(block2)+'_'+cam+'_'+grid_name+'_CM'+str(cloud)+'_L'+str(row_num)+'_S'+str(col_num)
    # img_format = ".jpg"
    # get_ipython().run_line_magic('matplotlib', 'inline')

    plt.gray() # This will show the images in grayscale as default

    plt.figure(figsize=(20,20))  # set the figure size

#     plt.imshow(in_arr, cmap='viridis')
    plt.imshow(in_arr, cmap='gray')

    plt.colorbar(shrink=0.2)
    
    plt.axhline(row_num, color='r', linewidth=0.8)
    plt.axvline(col_num, color='r', linewidth=0.8)
    
#     plt.show()
    
    
    if (write_mode):
        # print('save mode: on, we write images to disc')
        
        # out_img = img_label+img_format
        out_img_fullpath = os.path.join(img_dir, out_img)
#         print("output img is: %s" %out_img_fullpath)
        print("output image:")
        print(out_img_fullpath)
        
#         pltimg.imsave(out_img_fullpath, in_arr)
        plt.savefig(out_img_fullpath, dpi=300)

    # plt.show()

    return 0




def build_hist(in_data):
    
    data_flat = in_data.flatten()
    data_flat.shape
    # np.histogram(redband_img)
    ret_hist = plt.hist(data_flat, bins='auto')
    plt.show()
    return 0

################# functions end #################
######################################################################################################

# In[3]:

camera = "CF"  # to label output image

'''setup HDF ellipsoid and trainign DS path'''
hdf_home = '/data/gpfs/assoc/misr_roughness/2016/april_2016/hdf_files/hdf_inrange_15_30_april2016_9cams'

#> folder where labeled images will be saved
save_img_home = '/data/gpfs/assoc/misr_roughness/2016/april_2016/training_label_images'     # should be available on computer

#> training DS- atmmodel.csv with cmcombo double digits after filtering
csv_home = save_img_home
csv_fname = 'trainingDS_cloudmask_april_2016_k_zero_9cams3ANbands_withcmcombo_filtered4clouds.csv'


'''read training datset.csv'''

csv_df = pd.read_csv(os.path.join(csv_home, csv_fname)) # skip header?
print(csv_df.head())


# In[4]:


# ''' read first line of training data'''
''' read each line of training data in a for-loop'''

for line_num in range(csv_df.shape[0]):

    # line_num = 0
    print('\nexample: %s' % line_num)

    POBLS = csv_df.loc[line_num, ['path', 'orbit', 'img_block', 'line', 'sample', 'cloud']]
    # print(POBLS)


    # In[5]:


    '''find cloudy mode'''

    cloudymode = int(POBLS[-1])
    # print(cloudymode)


    # In[6]:


    '''find PO'''

    path_num = int(POBLS[0])
    orbit_num = int(POBLS[1])
    # print(path_num)


    # In[7]:


    ''' find block, line, sample
        to visualize a single block/or couple of blocks'''

    block = int(POBLS[2])
    line = int(POBLS[3])
    sample = int(POBLS[4])


    # In[8]:


    start_block = block
    end_block = block


    # In[9]:


    '''find hdf file based on PO pattern'''

    hdf_pattern = 'MISR_AM1_GRP_ELLIPSOID_GM_P'+str(path_num)+'_O0'+str(orbit_num)+'_'+camera+'_F03_0024.hdf'
    # hdf_pattern

    '''check if hdf file is available'''
    hdf_fpattern_fp = os.path.join(hdf_home, hdf_pattern)
    # hdf_fpattern_fp

    # print('check if HDF file exists')
    if (not os.path.isfile(hdf_fpattern_fp)):
        print('HDF file not exist, continue')
        continue


    # In[10]:


    hdf_fp = glob.glob(hdf_fpattern_fp)[0]
    print(hdf_fp)


    # In[11]:


    # open hdf file/ok
    # extract image block /ok
    # grid & field/ok
    # vis + save image


    # In[12]:




    # In[15]:


    '''set grid and field params for ellipsoid'''

    grid_name = 'RedBand'
    field_name = 'Red Radiance/RDQI'


    # In[18]:


    '''read HDF file'''

    input_hdf = MtkFile(hdf_fp)


    # In[49]:


    # '''some stats from HDF file'''

    # input_hdf.file_type
    # input_hdf.block
    # input_hdf.grid
    # input_hdf.grid_list
    # input_hdf.grid(grid_name).field_list
    # input_hdf.grid(grid_name).native_field_list


    # In[53]:


    '''define region and read HDF data'''

    region1 = MtkRegion(int(path_num), start_block, end_block)
    # type(region1)
    region_data_read = input_hdf.grid(grid_name).field(field_name).read(region1).data()


    # In[56]:


    '''check fill values
        note: 65515 is fill value for padding parts (sides) of the image = white part'''


    fill_value = input_hdf.grid(grid_name).field(field_name).fill_value # where does this value fill?
    # print('fill value is: %d' %fill_value)


    # In[58]:


    # '''some stats from data'''

    # print("region_data_read min: %d" %region_data_read.min())
    # print("region_data_read med: %d" %np.median(region_data_read))
    # print("region_data_read max: %d" %region_data_read.max())
    # print(region_data_read.shape)
    # print(type(region_data_read))


    # In[59]:


    # '''write data as binary cloudmask.msk'''

    # bin_fname = "cloudmask_P229_O086854_B149.msk"
    # bin_fname_dir = hdf_dir
    # bin_fname_fp = os.path.join(bin_fname_dir, bin_fname)

    # region_data_read.astype('uint8').tofile(bin_fname_fp) 
    # print(bin_fname_fp)


    # qn- what is the range of values in RCCM cloud mask data? [0,?]
    # 
    # A single RCCM value, established as follows: Assign numerical values of 0, 1, 2, 3 and 4 respectively to the conditions No Retrieval, CloudHC, CloudLC, ClearLC, and ClearHC for all RCCM’s that mapped to the subregion:
    # 
    # 0 = NoRetrieval
    # 
    # 1 = CloudHC
    # 
    # 2 = CloudLC
    # 
    # 3 = ClearLC
    # 
    # 4 = ClearHC
    # 
    # 
    # note: 255 is fill value around the edges of each block. for blocks that are mosaicked in a single path, since the edges are 255, the plot looks weird and we cannot see the cloudmask values properly. Use each block at a time.

    # In[65]:


    # '''look at read data'''
    # region_data_read.shape
    # region_data_read
    # print(region_data_read.dtype)
    # print(type(region_data_read))


    # In[38]:


    '''build histogram'''
    # build_hist(region_data_read)


    # In[72]:


    '''
    replace fill value with zero- helps see a clear image; 
    fill values are the dark edges on both sides of the image; we replace 65515 with -100;
    '''

    region_data_read_fillValuesRemoved = np.where(region_data_read > 65000, -100, region_data_read)  # (condition, x, y) == Where True, yield x, otherwise yield y.
    # region_data_read_fillValuesRemoved = np.where(region_data_read < 0, 0, region_data_read)  # (condition, x, y) == Where True, yield x, otherwise yield y.
    # print(region_data_read_fillValuesRemoved.shape)
    # print(type(region_data_read_fillValuesRemoved))


    # In[73]:


    # build_hist(region_data_read_fillValuesRemoved)


    # In[74]:


    # plot_and_write(region_data_read_fillValuesRemoved, hdf_dir, grid_name)


    # In[75]:


    # plot_here_and_write(flat_img_in_range_2D_int, hdf_dir)


    # In[76]:


    # In[77]:


    '''draw a line on the image and save it'''

    img_label = 'path_'+str(path_num)+'_'+'blocks_'+str(start_block)+'_to_'+str(end_block)+'_'+camera+'_'+grid_name+'_CM'+str(cloudymode)+'_L'+str(line)+'_S'+str(sample)
    img_format = ".jpg"
    output_img = img_label+img_format
    output_img_fp = os.path.join(save_img_home, output_img)
    # print(output_img_fp)

    if (os.path.isfile(output_img_fp)):
        print('image exists on computer')
        continue
    else:
        # visualize_drawline_save(region_data_read_fillValuesRemoved, save_img_home, path_num, start_block, end_block, camera, grid_name, cloudymode, line, sample)
        visualize_drawline_save(region_data_read_fillValuesRemoved, save_img_home, output_img, line, sample)



