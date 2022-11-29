/*
Ehsan Mosadegh, 10 Nov 2022
use for:
	change cloud shape of cloudmask files from (x,y) to (xxxx,yyyy)

note: I changed all success return values from 1 to 0 in the old code at the end of each function;


original: readMISRCloudMask.c
Sky Coyote 18 Apr 09
old collments:

	Read TERRAIN radiance data, perform terrain correction, convert to TOA BRF, 
	save as data and PNG.  Mark and fix terrain dropouts.  Discard files with
	sz >= 80.0.

*/

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

int readMISRCloudMask(char *fname, char *cloudmaskname);
// int write_data(char *fname, uint8 *data, int nlines, int nsamples);
int write_data(char *fname, uint8_t* data, int nlines, int nsamples);

//######################################################################################################################

int readMISRCloudMask(char *fname, char *cloudmaskname) 
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

	printf("chk-3 \n");

	if (VERBOSE) fprintf(stderr, "readMISRCloudMask: fname=%s, block=%d\n", fname, block);
	status = MtkFileType(fname, &filetype);
	
	if (status != MTK_SUCCESS) 
	{
		fprintf(stderr, "readMISRCloudMask: MtkFileType failed!!!, status = %d (%s)\n", status, errs[status]);
		return 1; // error
	}
	printf("chk-4 \n");

	// printf("filetype: %s \n" , filetype);
	// printf(filetype);

	printf("chk-5 \n");

	/*********************** for SDCM *********************/
	if (strcmp("SDCM", cloudmaskname)==0) // to compare 2 strings  // if (cloudmaskname=="SDCM")
	{
		printf("cloudmask mode: %s \n" , cloudmaskname);
		if (filetype != MTK_TC_CLOUD)  // we change it for SDCM
		{
			fprintf(stderr, "readMISRCloudMask: TC_CLOUS STEREO filetype issue!!!\n"); // fix this readMISRCloudMask
			return 1; // error
		}

		// setup SDCM grid and field names
		// printf("seting up grid & field for SDCM...\n");
		strcpy(gridName, "Stereo_WithoutWindCorrection_1.1_km"); 
		strcpy(fieldName, "StereoDerivedCloudMask_WithoutWindCorrection");
		printf("gridName: %s \n" , gridName);
		printf("fieldName: %s \n" , fieldName);
	}
	
	/*********************** for ASCM *********************/
	if (strcmp("ASCM", cloudmaskname)==0) // check this
	{
		printf("cloudmask mode: %s \n" , cloudmaskname);
		if (filetype != MTK_TC_CLASSIFIERS) 
		{
			fprintf(stderr, "readMISRCloudMask: ASCM filetype not supported!!!\n");
			return 1; // eror
		}


		// setup ASCM grid and field names - check gird and filed bellow
		strcpy(gridName, "ASCMParams_1.1_km");   
		strcpy(fieldName, "AngularSignatureCloudMask");
	}




	/*********************** for RCCM *********************/
	if (strcmp("RCCM", cloudmaskname)==0)
	{
		printf("cloudmask mode: %s \n" , cloudmaskname);
		
		// if (filetype != MTK_GRP_RCCM);  // check this data type again 
		// {
		// 	fprintf(stderr, "readMISRCloudMask: RCCM filetype not supported!!!\n"); // fix this readMISRCloudMask
		// 	return 1; // error
		// }

		printf("chk-6 \n");

		// setup SDCM grid and field names
		strcpy(gridName, "RCCM"); 
		strcpy(fieldName, "Cloud");
	}



	/*=================================================================*/
	/* read any cloud mask file */

	if (VERBOSE) fprintf(stderr, "readMISRCloudMask-1: grid=%s, field=%s\n", gridName, fieldName);
	status = MtkReadBlock(fname, gridName, fieldName, block, &Mtk_data_buf);
	
	if (status != MTK_SUCCESS) 
	{	//fprintf(stderr, "readMISRCloudMask: MtkReadBlock failed!!!, status = %d (%s)\n", status, errs[status]);
		fprintf(stderr, "readMISRCloudMask-3: MtkReadBlock failed!!!, gname = %s, fname = %s, status = %d (%s)\n", gridName, fieldName, status, errs[status]);
		return 1; // error
	}

	if (VERBOSE) fprintf(stderr, "readMISRCloudMask-4: nline=%d, nsample=%d, datasize=%d, datatype=%d (%s)\n", 
		Mtk_data_buf.nline, Mtk_data_buf.nsample, Mtk_data_buf.datasize, Mtk_data_buf.datatype, types[Mtk_data_buf.datatype]);

	if (Mtk_data_buf.nline != 128 || Mtk_data_buf.nsample != 512) 
	{
		fprintf(stderr, "readMISRCloudMask-5: %s is not 128x512: (%d, %d)\n", fieldName, Mtk_data_buf.nline, Mtk_data_buf.nsample);
		return 1; // error
	}

	// cmask0_ptr = (uint8 *) malloc(128 * 512 * 4 * 4 * sizeof(uint8)); // allocates mem- and returns a ptr to the 1st byte in the allocated mem- == cmask0_ptr
	cmask0_ptr = (uint8_t *) malloc(128 * 512 * 4 * 4 * sizeof(uint8_t)); // allocates mem- and returns a ptr to the 1st byte in the allocated mem- == cmask0_ptr

	if (!cmask0_ptr) 
	{ 	// check if ptr is NULL
		fprintf(stderr, "readMISRCloudMask-6: malloc failed (cmask0_ptr == 1st byte of allocated mem-)!!!\n");
		return 1; // error
	}

	// E: all commnets here are mine
	n = 0;
	for (j = 0; j < 128; j ++) // row
		for (i = 0; i < 512; i ++) // column
		{
			n ++;

			for (k = 0; k < 4; k++)
				for (l = 0; l < 4; l++) 
				{

					// printf("cloud mask value: %x \n", Mtk_data_buf.data.u8[j][i]);

					cmask0_ptr[(j * 4 + k) * 512*4 + i*4 + l] = Mtk_data_buf.data.u8[j][i];

				}
		}


	if (n != 128*512) // total pixels available in ascm_buf == (65,536)
	{
		fprintf(stderr, "readMISRCloudMask: cmask0_ptr fewer than (65,536) valid in %s: %d\n", fieldName, n);
		return 1; // error
	}
	return 0; // success
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
		return 1; // error
	}
	// write data
	// if (fwrite(data, sizeof(uint8), nlines * nsamples, f) != nlines * nsamples)
	if (fwrite(data, sizeof(uint8_t), (nlines * nsamples), f) != nlines * nsamples) // note: also change data-type here
	{
		fprintf(stderr, "write_data: couldn't write all data elements\n");
		return 1; // error
	}

	fclose(f);
	return 0; // success

	// printf("writing finished in C \n");
}

//######################################################################################################################

int main(int argc, char *argv[]) 
{
	/****************************************/
	/* check inout args */
	// printf("\ncmdline args count=%d\n", argc);
	
	/* First argument is executable name only */
 	printf("\nexe name=%s", argv[0]);
 	int e;
	for (e=1; e< argc; e++) 
	{
	    printf("\narg%d=%s", e, argv[e]);
	}
	printf("\n");
	/* check inout args */
	/****************************************/

	int i, j, i2, j2;

	char *cloudmaskname;

	if (argc != 5) // should be number of args that come to the code 
	{
		// fprintf(stderr, "Usage: readMISRCloudMask input-misr-file block output-data-file\n");
		fprintf(stderr, "Usage: <readMISRCloudMask> inputMISRFile block outputDataFile cloudMaskName\n");
		return 1; // error
	}

	printf("check-1 \n");

	// strcpy(cloudmaskname, argv[4]);
	strcpy(fname[0], argv[1]);
	block = atoi(argv[2]);
	strcpy(fname[1], argv[3]);
	strcpy(cloudmaskname, argv[4]); // copy argv[4] to cloudmaskname
	// printf("cloudMask mode: %s \n", cloudmaskname);

	printf("check-2 \n");
	// here we check error from a f(.)
	if (readMISRCloudMask(fname[0], cloudmaskname)) return 1; // error
	if (write_data(fname[1], cmask0_ptr, 512, 2048)) return 1; // E- we only write cmask0_ptr data as output! all elements are checked to be total of (512*2048); !0 = 1 = error signal from inside f(.)
	free(cmask0_ptr);
	// printf("cloudmask finished in C \n");
	return 0; // success
}
