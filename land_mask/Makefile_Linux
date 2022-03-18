exe_path=/home/ehsan/misr_lab/MISR-SeaIceRoughness/exe_dir
src_name=LandMask_3cams.c
exe_name=LandMask_3cams

all:
	gcc -I/home/ehsan/misr_lab/Mtk_tmp/Mtk-1.5.1/include	\
		-L/home/ehsan/misr_lab/Mtk_tmp/Mtk-1.5.1/lib	\
		-I/usr/local/hdfeoslibs/include	\
		-o ${exe_path}/${exe_name} ${src_name} -lMisrToolkit  -lm -lfftw3 -lpng -pthread
	
clean:

	rm ${exe_path}/${exe_name}



