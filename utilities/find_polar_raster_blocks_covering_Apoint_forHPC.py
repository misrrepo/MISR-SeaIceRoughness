#!/usr/bin/env python
# coding: utf-8

# In[1]:


import rasterio, os, glob, shutil, pyproj
from rasterio.windows import Window
from pyproj import Transformer


# ### How to use this code
# 
# first find the coordinates of the point of interest inside QGIS. decide the EPSG code. then run this code and find the blocks that POI falls inside them

# In[2]:


# find lat-lon inside block and use it here

# location of small island north of Greenland for 16/july/2016
lat = -809888  # X in QGIS
lon = -303799 # Y in QGIS


# In[3]:


# where all polar raster blocks are

home_dir = "/data/gpfs/assoc/misr_roughness/2016/july_2016/predict_roughness_k_zero_npts_10/roughness_subdir_2016_7_15/2016_7_rasters_noDataNeg99_TiffFileFloat64_max_geographicalMesh_withLatLonList"



# In[4]:


print(os.path.isdir(home_dir))


# In[5]:


# with rasterio.open(path) as rds:
#     # convert coordinate to raster projection
#     transformer = Transformer.from_crs("EPSG:3995", rds.crs, always_xy=True)
#     xx, yy = transformer.transform(lon, lat)
#     row, col = rds.index(xx, yy)
#     print('row: %s, col: %s' %(row,col))
# #     window = Window.from_slices(rows=(row-2, row+2), cols=(col-2, col+2))
# #     data = rds.read(window=window)


# In[6]:


# dataset = rasterio.open(path)


# In[7]:


# dataset.bounds


# In[8]:


file_pattern = 'raster_path*_3995.tif'
rasters_list = glob.glob(os.path.join(home_dir, file_pattern))
print(len(rasters_list))


# In[10]:


# create a directory inside home-dir
target_dir_name = 'blocks_covering_my_point' 
target_dir_fp = os.path.join(home_dir, target_dir_name)
if (os.path.isdir(target_dir_fp) == False):
    print('target directory NOT exist, making it...')
    os.mkdir(target_dir_fp)
else:
    print('target dir exists!')


# In[11]:


for src_counter , src_file_fp in enumerate(rasters_list):
    # open files
#     dataset = rasterio.open(src_file_fp)
    with rasterio.open(src_file_fp) as dataset:
        # find edges
        #left edge
        leftlon = dataset.bounds[0]
        #bottom edge
        bottomlat = dataset.bounds[1]
        #right edge
        rightlon = dataset.bounds[2]
        #top edge
        toplat = dataset.bounds[3]

        # decide
        if (bottomlat < lat < toplat) and (leftlon < lon < rightlon):
            print('point is inside block')
            #### move these blocks inside target directory and then mosaic them
            ##- move files from src dir to target dir
            print('moving file: %s' %src_counter)
            shutil.move(src_file_fp, target_dir_fp)





