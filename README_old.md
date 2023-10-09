# MISR-roughness
This repository includes programs that will calculate the surface roughness from imagery data sets from NASA MISR satellite. This project is based on the research that is addresed in the following paper: https://www.mdpi.com/2072-4292/11/1/50/htm

# Steps:
1- TOA radiance to reflectance (at $HOME/MISR-SeaIceRoughness/toa_rad_to_refl_to_blocks)
2- LandSea Mask (at $HOME/MISR-SeaIceRoughness/land_mask)
3- prepare atmmodel.csv (trainign dataset)
4- run MISR2roughness and prepare estimated/modeled roughness files (at $HOME/MISR-SeaIceRoughness/misr_to_roughness/)
