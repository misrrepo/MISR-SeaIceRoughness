#!/usr/bin/env python
# coding: utf-8

# In[1]:


# '''on my Mac'''

# # stereo_dir = "/Users/ehsanmos/MLP_dataset/cloud_mask_data/test_cloudmask_consensus/stereo"
# # angular_dir = "/Users/ehsanmos/MLP_dataset/cloud_mask_data/test_cloudmask_consensus/angular"
# cmcombo_dir = "/Users/ehsanmos/MLP_dataset/cloud_mask_data/test_cloudmask_consensus/cloudmask_combo"


# In[ ]:


'''on PH'''

cmcombo_dir = "/data/gpfs/assoc/misr_roughness/2016/cloud_masks/cmcombo_stereo_angular"


# In[2]:


import pandas as pd
import os, glob


# In[3]:


# in_csv = os.path.join(cmcombo_dir, 'cmcombo_P233_O087029_B014.csv')


# In[4]:


csv_list = glob.glob(os.path.join(cmcombo_dir, 'cmcombo_P*.csv'))
# csv_list

cloudyHC_total = 0
clearHC_total = 0

# open and read each file
for csvfile in csv_list:
    in_df = pd.read_csv(csvfile, dtype=str) # use dtype=str to read all columns as str
#     print(in_df.shape)
    # count number of HC pixels
    cloudyHC_cnt = in_df[in_df['CMcombo']=='11'].shape[0]
    clearHC_cnt = in_df[in_df['CMcombo']=='44'].shape[0]
    # add to total
    cloudyHC_total += cloudyHC_cnt
    clearHC_total += clearHC_cnt
    

print('cloudyHC_total: %s' %cloudyHC_total)
print('clearHC_total: %s' %clearHC_total)


# In[5]:


# cloudyHC_total


# In[6]:


# clearHC_total 


# In[7]:


# in_df = pd.read_csv(in_csv, dtype=str) # use dtype=str to read all columns as str


# In[8]:


# in_df.columns


# In[9]:


# in_df.dtypes


# In[10]:


# find the count of clearHC and coudyHC in a csv file

# cloudyHC_cnt = in_df[in_df['CMcombo']=='11'].shape[0]
# clearHC_cnt = in_df[in_df['CMcombo']=='44'].shape[0]


# In[11]:


# cloudyHC_cnt


# In[12]:


# clearHC_cnt

