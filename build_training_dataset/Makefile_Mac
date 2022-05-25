exe_path=/Users/ehsanmos/Documents/MISR/MISR-SeaIceRoughness
src_name=ATMModel_9cams.c
exe_name=ATMModel_9cams

# all:
# 	gcc -I/home/ehsan/misr_lab/Mtk_tmp/Mtk-1.5.1/include	\
# 		-L/home/ehsan/misr_lab/Mtk_tmp/Mtk-1.5.1/lib	\
# 		-I/usr/local/hdfeoslibs/include	\
# 		-o ${exe_path}/${exe_name} ${src_name} -lMisrToolkit  -lm -lfftw3 -lpng -pthread
	
# clean:

# 	rm ${exe_path}/${exe_name}


all:
	gcc -I/Users/ehsanmos/Documents/RnD/MISR_lab/mtk.nosync/Mtk_install/include	\
		-L/Users/ehsanmos/Documents/RnD/MISR_lab/mtk.nosync/Mtk_install/lib	\
		-L/Users/ehsanmos/Documents/RnD/MISR_lab/mtk.nosync/fftw3_installed/lib \
		-L/usr/local/Cellar/libpng/1.6.37/lib \
		-I/Users/ehsanmos/Documents/RnD/MISR_lab/mtk.nosync/hdfeoslibs_installed/include \
		-o ${exe_path}/${exe_name} ${src_name} -lMisrToolkit  -lm -lfftw3 -lpng -pthread
	
clean:

	rm ${exe_path}/${exe_name}
