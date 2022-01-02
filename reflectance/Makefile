Exe=/home/mare/Ehsan_lab/MISR-roughness/exe_dir
program=TOARad2Refl
src=TOARad2Refl.c

all:
	gcc -I/home/mare/Ehsan_lab/Ehsan_Mtk_root/Mtk-1.4.5/include -I/home/mare/Ehsan_lab/Ehsan_Mtk_root/hdfeoslibs/include -L/home/mare/Ehsan_lab/Ehsan_Mtk_root/Mtk-1.4.5/lib -o ${Exe}/${program} ${src} -lMisrToolkit  -lm -lfftw3	-lpng
	
clean:

	rm ${Exe}/${program}
