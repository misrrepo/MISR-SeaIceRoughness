#!/usr/bin/env python


# Load the model to predict SIRoughness values for a path of blocks
from keras.models import load_model
## order of neurons is important
# open toa_refl.dat
import numpy as np
import os
from PIL import Image
from datetime import datetime
from glob import glob


model_name = "trained_model_2L50N_100epoch.h5"
model_dir = "/home/ehsan/misr_lab/MISR-roughness/prediction_MLP"
toa_refl_dir = "/media/ehsan/6T_part1/14528_apr2016/toa_refl_april_2016_9cam4bands_day1_30_p1_233_b1_46/toa_files_in_range_2016_4_25"
roughness_dir = toa_refl_dir




model_fp = os.path.join(model_dir, model_name) 
# load model
mlp_model_best = load_model(model_fp)
# summarize model.
mlp_model_best.summary()






# ## evaluate model
# test_metrics = mlp_model_best.evaluate(X_test, y_test, batch_size=20, verbose=1) # Returns the loss value & metrics values for the model in test mode
# import math
# mse_test = test_metrics[1]
# print("Test RMSE: %.2f (cm roughness)" %math.sqrt(mse_test))
# #     print("Test MAE: %.2f (cm roughness)" %(test_metrics[2]))






# get a list of toa_refl files

toa_refl_filePattern = "toa_refl_P*.dat"

toa_refl_list = glob(os.path.join(toa_refl_dir, toa_refl_filePattern))
print(len(toa_refl_list))





def remove_black_regions(in_arr, cam_name):
    # print("trimming camera: %s" %cam_name)
    
    for i in range(2048):
        if (in_arr[0,i] != -1.0):
            colL1 = i
            # print("colL1: %s" %colL1)
            break
            
    for i in range(2048):
        if (in_arr[511,i] != -1.0):
            colL2 = i
            # print("colL2: %s" %colL2)
            break
        
    cut_left = max(colL1, colL2)
    
    # print("cut-left: %s" %cut_left)
    if (cut_left != 0.0):
        in_arr = np.delete(in_arr, slice(0, cut_left), 1)
        # print(in_arr.shape)
    
    # print(in_arr.shape)

    for j in range(in_arr.shape[1]):
        if(in_arr[0,j] == -1.0):
            colR1 = j
            # print("colR1: %s" %colR1)
            break
        else:
            colR1 = in_arr.shape[1]
            
    for j in range(in_arr.shape[1]):
        if(in_arr[511,j] == -1.0):
            colR2 = j
            # print("colR2: %s" %colR2)
            break
        else:
            colR2 = in_arr.shape[1]
        
    cut_right = min(colR1, colR2)

    # print("cut-right: %s" %cut_right)

    if (cut_right != 0.0):
        in_arr = np.delete(in_arr, slice(cut_right, in_arr.shape[1]), 1)

    print("shape after trimming:")
    # print(in_arr.shape)
    arr_shape = in_arr.shape

    return in_arr, arr_shape








t1 = datetime.now()


p_o_process_list = []

for toa_refl in toa_refl_list:
    #print(toa_refl)
    file_name = toa_refl.split('/')[-1]
    #print(file_name.split('_')[2:4])
    p_o_list = file_name.split('_')[2:4]
    p_o = p_o_list[0]+"_"+p_o_list[1]
    #print(p_o)

    if p_o in p_o_process_list:
        continue
    else:
        p_o_process_list.append(p_o)
        print("added to list")

        for toa_block in range(1,47,1):  # check this number later

            if (toa_block < 10):
                toa_block = str(toa_block).rjust(2,'0')
            else:
                toa_block = str(toa_block)

            print(p_o)
            print(toa_block)
            
            # # check output on disk
            #     out_img_label = p_o+'_'+'B0'+toa_block+".tif"  # this image format supports saving neg- values in image
            #     out_img_fullpath = os.path.join(roughness_dir, out_img_label)



            out_raw_binary_label = 'roughness_toa_refl_'+p_o+'_'+'B0'+toa_block+".dat"  # this image format supports saving neg- values in image
            out_raw_binary_fullpath = os.path.join(roughness_dir, out_raw_binary_label)


            if (os.path.isfile(out_raw_binary_fullpath)==True):
                print(out_raw_binary_fullpath)
                print("binary exists- continue")
                continue

            ## define and open 9 cameras in order
            ## based on order of input to MLP- based on order of cameras in training dataset
            ## path,orbit,block,line,sample,lat,lon,Da_r,Ca_r,Ba_r,Aa_r,An_r,An_g,An_b,An_nir,Af_r,Bf_r,Cf_r,Df_r,mean_ATM_roughness

            
            da_r = "toa_refl_"+p_o+"_B0"+toa_block+"_da_red.dat"
            ## check file available
            if (os.path.isfile(os.path.join(toa_refl_dir, da_r))==False):
                print("block not found- continue")
                continue
            ## read array
            da_r_arr = np.fromfile(os.path.join(toa_refl_dir, da_r), dtype=np.double)[0:1048576].reshape((512,2048))
            ## check black- continue:
            if (np.median(da_r_arr) == -1.0):
                print("image black- continue")
                continue
            
            
            
            ca_r = "toa_refl_"+p_o+"_B0"+toa_block+"_ca_red.dat"
            ## check file available
            if (os.path.isfile(os.path.join(toa_refl_dir, ca_r))==False):
                print("block not found- continue")
                continue
            ca_r_arr = np.fromfile(os.path.join(toa_refl_dir, ca_r), dtype=np.double)[0:1048576].reshape((512,2048))
            ## check black- continue:
            if (np.median(ca_r_arr) == -1.0):
                print("image black- continue")
                continue
            
            
            
            ba_r = "toa_refl_"+p_o+"_B0"+toa_block+"_ba_red.dat"
            ## check file available
            if (os.path.isfile(os.path.join(toa_refl_dir, ba_r))==False):
                print("block not found- continue")
                continue
            ba_r_arr = np.fromfile(os.path.join(toa_refl_dir, ba_r), dtype=np.double)[0:1048576].reshape((512,2048))
            ## check black- continue:
            if (np.median(ba_r_arr) == -1.0):
                print("image black- continue")
                continue
            
            
            
            aa_r = "toa_refl_"+p_o+"_B0"+toa_block+"_aa_red.dat"
            ## check file available
            if (os.path.isfile(os.path.join(toa_refl_dir, aa_r))==False):
                print("block not found- continue")
                continue
            aa_r_arr = np.fromfile(os.path.join(toa_refl_dir, aa_r), dtype=np.double)[0:1048576].reshape((512,2048))
            ## check black- continue:
            if (np.median(aa_r_arr) == -1.0):
                print("image black- continue")
                continue
            
            
            
            an_r = "toa_refl_"+p_o+"_B0"+toa_block+"_an_red.dat"
            ## check file available
            if (os.path.isfile(os.path.join(toa_refl_dir, an_r))==False):
                print("block not found- continue")
                continue
            an_r_arr = np.fromfile(os.path.join(toa_refl_dir, an_r), dtype=np.double)[0:1048576].reshape((512,2048))
            ## check black- continue:
            if (np.median(an_r_arr) == -1.0):
                print("image black- continue")
                continue
            


            af_r = "toa_refl_"+p_o+"_B0"+toa_block+"_af_red.dat"
            ## check file available
            if (os.path.isfile(os.path.join(toa_refl_dir, af_r))==False):
                print("block not found- continue")
                continue
            af_r_arr = np.fromfile(os.path.join(toa_refl_dir, af_r), dtype=np.double)[0:1048576].reshape((512,2048))
            ## check black- continue:
            if (np.median(af_r_arr) == -1.0):
                print("image black- continue")
                continue
            
            
            
            
            bf_r = "toa_refl_"+p_o+"_B0"+toa_block+"_bf_red.dat"
            ## check file available
            if (os.path.isfile(os.path.join(toa_refl_dir, bf_r))==False):
                print("block not found- continue")
                continue
            bf_r_arr = np.fromfile(os.path.join(toa_refl_dir, bf_r), dtype=np.double)[0:1048576].reshape((512,2048))
            ## check black- continue:
            if (np.median(bf_r_arr) == -1.0):
                print("image black- continue")
                continue
            
            
        
            
            cf_r = "toa_refl_"+p_o+"_B0"+toa_block+"_cf_red.dat"
            ## check file available
            if (os.path.isfile(os.path.join(toa_refl_dir, cf_r))==False):
                print("block not found- continue")
                continue
            cf_r_arr = np.fromfile(os.path.join(toa_refl_dir, cf_r), dtype=np.double)[0:1048576].reshape((512,2048))
            ## check black- continue:
            if (np.median(cf_r_arr) == -1.0):
                print("image black- continue")
                continue
            
            
            
            
            df_r = "toa_refl_"+p_o+"_B0"+toa_block+"_df_red.dat"
            ## check file available
            if (os.path.isfile(os.path.join(toa_refl_dir, df_r))==False):
                print("block not found- continue")
                continue
            df_r_arr = np.fromfile(os.path.join(toa_refl_dir, df_r), dtype=np.double)[0:1048576].reshape((512,2048))
            ## check black- continue:
            if (np.median(df_r_arr) == -1.0):
                print("image black- continue")
                continue
            
            
            print("removing black regions")

            ## trim 9 images and get final shape
            da_noBlack, da_shape = remove_black_regions(da_r_arr, 'da')
            ca_noBlack, ca_shape = remove_black_regions(ca_r_arr, 'ca')
            ba_noBlack, ba_shape = remove_black_regions(ba_r_arr, 'ba')
            aa_noBlack, aa_shape = remove_black_regions(aa_r_arr, 'aa')
            an_noBlack, an_shape = remove_black_regions(an_r_arr, 'an')
            af_noBlack, af_shape = remove_black_regions(af_r_arr, 'af')
            bf_noBlack, bf_shape = remove_black_regions(bf_r_arr, 'bf')
            cf_noBlack, cf_shape = remove_black_regions(cf_r_arr, 'cf')
            df_noBlack, df_shape = remove_black_regions(df_r_arr, 'df')

            shape_list = [da_shape[1], ca_shape[1], ba_shape[1], 
                          aa_shape[1], an_shape[1], af_shape[1], 
                          bf_shape[1], cf_shape[1], df_shape[1]]


            min_col = min(shape_list)
            print("min column: %s" %min_col)


            ## trip all 9 images based on min-column
            da_noBlack_trim = np.delete(da_noBlack, slice(min_col, da_noBlack.shape[1]), 1)
            print(da_noBlack_trim.shape)

            ca_noBlack_trim = np.delete(ca_noBlack, slice(min_col, ca_noBlack.shape[1]), 1)
            print(ca_noBlack_trim.shape)

            ba_noBlack_trim = np.delete(ba_noBlack, slice(min_col, ba_noBlack.shape[1]), 1)
            print(ba_noBlack_trim.shape)

            aa_noBlack_trim = np.delete(aa_noBlack, slice(min_col, aa_noBlack.shape[1]), 1)
            print(aa_noBlack_trim.shape)

            an_noBlack_trim = np.delete(an_noBlack, slice(min_col, an_noBlack.shape[1]), 1)
            print(an_noBlack_trim.shape)

            af_noBlack_trim = np.delete(af_noBlack, slice(min_col, af_noBlack.shape[1]), 1)
            print(af_noBlack_trim.shape)

            bf_noBlack_trim = np.delete(bf_noBlack, slice(min_col, bf_noBlack.shape[1]), 1)
            print(bf_noBlack_trim.shape)

            cf_noBlack_trim = np.delete(cf_noBlack, slice(min_col, cf_noBlack.shape[1]), 1)
            print(cf_noBlack_trim.shape)

            df_noBlack_trim = np.delete(df_noBlack, slice(min_col, df_noBlack.shape[1]), 1)
            print(df_noBlack_trim.shape)



            ## loop and extract 9 cameras for each row & column, based on order of cameras (important)

            x_predict_block = []

            for row in range(da_noBlack_trim.shape[0]):
                for col in range(da_noBlack_trim.shape[1]):

                    x1 = da_noBlack_trim[row,col]
                    x2 = ca_noBlack_trim[row,col]
                    x3 = ba_noBlack_trim[row,col]
                    x4 = aa_noBlack_trim[row,col]
                    x5 = an_noBlack_trim[row,col]
                    x6 = af_noBlack_trim[row,col]
                    x7 = bf_noBlack_trim[row,col]
                    x8 = cf_noBlack_trim[row,col]
                    x9 = df_noBlack_trim[row,col]

                    nine_features = [x1,x2,x3,x4,x5,x6,x7,x8,x9]
                    #print(type(nine_features))
                    x_predict_block.append(nine_features)


            print("prediction...")
            # print(x_predict_block[0:3])
            ## prediction for each block
            y_predict_block = mlp_model_best.predict(x_predict_block, verbose=1)


            
            ## re-construct images to 2D
            print(y_predict_block.shape)
            print(type(y_predict_block))

            y_predict_block_2d = y_predict_block.reshape((512,-1))
            print(y_predict_block_2d.dtype)
            print(y_predict_block_2d.shape)

            print("min: %s" %y_predict_block_2d.min())
            print("max: %s" %y_predict_block_2d.max())

            
            ## flatten array and save as binary raw file
            y_predict_block_2d.flatten().astype(np.double).tofile(out_raw_binary_fullpath)
            print(out_raw_binary_fullpath)
            
            x_predict_block = None
            y_predict_block_2d = None
                # print(len(y_predict_block_2d))


                # ##- create PIL image from predicted roughness array 2D w/ negative values
                # predicted_roughness_img = Image.fromarray(y_predict_block_2d)
                # ##- save roughness PIL image on disc
                # predicted_roughness_img.save(out_img_fullpath)  
                # print("predicted image saved to:")
                # print(out_img_fullpath)

        



t2 = datetime.now()
period = t2-t1
print("run time: %s" %period)






