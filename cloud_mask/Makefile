# Ehsan Mosadegh, 10 Nov 2022

exe_path=/data/gpfs/home/emosadegh/MISR-SeaIceRoughness/exe_dir
src_name=ReadCloudMask.c
# exe_name=ReadASCMCloudMask
exe_name=ReadCloudMask

all:
	gcc -I/data/gpfs/home/emosadegh/Mtk-1.5.1/include	\
		-I/data/gpfs/home/emosadegh/hdfeoslibs/include	\
		-L/data/gpfs/home/emosadegh/Mtk-1.5.1/lib	\
		-I/data/gpfs/home/emosadegh/fftw/fftw-3.3.8_install/include 	\
		-L/data/gpfs/home/emosadegh/fftw/fftw-3.3.8_install/lib 	\
		-o ${exe_path}/${exe_name} ${src_name} -lMisrToolkit  -lm -lfftw3 -lpng -pthread
	
clean:

	rm ${exe_path}/${exe_name}


# all:
# 	gcc -I/Users/ehsanmos/Documents/MISR/mtk_root/Mtk-1.5.0/include 	\
# 		-I/Users/ehsanmos/Documents/MISR/mtk_root/hdfeoslibs/include 	\
# 		-L/Users/ehsanmos/Documents/MISR/mtk_root/Mtk-1.5.0/lib 		\
# 		-L/Users/ehsanmos/Documents/MISR/mtk_root/fftw-3.3.8_install/lib \
# 		-I/Users/ehsanmos/Documents/MISR/mtk_root/fftw-3.3.8_install/include \
# 		-o ${exe_path}/${exe_name} ${src_name} -lMisrToolkit  -lm -lfftw3 -lpng
	
# clean:

# 	rm ${exe_path}/${exe_name}