#!/usr/bin/python3.6
####################################
import pandas as pd
import glob, os, csv
####################################
trainingDS_dir = "/media/ehsan/6T_part1/14528_apr2016/project_april_2016_9cam3bands/training_dataset/test_runtime_averaging"
####################################
output_final_ds_label = "merged_april_2016_9cam3bands.csv"
####################################

## now merge all csv files into a single dataframe and write the output
print("\n")
print("now merging all CSV files...")
csv_filePattern = "april_2016_9cam3bands_ILATM2*"+".csv"
csv_files_list = glob.glob(os.path.join(trainingDS_dir, csv_filePattern))
print("found: %d" %len(csv_files_list))
## join files
final_df = pd.concat(map(pd.read_csv, csv_files_list), ignore_index=False, axis=0)  # true: assigns a continuous index numbers for the merged DF; axis=0 along the rows
out_DS  = os.path.join(trainingDS_dir, output_final_ds_label)
final_df.to_csv(out_DS, index=False) # do not include index column in output csv
print(out_DS)
print('successfully finished writing final DataFrame!')