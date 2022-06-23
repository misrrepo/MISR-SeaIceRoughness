// MaskS.c
// Read surface data files, mask and save
// Gene Mar 25 Apr 18
// Ehsan Nov 12 2019
// notes:
// selects toa_refl OR surf_refl files based on the available files inside the input directory
// to-do:
/* should read from 3 cam directories */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <png.h>
#include <dirent.h>
#include <unistd.h>  // for access()


#define NO_DATA -999999.0
#define BACKGROUND -999998.0
#define FOREGROUND -999997.0
#define TDROPOUT -999996.0
#define CMASKED -999995.0
#define LMASKED -999994.0
#define VERBOSE 0

char misr_file_fullpath[4][256];
unsigned char *mask = 0;
char **misr_file_list = 0; // array of char ptrs
int misr_total_files_found;
// int max_nfiles = 0; // Ehsan- turned off to read all files in different directories with different number of files, such as AN with 4 bands
int nlines = 512;
int nsamples = 2048;
double* data = 0; // %lf
 
png_structp png_ptr = 0;
png_infop info_ptr = 0;

char *data2image(double *data, int ny, int nx);
int write_png(char *misr_file_fullpath, char *image, int ny, int nx);
int write_data(char *misr_file_fullpath, double *data, int nlines, int nsamples);
int read_data(char *misr_file_fullpath, double **data, int nlines, int nsamples);
int read_byte_data(char *misr_file_fullpath, unsigned char **data, int nlines, int nsamples);
int maskData(void);
int getFileList(char *dir);
char *strsub(char *s, char *a, char *b);

int test_data_before_write_byEhsan(void);

//#####################################################################################################

int maskData(void) {

	int i, j, i2, j2;
	double z;

	for (j = 0; j < nlines; j ++)
		for (i = 0; i < nsamples; i ++) {
			//z = data[i + j * nsamples];
			//if (z == NO_DATA) continue;
			//if (z == TDROPOUT) continue;
			
			if (mask[i + j * nsamples] == 0) continue;
				// printf("data: %lf \n" , data[i + j * nsamples]);

				data[i + j * nsamples] = LMASKED;
			}

	return 1;
}

//#####################################################################################################
int test_data_before_write_byEhsan(void) {
	int i, j;

	for (j = 0; j < nlines; j ++)
		for (i = 0; i < nsamples; i ++) {
			
			printf("pixel data before write is: %lf \n" , data[i + j * nsamples]);

		}

	return 1;
}

//#####################################################################################################

char *data2image(double *data, int nlines, int nsamples)
{
char *image;
double min, max, z, dz;
int i;

min = 1.0e23;
max = -1.0e23;
for (i = 0; i < nlines * nsamples; i ++)
	{
	z = data[i];
	if (z == NO_DATA) continue;
	if (z == TDROPOUT) continue;
	if (z == CMASKED) continue;
	if (z == LMASKED) continue;
	if (z < min) min = z;
	else if (z > max) max = z;
	}

if (VERBOSE) fprintf(stderr, "data2image: Gmin=%.3f, max=%.3f\n", min, max);
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
	else if (z == CMASKED) image[i] = 0;
	else if (z == LMASKED) image[i] = 0;
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

//#####################################################################################################

// int write_png(char *misr_file_fullpath, char *image, int ny, int nx)
// {
// FILE *fp;
// png_bytepp row_ptrs;
// int j;

// row_ptrs = (png_bytepp) malloc(ny * sizeof(png_bytep));
// if (!row_ptrs)
//  {
//  fprintf(stderr, "write_png: couldn't malloc row_ptrs\n");
//  return 0;
//  }
// for (j = 0; j < ny; j ++) row_ptrs[j] = (png_bytep)(image + j * nx);

// fp = fopen(misr_file_fullpath, "wb");
// if (!fp)
//  {
//  fprintf(stderr, "write_png: couldn't open %s\n", misr_file_fullpath);
//  return 0;
//  }

// png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 
//  png_voidp_NULL, png_error_ptr_NULL, png_error_ptr_NULL);
// if (!png_ptr)
//  {
//  fclose(fp);
//  fprintf(stderr, "write_png: png_create_write_struct failed\n");
//  return 0;
//  }

// info_ptr = png_create_info_struct(png_ptr);
// if (!info_ptr)
//  {
//  png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
//  fclose(fp);
//  fprintf(stderr, "write_png: png_create_info_struct failed\n");
//  return 0;
//  }

// if (setjmp(png_jmpbuf(png_ptr)))
//  {
//  png_destroy_write_struct(&png_ptr, &info_ptr);
//  fclose(fp);
//  fprintf(stderr, "write_png: longjmp from png error\n");
//  return 0;
//  }

// png_init_io(png_ptr, fp);

// png_set_IHDR(png_ptr, info_ptr, nx, ny, 8, PNG_COLOR_TYPE_GRAY, 
//  PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	
// png_set_rows(png_ptr, info_ptr, row_ptrs);

// png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
	   
// png_destroy_write_struct(&png_ptr, &info_ptr);

// fclose(fp);
// if (row_ptrs) free(row_ptrs);
// return 1;
// }

//#####################################################################################################

int write_data(char* misr_file_fullpath, double* data, int nlines, int nsamples) {
	FILE *f;

	f = fopen(misr_file_fullpath, "wb"); // write data to this file
	if (!f) {
		fprintf(stderr, "c: ERROR1: write_data: couldn't open file; %s\n", misr_file_fullpath);
		return 0;
		}
		
	if (fwrite(data, sizeof(double), (nlines * nsamples), f) != (nlines * nsamples)) {
		fprintf(stderr, "c: ERROR2: write_data: total elements returned error, couldn't write data \n");
		return 0;
		}

	// printf("pixel value/data= %lf" , *data);
		
	fclose(f);
	return 1;
}

//#####################################################################################################

int read_data(char* misr_file_fullpath, double** data, int nlines, int nsamples) {

	FILE *filePtr;

	filePtr = fopen(misr_file_fullpath, "rb");

	if (!filePtr) {
		fprintf(stderr, "c: read_data1: couldn't open %s\n", misr_file_fullpath);
		return 0;
	}
		
	*data = (double *) malloc(nlines * nsamples * sizeof(double));

	if (!*data) {
		fprintf(stderr, "c: read_data2: couldn't malloc data\n");
		return 0;
	}
		
	if (fread(*data, sizeof(double), nlines * nsamples, filePtr) != nlines * nsamples) { // reads all pixels inside surf_file into <data>
		fprintf(stderr, "c: read_data3: couldn't read TOA data\n");
		return 0;
	}
		
	fclose(filePtr);
	return 1;
}

//#####################################################################################################

int read_byte_data(char *misr_file_fullpath, unsigned char **data, int nlines, int nsamples) {

	FILE *f;

	f = fopen(misr_file_fullpath, "rb");
	if (!f)
	{
		fprintf(stderr, "c: read_byte_data1: couldn't open %s \n", misr_file_fullpath);
		return 0;
	}
		
	*data = (unsigned char *) malloc(nlines * nsamples * sizeof(unsigned char));
	if (!*data)
	{
		fprintf(stderr, "c: read_byte_data2: couldn't malloc data \n");
		return 0;
	}
		
	if (fread(*data, sizeof(unsigned char), nlines * nsamples, f) != nlines * nsamples)
	{
		fprintf(stderr, "c: read_byte_data3: couldn't read data2 \n");
		return 0;
	}
		
	fclose(f);

	return 1;
}

//#####################################################################################################

int getFileList(char* dir)
{ // returns misr_file_list
	// DIR* dirPtr;
	struct dirent* entryPtr; //char* d_name

	DIR* dirPtr = opendir(dir); // returns pointer to directory; creates stream; dtype=DIR
	if (!dirPtr) 
	{ // if reverse is correct?
		printf("c: getFileList: issue with dirPtr; couldn't open %s\n", dir);
		return 0;
	}
		
	while ((entryPtr = readdir(dirPtr)) != NULL)  // readdir returns a ptr to next entry in stream
	{

		//printf("entryPtr->d_name %s \n" , entryPtr->d_name);
		if (strstr(entryPtr->d_name, ".hdr")) continue; //if returns a pointer, continue
		if (!strstr(entryPtr->d_name, ".dat")) continue; //if not return pointer, continue
		//if (!strstr(entryPtr->d_name, "sdcm_")) continue;
		//if (!strstr(entryPtr->d_name, "rms_")) continue;
		if (misr_file_list == 0) 
		{
			//printf("misr_total_files_found is zero, we pass \n");
			misr_file_list = (char **) malloc(sizeof(char *)); // E-why this? initiate?

			if (!misr_file_list) 
			{
				printf("c: getFileList: couldn't malloc misr_file_list \n");
				return 0;
			}
		}
		else 
		{
			// if (misr_total_files_found > max_nfiles) 
			
				misr_file_list = (char **) realloc(misr_file_list, (misr_total_files_found + 1) * sizeof(char *));
				if (!misr_file_list) 
				{
					printf("c: getFileList: couldn't realloc misr_file_list\n");
					return 0;
				}
		}

		misr_file_list[misr_total_files_found] = (char *) malloc(strlen(entryPtr->d_name) + 1); // alloc mem- for a single char ptr

		if (!misr_file_list[misr_total_files_found]) 
		{
			printf("c: getFileList: couldn't malloc misr_file_list[misr_total_files_found]\n");
			return 0;
		}

		strcpy(misr_file_list[misr_total_files_found], entryPtr->d_name); // fill file list=misr_file_list with files in dir == An/Cf/Ca at a time

		//printf("misr_file_list= %s\n", misr_file_list[misr_total_files_found]); E
		misr_total_files_found ++;
	}
			
		closedir(dirPtr);
		return 1;
}

//#####################################################################################################

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


// ################################  main()  #############################

int main(int argc, char* argv[]) 
{
	// input dir
	char input_toa_home[256] = ""; //  empty string
	char lsmask_files_fullpath[256] = ""; // empty string
	
	// output dir 
	char output_dir_fullpath[256] = ""; 
	
	// copy from python cmdline to empty string
	strcpy(input_toa_home, argv[1]);
	strcpy(lsmask_files_fullpath, argv[2]);
	strcpy(output_dir_fullpath, argv[3]);

	// printf("c: argv[1]: %s \n" , argv[1]);
	// printf("c: argv[2]: %s \n" , argv[2]);
	// printf("c: argv[3]: %s \n" , argv[3]);

	printf("c: input_toa_home: %s \n" , input_toa_home);
	printf("c: lsmask_files_fullpath: %s \n" , lsmask_files_fullpath);
	printf("c: output_dir_fullpath: %s \n" , output_dir_fullpath);
	
	// other variables
	char input_home_dir[256], lsMaskedFile[256];
	char outputDir[256];
	char misr_file_fullpath[256];
	char final_output_fullpath[256], ofile1[256];
	char spath[10], sblock[10];
	int camera_mode, j, k;
	// int i_init = 0; //E- added to constrain to 2=Cf
	char output_masked_fileLabel[256];
	char s[256];

	strcpy(output_masked_fileLabel, "masked_");  // 1st fill it here before for-loop

	char cam_name[5] = "";
	// printf("cam name: %s \n" , cam_name);

	for (camera_mode = 0; camera_mode < 9; camera_mode++ ) {   //starts from camera= 0 to 2

		printf("\n****************************************************************\n");

		strcpy(input_home_dir, input_toa_home);  // copies the pointer
		
		if (camera_mode == 0) // adds this dir to the end of surf file dir
		{
			strcpy(cam_name, "/Da/");
			printf("c: processing camera dir: %s \n" , cam_name);
			strcat(input_home_dir, "/Da/");
		} 
		if (camera_mode == 1) 
		{
			strcpy(cam_name, "/Ca/");
			printf("c: processing camera dir: %s \n" , cam_name);
			strcat(input_home_dir, "/Ca/");
		}
		if (camera_mode == 2) 
		{
			strcpy(cam_name, "/Ba/");
			printf("c: processing camera dir: %s \n" , cam_name);
			strcat(input_home_dir, "/Ba/");
		}
		if (camera_mode == 3) 
		{
			strcpy(cam_name, "/Aa/");
			printf("c: processing camera dir: %s \n" , cam_name);
			strcat(input_home_dir, "/Aa/");
		}
		if (camera_mode == 4) 
		{
			strcpy(cam_name, "/An/");
			printf("c: processing camera dir: %s \n" , cam_name);
			strcat(input_home_dir, "/An/");
		}
		if (camera_mode == 5) 
		{
			strcpy(cam_name, "/Af/");
			printf("c: processing camera dir: %s \n" , cam_name);
			strcat(input_home_dir, "/Af/");
		}
		if (camera_mode == 6) 
		{
			strcpy(cam_name, "/Bf/");
			printf("c: processing camera dir: %s \n" , cam_name);
			strcat(input_home_dir, "/Bf/");
		}
		if (camera_mode == 7) 
		{
			strcpy(cam_name, "/Cf/");
			printf("c: processing camera dir: %s \n" , cam_name);
			strcat(input_home_dir, "/Cf/");
		}

		if (camera_mode == 8) 
		{
			strcpy(cam_name, "/Df/");
			printf("c: processing camera dir: %s \n" , cam_name);
			strcat(input_home_dir, "/Df/");
		}

		strcpy(outputDir, output_dir_fullpath);  // copies the pointer

		if (camera_mode == 0) strcat(outputDir, "/Da/");
		if (camera_mode == 1) strcat(outputDir, "/Ca/");
		if (camera_mode == 2) strcat(outputDir, "/Ba/");
		if (camera_mode == 3) strcat(outputDir, "/Aa/");
		if (camera_mode == 4) strcat(outputDir, "/An/");
		if (camera_mode == 5) strcat(outputDir, "/Af/");
		if (camera_mode == 6) strcat(outputDir, "/Bf/");
		if (camera_mode == 7) strcat(outputDir, "/Cf/");
		if (camera_mode == 8) strcat(outputDir, "/Df/");

		printf("c: input_home_dir (TOA-refl.dat): %s \n" , input_home_dir);
		// printf("total toa.dat found: %d \n" , misr_total_files_found);
		// printf("max misr_total_files_found = %d \n" , max_nfiles);

		// if (misr_total_files_found > max_nfiles) max_nfiles = misr_total_files_found; //E- why? to remember how many iterations/files we did in past step?
		
		misr_total_files_found = 0; // defined as initial value- inside main() as local variable so that it can be set to zero again 
		
		if (!getFileList(input_home_dir)) {

			printf("c: ERROR: from getFileList. \n");
			return 1;
		} // returns misr_file_list

		printf("c: total TOA.dat files found: %d \n" , misr_total_files_found); // E
		printf("%s \n" , "****************************************************************");

		//misr_total_files_found = 40; //E-i added to test it
		for (j=0; j < misr_total_files_found; j++) { // loop over all misr_total_files_found == num of TOA refl in AN dir

			sprintf(misr_file_fullpath, "%s%s", input_home_dir, misr_file_list[j]); // copies surf_file name into misr_file_fullpath
			printf("\nc: processing TOA file (%d/%d/%s) \n" , j+1, misr_total_files_found, cam_name); // E
			printf("%s \n" , misr_file_fullpath);

			if (!read_data(misr_file_fullpath, &data, nlines, nsamples)) 
			{
				printf("c: ERROR: from read_data\n");
				return 1;
			} // reads surf_refl and returns mem-add od data

			if (strstr(misr_file_fullpath, "_P")) 
			{ // finds path#
				strncpy(spath, strstr(misr_file_fullpath, "_P") + 2, 3);
				spath[3] = 0;
				printf("c: spath: %s\n" , spath);
			}
			else 
			{
				fprintf(stderr, "No path info in file name\n");
				return 1;
			}

			if (strstr(misr_file_fullpath, "_B")) 
			{ // finds block#
				strncpy(sblock, strstr(misr_file_fullpath, "_B") + 2, 3); // note: change 2 to 3 if blocks go larger than 99 (100)
				sblock[3] = 0; // should get blocks with 3 digits
				printf("c: sblock= %s \n" , sblock);
			}
			else 
			{
				fprintf(stderr, "No block info in file name\n");
				return 1;
			}

			/* now define land mask file here*/
			strcpy(lsMaskedFile, lsmask_files_fullpath);
			printf("c: lsMaskedFile: %s \n" , lsMaskedFile);
			
			strsub(lsMaskedFile, "PPP", spath);
			strsub(lsMaskedFile, "BBB", sblock);
			printf("c: lsMaskedFile to read_byte func: %s\n", lsMaskedFile);


			if (!read_byte_data(lsMaskedFile, &mask, nlines, nsamples)) 
			{
				printf("c: ERROR: from read_byte_data. Exiting.\n");
				return 1;
			}
			
			if (!maskData()) 
			{
				printf("c: ERROR: from maskData(). Exiting. \n");
				return 1;
			}

			strcat(output_masked_fileLabel, misr_file_list[j]);
			printf("c: output file label= %s \n" , output_masked_fileLabel);

			// strcpy(ofile1, misr_file_list[j]); //E

			//ofile1 = ("output_masked_fileLabel%s", ofile1);
			//strsub(ofile1, "surf", "surf_lsm");
			//strcat("output_masked_fileLabel", ofile1);
			strcpy(final_output_fullpath, outputDir);
			//printf("ofile1: %s \n", final_output_fullpath);

			//strcat(final_output_fullpath, ofile1); //E
			strcat(final_output_fullpath, output_masked_fileLabel);

			// test_data_before_write_byEhsan();

			// check if output file exists
			if (access(final_output_fullpath, F_OK) == 0)   // if file exists, success returns == 0
			{
				printf("c: **** output file EXIST *** we continue to next file. \n");

				free(data);
				free((unsigned char *) mask);
				memset(final_output_fullpath, '\0', sizeof(final_output_fullpath));
				memset(output_masked_fileLabel, '\0', sizeof(output_masked_fileLabel));
				strcpy(output_masked_fileLabel, "masked_"); //E= write again to it to use it inside for-loop
				continue;
			}
			else
			{
				printf("c: output file NOT exist, writing output to= %s \n", final_output_fullpath);

			}

			if (!write_data(final_output_fullpath, data, nlines, nsamples)) 
			{  // int write_data(char* outputFileStream, double* data, int nlines, int nsamples) // pass-by-value == copying data

				printf("c: ERROR: from write_data. Exiting.\n");
				return 1;
			}

			// strsub(final_output_fullpath, ".dat", ".png");
			// if (!write_png(final_output_fullpath, data2image(data, nlines, nsamples), nlines, nsamples)) return 1;

			// for (k=0; k < nlines*nsamples; k++) { // issue here? who added this?
			   //  data[k] = 0;
			   //  mask[k] = '\0';
			// }

			free(data);
			free((unsigned char *) mask);
			memset(final_output_fullpath, '\0', sizeof(final_output_fullpath));
			memset(output_masked_fileLabel, '\0', sizeof(output_masked_fileLabel));
			strcpy(output_masked_fileLabel, "masked_"); //E= write again to it to use it inside loop

		}   //end of j for loop
	}   //end of i for loop

	printf("\n***** SUCCESSFULLY FINISHED! *****\n");
	return 0;
} // end of main
