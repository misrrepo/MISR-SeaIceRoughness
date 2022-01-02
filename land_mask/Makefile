exe_path=/home/ehsan/misr_lab/MISR-roughness/exe_dir
src_name=LandMask_allCameras.c
exe_name=LandMask_allCameras

all:
	gcc -I/home/ehsan/misr_lab/mtk_src/Mtk-1.5.1/include	\
		-L/home/ehsan/misr_lab/mtk_src/Mtk-1.5.1/lib	\
		-I/home/ehsan/misr_lab/hdfeoslibs/include	\
		-o ${exe_path}/${exe_name} ${src_name} -lMisrToolkit  -lm -lfftw3 -lpng -pthread
	
clean:

	rm ${exe_path}/${exe_name}



