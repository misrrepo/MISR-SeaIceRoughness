// PlotCombinedBlocks.c : plot combined Cf, An, Ca blocks
// Gene Mar 25 Jun 17

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <png.h>
#include <MisrToolkit.h>
#include <MisrError.h>
#include <dirent.h>

#define NO_DATA -999999.0
#define BACKGROUND -999998.0
#define FOREGROUND -999997.0
#define TDROPOUT -999996.0
#define MASKED -999995.0
#define LMASKED -999994.0
#define VERBOSE 0

typedef struct
{
int path;
int orbit;
int block;
double *cf;
double *an;
double *ca;
double *lat;
double *lon;
double somy[2];
int offset;
} block_type;

int path;
int block;
int block_min, block_max;
int orbit;
int nfiles;
double *data = 0;
double *cfdata = 0;
double *londata = 0;
double *latdata = 0;
double *x, *y, *z;
int nlines = 512;
int nsamples = 2048;
int lmargin;
block_type *block_list;
int nblocks = 0;
int xblock = 0;
int xblock_min = 0;
int nvalid;
int ndropouts;
double min;
double max;
double mean;
double stddev;
char fname[256];
char **flist = 0;
png_structp png_ptr = 0;
png_infop info_ptr = 0;

int read_block(char *fname, int mode);
int read_data(char *fname, double **data, int nlines, int nsamples);
int getDataStats(double *data, int nlines, int nsamples);
int getSOMY(int path, int block, int line, int sample, double *somy);
int combineBlocks(void);
char *data2image(double *data, int ny, int nx);
int write_png(char *fname, char *image, int ny, int nx);
int write_data(char *fname, double *data, int nlines, int nsamples);
int getFileList(char *dir);
char *strsub(char *s, char *a, char *b);

char *data2image(double *data, int nlines, int nsamples)
{
char *image;
double /*min, max,*/ z, dz;
int i;

/*min = 1.0e23;
max = -1.0e23;
for (i = 0; i < nlines * nsamples; i ++)
	{
	z = data[i];
	if (z == NO_DATA) continue;
	if (z == TDROPOUT) continue;
	if (z == MASKED) continue;
	if (z < min) min = z;
	else if (z > max) max = z;
	}*/
if (VERBOSE) fprintf(stderr, "data2image: min=%.3f, max=%.3f\n", min, max);
if (max != min) dz =  255.0 / (max - min);
else dz = 0.0;

image = (char *) malloc(nlines * nsamples);
if (!image)
	{
	fprintf(stderr, "data2image: couldn't malloc image\n");
	return 0;
	}

for (i = 0; i < nlines * nsamples; i ++)
	{
	z = data[i];
	if (z == NO_DATA) image[i] = 0;
	else if (z == TDROPOUT) image[i] = 0;
	else if (z == MASKED) image[i] = -64;
	else 
		{
		z = (z - min) * dz;
		if (z > 255.0) image[i] = 255;
		else if (z < 0.0) image[i] = 0;
		else image[i] = z;
		}
	}

return image;
}


int write_png(char *fname, char *image, int ny, int nx)
{
FILE *fp;
png_bytepp row_ptrs;
int j;

row_ptrs = (png_bytepp) malloc(ny * sizeof(png_bytep));
if (!row_ptrs)
	{
	fprintf(stderr, "write_png: couldn't malloc row_ptrs\n");
	return 0;
	}
for (j = 0; j < ny; j ++) row_ptrs[j] = (png_bytep)(image + j * nx);

fp = fopen(fname, "wb");
if (!fp)
	{
	fprintf(stderr, "write_png: couldn't open %s\n", fname);
	return 0;
	}

png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 
	png_voidp_NULL, png_error_ptr_NULL, png_error_ptr_NULL);
if (!png_ptr)
	{
	fclose(fp);
	fprintf(stderr, "write_png: png_create_write_struct failed\n");
	return 0;
	}

info_ptr = png_create_info_struct(png_ptr);
if (!info_ptr)
	{
	png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
	fclose(fp);
	fprintf(stderr, "write_png: png_create_info_struct failed\n");
	return 0;
	}

if (setjmp(png_jmpbuf(png_ptr)))
	{
	png_destroy_write_struct(&png_ptr, &info_ptr);
	fclose(fp);
	fprintf(stderr, "write_png: longjmp from png error\n");
	return 0;
	}

png_init_io(png_ptr, fp);

png_set_IHDR(png_ptr, info_ptr, nx, ny, 8, PNG_COLOR_TYPE_GRAY, 
	PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	
png_set_rows(png_ptr, info_ptr, row_ptrs);

png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
       
png_destroy_write_struct(&png_ptr, &info_ptr);

fclose(fp);
if (row_ptrs) free(row_ptrs);
return 1;
}

int write_data(char *fname, double *data, int nlines, int nsamples)
{
FILE *f;

f = fopen(fname, "wb");
if (!f)
	{
	fprintf(stderr, "write_data: couldn't open %s\n", fname);
	return 0;
	}
	
//if (fwrite(data, sizeof(double), nlines * nsamples, f) != nlines * nsamples)
if (fwrite(data, sizeof(double), 3*nlines * nsamples, f) != 3*nlines * nsamples)
	{
	fprintf(stderr, "write_data: couldn't write data\n");
	return 0;
	}
	
fclose(f);
return 1;
}


int getDataStats(double *data, int nlines, int nsamples)
{
int i;
double z;

min = 1.0e23;
max = -1.0e23;
mean = 0.0;
stddev = 0.0;
nvalid = 0;
ndropouts = 0;

for (i = 0; i < nlines * nsamples; i ++)
	{
	z = data[i];
	if (z == NO_DATA) continue;
	if (z == BACKGROUND) continue;
	if (z == FOREGROUND) continue;
	if (z == TDROPOUT) 
		{
		ndropouts ++;
		continue;
		}
	if (z == MASKED) continue;
	nvalid ++;
	if (z < min) min = z;
	if (z > max) max = z;
	mean += z;
	}
	
if (nvalid > 0) 
	{
	mean /= nvalid;
	if (nvalid > 1)
		{
		for (i = 0; i < nlines * nsamples; i ++)
			{
			z = data[i];
			if (z == NO_DATA) continue;
			if (z == BACKGROUND) continue;
			if (z == FOREGROUND) continue;
			if (z == TDROPOUT) continue;
			if (z == MASKED) continue;
			stddev += (z - mean) * (z - mean);
			}
		stddev = sqrt(stddev / (nvalid - 1.0));
		}
	}
else
	{
	min = -1.0;
	max = -1.0;
	mean = -1.0;
	stddev = -1.0;	
	}

return 1;
}


int read_data(char *fname, double **data, int nlines, int nsamples)
{
FILE *f;

f = fopen(fname, "r");
if (!f)
	{
	fprintf(stderr,  "read_data: couldn't open %s\n", fname);
	return 0;
	}
	
*data = (double *) malloc(3*nlines * nsamples * sizeof(double));
//*data = (double *) malloc(nlines * nsamples * sizeof(double));
if (!*data)
	{
	fprintf(stderr,  "read_data: couldn't malloc data\n");
	return 0;
	}
	
if (fread(*data, sizeof(double), 3*nlines * nsamples, f) != 3*nlines * nsamples)
//if (fread(*data, sizeof(double), nlines * nsamples, f) != nlines * nsamples)
	{
	fprintf(stderr,  "read_data: couldn't read data in %s\n", fname);
	return 0;
	}

fclose(f);
return 1;
}


int read_block(char *fname, int mode)
{
char s[256];
char *errs[] = MTK_ERR_DESC;
int path, orbit, block;
int dsize = nsamples * nlines;
int i, r, c;
int status;
double lat, lon;

//fprintf(stderr, "read_block: reading %s\n", fname);

if (strstr(fname, "_p"))
	{
	strncpy(s, strstr(fname, "_p") + 2, 3);
	s[3] = 0;
	path = atoi(s);
	if (VERBOSE > 1) fprintf(stderr, "%d\n", path);
	}
else
	{
	fprintf(stderr,  "read_block: No path info in file name\n");
	return 0;
	}

if (strstr(fname, "_o"))
	{
	strncpy(s, strstr(fname, "_o") + 2, 6);
	s[6] = 0;
	orbit = atoi(s);
	if (VERBOSE > 1) fprintf(stderr, "%d\n", orbit);
	}
else
	{
	fprintf(stderr,  "read_block: No orbit info in file name\n");
	return 0;
	}
	
if (strstr(fname, "_b"))
	{
	strncpy(s, strstr(fname, "_b") + 2, 3);
	s[3] = 0;
	block = atoi(s);
	if (VERBOSE > 1) fprintf(stderr, "%d\n", block);
	}
else
	{
	fprintf(stderr,  "read_block: No block info in file name\n");
	return 0;
	}
	
if (VERBOSE) fprintf(stderr, "read_block: path = %d, orbit = %d, block = %d\n", path, orbit, block);
	
if (!read_data(fname, &data, nlines, nsamples)) return 0;
if (!getDataStats(data, nlines, nsamples)) return 0;

if (mode == 0)
	{
	if (xblock == 0) block_list = (block_type * ) malloc(sizeof(block_type));
	else block_list = (block_type * ) realloc(block_list, (xblock + 1) * sizeof(block_type));
	if (!block_list)
		{
		fprintf(stderr,  "read_block: couldn't malloc/realloc block_list\n");
		return 0;
		}
	
	x = (double *) malloc(nlines * nsamples * sizeof(double));
	y = (double *) malloc(nlines * nsamples * sizeof(double));
	z = (double *) malloc(nlines * nsamples * sizeof(double));

	block_list[xblock].path = path;
	block_list[xblock].orbit = orbit;
	block_list[xblock].block = block;
	for (r=0; r<nlines; r++) {
	    for (c=0; c<nsamples; c++) {
		x[r*nsamples+c] = data[r*nsamples+c];
		y[r*nsamples+c] = data[dsize+r*nsamples+c];
		z[r*nsamples+c] = data[2*dsize+r*nsamples+c];
	    }
	}
	block_list[xblock].cf = x;
	block_list[xblock].lat = y;
	block_list[xblock].lon = z;

	//printf(" %f %f %f\n", block_list[xblock].cf[0], block_list[xblock].lat[0], block_list[xblock].lon[0]);

	if (!getSOMY(path, block, 0, 0, &block_list[xblock].somy[0])) return 0;
	if (!getSOMY(path, block, 511, 0, &block_list[xblock].somy[1])) return 0;

	if (block_list[xblock].somy[1] != block_list[xblock].somy[0])
		{
		fprintf(stderr,  "read_block: somy mismatch: %14.6f  %14.6f\n", 
			block_list[xblock].somy[0], block_list[xblock].somy[1]);
		return 0;
		}
	//if (xblock == 0) block_list[xblock].offset = 0;
	//else block_list[xblock].offset = (block_list[xblock].somy[0] - block_list[0].somy[0]) / 275.0;
	block_list[xblock].offset = block_list[xblock].somy[0];
	}
else if (mode == 1) block_list[xblock].an = data;
else if (mode == 2) block_list[xblock].ca = data;
/*
fprintf(stderr, "%d %03d  %06d  %03d  %2s  %5d  %5d  %10d  %14.6f  %14.6f  %14.6f  %14.6f  %16.6f  %5d\n", 
	xblock, path, orbit, block, mode == 0 ? "cf" : mode == 1 ? "an" : "ca", nlines, nsamples, nvalid, 
	min, max, mean, stddev, 
	block_list[xblock].somy[0], block_list[xblock].offset);
*/
//free(x);
//free(y);
//free(z);

if (mode == 0) xblock++;
	
return 1;
}


int getSOMY(int path, int block, int line, int sample, double *somy)
{
int status;
char *errs[] = MTK_ERR_DESC;
double somx;

status = MtkBlsToSomXY(path, 275, block, 1.0 * line, 1.0 * sample, &somx, somy);
if (status != MTK_SUCCESS) 
	{
	fprintf(stderr,  "getSOMY: MtkBlsToSomXY failed!!!, status = %d (%s)\n", status, errs[status]);
	return 0;
	}

return 1;
}


int combineBlocks(void)
{
int i, j, k, rmargin = 0;
double cf, x, y, z;

lmargin = 0;
for (i = 0; i < xblock; i ++)
	{
	//printf("%d block = %d  offset = %d\n", i, block_list[i].block, block_list[i].offset);
	block_list[i].offset = (block_list[i].offset - block_list[xblock_min].somy[0])/275.0;
	if (block_list[i].offset < lmargin) lmargin = block_list[i].offset;
	if (block_list[i].offset > rmargin) rmargin = block_list[i].offset;
	}
lmargin = abs(lmargin);
fprintf(stderr, "combineBlocks: lmargin = %d, rmargin = %d\n", lmargin, rmargin);

//nlines = 512 * (block_list[nblocks - 1].block - block_list[0].block + 1);
nlines = 512 * nblocks;
nsamples = 2048 + lmargin + rmargin;
printf("%d %d %d %d %d %d\n", block_max, block_min, nlines, nsamples, xblock, xblock_min);
cfdata = (double *) malloc(nlines * nsamples * sizeof(double));
if (!cfdata)
	{
	fprintf(stderr, "combineBlocks: couldn't malloc cfdata\n");
	return 0;
	}
	
latdata = (double *) malloc(nlines * nsamples * sizeof(double));
if (!latdata)
	{
	fprintf(stderr, "combineBlocks: couldn't malloc latdata\n");
	return 0;
	}
	
londata = (double *) malloc(nlines * nsamples * sizeof(double));
if (!londata)
	{
	fprintf(stderr, "combineBlocks: couldn't malloc londata\n");
	return 0;
	}
	
for (i = 0; i < nlines * nsamples; i ++) {
    cfdata[i] = NO_DATA;
    latdata[i] = NO_DATA;
    londata[i] = NO_DATA;
}
for (k = 0; k < xblock; k ++) {
	//printf("%d %d\n", k, block_list[k].block);
	for (j = 0; j < 512; j ++) {
		for (i = 0; i < 2048; i ++)
			{
			cf = block_list[k].cf[i + j * 2048];
			y = block_list[k].lat[i + j * 2048];
			z = block_list[k].lon[i + j * 2048];
			//if ( i ==0 && j == 0) printf(" %f %f %f\n", cf, y, z);
			//an = block_list[k].an[i + j * 2048];
			//ca = block_list[k].ca[i + j * 2048];
			//if (cf == NO_DATA || an == NO_DATA || ca == NO_DATA) continue;
			//if (cf == MASKED || an == MASKED || ca == MASKED) x = MASKED;
			//else if (cf == TDROPOUT || an == TDROPOUT || ca == TDROPOUT) x = TDROPOUT;
			//if (cf == NO_DATA ) continue;
			//if (cf == MASKED) x = MASKED;
			//else if (cf == TDROPOUT) x = TDROPOUT;
			//else x = (cf + an + ca) / 3.0;
			//else x = cf;
			cfdata[lmargin + block_list[k].offset + i + j * nsamples 
				+ (block_list[k].block - block_min) * 512 * nsamples] 
				= cf;
			latdata[lmargin + block_list[k].offset + i + j * nsamples 
				+ (block_list[k].block - block_min) * 512 * nsamples] 
				= y;
			londata[lmargin + block_list[k].offset + i + j * nsamples 
				+ (block_list[k].block - block_min) * 512 * nsamples] 
				= z;
			}
	}
}

return 1;
}

int getFileList(char *dir)
{
DIR *dp;
struct dirent *ep;

dp = opendir(dir);
if (!dp)
	{
	printf("getFileList: couldn't open %s\n", dir);
	return 0;
	}
	
while (ep = readdir(dp))
	{
	if (strstr(ep->d_name, ".hdr")) continue;
	if (!strstr(ep->d_name, ".dat")) continue;
	//if (!strstr(ep->d_name, "sdcm_")) continue;
	//if (!strstr(ep->d_name, "rms_")) continue;
	if (flist == 0)
		{
		flist = (char **) malloc(sizeof(char *));
		if (!flist)
			{
			printf("getFileList: couldn't malloc flist\n");
			return 0;
			}
		}
	else
		{
		flist = (char **) realloc(flist, (nfiles + 1) * sizeof(char *));
		if (!flist)
			{
			printf("getFileList: couldn't realloc flist\n");
			return 0;
			}
		}
	flist[nfiles] = (char *) malloc(strlen(ep->d_name) + 1);
	if (!flist[nfiles])
		{
		printf("getFileList: couldn't malloc flist[nfiles]\n");
		return 0;
		}
	strcpy(flist[nfiles], ep->d_name);
	//printf("%s\n", flist[nfiles]);
	nfiles ++;
	}
	
closedir(dp);
return 1;
}

char *strsub(char *s, char *a, char *b)
{
    char *p, t[256];
    p = strstr(s, a);
    if(p==NULL) return NULL;  /*a not found */
    t[0] = 0; /*now t contains empty string */
    strncat(t, s, p-s); /*copy part of s preceding a */
    strcat(t, b); /*add the substitute word */
    strcat(t, p+strlen(a)); /*add on the rest of s */
    strcpy(s, t);
    return p+strlen(b); /*p+s points after substitution */
}

//############################################### main ######################################################

int main(int argc, char *argv[])
{
int i, j, n, mode = 0, block1, block2;
char s[256];
int dsize;
int norbits = 0;
int match;
int *orbit_table;
char sorbit[10], spath[10];
char sblock1[10], sblock2[10];
//char idir[256];
//char ofile[256] = "/home/mare/Nolin/SeaIce/Roughness/RMS_MISR_orbits/rms_misr_pX_oY_bC-D.png";
//char ofile[256] = "/home/mare/Nolin/SeaIce/Roughness/RMS_MISR_orbits/April2013_SeaIce_Model/May/rms_misr_pX_oY_bC-D.png";
char ofile[256] = 	"/home/mare/Nolin/2016/RMSBlocks/AprMay2016_SeaIce_Model/AprMay_SOMOrbitalPaths/rms_pX_oY_bD-E.png"; // Ehsan: output dir? example format: rms_lmasked_p72_o86917_b031-040.dat; what is E?
//char ofile[256] = "/home/mare/Nolin/2016/RMSBlocks/AprMay2016_SeaIce_Model/AprMay_SOMOrbitalPaths/rms_lmasked_pX_oY_bD-E.png"; 
/***
if (argc < 2)
	{
	fprintf(stderr, "Usage: PlotCombinedBlocks dir\n");
	return 1;
	}

strcpy(idir, argv[1]);
***/
//char idir[256] = 	"/home/mare/Nolin/2013/RMSBlocks/April2013_SeaIce_Model_noSMAC/Apr_LandMasked_v2/";
//char idir[256] = 	"/home/mare/Nolin/2016/RMSBlocks/AprMay2016_SeaIce_Model/AprMay020/";
char idir[256] =	"/home/mare/Nolin/data_2000_2016/2016/Surface3_LandMasked/AprMay/An/"; // input dir?
//char idir[256] = 	"/home/mare/Projects/MISR/Julienne/IceBridge2016/AprMay/"
if (!getFileList(idir)) return 1;
//printf("Completed getFileList %d\n", nfiles);

//Determine all unique orbits
orbit_table = (int *) malloc(nfiles * sizeof(int));
for (i = 0; i < nfiles; i ++) {
    match = 0;
    sprintf(fname, "%s%s", idir, flist[i]);
    //printf("%5d:  %s\n", i + 1, fname);

    if (strstr(fname, "_o")) {
	strncpy(sorbit, strstr(fname, "_o") + 2, 6);
	sorbit[6] = 0;
	orbit = atoi(sorbit);
    }
    else {
	printf("No orbit info in file name\n");
	return 1;
    }

    if (norbits == 0) {
	orbit_table[norbits] = orbit;
	//printf("%d\n", orbit);
	norbits++;
    }
    else {
	for (j=0; j<norbits; j++) {
	    if (orbit == orbit_table[j]) match = 1;
	}
	if (match == 0) {
	    orbit_table[norbits] = orbit;
	    //printf("%d\n", orbit);
	    norbits++;
	}
    }
}

for (n=0; n<norbits; n++) {
    //if (n < 97) continue;
    orbit = orbit_table[n];
    if (orbit != 86902) continue;
    sprintf(sorbit, "%d", orbit_table[n]);
    nlines = 512;
    nsamples = 2048;
    xblock = 0;
    block_min = INT_MAX; 
    block_max = INT_MIN; 
    printf("orbit[%d] = %d\n", n, orbit);
    for (i=0; i<nfiles; i++) {
	sprintf(fname, "%s%s", idir, flist[i]);
	if (strstr(fname, sorbit) != NULL) {
            if (strstr(fname, "_b")) {
	    	strncpy(s, strstr(fname, "_b") + 2, 3);
	    	s[3] = 0;
	    	block = atoi(s);
		if (block < block_min) block_min = block; 
		if (block > block_max) block_max = block; 
            }
            else {
	    	printf("No block info in file name\n");
	    	return 1;
            }
	}
    }

    block_min = 11;
    block_max = 20;
    nblocks = block_max - block_min + 1;
    for (i=0; i<nfiles; i++) {
	sprintf(fname, "%s%s", idir, flist[i]);
	if (strstr(fname, sorbit) != NULL) {
            if (strstr(fname, "_b")) {
	    	strncpy(s, strstr(fname, "_b") + 2, 3);
	    	s[3] = 0;
	    	block = atoi(s);
		if (block < block_min) continue;
		if (block > block_max) continue;
		if (block == block_min) xblock_min = xblock;
    	    	//printf("%d %d %s\n", orbit, nblocks, fname);
	    	if (!read_block(fname, mode)) return 1;
	    }
	}
    }
    
    if (!combineBlocks()) return 1;

    if (!getDataStats(cfdata, nlines, nsamples)) return 1;
    fprintf(stderr, "Combined:\n");
    fprintf(stderr, "     %5d  %5d  %10d  %14.6f  %14.6f  %14.6f  %14.6f\n", 
	nlines, nsamples, nvalid, min, max, mean, stddev);

    sprintf(sblock1, "%03d", block_min);
    sprintf(sblock2, "%03d", block_max);
    sprintf(spath, "%d", block_list[0].path);
    strcpy(fname, ofile);
    strsub(fname, "X", spath);
    strsub(fname, "Y", sorbit);
    strsub(fname, "D", sblock1);
    strsub(fname, "E", sblock2);
    if (!write_png(fname, data2image(cfdata, nlines, nsamples), nlines, nsamples)) return 1;

    data = (double *) malloc(3*nlines*nsamples * sizeof(double));
    //data = (double *) malloc(nlines*nsamples * sizeof(double));

    dsize = nlines * nsamples;
    memcpy(data, cfdata, dsize * sizeof(double));
    memcpy(data + dsize, latdata, dsize * sizeof(double));
    memcpy(data + 2*dsize, londata, dsize * sizeof(double));

    strsub(fname, ".png", ".dat");
    printf("Writing to %s\n\n\n", fname);

    if (!write_data(fname, data, nlines, nsamples)) return 1;

    if (cfdata) {
	free(cfdata);
	free(latdata);
	free(londata);
	free(data);
	free(x);
	free(y);
	free(z);
	free(block_list);
    }

}

return 0;
}
