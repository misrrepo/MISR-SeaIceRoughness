exe_path=/home/ehsan/misr_lab/MISR-SeaIceRoughness/exe_dir
src_name=MISR2Roughness_serial.c
exe_name=MISR2Roughness_serial

all:
	gcc -I/home/ehsan/misr_lab/Mtk_tmp/Mtk-1.5.1/include	\
		-L/home/ehsan/misr_lab/Mtk_tmp/Mtk-1.5.1/lib	\
		-I/usr/local/hdfeoslibs/include	\
		-o ${exe_path}/${exe_name} ${src_name} -lMisrToolkit  -lm -lfftw3 -lpng -pthread
	
clean:

	rm ${exe_path}/${exe_name}



