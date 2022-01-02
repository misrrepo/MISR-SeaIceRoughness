// ReadASCMFile.c
// Read TERRAIN radiance data, perform terrain correction, convert to TOA BRF, 
//   save as data and PNG.  Mark and fix terrain dropouts.  Discard files with
//   sz >= 80.0.
// Sky Coyote 18 Apr 09

#include <stdlib.h>
#include <math.h>
#include <fftw3.h>
#include <png.h>
#include <MisrToolkit.h>
#include <MisrError.h>

#define NO_DATA -999999.0
#define BACKGROUND -999998.0
#define FOREGROUND -999997.0
#define TDROPOUT -999996.0
#define MASKED 0
#define VERBOSE 0

char fname[2][256];
int block = 0;
uint8 *cmask0 = 0;
uint8 *cmask1 = 0;
double *cfcbe = 0;
int16 *cfpmh = 0;
double cfcbe_thresh = 0.1; // between 0 and 1.0
double cfpmh_thresh = 0.0; // meters

int readASCMFile(char *fname);
int write_data(char *fname, uint8 *data, int nlines, int nsamples);

int readASCMFile(char *fname)
{
MTKt_DataBuffer szbuf = MTKT_DATABUFFER_INIT;
char gridName[256];
char fieldName[256];
int status;
MTKt_FileType filetype;
char *types[] = MTKd_DataType;
char *errs[] = MTK_ERR_DESC;
int i, j, k, l, n;
int maskline = 512;
int masksamples = 2048;
uint8 mask;

if (VERBOSE) fprintf(stderr, "readASCMFile: fname=%s, block=%d\n", fname, block);
status = MtkFileType(fname, &filetype);
if (status != MTK_SUCCESS) 
	{
	fprintf(stderr, "readASCMFile: MtkFileType failed!!!, status = %d (%s)\n", status, errs[status]);
	return 0;
	}

if (filetype != MTK_TC_CLASSIFIERS)
	{
	fprintf(stderr, "readASCMFile: TC_CLASSIFIERS are supported!!!\n");
	return 0;
	}
/***
int num_attrs;
char **attrlist;
status = MtkGridAttrList(fname, "CloudClassifiers_2.2_km", &num_attrs, &attrlist);
if (status != MTK_SUCCESS) 
	{
	fprintf(stderr, "readASCMFile: MtkGridAttrList failed!!!, status = %d (%s)\n", status, errs[status]);
	return 0;
	}
for (i = 0; i < num_attrs; i++)
	{
	fprintf(stderr, "%d %s\n", num_attrs, attrlist[i]);
	}
***/
strcpy(gridName, "CloudFractions_17.6_km"); 
strcpy(fieldName, "CombinedFractionCloudBestEstimate");
if (VERBOSE) fprintf(stderr, "readASCMFile: grid=%s, field=%s\n", gridName, fieldName);
status = MtkReadBlock(fname, gridName, fieldName, block, &szbuf);
if (status != MTK_SUCCESS) 
	{
	fprintf(stderr, "readASCMFile: MtkReadBlock failed!!!, gname = %s, fname = %s, status = %d (%s)\n", gridName, fieldName, status, errs[status]);
	return 0;
	}

if (VERBOSE) fprintf(stderr, "readASCMFile: nline=%d, nsample=%d, datasize=%d, datatype=%d (%s)\n", 
	szbuf.nline, szbuf.nsample, szbuf.datasize, szbuf.datatype, types[szbuf.datatype]);
if (szbuf.nline != 8 || szbuf.nsample != 32)
	{
	fprintf(stderr, "readASCMFile: %s is not 8x32: (%d, %d)\n", fieldName, szbuf.nline, szbuf.nsample);
	return 0;
	}

cfcbe = (double *) malloc(8 * 32 * 64 * 64 * sizeof(double));
if (!cfcbe)
	{
	fprintf(stderr, "readASCMFile: cfcbe malloc failed!!!\n");
	return 0;
	}

n = 0;
for (j = 0; j < 8; j ++)
	for (i = 0; i < 32; i ++)
		{
		n ++;
		for (k = 0; k < 64; k++)
			for (l = 0; l < 64; l++)
				{
				cfcbe[(j * 64 + k) * 32*64 + i*64 + l] = szbuf.data.d[j][i];
				}
		}

if (n != 8*32)
	{
	fprintf(stderr, "readASCMFile: cfmask fewer than 256 valid in %s: %d\n", fieldName, n);
	return 0;
	}

strcpy(gridName, "CloudFractions_17.6_km"); 
strcpy(fieldName, "MedianPrelimCloudHeight");
if (VERBOSE) fprintf(stderr, "readASCMFile: grid=%s, field=%s\n", gridName, fieldName);
status = MtkReadBlock(fname, gridName, fieldName, block, &szbuf);
if (status != MTK_SUCCESS) 
	{
	fprintf(stderr, "readASCMFile: MtkReadBlock failed!!!, gname = %s, fname = %s, status = %d (%s)\n", gridName, fieldName, status, errs[status]);
	return 0;
	}

if (VERBOSE) fprintf(stderr, "readASCMFile: nline=%d, nsample=%d, datasize=%d, datatype=%d (%s)\n", 
	szbuf.nline, szbuf.nsample, szbuf.datasize, szbuf.datatype, types[szbuf.datatype]);
if (szbuf.nline != 8 || szbuf.nsample != 32)
	{
	fprintf(stderr, "readASCMFile: %s is not 8x32: (%d, %d)\n", fieldName, szbuf.nline, szbuf.nsample);
	return 0;
	}

cfpmh = (int16 *) malloc(8 * 32 * 64 * 64 * sizeof(int16));
if (!cfpmh)
	{
	fprintf(stderr, "readASCMFile: cfpmh malloc failed!!!\n");
	return 0;
	}

n = 0;
for (j = 0; j < 8; j ++)
	for (i = 0; i < 32; i ++)
		{
		n ++;
		mask = 0;
		for (k = 0; k < 64; k++)
			for (l = 0; l < 64; l++)
				{
				cfpmh[(j * 64 + k) * 32*64 + i*64 + l] = szbuf.data.i16[j][i];
				}
		}

if (n != 8*32)
	{
	fprintf(stderr, "readASCMFile: cfpmh fewer than 256 valid in %s: %d\n", fieldName, n);
	return 0;
	}

strcpy(gridName, "ASCMParams_1.1_km"); 
strcpy(fieldName, "AngularSignatureCloudMask");
if (VERBOSE) fprintf(stderr, "readASCMFile: grid=%s, field=%s\n", gridName, fieldName);
status = MtkReadBlock(fname, gridName, fieldName, block, &szbuf);
if (status != MTK_SUCCESS) 
	{
	//fprintf(stderr, "readASCMFile: MtkReadBlock failed!!!, status = %d (%s)\n", status, errs[status]);
	fprintf(stderr, "readASCMFile: MtkReadBlock failed!!!, gname = %s, fname = %s, status = %d (%s)\n", gridName, fieldName, status, errs[status]);
	return 0;
	}

if (VERBOSE) fprintf(stderr, "readASCMFile: nline=%d, nsample=%d, datasize=%d, datatype=%d (%s)\n", 
	szbuf.nline, szbuf.nsample, szbuf.datasize, szbuf.datatype, types[szbuf.datatype]);
if (szbuf.nline != 128 || szbuf.nsample != 512)
	{
	fprintf(stderr, "readASCMFile: %s is not 128x512: (%d, %d)\n", fieldName, szbuf.nline, szbuf.nsample);
	return 0;
	}

cmask0 = (uint8 *) malloc(128 * 512 * 16 * sizeof(uint8));
if (!cmask0)
	{
	fprintf(stderr, "readASCMFile: cmask0 malloc failed!!!\n");
	return 0;
	}
n = 0;
for (j = 0; j < 128; j ++)
	for (i = 0; i < 512; i ++)
		{
		n ++;
		mask = 0;
		if (szbuf.data.u8[j][i] == 4) mask = 1;
		for (k = 0; k < 4; k++)
			for (l = 0; l < 4; l++)
				{
				//if ((cfcbe[(j * 4 + k) * 512*4 + i*4 + l] >= 0.0) && (cfcbe[(j * 4 + k) * 512*4 + i*4 + l] <= cfcbe_thresh) && (cfpmh[(j * 4 + k) * 512*4 + i*4 + l] >= 0.0) && (cfpmh[(j * 4 + k) * 512*4 + i*4 + l] <= cfpmh_thresh))
				if ((cfcbe[(j * 4 + k) * 512*4 + i*4 + l] >= 0.0) && (cfcbe[(j * 4 + k) * 512*4 + i*4 + l] < cfcbe_thresh))
				    cmask0[(j * 4 + k) * 512*4 + i*4 + l] = mask;
				}
		}

if (n != 128*512)
	{
	fprintf(stderr, "readASCMFile: cmask0 fewer than 65536 valid in %s: %d\n", fieldName, n);
	return 0;
	}

strcpy(gridName, "CloudClassifiers_2.2_km"); 
strcpy(fieldName, "ConsensusCloudMaskFineResolution");
if (VERBOSE) fprintf(stderr, "readASCMFile: grid=%s, field=%s\n", gridName, fieldName);
status = MtkReadBlock(fname, gridName, fieldName, block, &szbuf);
if (status != MTK_SUCCESS) 
	{
	fprintf(stderr, "readASCMFile: MtkReadBlock failed!!!, gname = %s, fname = %s, status = %d (%s)\n", gridName, fieldName, status, errs[status]);
	return 0;
	}

if (VERBOSE) fprintf(stderr, "readASCMFile: nline=%d, nsample=%d, datasize=%d, datatype=%d (%s)\n", 
	szbuf.nline, szbuf.nsample, szbuf.datasize, szbuf.datatype, types[szbuf.datatype]);
if (szbuf.nline != 64 || szbuf.nsample != 256)
	{
	fprintf(stderr, "readASCMFile: %s is not 64x256: (%d, %d)\n", fieldName, szbuf.nline, szbuf.nsample);
	return 0;
	}

cmask1 = (uint8 *) malloc(64 * 256 * 64 * sizeof(uint8));
if (!cmask1)
	{
	fprintf(stderr, "readASCMFile: cmask1 malloc failed!!!\n");
	return 0;
	}

n = 0;
for (j = 0; j < 64; j ++)
	for (i = 0; i < 256; i ++)
		{
		n ++;
		mask = 0;
		if (szbuf.data.u8[j][i] == 3) mask = 1;
		for (k = 0; k < 8; k++)
			for (l = 0; l < 8; l++)
				{
				cmask1[(j * 8 + k) * 256*8 + i*8 + l] = mask;
				}
		}

if (n != 64*256)
	{
	fprintf(stderr, "readASCMFile: cmask1 fewer than 16384 valid in %s: %d\n", fieldName, n);
	return 0;
	}

return 1;
}

int write_data(char *fname, uint8 *data, int nlines, int nsamples)
{
FILE *f;

f = fopen(fname, "wb");
if (!f)
	{
	fprintf(stderr, "write_data: couldn't open %s\n", fname);
	return 0;
	}
	
if (fwrite(data, sizeof(uint8), nlines * nsamples, f) != nlines * nsamples)
	{
	fprintf(stderr, "write_data: couldn't write data\n");
	return 0;
	}
	
fclose(f);
return 1;
}

int main(int argc, char *argv[])
{
	int i, j, i2, j2;
	char s[256];
	char * pch;

	if (argc < 4)
		{
		fprintf(stderr, "Usage: readASCMFile input-misr-file block output-data-file\n");
		return 1;
		}
		

	strcpy(fname[0], argv[1]);
	block = atoi(argv[2]);
	strcpy(fname[1], argv[3]);

	if (!readASCMFile(fname[0])) return 1;

	if (!write_data(fname[1], cmask0, 512, 2048)) return 1;

	return 0;
}
