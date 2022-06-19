/*
TOA.c
Read ELLIPSOID radiance data, convert to TOA BRF, 
  save as data.  Discard files with
  sz >= 80.0.
Sky Coyote 18 Apr 09
Anne Nolin 11 Oct 2019
Ehsan 11 Nov 2019 - I created the final version after multiple editting
notes: 
this program receives each hdf file which includes surface-projected TOA radiance to the ellipsoid and processes each files.
each file has 180 blocks. This code allocates each GRP datafile to blocks.
to-do:
input file format?
output???
*/
/*
note: check line 850: why -1 is used? 
*/

// these inside /usr/include
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "fftw3.h"
#include <png.h>
#include <MisrToolkit.h>
#include <MisrError.h>

// constants
#define NO_DATA -999999.0
#define BACKGROUND -999998.0
#define FOREGROUND -999997.0
//#define TDROPOUT -999996.0
#define SINC_DIST 10.0
#define BORDER 10
#define DAMPING 3.25
#define VERBOSE 0
//#define PNG_NLINES 512 // is inside main
//#define PNG_NSAMPLES 2048 // is inside main
// #define ZOOM 64


// Global variables 
char* command = "date";



// setting for running for BRF conversion or toa radiance output- turn on (= 1) the option you need
int TOARadianceOut = 0; // Ehsan: turns on TOAradiance, outputs TOA radiance
int doTOA_BRFConversion = 1; // Ehsan: turns on BRF conversion, outputs TOA reflectance; note: we used an equation from MISR docs to convert TOA-radance to TOA-reflectance

// other settings
char fname[3][256];
int PNG_NLINES, PNG_NSAMPLES, ZOOM;
// int ZOOM = 16;
//int demlines = 3480;
//int demsamples = 3720;
//double *slope = 0;
//double *aspect = 0;
int path = 0;
int orbit = 0;
int block = 0;
int band = 2; // red-band
int camera = 1;
int nlines = 0;
int nsamples = 0;
double *data = 0;
int noData = 0; // turned off here
int nvalid = 0;
int ndropouts = 0;
int minnaert = 0; // set to zero to turn off the correction

double min;
double max;
double mean;
double stddev;
double meanSZ;
int FileLines, FileSamples;

// double displayMax = -1.0;
//png_structp png_ptr = 0;
//png_infop info_ptr = 0;


////////// declaration of functions //////////
char *data2image(double *data, int nlines, int nsamples, int mode);
//int write_png(char *fname, char *image, int ny, int nx);
int pixel2grid(int path, int block, int line, int sample, int *j, int *i); // needed: adds georeference info to data
int ll2grid(double lat, double lon, int *j, int *i);
//int getAspectSlope(int line, int sample, double *za, double *zs); AN
int readEllipsoidFile(char *fname);
int getDataStats(double *data, int nlines, int nsamples);
double sinc(double x);
double *convolve2d(double *data, double *filter, int nlines, int nsamples);
double *zoom2d(double *data, int nlines, int nsamples, int zoom);
double *extendArray(double *data, int nlines, int nsamples, int border);
double *extractArray(double *data, int nlines, int nsamples, int border);
double *zoomArray(double *data, int nlines, int nsamples, int zoom);
int write_data(char *fname, double *data, int nlines, int nsamples);
int read_data(char *fname, double **data, int nlines, int nsamples);


// /////////////////////////////////// data2image ////////////////////////////////////////////
// Q- what does this do? turn off? E: turnedoff 
// char *data2image(double *data, int nlines, int nsamples, int mode)
// {
// char *image; // local
// double min, max, z, dz; // local
// int i;

// if (displayMax > 0.0) 
// 	{
// 	max = displayMax;
// 	min = 0.0;
// 	}
// else
// 	{
// 	max = -1.0e23;
// 	for (i = 0; i < nlines * nsamples; i ++)
// 		{
// 		z = data[i];
// 		if (z == NO_DATA) continue;
// 		if (z == BACKGROUND) continue;
// 		if (z == FOREGROUND) continue;
// 		// if (z == TDROPOUT) continue;
// 		if (z > max) max = z;
// 		}
// 	if (max == -1.0e23)
// 		{
// 		if (VERBOSE) fprintf(stderr, "data2image: no valid data\n");
// 		max = 0.0;
// 		//return 0;
// 		}
// 	if (mode > 0) 
// 		{
// 		min = 1.0e23;
// 		for (i = 0; i < nlines * nsamples; i ++)
// 			{
// 			z = data[i];
// 			if (z == NO_DATA) continue;
// 			if (z == BACKGROUND) continue;
// 			if (z == FOREGROUND) continue;
// 			// if (z == TDROPOUT) continue;
// 			if (z < min) min = z;
// 			}
// 		if (min == 1.0e23)
// 			{
// 			if (VERBOSE) fprintf(stderr, "data2image: no valid data\n");
// 			min = 0.0;
// 			//return 0;
// 			}
// 		}
// 	else min = 0.0;
// 	}
// if (VERBOSE) fprintf(stderr, "data2image: min=%.3f, max=%.3f\n", min, max);
// if (max != min) dz =  255.0 / (max - min);
// else dz = 0.0;

// image = (char *) malloc(nlines * nsamples);
// if (!image)
// 	{
// 	fprintf(stderr, "data2image: couldn't malloc image\n");
// 	return 0;
// 	}

// for (i = 0; i < nlines * nsamples; i ++)
// 	{
// 	z = data[i];
// 	if (z == NO_DATA) image[i] = 0;
// 	else if (z == BACKGROUND) image[i] = 0;
// 	else if (z == FOREGROUND) image[i] = 32;
// 	//else if (z == TDROPOUT) image[i] = 0;
// 	else
// 		{
// 		z = (z - min) * dz;
// 		if (z > 255.0) image[i] = 255;
// 		else if (z < 0.0) image[i] = 0;
// 		else image[i] = z;
// 		}
// 	}

// return image;
// }
///////////////////////////////////// data2image ////////////////////////////////////////////

///////////////////////////////////// write_png ////////////////////////////////////////////

// int write_png(char *fname, char *image, int ny, int nx)
// {
// FILE *fp;
// png_bytepp row_ptrs;
// int j;

// if (!image)
// 	{
// 	fprintf(stderr, "write_png: null image\n");
// 	return 0;
// 	}

// row_ptrs = (png_bytepp) malloc(ny * sizeof(png_bytep));
// if (!row_ptrs)
// 	{
// 	fprintf(stderr, "write_png: couldn't malloc row_ptrs\n");
// 	return 0;
// 	}
// for (j = 0; j < ny; j ++) row_ptrs[j] = (png_bytep)(image + j * nx);

// fp = fopen(fname, "wb");
// if (!fp)
// 	{
// 	fprintf(stderr, "write_png: couldn't open %s\n", fname);
// 	return 0;
// 	}

// png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 
// 	png_voidp_NULL, png_error_ptr_NULL, png_error_ptr_NULL);
// if (!png_ptr)
// 	{
// 	fclose(fp);
// 	fprintf(stderr, "write_png: png_create_write_struct failed\n");
// 	return 0;
// 	}

// info_ptr = png_create_info_struct(png_ptr);
// if (!info_ptr)
// 	{
// 	png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
// 	fclose(fp);
// 	fprintf(stderr, "write_png: png_create_info_struct failed\n");
// 	return 0;
// 	}

// if (setjmp(png_jmpbuf(png_ptr)))
// 	{
// 	png_destroy_write_struct(&png_ptr, &info_ptr);
// 	fclose(fp);
// 	fprintf(stderr, "write_png: longjmp from png error\n");
// 	return 0;
// 	}

// png_init_io(png_ptr, fp);

// png_set_IHDR(png_ptr, info_ptr, nx, ny, 8, PNG_COLOR_TYPE_GRAY, 
// 	PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	
// png_set_rows(png_ptr, info_ptr, row_ptrs);

// png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
       
// png_destroy_write_struct(&png_ptr, &info_ptr);

// fclose(fp);
// if (row_ptrs) free(row_ptrs);
// return 1;
// }
///////////////////////////////////// write_png ////////////////////////////////////////////

/////////////////////////////////// pixel2grid ////////////////////////////////////////////

int pixel2grid(int path, int block, int line, int sample, int* j, int* i){
	int status; // local variables
	char *errs[] = MTK_ERR_DESC;
	double lat, lon;
	double r = 6371228.0;
	double r2 = 2.0 * r;
	double c = 626.688125;
	double x0 = -2443770.3;
	double y0 = -313657.41;
	double x, y, z;

	status = MtkBlsToLatLon(path, 275, block, line * 1.0, sample * 1.0, &lat, &lon); // MISR blocks to LatLon
	if (status != MTK_SUCCESS) 
		{
		fprintf(stderr, "pixel2grid: MtkBlsToLatLon failed!!!, status = %d (%s) %d %d %d %d\n", status, errs[status], path, block, line, sample);
		return 0;
		}
		
	if (VERBOSE) fprintf(stderr, "pixel2grid: lat = %.6f, lon = %.6f\n", lat, lon);

	lat *= M_PI / 180.0;
	lon -= 90.0;
	lon *= M_PI / 180.0;
	z = sin(M_PI_4 - lat / 2.0);
	x = r2 * cos(lon) * z;
	y = r2 * sin(lon) * z;
	if (VERBOSE) fprintf(stderr, "pixel2grid: absolute x = %.6f, y = %.6f\n", x, y);
	x = x - x0 + c / 2.0;
	y = y - y0 - c / 2.0;
	if (VERBOSE) fprintf(stderr, "pixel2grid: relative x = %.6f, y = %.6f\n", x, y);
	x = x / c;
	y = y / c;
	*i = rint(x); // lon? // dereferenced to update variable outside f()
	*j = rint(-y); // lat? // dereferenced to update variable outside f()

	if (VERBOSE) fprintf(stderr, "pixel2grid: scaled x = %.6f, y = %.6f\n", x, y);

	return 1;}
/////////////////////////////////// pixel2grid ////////////////////////////////////////////

///////////////////////////////////// ll2grid ////////////////////////////////////////////
int ll2grid(double lat, double lon, int* j, int* i)
{
double r = 6371228.0;
double r2 = 2.0 * r;
double c = 626.688125;
double x0 = -2443770.3;
double y0 = -313657.41;
double x, y, z;

if (VERBOSE) fprintf(stderr, "ll2grid: lat = %.6f, lon = %.6f\n", lat, lon);

lat *= M_PI / 180.0;
lon -= 90.0;
lon *= M_PI / 180.0;
z = sin(M_PI_4 - lat / 2.0);
x = r2 * cos(lon) * z;
y = r2 * sin(lon) * z;
if (VERBOSE) fprintf(stderr, "ll2grid: absolute x = %.6f, y = %.6f\n", x, y);
x = x - x0 + c / 2.0;
y = y - y0 - c / 2.0;
if (VERBOSE) fprintf(stderr, "ll2grid: relative x = %.6f, y = %.6f\n", x, y);
x = x / c;
y = y / c;
*i = rint(x); // dereferenced to reflect back in calling f(); updates input arg outside the scope of f() 
*j = rint(-y); //

if (VERBOSE) fprintf(stderr, "ll2grid: scaled x = %.6f, y = %.6f\n", x, y);
fprintf(stderr, "ll2grid: j = %d, i = %d\n", *j, *i);

return 1;
}
///////////////////////////////////// ll2grid ////////////////////////////////////////////

///////////////////////////////////// getAspectSlope ////////////////////////////////////////////

//AN: int getAspectSlope(int line, int sample, double *za, double *zs)
// {
// int i, j;

// if (!pixel2grid(path, block, line, sample, &j, &i)) return 0;

// if (j < 0 || j >= demlines || i < 0 || i >= demsamples) 
// 	{
// 	*za = 0.0;
// 	*zs = 0.0;
// 	}
// else 
// 	{
// 	*za = aspect[i + j * demsamples];
// 	*zs = slope[i + j * demsamples];
// 	if (*za == NO_DATA || *zs == NO_DATA) 
// 		{
// 		*za = 0.0;
// 		*zs = 0.0;
// 		}
// 	}

// return 1;
// }
///////////////////////////////////// getAspectSlope ////////////////////////////////////////////

///////////////////////////////////// readEllipsoidFile ////////////////////////////////////////

int readEllipsoidFile(char* fname) // gets the hdf file
{

	MTKt_DataBuffer radianceBuffer = MTKT_DATABUFFER_INIT;
	MTKt_DataBuffer scaleFactorBuffer = MTKT_DATABUFFER_INIT;
	MTKt_DataBuffer fillbuf = MTKT_DATABUFFER_INIT;
	MTKt_DataBuffer conversionFactorBuffer = MTKT_DATABUFFER_INIT;
	MTKt_DataBuffer solarAzimuthBuffer = MTKT_DATABUFFER_INIT;
	MTKt_DataBuffer solarZenithBuffer = MTKT_DATABUFFER_INIT;
	char gridName[256];
	char fieldName[256];
	int status;
	MTKt_FileType filetype;
	char *types[] = MTKd_DataType;
	char *errs[] = MTK_ERR_DESC;
	int i, j, n, wrap = 0; // local var
	double *brfcf1 = 0, *brfcf2 = 0, *tmp = 0, *sz = 0, *sa = 0;
	//double radiance, solarAzimuth, solarZenith, terrainAspect, terrainSlope, p180 = M_PI / 180.0, cosi;
	double radiance, p180 = M_PI / 180.0, cosi, solarZenith, solarAzimuth;

	noData = 0; // is off

	if (VERBOSE) fprintf(stderr, "readEllipsoidFile: fname=%s, block=%d, band=%d\n", fname, block, band);

	status = MtkFileType(fname, &filetype);
	if (status != MTK_SUCCESS) 
		{
		fprintf(stderr, "readEllipsoidFile: MtkFileType failed!!!, status = %d (%s)\n", status, errs[status]);
		return 0;
		}
		
	//if (filetype != MTK_GRP_TERRAIN_GM)
	if (filetype != MTK_GRP_ELLIPSOID_GM)
		{
		fprintf(stderr, "readEllipsoidFile: only L1B2 global mode Ellipsoid files are supported!!!\n");
		return 0;
		}

	////----------------------------------------- [12/Oct/2021] Ehsan: turned off because we might not need geo-params& solar Azimuth --------------

	// strcpy(gridName, "GeometricParameters"); 
	// strcpy(fieldName, "SolarZenith");

	// if (VERBOSE) fprintf(stderr, "readEllipsoidFile: grid=%s, field=%s\n", gridName, fieldName);

	// status = MtkReadBlock(fname, gridName, fieldName, block, &solarZenithBuffer);

	// if (status != MTK_SUCCESS) 
	// 	{
	// 	fprintf(stderr, "readEllipsoidFile: MtkReadBlock [1] failed!!!, status = %d (%s) \n", status, errs[status]);
	// 	return 0;
	// 	}

	// if (VERBOSE) fprintf(stderr, "readEllipsoidFile: nline=%d, nsample=%d, datasize=%d, datatype=%d (%s) \n", 
	// 	solarZenithBuffer.nline, solarZenithBuffer.nsample, solarZenithBuffer.datasize, solarZenithBuffer.datatype, types[solarZenithBuffer.datatype]);

	// if (solarZenithBuffer.nline != 8 || solarZenithBuffer.nsample != 32)
	// 	{
	// 	fprintf(stderr, "readEllipsoidFile: %s is not 8x32: (%d, %d)\n", fieldName, solarZenithBuffer.nline, solarZenithBuffer.nsample);
	// 	return 0;
	// 	}

	// tmp = (double *) malloc(8 * 32 * sizeof(double));
	// if (!tmp)
	// 	{
	// 	fprintf(stderr, "readEllipsoidFile: tmp malloc failed!!!\n");
	// 	return 0;
	// 	}

	// n = 0; // E: counts num of pixels
	// for (j = 0; j < 8; j ++) // E: whats going on here? j=row?
	// 	for (i = 0; i < 32; i ++) // i=col?
	// 	{
	// 		if (solarZenithBuffer.data.d[j][i] >= 0.0) // ?
	// 		{
	// 			n ++; // check pixel values?
	// 			tmp[i + j * 32] = solarZenithBuffer.data.d[j][i];
	// 		}
	// 		else tmp[i + j * 32] = NO_DATA;
	// 	}
	// if (n != 256) // n=num of pixels?
	// 	if (VERBOSE) fprintf(stderr, "readEllipsoidFile: fewer than 256 valid (pixels?) in %s: %d\n", fieldName, n);

	// //printf("C: n1 is: %d \n" , (n));

	// if (n > 0) // num. of pixels?
	// { // if n pixels=positive
		
	// 	//printf("n1 = pos: %d \n", (n));
	// 	getDataStats(tmp, 8, 32);
	// 	meanSZ = mean;
	// 	//printf("meanSZ: %lf \n" , (meanSZ));

	// 	// E- turned off to try to include blocks=1-6
	// 	// if (meanSZ >= 80.0) // E- why 80? what is 80?! skips the blocks 1-6 here
	// 	// 	{
	// 	// 	noData = 1; // turns on
	// 	// 	return 1;
	// 	// 	}

	// 	//sz = zoomArray(tmp, 8, 32, 64); // not needed
	// 	sz = zoomArray(tmp, 8, 32, ZOOM);		// for PSU cloudmask data ZOOM = 16;  Ehsan: need this one to resample solar data to MISR image, 
	// 	if (!sz) 
	// 	{
	// 		printf("problem w/ sz. \n");
	// 		return 0;
	// 	}
	// }
	// else 
	// { // n is neg
	// 	noData = 1; // on=True --> all number of pixels?
	// 	return 1;
	// }


	////----------------------------------------------------------------------------------------------------------------------------
	if 		(band == 0) { strcpy(gridName, "BlueBand"); strcpy(fieldName, "Blue Radiance/RDQI"); }
	else if (band == 1) { strcpy(gridName, "GreenBand"); strcpy(fieldName, "Green Radiance/RDQI"); }
	else if (band == 2) { strcpy(gridName, "RedBand"); strcpy(fieldName, "Red Radiance/RDQI"); }
	else if (band == 3) { strcpy(gridName, "NIRBand"); strcpy(fieldName, "NIR Radiance/RDQI"); }
	else
		{
		fprintf(stderr, "readEllipsoidFile: unsupported band = %d\n", band);
		return 0;
		}

	if (VERBOSE) fprintf(stderr, "readEllipsoidFile: grid=%s, field=%s\n", gridName, fieldName);

	status = MtkReadBlock(fname, gridName, fieldName, block, &radianceBuffer); // radiance data 
	printf("C: status= %d, MTK_SUCCESS= %d \n" , status, MTK_SUCCESS);

	if (status != MTK_SUCCESS) 
		{
		fprintf(stderr, "readEllipsoidFile: MtkReadBlock [2] failed!!!, status = %d (%s)\n", status, errs[status]);
		return 0;
		}
		
	if (VERBOSE) fprintf(stderr, "readEllipsoidFile: nline=%d, nsample=%d, datasize=%d, datatype=%d (%s)\n", 
		radianceBuffer.nline, radianceBuffer.nsample, radianceBuffer.datasize, radianceBuffer.datatype, types[radianceBuffer.datatype]);
		
	//if (radianceBuffer.nline != 512 || radianceBuffer.nsample != 2048)
	if (radianceBuffer.nline != PNG_NLINES || radianceBuffer.nsample != PNG_NSAMPLES) // leave it here only for error checking
		{
		fprintf(stderr, "readEllipsoidFile: radianceBuffer is not 512x2048: (%d, %d)\n", radianceBuffer.nline, radianceBuffer.nsample);
		return 0;
		}

	////

	status = MtkFillValueGet(fname, gridName, fieldName, &fillbuf);
	if (status != MTK_SUCCESS) 
		{
		fprintf(stderr, "readEllipsoidFile: MtkFillValueGet failed!!!, status = %d (%s)\n", status, errs[status]);
		return 0;
		}

	/* If filetype is TERRAIN adjust fill value to account for obscured by topography flag. */
	status = MtkFileType(fname, &filetype);
	if (status != MTK_SUCCESS) 
		{
		fprintf(stderr, "readEllipsoidFile: MtkFileType failed!!!, status = %d (%s)\n", status, errs[status]);
		return 0;
		}

	//if (filetype ==  MTK_GRP_TERRAIN_GM || filetype ==  MTK_GRP_TERRAIN_LM)
	if (filetype ==  MTK_GRP_ELLIPSOID_GM || filetype ==  MTK_GRP_ELLIPSOID_LM)
		fillbuf.data.u16[0][0] -= 4;

	if (VERBOSE) fprintf(stderr, "readEllipsoidFile: FillValue: nline=%d, nsample=%d, datasize=%d, datatype=%d (%s), value=%u\n", 
		fillbuf.nline, fillbuf.nsample, fillbuf.datasize, fillbuf.datatype, types[fillbuf.datatype], fillbuf.data.u16[0][0]);

	////
	status = MtkGridAttrGet(fname, gridName, "Scale factor", &scaleFactorBuffer);
	if (status != MTK_SUCCESS) 
		{
		fprintf(stderr, "readEllipsoidFile: MtkGridAttrGet failed!!!, status = %d (%s)\n", status, errs[status]);
		return 0;
		}
	if (VERBOSE) fprintf(stderr, "readEllipsoidFile: Scale factor: nline=%d, nsample=%d, datasize=%d, datatype=%d (%s), value=%f\n", 
		scaleFactorBuffer.nline, scaleFactorBuffer.nsample, scaleFactorBuffer.datasize, scaleFactorBuffer.datatype, types[scaleFactorBuffer.datatype], scaleFactorBuffer.data.d[0][0]);

	//##########################################################################################################################

	if (band == 0) { strcpy(gridName, "BRF Conversion Factors"); strcpy(fieldName, "BlueConversionFactor"); }
	else if (band == 1) { strcpy(gridName, "BRF Conversion Factors"); strcpy(fieldName, "GreenConversionFactor"); }
	else if (band == 2) { strcpy(gridName, "BRF Conversion Factors"); strcpy(fieldName, "RedConversionFactor"); }		// we usually this band for our work 
	else if (band == 3) { strcpy(gridName, "BRF Conversion Factors"); strcpy(fieldName, "NIRConversionFactor"); }
	else
		{
		fprintf(stderr, "readEllipsoidFile: unsupported band = %d\n", band);
		return 0;
		}

	if (VERBOSE) fprintf(stderr, "readEllipsoidFile: grid=%s, field=%s\n", gridName, fieldName);

	if (VERBOSE) fprintf(stdout, "EHSAN: fname= %s, gridName: %s, fieldName: %s, block: %d \n", fname, gridName, fieldName, block);

	status = MtkReadBlock(fname, gridName, fieldName, block, &conversionFactorBuffer); // conversion factor
	if (status != MTK_SUCCESS) 
		{
		fprintf(stderr, "readEllipsoidFile: MtkReadBlock [3] failed!!!, status = %d (%s)\n", status, errs[status]);
		return 0;
		}
		
	if (VERBOSE) fprintf(stderr, "readEllipsoidFile: nline=%d, nsample=%d, datasize=%d, datatype=%d (%s)\n", 
		conversionFactorBuffer.nline, conversionFactorBuffer.nsample, conversionFactorBuffer.datasize, conversionFactorBuffer.datatype, types[conversionFactorBuffer.datatype]);

	if (conversionFactorBuffer.nline != 8 || conversionFactorBuffer.nsample != 32)
		{
		fprintf(stderr, "readEllipsoidFile: conversionFactorBuffer is not 8x32: (%d, %d)\n", conversionFactorBuffer.nline, conversionFactorBuffer.nsample);
		return 0;
		}

	////

	brfcf1 = (double *) malloc(8 * 32 * sizeof(double)); // biDirectional refl factor conv-factor

	if (!brfcf1) {
		fprintf(stderr, "readEllipsoidFile: brfcf malloc failed!!!\n");
		return 0;
	}

	n = 0;
	for (j = 0; j < 8; j ++)
		for (i = 0; i < 32; i ++) 
		{
			if (conversionFactorBuffer.data.f[j][i] > 0.0) 
			{
				n ++;
				brfcf1[i + j * 32] = conversionFactorBuffer.data.f[j][i]; // brfcf1= biD refl factor conv-factor
			}
			else 
			{
				brfcf1[i + j * 32] = NO_DATA;
			}
		}

	if (n != 256) 
		if (VERBOSE) fprintf(stderr, "readEllipsoidFile: fewer than 256 valid in conversionFactorBuffer: %d\n", n);

	//printf("C: n2 is: %d \n" , (n));

	if (n > 0)
	{
		//brfcf2 = zoomArray(brfcf1, 8, 32, 64);
		brfcf2 = zoomArray(brfcf1, 8, 32, ZOOM); // zoom to what? brfcf2 from brfcf1
		if (!brfcf2) return 0;
	}
	else // n is neg
	{
		noData = 1; // num of pixels
		return 1;
	}

	////----------------------------------------- [12/Oct/2021] Ehsan: turned off because we might not need geo-params& solar Azimuth--------------

	// strcpy(gridName, "GeometricParameters"); 
	// strcpy(fieldName, "SolarAzimuth");

	// if (VERBOSE) fprintf(stderr, "readEllipsoidFile: grid=%s, field=%s\n", gridName, fieldName);

	// status = MtkReadBlock(fname, gridName, fieldName, block, &solarAzimuthBuffer);
	// if (status != MTK_SUCCESS) 
	// 	{
	// 	fprintf(stderr, "readEllipsoidFile: MtkReadBlock [4] failed!!!, status = %d (%s)\n", status, errs[status]);
	// 	return 0;
	// 	}
	// if (VERBOSE) fprintf(stderr, "readEllipsoidFile: nline=%d, nsample=%d, datasize=%d, datatype=%d (%s)\n", 
	// 	solarAzimuthBuffer.nline, solarAzimuthBuffer.nsample, solarAzimuthBuffer.datasize, solarAzimuthBuffer.datatype, types[solarAzimuthBuffer.datatype]);

	// if (solarAzimuthBuffer.nline != 8 || solarAzimuthBuffer.nsample != 32)
	// 	{
	// 	fprintf(stderr, "readEllipsoidFile: %s is not 8x32: (%d, %d)\n", fieldName, solarAzimuthBuffer.nline, solarAzimuthBuffer.nsample);
	// 	return 0;
	// 	}
	// tmp = (double *) malloc(8 * 32 * sizeof(double));
	// if (!tmp)
	// 	{
	// 	fprintf(stderr, "readEllipsoidFile: tmp malloc failed!!!\n");
	// 	return 0;
	// 	}

	// n = 0;
	// for (j = 0; j < 8; j ++)
	// 	for (i = 0; i < 32; i ++)
	// 		{
	// 		if (solarAzimuthBuffer.data.d[j][i] >= 0.0) 
	// 			{
	// 			n ++;
	// 			tmp[i + j * 32] = solarAzimuthBuffer.data.d[j][i];
	// 			}
	// 		else tmp[i + j * 32] = NO_DATA;
	// 		}

	// if (n != 256)
	// 	if (VERBOSE) fprintf(stderr, "readEllipsoidFile: fewer than 256 valid in %s: %d\n", fieldName, n);

	// //printf("C: n3 is: %d \n" , (n));

	// if (n > 0)
	// 	{
	// 	getDataStats(tmp, 8, 32);
	// 	if (min <= 30.0 && max >= 330.0)
	// 		{
	// 		for (i = 0; i < 8 * 32; i ++)
	// 			{
	// 			if (tmp[i] == NO_DATA) continue;
	// 			if (tmp[i] < 180.0) tmp[i] += 360.0;
	// 			}
	// 		wrap = 1;
	// 		}
	// 	// sa = zoomArray(tmp, 8, 32, 64);
	// 	sa = zoomArray(tmp, 8, 32, ZOOM);
	// 	if (!sa) return 0;
	// 	if (wrap) 
	// 		{
	// 		for (i = 0; i < PNG_NLINES * PNG_NSAMPLES; i ++)
	// 			{
	// 			if (sa[i] == NO_DATA) continue;
	// 			if (sa[i] >= 360.0) sa[i] -= 360.0;
	// 			}
	// 		wrap = 0;
	// 		}
	// 	}
	// else // 
	// 	{
	// 	noData = 1;  // --> turns on True here, later, in main() if (1==True==Go) 
	// 	return 1;
	// 	}	

	//-------------------------------------------------------------------------------------------
	//-------------------------------------------- TOARadianceOut --------------------------------

	if (TOARadianceOut)
	{
		printf("\nC: NOTE: TOARadianceOut is on, we output TOA radiance\n");

		nlines = radianceBuffer.nline;
		nsamples = radianceBuffer.nsample;

		data = (double *) malloc(nlines * nsamples * sizeof(double));

		if (!data)
			{
			fprintf(stderr, "readEllipsoidFile: data malloc failed!!!\n");
			return 0;
			}

		for (j = 0; j < nlines; j ++)
			for (i = 0; i < nsamples; i ++)
				{
				if (radianceBuffer.data.u16[j][i] < fillbuf.data.u16[0][0])

					data[i + j * nsamples] = (radianceBuffer.data.u16[j][i] >> 2) * scaleFactorBuffer.data.d[0][0]; // * conversionFactorBuffer.data.f[j / FileLines][i / FileSamples]

				else 
					data[i + j * nsamples] = NO_DATA;
				}
		//----------------------------------------------------------------------------------------
		/* Minnaert Correction */   // Ehsan - needed?

		if (minnaert) // if minnert=1 from python run script, we run this section....
		{
		printf("\nC: NOTE: minnaert correction is on!\n");
		for (j = 0; j < nlines; j ++)
			for (i = 0; i < nsamples; i ++)
				{
				radiance = data[i + j * nsamples];
				if (radiance == NO_DATA) continue;
				// solarAzimuth = sa[i + j * 2048];
				solarAzimuth = sa[i + j * PNG_NSAMPLES];  // defined inside main

				if (solarAzimuth == NO_DATA) 
					{
					data[i + j * nsamples] = NO_DATA;
					continue;
					}
				// solarZenith = sz[i + j * 2048];
				solarZenith = sz[i + j * PNG_NSAMPLES]; // defined inside main
				if (solarZenith == NO_DATA)
					{
					data[i + j * nsamples] = NO_DATA; 
					continue;
					}

				// Ehsan - turnedoff - for calculating terrain
				// if (!getAspectSlope(j, i, &terrainAspect, &terrainSlope)) return 0; 

				// if (terrainAspect == NO_DATA) E: turnedoff
				// 	{
				// 	data[i + j * nsamples] = NO_DATA; // update data
				// 	continue;
				// 	}
				// if (terrainSlope == NO_DATA)
				// 	{
				// 	data[i + j * nsamples] = NO_DATA; // update data
				// 	continue;
				// 	}

				// // Ehsan - how about this section?
				// solarAzimuth -= 180.0;
				// if (solarAzimuth < 0.0) solarAzimuth += 360.0;
				// solarAzimuth *= p180;
				// solarZenith *= p180;
				// terrainAspect *= p180; 
				// terrainSlope *= p180;
				// // Ehsan for correction?
				// cosi = cos(solarZenith) * cos(terrainSlope) + sin(solarZenith) * sin(terrainSlope) * cos(terrainAspect - solarAzimuth);
				// if (cosi <= 0.0) // Ehsan: cosi is used for correction, turnedoff
				// 	{
				// 	fprintf(stderr, "readEllipsoidFile: cosi = %14.6f, ta = %14.6f, sa = %14.6f, ta - sa = %14.6f\n", 
				// 		cosi, terrainAspect / p180, solarAzimuth / p180, (terrainAspect - solarAzimuth)  / p180);
				// 	// we don't need to correct for terrain dropouts
				// 	data[i + j * nsamples] = TDROPOUT; E: turnedoff
				// 	continue;
				// 	}

				// // we don't need to correct for slope and aspect. AN
					// data[i + j * nsamples] = (radiance * cos(terrainSlope)) / (pow(cosi, 0.17) * pow(cos(terrainSlope), 0.17)); E: turnedoff
				}
		}

		//----------------------------------------------------------------------------------------

		for (j = 0; j < nlines; j ++) // check for no-data
			for (i = 0; i < nsamples; i ++)
				{ // QA check?
				if (data[i + j * nsamples] == NO_DATA) continue; // what is this slice?
				// if (data[i + j * nsamples] == TDROPOUT) continue; Ehsan - turnedoff
				//if (brfcf2[i + j * 2048] == NO_DATA) 
				if (brfcf2[i + j * PNG_NSAMPLES] == NO_DATA) // Q- what does it do?
					{
					data[i + j * nsamples] = NO_DATA;
					continue;
					}
				
				//data[i + j * nsamples] *= brfcf2[i + j * 2048];
				//we don't need png files

				//data[i + j * nsamples] *= brfcf2[i + j * PNG_NSAMPLES]; //output is data???
				}

		if (VERBOSE) fprintf(stderr, "readEllipsoidFile: data is %d x %d\n", nlines, nsamples);

		return 1; // returns data array
	}

	//-------------------------------------------------------------------------------------------
	//-------------------------------------------- doTOA_BRFConversion --------------------------------
	// note: converts TOA-rad to TOA-refl and outputs TOA reflectance, at TOA == sensor level

	if (doTOA_BRFConversion)
	{
		printf("\nC: NOTE: doTOA_BRFConversion is on, we calculate TOA BRF conversion\n");

		FileLines = radianceBuffer.nline / conversionFactorBuffer.nline; // radianceBuffer and conversionFactorBuffer come from MtkReadBlock==read data in mem-
		FileSamples = radianceBuffer.nsample / conversionFactorBuffer.nsample;	// so data is in mem-
			
		if (data != 0) free(data);

		nlines = radianceBuffer.nline; // from Radiance
		nsamples = radianceBuffer.nsample;

		data = (double *) malloc(nlines * nsamples * sizeof(double));

		if (!data) // check mem-allocation is right
		{
			printf("doTOA_BRFConversion: malloc of double array failed!!!\n");
			nlines = 0;
			nsamples = 0;
			nvalid = 0;
			return 0;
		}

		nvalid = 0; // *************   here: conversion from toa-rad to toa-reflectance
		if (doTOA_BRFConversion)
		{
		    for (j = 0; j < nlines; j++)
		        for (i = 0; i < nsamples; i++)
		            {
		            if (radianceBuffer.data.u16[j][i] < fillbuf.data.u16[0][0] && // check this condition definitely to see what it filters
		                conversionFactorBuffer.data.f[j/FileLines][i/FileSamples] > 0.0)  // Q- why we need this condition?
		                {
		                data[i + j * nsamples] = (radianceBuffer.data.u16[j][i] >> 2) * scaleFactorBuffer.data.d[0][0] * conversionFactorBuffer.data.f[j / FileLines][i / FileSamples]; // update data with these 3; is it toa_refl? or something else?
		                nvalid++;
		                }
		            else // what condition????
		                {
		                data[i + j * nsamples] = -1.0;
		                }
		            //printf("BRF data pixel: %lf\n" , data[i + j * nsamples]);
		            }
	    }
		else
	    {
		    for (j = 0; j < nlines; j++)
		        for (i = 0; i < nsamples; i++)
		            {
		            if (radianceBuffer.data.u16[j][i] < fillbuf.data.u16[0][0]) // Q- why we need this condition?
		                {
		                data[i + j * nsamples] = (radianceBuffer.data.u16[j][i] >> 2) * scaleFactorBuffer.data.d[0][0];
		                nvalid++;
		                }
		            else
		                {
		                data[i + j * nsamples] = -1.0;
		                }
		                //printf("notBRF data pixel: %lf\n" , data[i + j * nsamples]);
		            }
	    }

		//if (!resizeData(index)) return 0;

		MtkDataBufferFree(&radianceBuffer);
		MtkDataBufferFree(&scaleFactorBuffer);
		MtkDataBufferFree(&fillbuf);
		MtkDataBufferFree(&conversionFactorBuffer);
		return 1;
	}

}
///////////////////////////////////////////////// readEllipsoidFile - end ////////////////////////////////////////

///////////////////////////////////////////////// getDataStats ////////////////////////////////////////

int getDataStats(double *data, int nlines, int nsamples)
{
	int i;
	double z;

	min = 1.0e23;
	max = -1.0e23;
	mean = 0.0;
	stddev = 0.0;
	nvalid = 0;
	// ndropouts = 0; E: turnedoff

	for (i = 0; i < nlines * nsamples; i ++)
		{
		z = data[i]; //E- DN?
		if (z == NO_DATA) continue;
		if (z == BACKGROUND) continue;
		if (z == FOREGROUND) continue;
		// if (z == TDROPOUT) Ehsan - turnedoff
		// 	{
		// 	ndropouts ++;
		// 	continue;
		// 	}
		nvalid ++;
		if (z < min) min = z;
		if (z > max) max = z;
		mean += z; //E- sum
		}
		
	if (nvalid > 0) 
		{
		mean /= nvalid; //E- mean= sum/total numbers; what does mean represent?
		if (nvalid > 1)
			{
			for (i = 0; i < nlines * nsamples; i ++)
				{
				z = data[i];
				if (z == NO_DATA) continue;
				if (z == BACKGROUND) continue;
				if (z == FOREGROUND) continue;
				//if (z == TDROPOUT) continue; Ehsan - turnedoff
				stddev += (z - mean) * (z - mean);
				}
			stddev = sqrt(stddev / (nvalid - 1.0));
			}
		}

	return 1;
}

///////////////////////////////////////////////// getDataStats ////////////////////////////////////////

////////////////////////////////////////////////// sinc ///////////////////////////////////////////////

double sinc(double x)
{
if (x == 0.0) return 1.0;
x *= M_PI;
return sin(x) / x;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

double *convolve2d(double* data, double* filter, int nlines, int nsamples)
{
double *result;
int i, j, i0, j0, n, n2;
double *rbuf1, *rbuf2;
fftw_complex *cbuf1, *cbuf2, *cbuf3;
fftw_plan p;

n = nsamples * nlines;
n2 = (nsamples / 2 + 1) * nlines;
rbuf1 = (double *) fftw_malloc(n * sizeof(double));
if (!rbuf1)
	{
	fprintf(stderr, "convolve2d: couldn't malloc rbuf1\n");
	return 0;
	}
rbuf2 = (double *) fftw_malloc(n * sizeof(double));
if (!rbuf2)
	{
	fprintf(stderr, "convolve2d: couldn't malloc rbuf2\n");
	return 0;
	}
cbuf1 = (fftw_complex *) fftw_malloc(n2 * sizeof(fftw_complex));
if (!cbuf1)
	{
	fprintf(stderr, "convolve2d: couldn't malloc cbuf1\n");
	return 0;
	}
cbuf2 = (fftw_complex *) fftw_malloc(n2 * sizeof(fftw_complex));
if (!cbuf2)
	{
	fprintf(stderr, "convolve2d: couldn't malloc cbuf2\n");
	return 0;
	}
cbuf3 = (fftw_complex *) fftw_malloc(n2 * sizeof(fftw_complex));
if (!cbuf3)
	{
	fprintf(stderr, "convolve2d: couldn't malloc cbuf3\n");
	return 0;
	}

for (i = 0; i < n; i ++) rbuf1[i] = data[i];
for (i = 0; i < n; i ++) rbuf2[i] = filter[i];

if (VERBOSE > 1) fprintf(stderr, "fftw_plan_dft_r2c_2d\n");	
p = fftw_plan_dft_r2c_2d(nlines, nsamples, rbuf1, cbuf1, FFTW_ESTIMATE);
if (VERBOSE > 1) fprintf(stderr, "fftw_execute\n");	
fftw_execute(p);
fftw_destroy_plan(p);

if (VERBOSE > 1) fprintf(stderr, "fftw_plan_dft_r2c_2d\n");	
p = fftw_plan_dft_r2c_2d(nlines, nsamples, rbuf2, cbuf2, FFTW_ESTIMATE);
if (VERBOSE > 1) fprintf(stderr, "fftw_execute\n");	
fftw_execute(p);
if (VERBOSE > 1) fprintf(stderr, "fftw_destroy_plan\n");	
fftw_destroy_plan(p);

for (i = 0; i < n2; i ++)
	{
	cbuf3[i][0] = cbuf1[i][0] * cbuf2[i][0] - cbuf1[i][1] * cbuf2[i][1];
	cbuf3[i][1] = cbuf1[i][0] * cbuf2[i][1] + cbuf1[i][1] * cbuf2[i][0];
	}

if (VERBOSE > 1) fprintf(stderr, "fftw_plan_dft_c2r_2d\n");	
p = fftw_plan_dft_c2r_2d(nlines, nsamples, cbuf3, rbuf1, FFTW_ESTIMATE);
if (VERBOSE > 1) fprintf(stderr, "fftw_execute\n");	
fftw_execute(p);
if (VERBOSE > 1) fprintf(stderr, "fftw_destroy_plan\n");	
fftw_destroy_plan(p);

result = (double *) malloc(nsamples * nlines * sizeof(double));
if (!result)
	{
	fprintf(stderr, "convolve2d: couldn't malloc result\n");
	return 0;
	}
	
j0 = nlines / 2;
i0 = nsamples / 2;
for (j = 0; j < nlines; j ++) 
	for (i = 0; i < nsamples; i ++) 
		result[j * nsamples + i] = 
			rbuf1[((j0 + j) % nlines) * nsamples + ((i0 + i) % nsamples)] / n;

fftw_free(rbuf1);
fftw_free(rbuf2);
fftw_free(cbuf1);
fftw_free(cbuf2);
fftw_free(cbuf3);

return result;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

double *zoom2d(double *data, int nlines, int nsamples, int zoom)
{
double dx, dy, *data2, *filter, *result, dd2 = DAMPING * DAMPING;
int i, j, i0, j0, nlines2 = nlines * zoom, nsamples2 = nsamples * zoom;

if (VERBOSE) fprintf(stderr, "zoom2d: nlines=%d, nsamples=%d, zoom=%d\n", nlines, nsamples, zoom);

data2 = (double *) malloc(nlines2 * nsamples2 * sizeof(double));
if (!data2)
	{
	fprintf(stderr, "zoom2d: couldn't malloc data2\n");
	return 0;
	}
	
for (i = 0; i < nsamples2 * nlines2; i ++) data2[i] = 0.0;
i0 = (nsamples2 - nsamples) / 2;
j0 = (nlines2 - nlines) / 2;
for (j = 0; j < nlines; j ++)
	for (i = 0; i < nsamples; i ++)
		data2[i * zoom + zoom/2 + (j * zoom + zoom/2) * nsamples2] = data[i + j * nsamples];
		
filter = (double *) malloc(nsamples2 * nlines2 * sizeof(double));
if (!filter)
	{
	fprintf(stderr, "zoom2d: couldn't malloc filter\n");
	return 0;
	}
	
for (i = 0; i < nsamples2 * nlines2; i ++) filter[i] = 0.0;
for (j = 0; j < nlines2; j ++)
	for (i = 0; i < nsamples2; i ++)
		{
		dx = fabs(1.0 / (2.0 * zoom) + (1.0 * i) / zoom - nsamples / 2);
		dy = fabs(1.0 / (2.0 * zoom) + (1.0 * j) / zoom - nlines / 2);
		if (dx <= SINC_DIST && dy <= SINC_DIST) 
			filter[i + j * nsamples2] = sinc(dx) * sinc(dy) 
				* exp(-(dx * dx) / dd2) * exp(-(dy * dy) / dd2);
		}			
		
result = convolve2d(data2, filter, nlines2, nsamples2);

if (data2) free(data2);
if (filter) free(filter);
return result;
}

////////////////////////////////////////// extendArray ////////////////////////////////////////// ???

double *extendArray(double *data, int nlines, int nsamples, int border)
{
double *data2;
int k, j, i, nlines2 = nlines + 2 * border, nsamples2 = nsamples + 2 * border;

if (VERBOSE) fprintf(stderr, "extendArray: nlines=%d, nsamples=%d, border=%d\n", nlines, nsamples, border);

data2 = (double *) malloc(nlines2 * nsamples2 * sizeof(double));
if (!data2)
	{
	fprintf(stderr, "extendArray: couldn't malloc data2\n");
	return 0;
	}
	
for (i = 0; i < nlines2 * nsamples2; i ++) data2[i] = NO_DATA;
	
for (j = 0; j < nlines; j ++)
	for (i = 0; i < nsamples; i ++)
		data2[i + border + (j + border) * nsamples2] = data[i + j * nsamples];

for (j = 0; j < nlines2; j ++)
	{
	for (i = 0; i < nsamples2; i ++)
		if (data2[i + j * nsamples2] != NO_DATA)
			{
			for (k = 0; k < i; k ++)
				data2[k + j * nsamples2] = data2[i + j * nsamples2];
			break;
			}
	for (i = nsamples2 - 1; i > -1; i --)
		if (data2[i + j * nsamples2] != NO_DATA)
			{
			for (k = nsamples2 - 1; k > i; k --)
				data2[k + j * nsamples2] = data2[i + j * nsamples2];
			break;
			}
	}
	
for (i = 0; i < nsamples2; i ++)
	{
	for (j = 0; j < nlines2; j ++)
		if (data2[i + j * nsamples2] != NO_DATA)
			{
			for (k = 0; k < j; k ++)
				data2[i + k * nsamples2] = data2[i + j * nsamples2];
			break;
			}
	for (j = nlines2 - 1; j > -1; j --)
		if (data2[i + j * nsamples2] != NO_DATA)
			{
			for (k = nlines2 - 1; k > j; k --)
				data2[i + k * nsamples2] = data2[i + j * nsamples2];
			break;
			}
	}
	
for (i = 0; i < nlines2 * nsamples2; i ++) 
	if (data2[i] == NO_DATA)
		{
		fprintf(stderr, "extendArray: holes in data array, updated with NO_DATA val\n");
		//return 0;
		// data2[i] = TDROPOUT; Ehsan turnedoff 
    data2[i] = NO_DATA; // replaced with previous line
		}

return data2;
}
////////////////////////////////////////// extendArray ////////////////////////////////////////// ???

////////////////////////////////////////// extractArray ////////////////////////////////////////// ???

double *extractArray(double *data, int nlines, int nsamples, int border)
{
double *data2;
int j, i, nlines2 = nlines - 2 * border, nsamples2 = nsamples - 2 * border;

if (VERBOSE) fprintf(stderr, "extractArray: nlines=%d, nsamples=%d, border=%d\n", nlines, nsamples, border);

data2 = (double *) malloc(nlines2 * nsamples2 * sizeof(double));
if (!data2)
	{
	fprintf(stderr, "extractArray: couldn't malloc data2\n");
	return 0;
	}
	
for (j = 0; j < nlines2; j ++)
	for (i = 0; i < nsamples2; i ++)
		data2[i + j * nsamples2] = data[i + border + (j + border) * nsamples];

return data2;
}
////////////////////////////////////////// extractArray ////////////////////////////////////////// ???

////////////////////////////////////////// zoomArray ////////////////////////////////////////// ???

double *zoomArray(double* data, int nlines, int nsamples, int zoom){ // what for???

	double *data2 = 0, *data3 = 0, *data4 = 0; // local var
	int nlines2, nsamples2, nlines3, nsamples3, nlines4, nsamples4;
	int i, j;
	int *mask;

	mask = (int *) malloc(nlines * nsamples * sizeof(int));
	if (!mask)
		{
		fprintf(stderr, "zoomArray: couldn't malloc mask\n");
		return 0;
		}
	for (j = 0; j < nlines; j ++) 
		for (i = 0; i < nsamples; i ++) 
			if (data[i + j * nsamples] != NO_DATA) mask[i + j * nsamples] = 1;
			else mask[i + j * nsamples] = 0;

	data2 = extendArray(data, nlines, nsamples, BORDER);
	if (!data2) return 0;
	nlines2 = nlines + 2 * BORDER;
	nsamples2 = nsamples + 2 * BORDER;
	//if (!write_png("extended.png", data2image(data2, nlines2, nsamples2, 1), nlines2, nsamples2)) return 0;
	data3 = zoom2d(data2, nlines2, nsamples2, zoom);
	if (!data3) return 0;
	nlines3 = nlines2 * zoom;
	nsamples3 = nsamples2 * zoom;
	data4 = extractArray(data3, nlines3, nsamples3, zoom * BORDER);
	if (!data4) return 0;
	nlines4 = nlines3 - 2 * zoom * BORDER;
	nsamples4 = nsamples3 - 2 * zoom * BORDER;
	if (nlines4 != nlines * zoom)
		{
		fprintf(stderr, "zoomArray: zoomed nlines mismatch: %d, %d\n", nlines4, nlines * zoom);
		return 0;
		}
	if (nsamples4 != nsamples * zoom)
		{
		fprintf(stderr, "zoomArray: zoomed nsamples mismatch: %d, %d\n", nsamples4, nsamples * zoom);
		return 0;
		}

	for (j = 0; j < nlines4; j ++) 
		for (i = 0; i < nsamples4; i ++) 
			if (mask[i / zoom + (j / zoom) * nsamples] == 0) data4[i + j * nsamples4] = NO_DATA;

	if (data2) free(data2);
	if (data3) free(data3);
	if (mask) free(mask);
	return data4;
}
////////////////////////////////////////// zoomArray ////////////////////////////////////////// ???

////////////////////////////////////////// read_data ////////////////////////////////////////// ???

int read_data(char *fname, double **data, int nlines, int nsamples)
{
FILE *f;

f = fopen(fname, "rb");
if (!f) // if (not T)
	{
	fprintf(stderr, "read_data: couldn't open %s\n", fname);
	return 0;
	}
	
*data = (double *) malloc(nlines * nsamples * sizeof(double));
if (!*data)
	{
	fprintf(stderr, "read_data: couldn't malloc data\n");
	return 0;
	}
	
if (fread(*data, sizeof(double), nlines * nsamples, f) != nlines * nsamples)
	{
	fprintf(stderr, "read_data: couldn't read data\n");
	return 0;
	}
	
fclose(f);
return 1;
}
////////////////////////////////////////// read_data ////////////////////////////////////////// ???

////////////////////////////////////////// write_data ////////////////////////////////////////// ???

int write_data(char *fname, double *data, int nlines, int nsamples)
{
FILE *f;

f = fopen(fname, "wb"); // f=stdout?
if (!f)
	{
	fprintf(stderr, "write_data: couldn't open %s\n", fname);
	return 0;
	}
	
if (fwrite(data, sizeof(double), nlines * nsamples, f) != nlines * nsamples)
	{
	fprintf(stderr, "write_data: couldn't write data\n");
	return 0;
	}
	
fclose(f);
return 1;
}
////////////////////////////////////////// write_data ////////////////////////////////////////// ???

////////////////////////////////////////// fix_dropouts ////////////////////////////////////////// ???

double *fix_dropouts(double *data, int nlines, int nsamples)
{
double *data2 = 0;
int i, j, i2, j2, i3, j3, n;
int w2 = 2;

data2 = (double *) malloc(nlines * nsamples * sizeof(double));
if (!data2)
	{
	fprintf(stderr, "fix_dropouts: couldn't malloc data2\n");
	return 0;
	}
	
for (i = 0; i < nlines * nsamples; i ++) data2[i] = data[i];

for (j = 0; j < nlines; j ++)
	for (i = 0; i < nsamples; i ++)
		{
		if (data[i + j * nsamples] == NO_DATA) continue;
		// if (data[i + j * nsamples] == TDROPOUT) continue; Ehsan turnedoff
		n = 0;
		for (j2 = -w2; j2 <= w2; j2 ++)
			for (i2 = -w2; i2 <= w2; i2 ++)
				{
				i3 = i + i2;
				j3 = j + j2;
				// if (j3 >= 0 && j3 < nlines && i3 >= 0 && i3 < nsamples && data[i3 + j3 * nsamples] == TDROPOUT) // Ehsan turnedoff
        if (j3 >= 0 && j3 < nlines && i3 >= 0 && i3 < nsamples && data[i3 + j3 * nsamples] == NO_DATA) // E: updated to NO_DATA

					n ++;
				}
		// if (n > 0) data2[i + j * nsamples] = TDROPOUT; Ehsan turnedoff
    if (n > 0) data2[i + j * nsamples] = NO_DATA; // E: updated with NO_DATA
		}

return data2;
}


//############################################### main ######################################################

int main(int argc, char* argv[]){ // return 0= success, return 1= error

	// printf("date/time is ...\n");
	printf("\n");
	system(command);


	int i, j, i2, j2;
	char s[256];
	//char msg[25]='TOA program processing';

	/*ll2grid(67.700236, -82.694239, &j, &i);
	ll2grid(87.002838, -19.900249, &j, &i);
	ll2grid(58.193210, -44.425534, &j, &i);
	ll2grid(67.406094, -2.604521, &j, &i);
	exit(1);*/
	//	printf("C: inside C\n");
	if (argc == 6)
	{
		printf("C: received 6 arguments. \n");
	}

	if (argc < 5) // this might happen later, cos I turnedoff fname[2]
	{
		fprintf(stderr, "Usage: TOARad2BlocksAllBlocks_exe, GRP_ELLIPSOID.hdf, block, band, minnaert, toa_data_file \n"); // updated
		// fprintf(stderr, "Usage: TOA3 input-misr-file block band minnaert output-data-file output-image-file-Ehsan--noNeed\n"); old with image

		return 1;
	}


	// Ehsan - we get the cmd line args from python script and chage them to input variables
	// note: the value of argv[0] is the name of the program to execute. C kinda reserves the argv[0] index for this purpose
	strcpy(fname[0], argv[1]); // str- copy hdf file label to fname[0]
	block = atoi(argv[2]);	// int
	band = atoi(argv[3]);		// int; band=3 for red
	minnaert = atoi(argv[4]);		// int, is it 1st or 4th arg from oython?! if it is 4th from python, correct it here! note: Jun6 corrected it here to 4th arg
	strcpy(fname[1], argv[5]);		// str- fname[1]= toa data file
	// strcpy(fname[2], argv[6]);		// str- image file; Ehsan - turnedoff 
	// printf("C: MISR_GRP_ELLIPSOID_file: %s \n" , fname[0]);
	//printf("C: block: %d \n" , (block));
	// printf("C: band: %d \n" , (band));
	// printf("C: minnert: %d \n" , (minnaert));
	// printf("C: toa_data_file: %s \n" , (fname[1]));



	// Ehsan - delete?
	// if (!read_data("slope.dat", &slope, demlines, demsamples)) return 1;
	// if (!read_data("aspectN.dat", &aspect, demlines, demsamples)) return 1;


	if (strstr(fname[0], "_P")) // looks inside MISR hdf filename for _P* Note= path with capital P
	{
		//printf("fining path from %s\n", fname[0]);
		strncpy(s, strstr(fname[0], "_GM_P") + 5, 3);  // input to s, hdf file name for ELLIPOSIND data that starts from GM_, move pointer+5 and get the next 3 characters from the whole str
		s[3] = 0; // why?
		//printf("path string: %s\n", s);
		path = atoi(s); // str to int
		// printf("path: %d \n", path);
	}
	else
	{
		fprintf(stderr, "C: No path info in file name\n");
		return 1;
	}

	if (strstr(fname[0], "_O")) // orbit with capital O inside fname[0]
	{
		strncpy(s, strstr(fname[0], "_O") + 2, 6); // returns str starting from _O + 2
		s[6] = 0; // why?
		orbit = atoi(s); // str to int
		// printf("orbit: %d \n" , orbit);
	}
	else
	{
		fprintf(stderr, "C: No orbit info in file name\n");
		return 1;
	}
		// check supported cameras; how about camera Df and Da? order of cameras? F->A? or A->F?
	if      (strstr(fname[0], "_CF_")) camera = 1; // check if there if CF
	else if (strstr(fname[0], "_BF_")) camera = 2;
	else if (strstr(fname[0], "_AF_")) camera = 3;
	else if (strstr(fname[0], "_AN_")) camera = 4; // change to 5 later
	else if (strstr(fname[0], "_AA_")) camera = 5;
	else if (strstr(fname[0], "_BA_")) camera = 6;
	else if (strstr(fname[0], "_CA_")) camera = 7;
	else
	{
		fprintf(stderr, "Unsupported camera\n");
		return 1; // if camera not found= error= return 1
	}

	// for band=2; is set based on band number
	PNG_NLINES = 512; // no of pixels in column; for error checking
	PNG_NSAMPLES = 2048; // no of pixels in each row of image
	ZOOM = 64;
	// for band !=3
	if ((band != 2) && (camera != 4)) // if not nadir==vertical camera... E: should we check camera too? or just band is enough? // change camera to 5 later
	{
	    PNG_NLINES = 128; // check these two
	    PNG_NSAMPLES = 512;
	    ZOOM = 16;
	}

	//printf("C: nvalid0: %d \n" , (nvalid));

	if (!readEllipsoidFile(fname[0]))
	{
		printf("C: exiting from readEllipsoidFile. \n");
		return 1; // read hdf file and return <data> array
	}

	// printf("C: nvalid1: %d \n" , (nvalid));
	// printf("C: noData: %d \n" , (noData));

	if (noData)  // if noData == 1 ==> True ==> Go inside
	{
		printf("C: exiting from noData. \n");
		return 0;
	} // why noData= return 0? what does it mean?

	//printf("C: nvalid2: %d \n" , (nvalid));

	data = fix_dropouts(data, nlines, nsamples); // Q- should be turnedoff?

	//printf("C: nvalid3: %d \n" , (nvalid));

	if (!data) 
	{
		printf("C: exiting from !data \n");
		return 1;
	}

	//printf("C: nvalid4: %d \n" , (nvalid));

	if (!getDataStats(data, nlines, nsamples)) 
	{
		printf("C: exiting from getDataStats. \n");
		return 1;
	}

	//printf("C: nvalid-final: %d \n" , (nvalid));

	if (nvalid > 0)
	{
		// printf("%03d  %06d  %03d  %s  %5d  %5d  %10d  %14.6f  %14.6f  %14.6f  %14.6f  %14.6f  %10d\n", 
		// 	path, orbit, block, camera == 1 ? "cf" : camera == 4 ? "an" : camera == 7 ? "ca" : camera == 2 ? "bf" : camera == 3 ? "af" : camera == 5 ? "aa" : camera == 6 ? "ba" : "??", 
		// 	nlines, nsamples, nvalid, min, max, mean, stddev, meanSZ, ndropouts); // formatted output to stdout
		printf("\n");
		printf("%s %03d  %06d  %03d  %s  %5d  %5d  %10d  %14.6f  %14.6f  %14.6f  %14.6f  %14.6f \n", 
		"C:", path, orbit, block, camera == 1 ? "cf" : camera == 4 ? "an" : camera == 7 ? "ca" : camera == 2 ? "bf" : camera == 3 ? "af" : camera == 5 ? "aa" : camera == 6 ? "ba" : "??",
			nlines, nsamples, nvalid, min, max, mean, stddev, meanSZ ); // Ehsan - removed ndropouts, %10d from output

		fflush(stdout); // flush to clean it

		if (!write_data(fname[1], data, nlines, nsamples)) return 1; // TOA writes hdf data into file fname[1] 
		// if (!write_png(fname[2], data2image(data, nlines, nsamples, 1), nlines, nsamples)) return 1; E: turnedoff this function & data2image, fname[2]- anything that goes to this function
	}

	printf("CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC\n");

	return 0;

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
