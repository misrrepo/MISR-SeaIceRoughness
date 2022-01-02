#!/usr/bin/env python

# import cv2
# import os

#==========================================================

# def get_the_day(list_elem):
# 	"gets each element and returns the first 2 characters"
# 	return list_elem[0:2]

# #==========================================================
# day='4'
# month='april'

# image_folder = '/Users/ehsanmos/Documents/RnD/MISR_lab/plot_dir.nosync/recent_test_plots/animation_test'
# #image_folder = '/storage/ehsanm/~/daily_plots/'+'scen_'+day+'/'+month

# print(f'-> day is= {day}')
# print(f'-> month is= {month}')
# print(f'-> image folder is: {image_folder}')

# video_name = 'animated_misr_daily_plots_day_'+day+'_'+month+'.avi' # only avi

# #==========================================================

# print('-> making the video...')
# # sorting based on fig number??????

# # image_list_unsorted = [img for img in os.listdir(image_folder) if img.endswith(".png")]
# image_list_unsorted = [img for img in os.listdir(image_folder) if img.endswith(".jpg")]  # should only be png?

# print('-> original list=')
# print(image_list_unsorted)

# image_list_sorted = sorted(image_list_unsorted , key=get_the_day)  # sorts the list based on first 2 days on the nametag
# #print('-> sorted list=')
# #print(image_list_sorted)

# #==========================================================

# frame = cv2.imread(os.path.join(image_folder, image_list_sorted[0]))
# height, width, layers = frame.shape
# fps=0.5
# video = cv2.VideoWriter(video_name, 0, fps, (width,height))

# print('-> writing the video from the following plots...')

# for image in image_list_sorted:

# 		print('-> adding the following plot to the video:')
# 		print(image)

# 		video.write(cv2.imread(os.path.join(image_folder, image)))

# cv2.destroyAllWindows()
# video.release()

########################################################################################################################
#~~ second test
# import glob
# import os

# gif_name = 'outputName'
# file_list = glob.glob('*.jpg') # Get all the pngs in the current directory
# list.sort(file_list, key=lambda x: int(x.split('_')[1].split('-')[0])) # Sort the images by #, this may need to be tweaked for your use case

# with open('image_list.txt', 'w') as file:
#     for item in file_list:
#         file.write("%s\n" % item)

# os.system('convert @image_list.txt {}.gif'.format(gif_name)) # On windows convert is 'magick'
########################################################################################################################
#~~ new test w/ matplotlib
# import matplotlib.animation as ani







# convert -delay 200 -loop 0 *.jpg animated.gif










########################################################################################################################
# #~~ test moviePy
# import glob
# import moviepy.editor as mpy

# #~~ writes out gif in the same directory
# gif_name = 'test_misr_images'
# fps = 10

# file_list = glob.glob('*.jpg') # Get all the pngs in the current directory

# list.sort(file_list, key=lambda x: int(x.split('_')[1].split('-')[0])) 	# Sort the images by #, this may need to be tweaked for your use case

# clip = mpy.ImageSequenceClip(file_list, fps=fps)
# clip.write_gif('{}.gif'.format(gif_name), fps=fps, progress_bar=True)  # formats= mp4 & gif
# ########################################################################################################################









