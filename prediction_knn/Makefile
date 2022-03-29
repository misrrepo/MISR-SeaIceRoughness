exe_path=/data/gpfs/home/emosadegh/MISR-SeaIceRoughness/exe_dir

# src_name=MISR2Roughness_parallel.c
# exe_name=MISR2Roughness_parallel

src_name=MISR2Roughness_serial.c
exe_name=MISR2Roughness_serial

all:
	gcc -I/data/gpfs/home/emosadegh/Mtk-1.5.1/include	\
		-I/data/gpfs/home/emosadegh/hdfeoslibs/include	\
		-L/data/gpfs/home/emosadegh/Mtk-1.5.1/lib	\
		-I/data/gpfs/home/emosadegh/fftw/fftw-3.3.8_install/include 	\
		-L/data/gpfs/home/emosadegh/fftw/fftw-3.3.8_install/lib 	\
		-o ${exe_path}/${exe_name} ${src_name} -lMisrToolkit  -lm -lfftw3 -lpng -pthread
	
	
clean:

	rm ${exe_path}/${exe_name}



