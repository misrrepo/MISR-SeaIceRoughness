
src_name=MISR2Roughness_parallel.c
exe_path=/Users/ehsanmos/Documents/MISR/MISR-roughness/exe_dir
exe_name=MISR2Roughness_parallel

all:
	gcc -I/Users/ehsanmos/Documents/MISR/mtk_root/Mtk-1.5.0/include 	\
		-I/Users/ehsanmos/Documents/MISR/mtk_root/hdfeoslibs/include 	\
		-L/Users/ehsanmos/Documents/MISR/mtk_root/Mtk-1.5.0/lib 		\
		-L/Users/ehsanmos/Documents/MISR/mtk_root/fftw-3.3.8_install/lib \
		-I/Users/ehsanmos/Documents/MISR/mtk_root/fftw-3.3.8_install/include \
		-o ${exe_path}/${exe_name} ${src_name} -lMisrToolkit  -lm -lfftw3 -lpng -pthread
	
clean:

	rm ${exe_path}/${exe_name}

