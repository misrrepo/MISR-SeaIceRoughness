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
#include <stdint.h> // to define 8-bit int; ref: https://pubs.opengroup.org/onlinepubs/009695399/basedefs/stdint.h.html

#define NO_DATA -999999.0
#define BACKGROUND -999998.0
#define FOREGROUND -999997.0
#define TDROPOUT -999996.0
#define MASKED 0
#define VERBOSE 0
// #define NODATA2 -9.0 // Ehsan, fill value for condition when cfcbe has negative values

// int8_t cfcbe_fill = -99; // Ehsan, changed to unsigned-int-8 bits

char fname[2][256];
int block = 0;
// uint8 *cmask0_ptr = 0;
uint8_t* cmask0_ptr = 0;

uint8_t* cmask1 = 0;
double* fraction_cloudBestEst_buf = 0;
int16_t *cfpmh = 0;
double cfcbe_thresh = 0.1; // between 0 and 1.0
double cfpmh_thresh = 0.0; // meters

//######################################################################################################################

int readASCMFile(char *fname);
// int write_data(char *fname, uint8 *data, int nlines, int nsamples);
int write_data(char *fname, uint8_t* data, int nlines, int nsamples);

//######################################################################################################################

int readASCMFile(char *fname) 
{

	MTKt_DataBuffer Mtk_data_buf = MTKT_DATABUFFER_INIT;
	char gridName[256];
	char fieldName[256];
	int status;
	MTKt_FileType filetype;
	char *types[] = MTKd_DataType;
	char *errs[] = MTK_ERR_DESC;
	int i, j, k, l, n;
	int maskline = 512;
	int masksamples = 2048;
	uint8_t mask;

	if (VERBOSE) fprintf(stderr, "readASCMFile: fname=%s, block=%d\n", fname, block);
	status = MtkFileType(fname, &filetype);
	
	if (status != MTK_SUCCESS) {
		fprintf(stderr, "readASCMFile: MtkFileType failed!!!, status = %d (%s)\n", status, errs[status]);
		return 0;
	}

	if (filetype != MTK_TC_CLASSIFIERS) {
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
	/*=================================================================*/
	//grid-field-1

	// strcpy(gridName, "CloudFractions_17.6_km"); 
	// strcpy(fieldName, "CombinedFractionCloudBestEstimate");
	// if (VERBOSE) fprintf(stderr, "readASCMFile: grid=%s, field=%s\n", gridName, fieldName);
	// // read hdf file
	// status = MtkReadBlock(fname, gridName, fieldName, block, &Mtk_data_buf);
	// if (status != MTK_SUCCESS) {
	// 	fprintf(stderr, "readASCMFile-1: MtkReadBlock failed!!!, gname = %s, fname = %s, status = %d (%s)\n", gridName, fieldName, status, errs[status]);
	// 	return 0;}

	// if (VERBOSE) fprintf(stderr, "readASCMFile: nline=%d, nsample=%d, datasize=%d, datatype=%d (%s)\n", 
	// 	Mtk_data_buf.nline, Mtk_data_buf.nsample, Mtk_data_buf.datasize, Mtk_data_buf.datatype, types[Mtk_data_buf.datatype]);

	// if (Mtk_data_buf.nline != 8 || Mtk_data_buf.nsample != 32)
	// 	{
	// 	fprintf(stderr, "readASCMFile: %s is not 8x32: (%d, %d)\n", fieldName, Mtk_data_buf.nline, Mtk_data_buf.nsample);
	// 	return 0;
	// 	}

	// fraction_cloudBestEst_buf = (double *) malloc(8 * 32 * 64 * 64 * sizeof(double)); // CombinedFractionCloudBestEstimate

	// if (!fraction_cloudBestEst_buf)
	// 	{
	// 	fprintf(stderr, "readASCMFile: fraction_cloudBestEst_buf malloc failed!!!\n");
	// 	return 0;
	// 	}

	// n = 0;
	// for (j = 0; j < 8; j ++)
	// 	for (i = 0; i < 32; i ++)
	// 	{
	// 		n ++;
	// 		for (k = 0; k < 64; k++)
	// 			for (l = 0; l < 64; l++)
	// 			{
	// 				fraction_cloudBestEst_buf[(j * 64 + k) * 32*64 + i*64 + l] = Mtk_data_buf.data.d[j][i];
	// 			}
	// 	}

	// if (n != 8*32)
	// 	{
	// 	fprintf(stderr, "readASCMFile: cfmask fewer than 256 valid in %s: %d\n", fieldName, n);
	// 	return 0;
	// 	}

	/*=================================================================*/
	//grid-field-2

	// strcpy(gridName, "CloudFractions_17.6_km"); 
	// strcpy(fieldName, "MedianPrelimCloudHeight"); // E- this field does not exist in file! i turned it off
	// if (VERBOSE) fprintf(stderr, "readASCMFile: grid=%s, field=%s\n", gridName, fieldName);
	// // read data
	// status = MtkReadBlock(fname, gridName, fieldName, block, &mpch_buf);
	// if (status != MTK_SUCCESS) 
	// 	{
	// 	fprintf(stderr, "readASCMFile-2: MtkReadBlock failed!!!, gname = %s, fname = %s, status = %d (%s)\n", gridName, fieldName, status, errs[status]);
	// 	return 0;
	// 	}

	// if (VERBOSE) fprintf(stderr, "readASCMFile: nline=%d, nsample=%d, datasize=%d, datatype=%d (%s)\n", 
	// 	mpch_buf.nline, mpch_buf.nsample, mpch_buf.datasize, mpch_buf.datatype, types[mpch_buf.datatype]);
	// if (mpch_buf.nline != 8 || mpch_buf.nsample != 32)
	// 	{
	// 	fprintf(stderr, "readASCMFile: %s is not 8x32: (%d, %d)\n", fieldName, mpch_buf.nline, ascm_buf.nsample);
	// 	return 0;
	// 	}

	// cfpmh = (int16 *) malloc(8 * 32 * 64 * 64 * sizeof(int16));
	// if (!cfpmh)
	// 	{
	// 	fprintf(stderr, "readASCMFile: cfpmh malloc failed!!!\n");
	// 	return 0;
	// 	}

	// n = 0;
	// for (j = 0; j < 8; j ++)
	// 	for (i = 0; i < 32; i ++)
	// 		{
	// 		n ++;
	// 		mask = 0;
	// 		for (k = 0; k < 64; k++)
	// 			for (l = 0; l < 64; l++)
	// 				{
	// 				cfpmh[(j * 64 + k) * 32*64 + i*64 + l] = mpch_buf.data.i16[j][i];
	// 				}
	// 		}

	// if (n != 8*32)
	// 	{
	// 	fprintf(stderr, "readASCMFile: cfpmh fewer than 256 valid in %s: %d\n", fieldName, n);
	// 	return 0;
	// 	}
	/*=================================================================*/
	//E- grid-field-3: ASCM

	strcpy(gridName, "ASCMParams_1.1_km"); 
	strcpy(fieldName, "AngularSignatureCloudMask");
	if (VERBOSE) fprintf(stderr, "readASCMFile: grid=%s, field=%s\n", gridName, fieldName);
	status = MtkReadBlock(fname, gridName, fieldName, block, &Mtk_data_buf);
	
	if (status != MTK_SUCCESS) {	//fprintf(stderr, "readASCMFile: MtkReadBlock failed!!!, status = %d (%s)\n", status, errs[status]);
		fprintf(stderr, "readASCMFile-3: MtkReadBlock failed!!!, gname = %s, fname = %s, status = %d (%s)\n", gridName, fieldName, status, errs[status]);
		return 0;
	}

	if (VERBOSE) fprintf(stderr, "readASCMFile: nline=%d, nsample=%d, datasize=%d, datatype=%d (%s)\n", 
		Mtk_data_buf.nline, Mtk_data_buf.nsample, Mtk_data_buf.datasize, Mtk_data_buf.datatype, types[Mtk_data_buf.datatype]);

	if (Mtk_data_buf.nline != 128 || Mtk_data_buf.nsample != 512) {
		fprintf(stderr, "readASCMFile: %s is not 128x512: (%d, %d)\n", fieldName, Mtk_data_buf.nline, Mtk_data_buf.nsample);
		return 0;
	}

	// cmask0_ptr = (uint8 *) malloc(128 * 512 * 4 * 4 * sizeof(uint8)); // allocates mem- and returns a ptr to the 1st byte in the allocated mem- == cmask0_ptr
	cmask0_ptr = (uint8_t *) malloc(128 * 512 * 4 * 4 * sizeof(uint8_t)); // allocates mem- and returns a ptr to the 1st byte in the allocated mem- == cmask0_ptr

	if (!cmask0_ptr) { 	// check if ptr is NULL
		fprintf(stderr, "readASCMFile: malloc failed (cmask0_ptr == 1st byte of allocated mem-)!!!\n");
		return 0;
	}
		

	// int cmask0_pixel;

	// E: all commnets here are mine
	n = 0;
	for (j = 0; j < 128; j ++)
		for (i = 0; i < 512; i ++) {
			n ++;

			/* E: here we map 4 cloudy conditions from [1,2,3,4] conditions (based on dics p.13) to [0-1] space */
			mask = 0; // E- any pixel w/cloud== cloudHC = 0 based on CM docs p.13, we set cm=0 to zero that pixel
			
			// if (Mtk_data_buf.data.u8[j][i] == 4) mask = 1; // E- 4 means clear w/HC==sunny sky; 
			// if (Mtk_data_buf.data.u8[j][i] == 4) mask = 1; // E- 4 means clear w/HC==sunny sky; 

			for (k = 0; k < 4; k++)
				for (l = 0; l < 4; l++) {
					// cmask0_pixel = (j * 4 + k) * 512*4 + i*4 + l;
					// printf("pixel index shoud reach 512*2048: %d \n" , cmask0_pixel+1); // shoudl be 512*2048
					// printf("fraction_cloudBestEst_buf pixel value: %f \n" , fraction_cloudBestEst_buf[(j * 4 + k) * 512*4 + i*4 + l] );


					//if ((fraction_cloudBestEst_buf[(j * 4 + k) * 512*4 + i*4 + l] >= 0.0) && (fraction_cloudBestEst_buf[(j * 4 + k) * 512*4 + i*4 + l] <= cfcbe_thresh) && (cfpmh[(j * 4 + k) * 512*4 + i*4 + l] >= 0.0) && (cfpmh[(j * 4 + k) * 512*4 + i*4 + l] <= cfpmh_thresh))
					
					// we apply this condition if fraction_cloudBestEst_buf pixel value is in range [0.0, cfcbe_thresh == 0.1] meaning less than 10% cloudy
					// if ((fraction_cloudBestEst_buf[(j * 4 + k) * 512*4 + i*4 + l] >= 0.0) && (fraction_cloudBestEst_buf[(j * 4 + k) * 512*4 + i*4 + l] < cfcbe_thresh)) { // E: means if upto 0.1 we can take that as not cloudy?
						/* we make more pixels cloudy == 0 // E: fraction_cloudBestEst_buf = CombinedFractionCloudBestEstimate */
						
					// cmask0_ptr[(j * 4 + k) * 512*4 + i*4 + l] = mask ; // E: original; so cmask0_ptr is either 0 or 1, 0==cloudy 1==clear
					
					if (Mtk_data_buf.data.u8[j][i] == 4) { // E- 4 means clear w/HC==sunny sky; 
						
						cmask0_ptr[(j * 4 + k) * 512*4 + i*4 + l] = 1 ; // E: mine; so cmask0_ptr is either 0 or 1, 0==cloudy 1==clear ; E: replaced mask w/ 1 to test
					}
					
					// Ehsan: 
					else {  // fraction_cloudBestEst_buf[(j * 64 + k) * 32*64 + i*64 + l] = Mtk_data_buf.data.d[j][i];
					
						// printf("what should we do if a fraction_cloudBestEst_buf pixel wasn't in range [0, 0.1]? filled w/NODATA2 \n");
						// cmask0_ptr[(j * 4 + k) * 512*4 + i*4 + l] = NODATA2 ;
						cmask0_ptr[(j * 4 + k) * 512*4 + i*4 + l] = mask ; // E: changed it to test
					}
					

				}
		}

	if (n != 128*512) // total pixels available in ascm_buf == 65,536
	{
		fprintf(stderr, "readASCMFile: cmask0_ptr fewer than 65,536 valid in %s: %d\n", fieldName, n);
		return 0;
	}

	/*=================================================================*/
	//grid-field-4

	// strcpy(gridName, "CloudClassifiers_2.2_km"); 
	// strcpy(fieldName, "ConsensusCloudMaskFineResolution");
	// if (VERBOSE) fprintf(stderr, "readASCMFile: grid=%s, field=%s\n", gridName, fieldName);
	// // read adata
	// status = MtkReadBlock(fname, gridName, fieldName, block, &Mtk_data_buf);
	// if (status != MTK_SUCCESS) 
	// 	{
	// 	fprintf(stderr, "readASCMFile-4: MtkReadBlock failed!!!, gname = %s, fname = %s, status = %d (%s)\n", gridName, fieldName, status, errs[status]);
	// 	return 0;
	// 	}

	// if (VERBOSE) fprintf(stderr, "readASCMFile: nline=%d, nsample=%d, datasize=%d, datatype=%d (%s)\n", 
	// 	Mtk_data_buf.nline, Mtk_data_buf.nsample, Mtk_data_buf.datasize, Mtk_data_buf.datatype, types[Mtk_data_buf.datatype]);
	// if (Mtk_data_buf.nline != 64 || Mtk_data_buf.nsample != 256)
	// 	{
	// 	fprintf(stderr, "readASCMFile: %s is not 64x256: (%d, %d)\n", fieldName, Mtk_data_buf.nline, Mtk_data_buf.nsample);
	// 	return 0;
	// 	}

	// cmask1 = (uint8 *) malloc(64 * 256 * 8 * 8 * sizeof(uint8));
	// if (!cmask1)
	// 	{
	// 	fprintf(stderr, "readASCMFile: cmask1 malloc failed!!!\n");
	// 	return 0;
	// 	}

	// n = 0;
	// for (j = 0; j < 64; j ++)
	// 	for (i = 0; i < 256; i ++)
	// 		{
	// 		n ++;
	// 		mask = 0;
	// 		if (Mtk_data_buf.data.u8[j][i] == 3) mask = 1;
	// 		for (k = 0; k < 8; k++)
	// 			for (l = 0; l < 8; l++)
	// 				{
	// 				cmask1[(j * 8 + k) * 256*8 + i*8 + l] = mask;
	// 				}
	// 		}

	// if (n != 64*256)
	// 	{
	// 	fprintf(stderr, "readASCMFile: cmask1 fewer than 16384 valid in %s: %d\n", fieldName, n);
	// 	return 0;
	// 	}

	return 1;
}

//######################################################################################################################

// int write_data(char* fname, uint8* data, int nlines, int nsamples) // data == ptr to 1st byte of mem-
int write_data(char* fname, uint8_t* data, int nlines, int nsamples) // data == ptr to 1st byte of mem-

{
	FILE* f;

	f = fopen(fname, "wb");
	if (!f)
	{
		fprintf(stderr, "write_data: couldn't open %s\n", fname);
		return 0;
	}
	// write data
	// if (fwrite(data, sizeof(uint8), nlines * nsamples, f) != nlines * nsamples)
	if (fwrite(data, sizeof(uint8_t), (nlines * nsamples), f) != nlines * nsamples) // note: also change data-type here
	{
		fprintf(stderr, "write_data: couldn't write all data elements\n");
		return 0;
	}
		
	fclose(f);
	return 1;
}

//######################################################################################################################

int main(int argc, char* argv[]) {

	int i, j, i2, j2;
	char s[256];
	char * pch;

	if (argc < 4) {
		fprintf(stderr, "Usage: readASCMFile input-misr-file block output-data-file\n");
		return 1;
	}
		

	strcpy(fname[0], argv[1]);
	block = atoi(argv[2]);
	strcpy(fname[1], argv[3]);

	if (!readASCMFile(fname[0])) return 1;

	if (!write_data(fname[1], cmask0_ptr, 512, 2048)) return 1; // E- we only write cmask0_ptr data as output! all elemenst are checked to be total of 512*2048

	free(cmask0_ptr);

	return 0;
}
