#!/usr/bin/env python3


import csv, os
import pandas as pd
import numpy as np
import datetime as dt
from platform import python_version

# home where input is and output will be
home_dir = "/Users/ehsanmos/Documents/RnD/MISR_lab/ML_research/training_dataset/test_1000sample_DS"


# input merged.csv that will be filtered
in_file = "training_dataset_myMethod_merged_april_2016_9cam_4bands_1000sample.csv"


# name of output averaged csv- output will be saved at home directory
output_csv_label = "training_dataset_myMethod_averaged_april_2016_9cam3bands_pandasMethod_finalDS_forML_withCount_1000samples_2.csv"

##################################################################

# path to input dir
in_ds = os.path.join(home_dir, in_file)

# read input data to pandas DF
df1 = pd.read_csv(in_ds)
print("df1 shape: (%s, %s)" %df1.shape)



# change dtype of lin-sample columns from float to int and later filter based on int dtype
# print(df1.info())
print(df1['line'][0])
df1 = df1.astype({'path':int, 'orbit': int, 'block':int, 'line':int, 'sample':int})
# print(df1.info())
print(df1['line'][0])



# create DF2
df2_columns = ['path','orbit','block','line','sample','count','lat','lon','Da_r','Ca_r','Ba_r','Aa_r','An_r','An_g','An_b','An_nir','Af_r','Bf_r','Cf_r','Df_r','mean_ATM_roughness']
df2 = pd.DataFrame(columns=df2_columns)
print("df2 shape: (%s, %s)" %df2.shape)


average_cols = ['Da_r','Ca_r','Ba_r','Aa_r','An_r','An_g','An_b','An_nir','Af_r','Bf_r','Cf_r','Df_r','mean_ATM_roughness']
median_cols = ['lat', 'lon']

# calculate run time
t1 = dt.datetime.now()
print(t1)


# for irow in range(1000): #range(in_df.shape[0]):
for irow in range(df1.shape[0]):
    
    # print('\nrow: %d' %irow)
    path_num = df1['path'][irow]
    orbit_num = df1['orbit'][irow]
    block_num = df1['block'][irow]
    line_num = df1['line'][irow]
    sample_num = df1['sample'][irow]
    
    # print(type(path_num))
    # print(type(orbit_num))
    # print(type(block_num))
    # print(type(line_num))
    # print(type(sample_num))

    
    # check if POBLS exists in df2... 
    # https://stackoverflow.com/questions/17071871/how-do-i-select-rows-from-a-dataframe-based-on-column-values 
    row_in_df2 = df2.loc[(df2['path']==path_num) & (df2['orbit']==orbit_num) & (df2['block']==block_num) & (df2['line']==line_num) & (df2['sample']==sample_num)] # HOW DOES IT FILTER?
    # print(len(row_in_df2))

    if (len(row_in_df2 != 0)):
        # print('POBLS found inside df2, continue...')
        continue


    else: # filter df1 for POBLS and average the block/ or pixel? and join it to df2
        print('\nfound new-row not in df2...')
        # filter df1 for POBLS
        # define conditions seperately
        cond1 = (df1['path']==path_num)
        cond2 = (df1['orbit']==orbit_num)
        cond3 = (df1['block']==block_num)
        cond4 = (df1['line']==line_num)
        cond5 = (df1['sample']==sample_num)

        # apply filters to all df1
        df1_chunk = df1.loc[cond1 & cond2 & cond3 & cond4 & cond5]
        print('row: %d' %irow)
        # print(df1_chunk)
        print('chunk shape: (%d,%d)' %df1_chunk.shape)

        # average the filtered block
        df1_chunk_average = df1_chunk[average_cols].mean(axis=0)
        df1_chunk_median = df1_chunk[median_cols].median(axis=0)
        df1_chunk_count = df1_chunk['path'].count()

        # make a series from filtered criteria
        poblsc = pd.Series({
        'path':path_num,
        'orbit':orbit_num,
        'block':block_num,
        'line':line_num,
        'sample':sample_num,
        'count':df1_chunk_count
        }, dtype=int)

        # create a new row
        new_row = pd.concat([poblsc, df1_chunk_median, df1_chunk_average], axis=0) # axis=0 ==> along row/x axis
        # append it to df2
        print('appending new-row to df2')
        df2 = df2.append(new_row, ignore_index=True)  # check what happens to old df2, does it copy df2 to df2? what happens here?



print("\ndf2 final shape: (%s, %s)" %df2.shape)

# write out output
output_csv = os.path.join(home_dir, output_csv_label)
df2.to_csv(output_csv, index=False)
print(output_csv)

# calculate time
t2 = dt.datetime.now()
runtime = t2-t1
print("runtime: %s" %runtime)













# other tests for filtering df2
####################################################################    
# check if POBLS exists in df2... 
# wroooooooooong
# path_found_in_df2 = df2[df2['path']==path_num].index
# print(path_found_in_df2)
# orbit_found_in_df2 = df2[df2['orbit']==orbit_num].index
# block_found_in_df2 = df2[df2['block']==block_num].index
# line_found_in_df2 = df2[df2['line']==line_num].index
# sample_found_in_df2 = df2[df2['sample']==sample_num].index

####################################################################
# WROOOOOOOOOOOOOOOOOOOOOOONG!
# path_found_in_df2 = len(df2.loc[df2['path']==path_num])
# print(path_found_in_df2)
# orbit_found = len(df2.loc[df2['orbit']==orbit_num])
# print(orbit_found)
# block_found = len(df2.loc[df2['block']==block_num])
# print(block_found)
# line_found = len(df2.loc[df2['line']==line_num])
# print(line_found)
# sample_found = len(df2.loc[df2['sample']==sample_num])
# print(sample_found)

# if ((path_found!=0) & (orbit_found!=0) & (block_found!=0) & (line_found!=0) & (sample_found!=0)): # maybe issue here? why takes 104-2046 as found?
#     print('POBLS found inside df2, continue...')
#     continue

####################################################################
# WROOOOOOOOOOOOONG
# path_found_in_df2 = df2.loc[df2['path']==path_num]
# if (len(path_found_in_df2) != 0):
#     orbit_found_in_df2 = path_found_in_df2.loc[path_found_in_df2['orbit']==orbit_num]
#     if(len(orbit_found_in_df2 != 0)):
#         block_found_in_df2 = orbit_found_in_df2.loc[orbit_found_in_df2['block']==block_num]
#         if (len(block_found_in_df2 != 0 )):
#             line_found_in_df2 = block_found_in_df2.loc[block_found_in_df2['line']==line_num]
#             if(len(line_found_in_df2 != 0)):
#                 sample_found_in_df2 = line_found_in_df2.loc[line_found_in_df2['sample']==sample_num]
#                 if (len(sample_found_in_df2) !=0):
#                     print('POBLS found inside df2, continue...')
#                     continue
####################################################################



