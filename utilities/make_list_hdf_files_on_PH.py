#!/usr/bin/env python
# coding: utf-8

# it makes a list of available hdf files and writes out a CSV file

import os, glob, pandas as pd


src_dir = "/data/gpfs/assoc/misr_roughness/2016/july_2016/hdf_downloaded/july_2016_3cams"
output_dir = "/Users/ehsanmos/Documents/MISR/MISR-SeaIceRoughness/utilities"


# make a list 
hdf_file_pattern = "MISR_AM1_GRP_ELLIPSOID_GM*"
hdf_found_list = glob.glob(os.path.join(src_dir, hdf_file_pattern))


print(len(hdf_found_list))



# write a CSV file
output_csv_label = "list_of_hdf_files_on_PH.csv"
output_file_fp = os.path.join(output_dir, output_csv_label)

pd.DataFrame(hdf_found_list).to_csv(output_file_fp, index=False, header=False)
print(output_file_fp)






