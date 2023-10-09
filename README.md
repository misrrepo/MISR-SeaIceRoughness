
README


Welcome to the MISR sea ice roughness and cloud mask repository. 
----------------------------------------------------------------------------------------
This repository is a program library to build sea ice roughness and cloud mask data products from MISR L1B2 ellipsoid ancillary data products. The focus of this library is the Arctic, north of 60.

This code library has the ability to:

-	Download L1B2E HDF files
-	Convert radiance data to TOA reflectance 
-	Mask land in reflectance data for each block
-	Apply operational MISR cloud masks (ASCM and SDCM)
-	Produce NNCM cloud mask (using the angular signature from 9 cameras)
-	Build training dataset for data models
-	Build a prediction model based on KNN 
-	Produce georeferenced sea ice roughness raster files 
-	Produce mosaics from raster files

This library is tested on Linux Red Hat 4.8.5 and Mac OS 13.5.2. Most of the core processing programs are written in C. Each source code in C uses a Python script (3.6 supported) to run.


Before you start:
----------------------
Before building the source codes in this library on your machine, you need to install the MISR ToolKit (MTK) library on your Machine. You can follow the steps explained at the MISR Toolkit repository: https://github.com/nasa/MISR-Toolkit 

For the post-processing steps, you need to install the GDAL library on your machine. You can install the GDAL library using the following link: https://anaconda.org/conda-forge/gdal

Note: you can proceed to the next steps after you have successfully installed MISR-toolkit and GDAL libraries on your machine.


Installation instructions and steps:
--------------------------------------------


Step 1) Download HDF files
--------------------------
This library uses the following MISR data product: MISR Level 1B2 Ellipsoid Data- MI1B2E

You can order HDF files from the Langley Research Center repository at the following link: 
https://l0dup05.larc.nasa.gov/cgi-bin/MISR/main.cgi

You can use the following Python script to download the files:
/MISR-SeaIceRoughness/download_from_FTP/multi_download_NASA_modified_april2016ASCM.py


Step 2) Convert TOA radiance to reflectance for each MISR block
------------------------------------------------------------------------------------
This module processes each MI1B2E HDF file and converts Top of Atmosphere (TOA) radiance to reflectance for each MISR block. The source codes are written in C and the run scripts are written in Python:

MISR-SeaIceRoughness/toa_rad_to_refl_to_blocks/TOARad2Refl4AllBlocks_allCameras.c
MISR-SeaIceRoughness/toa_rad_to_refl_to_blocks/TOARad2Refl4AllBlocks_3Cameras.c
MISR-SeaIceRoughness/toa_rad_to_refl_to_blocks/run_TOARad2Refl4AllBlocks_allCameras.py
MISR-SeaIceRoughness/toa_rad_to_refl_to_blocks/un_ TOARad2Refl4AllBlocks_3Cameras.py

Note 1: Copy a Makefile based on the type of your machine (Linux or MacOS). Then, set up paths to libraries in the Makefile. The Makefile requires the following libraries to be installed on your machine: MTK, fftw, libpng, hdfeos, and math. You should also set up paths to a directory where executables will be written to.

Note 2: There are two source codes available for this module: one code that runs with 3 cameras as input features and one that runs all 9 cameras as input features. based on the need of your project, select either 3- or 9-camera version of the source code (C),  set up directory paths in the Python run script, build the executable from the C source code using the make command, set up the input and output paths in the Python run script, and run the Python run script. There are scripts that you can run your code as a job on an HPC. You need to set it up based on the resources available to your project on your HPC.


Step 3) apply Land-mask to toa-refl files
----------------------------------------------------
This module runs the following land-mask source codes that masks land in TOA reflectance files produced from the previous step.

MISR-SeaIceRoughness/land_mask/LandMask_3cams.c
MISR-SeaIceRoughness/land_mask/LandMask_allCameras.c
MISR-SeaIceRoughness/land_mask/run_LandMask_3cams.py
MISR-SeaIceRoughness/land_mask/run_LandMask_allCameras_april_2016.py

Note 1: set up paths in the Python run script, build the executable from the C source code using the make command, set up the input and output paths in the Python run script, and run the Python run script. There are scripts that you can run your code as a job on an HPC. You need to set it up based on the resources available to you on your HPC.


Step 4) Build Cloud-mask files
---------------------------------------
This module reads operational MISR cloud mask files and builds cloud mask binary files on your system. Cloud mask files are used to mask cloudy pixels in the training dataset (next step).
This module runs a program that can process the MISR cloud mask data products, i.e. ASCM, SDCM, and RCCM, and prepare cloud mask files for each block. This module can process the following data products: SDCM and ASCM.

MISR-SeaIceRoughness/cloud_mask/ReadMISRCloudMask.c
MISR-SeaIceRoughness/cloud_mask/run_ReadMISRCloudMask.c

Note 1: First, download MISR cloud mask files to your machine. You can find cloud mask data products from the download link at step 1.

Note 2: set up paths in the Python run script, build the executable from the C source code using the make command, set up the input and output paths in the Python run script, and run the Python run script. There are job scripts that you can run your code as a job on an HPC. You need to set up the resources in the job scripts based on the resources available to your project on your HPC.


Produce NNCM cloud mask (using the angular signature from 9 cameras)
-----------------------------------------------------------------------------------------------





Step 5) Build the training dataset
-------------------------------------------
This module builds a training dataset for building data models. The input features/samples are TOA reflectance values. To label each data sample, we used the lidar data from Airborne Topographic Mapper (ATM) with the following ATM file version from IceBridge Data Portal: IceBridge ATM L2 Icessn Elevation, Slope, and Roughness V002.
You can download ATM files from https://nsidc.org/icebridge/portal/map

This step uses masked-toa-refl files that are produced from the previous step, and ATM files to label each feature sample and cloud-mask files. The output of this step is a training dataset for the next step (SIR model). In this step, we have the option of using cloud-mask files. Read instructions in the Python run script and especially look for cloudMask_runMode flag in source code. 

MISR-SeaIceRoughness/build_training_dataset/build_trainingData_3cams.c
MISR-SeaIceRoughness/build_training_dataset/build_trainingData_9cams.c
MISR-SeaIceRoughness/build_training_dataset/run_build_trainingData_3cams.c
MISR-SeaIceRoughness/build_training_dataset/run_build_trainingData_9cams.c

Note 1: Set up paths in the Python run script, build the executable from the C source code using the make command, set up the input and output paths in the Python run script, and run the Python run script. There are scripts that you can run your code as a job on an HPC. You need to set it up based on the resources available to your project on your HPC.


Step 6) Build the prediction model
---------------------------------------------
This module builds the sea ice roughness model from the training dataset that was built from the previous step. The algorithm we used for our model is based on K-nearest neighbor algorithm that is described in Mosadegh and Nolin (2023). The inputs to this module are masked-toa-refl files from step-1 and the training dataset from previous step. The putout is the predicted roughness for each block in binary format. 

In this module, there are 2 versions of the roughness model that you can use based on the need of your project: The serial version, which only uses one CPU to produce the output, and the parallel version can use several CPU cores to run in the parallel mode. 

MISR-SeaIceRoughness/prediction_knn/build_trainingData_3cams.c
MISR-SeaIceRoughness/prediction_knn/build_trainingData_9cams.c
MISR-SeaIceRoughness/prediction_knn/run_build_trainingData_3cams.c
MISR-SeaIceRoughness/prediction_knn/run_build_trainingData_9cams.c

Note: Set up paths in the Python run script, build the executable from the C source code using the make command, set up the input and output paths in the Python run script, and run the Python run script. There are scripts that you can run your code as a job on an HPC. You need to set it up based on the resources available to your project on your HPC.


Step 7) Post-processing roughness data
---------------------------------------------------
This module post-processes binary sea ice roughness files from the previous step, produces georeferenced raster files in TIF format from those binary files, builds a mosaic from SIR raster TIF files to build a map. This module is written in Python. This python scripts uses the GDAL library.

The following are the order of programs to run in this module:

MISR-SeaIceRoughness/post_processing/roughness_array2raster_georefrencing.py
This python script reads each SIR binary (DAT) file, georeferences it, and outputs a SIR raster file.

MISR-SeaIceRoughness/post_processing/mosaic_part1_gdalvrt_build_VRT.py
This Python script reads all raster files to memory and builds a virtual VRT file in memory. 

MISR-SeaIceRoughness/post_processing/mosaic_part2_gdalvrt_build_mosaic_fromVRT.py
This Python script builds a mosaic from VRT file format and outputs the mosaic file to the local machine.

Then, you can plot the final mosaic.tif in QGIS.








