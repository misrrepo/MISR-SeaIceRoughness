# include libraries:
# 	Mtk include and lib directories
# 	hdfeos include dir
#	


exe_path=/Users/ehsanmos/Documents/MISR/MISR-roughness/exe_dir
exe_name=ArcticTiles2grid
src_name=ArcticTiles2grid.c #TOARad2Blocks_noblokcs1to6.c #TOARad2BlocksAllBlocks.c

all:
	gcc -I/Users/ehsanmos/Documents/MISR/mtk_root/Mtk-1.5.0/include 	\
		-I/Users/ehsanmos/Documents/MISR/mtk_root/hdfeoslibs/include 	\
		-L/Users/ehsanmos/Documents/MISR/mtk_root/Mtk-1.5.0/lib 		\
		-L/Users/ehsanmos/Documents/MISR/mtk_root/fftw-3.3.8_install/lib \
		-I/Users/ehsanmos/Documents/MISR/mtk_root/fftw-3.3.8_install/include \
		-o ${exe_path}/${exe_name} ${src_name} -lMisrToolkit  -lm -lfftw3	
	
clean:

	rm ${exe_path}/${exe_name}


# old from Anne Linux machine

# all:
# 	gcc -I/home/mare/Ehsan_lab/Ehsan_Mtk_root/Mtk-1.4.5/include \
# 	-I/home/mare/Ehsan_lab/Ehsan_Mtk_root/hdfeoslibs/include \
# 	-L/home/mare/Ehsan_lab/Ehsan_Mtk_root/Mtk-1.4.5/lib \
# 	-o ${exe_path}/${exe_name} ${src_name} -lMisrToolkit  -lm -lfftw3	
	
# clean:

# 	rm ${Exe}/${program}