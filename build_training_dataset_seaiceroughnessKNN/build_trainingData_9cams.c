/* 
Ehsan Jan 15, 2020
originally from dir: /home/mare/Nolin/SeaIce/Code/C (atmmodel.c)
name: atm_to_misr_pixels.c
usage: labels MISR pixels w/ATM roughness data


1- makes a list of all available ATM.csv files
2- finds date from each ATM file, finds k=-+1 day MISR images (yesterday, today, tomorrow), finds the corresponding pixel in each MISR image, updates dataset 
3- finds cloudy pixels if cloudMask_runMode==1
4- outputs atmmodel


26/May/2022
note: modified to include any camera even if one was missing, does not continue to next file if on camera was missing 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <MisrToolkit.h>
#include <MisrError.h>
#include <dirent.h>
#include <unistd.h> // E: to use access() if file exists.
#include <stdint.h> // E: to use uint8 bits

#define NO_DATA -999999.0
#define BACKGROUND -999998.0
#define FOREGROUND -999997.0
#define TDROPOUT -999996.0
#define CMASKED -999995.0
#define LMASKED -999994.0
#define VERBOSE 0


// // E- set to 1 if we use cloud mask
// int cloudMask_runMode = 1; // 0 == turn off cloud mask


typedef struct {
    int path;
    int orbit;
    int img_block;
    int line;
    int sample;
    double npts;
    double lat;
    double lon;
    // double an;
    // double ca;
    // double cf;
    // add all 9 cameras here
    double anr, ang, anb, annir, af, aa, bf, ba, cf, ca, df, da; 
    double rms;
    float weight;
    int8_t cloud; // E: maybe change to int8? cuz range={-1,0,1}
    double var;
} atm_dtype;

// define instance of struct
atm_dtype* trainingDS_dataStruct;


MTKt_status status;


int block_min, block_max;
int orbit;
int nfiles;
double *data = 0;
double *cfdata = 0;
double *londata = 0;
double *latdata = 0;

int nlines = 512;
int nsamples = 2048;

int read_data(char *atm_fname_fullpath, int line, int sample, double *data);
int read_data_cloudmask(char* atm_fname_fullpath, int line, int sample, uint8_t* data);
//int write_data(char *atm_fname_fullpath, double *data, int nlines, int nsamples);
char *strsub(char *s, char *a, char *b);


// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int read_data_cloudmask(char* fname_fullpath, int line, int sample, uint8_t* data) { // note: cloud-mask file should be in size (512 * 2048), but because it is in binary the shape should be (1048576,); dtype: 8-bit unsigned char
    FILE* in_stream; // input file obj
    int nlines = 512;
    int nsamples = 2048;
    uint8_t* in_buf;

    in_stream = fopen(fname_fullpath, "r"); // ptr to toa file

    if (!in_stream) 
    {
        fprintf(stderr,  "read_data_1: FileNotFound: %s\n", fname_fullpath);  // I turned off this print, cos there were a lot of prints/fileNotFound
        return 0;
    }
    
    in_buf = (uint8_t*) malloc(nlines * nsamples * sizeof(uint8_t)); // this is data on mem-

    if (!in_buf) 
    {    
        fprintf(stderr,  "read_data_2: couldn't malloc data\n");
        return 0;
    }
    // read data from both masked_toa.dat & cloudmask.cmk files w/ fread(dest-mem, elementSize, totalElements, inputStreamPtr(on hard?))
    if (fread(in_buf, sizeof(uint8_t), (nlines * nsamples), in_stream) != nlines * nsamples) 
    { // On success, it reads n items from the file and returns n. On error or end of the file, it returns a number less than n.
        
        fprintf(stderr,  "read_data_3: couldn't read data in %s\n", fname_fullpath);
        return 0;
    }

    *data = in_buf[line * nsamples + sample]; // dereference data by *data

    free(in_buf);

    fclose(in_stream);
    return 1;
}

// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int read_data(char* fname_fullpath, int line, int sample, double* data) {
    FILE* in_stream; // input file obj
    int nlines = 512;
    int nsamples = 2048;
    double* in_buf;

    in_stream = fopen(fname_fullpath, "r"); // ptr to toa file

    if (!in_stream) 
    {
    	fprintf(stderr,  "read_data_1: FileNotFound: %s\n", fname_fullpath);  // I turned off this print, cos there were a lot of prints/fileNotFound
    	return 0;
    }
	
    in_buf = (double *) malloc(nlines * nsamples * sizeof(double)); // this is data on mem-

    if (!in_buf) 
    {
    	
        fprintf(stderr,  "read_data_2: couldn't malloc data\n");
    	return 0;
    }
	// read data from both masked_toa.dat & cloudmask.cmk files w/ fread(dest-mem, elementSize, totalElements, inputStreamPtr(on hard?))
    if (fread(in_buf, sizeof(double), (nlines * nsamples), in_stream) != nlines * nsamples) 
    { // On success, it reads n items from the file and returns n. On error or end of the file, it returns a number less than n.
    	
        fprintf(stderr,  "read_data_3: couldn't read data in %s\n", fname_fullpath);
    	return 0;
    }

    *data = in_buf[line * nsamples + sample]; // dereference data by *data

    free(in_buf);

    fclose(in_stream);
    return 1;
}

// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *strsub(char* s, char *a, char* b)
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

// ///////////////////////////////////////////////////////// main //////////////////////////////////////////////////////

// int main(char argc, char *argv[]) { // Ehsan: retured error, so changed to int

int main(int argc, char *argv[])
{

    // DIR *dirp;
    // FILE *fp, *filePtr;
    // struct dirent* FileEntryPtr; // ptr to fileObj == struct
    //char atm_dir[256] = "/home/mare/Nolin/SeaIce/ILATM2.002";
    //char masked_toa_refl_home[256] = "/home/mare/Nolin/2013/MaskedSurf/April_sdcmClearHC";
    //char atmfile[256] = "/home/mare/Nolin/SeaIce/ILATM2.002/combined_atm.csv";
    //char atmmodel_csvfile[256] = "/home/mare/Nolin/SeaIce/ILATM2.002/SeaIce_Apr2013_atmmodel_csvfile.csv";

    // inputes
    // char atm_dir[256] = "/home/mare/Projects/MISR/Julienne/IceBridge2016/july_atm_Ehsan/ehsan_test_for_atm20160714" ; // start from ATM files == ILATM2 csv files
    // char atm_dir[256] = "/Users/ehsanmos/Documents/RnD/MISR_lab/misr_processing_dir/April2013_ATM_MIST.nosync/ATM_IceBridge_ILATM2.002/ATM_Apr_2013" ; // start from ATM files == ILATM2 csv files
    // char atm_dir[256] = "/Volumes/Ehsan7757420250/2016/april_2016/ATM_IceBridge_ILATM2_V002" ;
    char atm_dir[256];
    // char masked_toa_refl_home[256] = "/home/mare/Nolin/data_2000_2016/2016/Surface3_LandMasked/Jul"; // surf dat files
    // char masked_toa_refl_home[256] = "/Volumes/Ehsan7757420250/2016/april_2016/sample_ellipsoid_april2016/toa_refl_april2016_day1to16_p1to233_b1to46/masked_toa_refl_april2016_day1to16_p1to233_b1to46" ; // from LandMask.c; produce surf_masked files for specific day
    char masked_toa_refl_home[256];
    // char cloud_masked_dir[256] = "/home3/mare/Nolin/2016/MaskedSurf/Jul_sdcmClearHC_LandMasked" ; // original cloud mask data == lsdcm dat files
    // char cloud_masked_dir[256] = "/Volumes/easystore/from_home/Nolin_except_dataFolder/remainder_forExternalHD_3.1TB/2011/MaskedSurf/Apr_sdcmClearHC/" ;  // cloud mask data == lsdcm dat files, to do a testrun i renames files to sdcm_p* @ line:345
    // char cloud_masked_dir[256] = "/Volumes/Ehsan7757420250/2016/TC_CLASSIFIERS_F07/cloudmask_TC_CLASSIFIERS_F07_HC4_only" ;
    char cloud_masked_dir[256];
    

    // output
    char atmmodel_csvfile[256]; // writes output=atmmodel_csvfile to current dir
    // char atmmodel_csvfile[256] = "/home/mare/Projects/MISR/Julienne/IceBridge2016/SeaIce_Jul2016_atmmodel_csvfile_cloud_var.csv"; // old
    //char lsmask_dir[256] = "/home/mare/Nolin/SeaIce/LWData/MISR_LandSeaMask";

    //------------------------------------------------------------------------------------------------------------------
    // check number of inouts to this code from commandLine (from python wrapper)
    if (argc == 5) 
    {
        printf("C: OK! received 5 arguments. \n");
    }

    if (argc != 5) 
    { // this might happen later, cos I turnedoff fname[2]
        fprintf(stderr, "Usage: <exe-name> <ATM-dir> <maskedTOA-dir> <cloudMask-dir> <atmmodelCSV-file> \n"); // updated
        // fprintf(stderr, "Usage: TOA3 input-misr-file block band minnaert output-data-file output-image-file-Ehsan--noNeed\n"); old with image
        return 1;
    }
    //------------------------------------------------------------------------------------------------------------------
    // fill string from commandLine
    strcpy(atm_dir, argv[1]);
    strcpy(masked_toa_refl_home, argv[2]);
    strcpy(cloud_masked_dir, argv[3]);
    strcpy(atmmodel_csvfile, argv[4]);

    printf("ATM-dir: %s \n", atm_dir);
    printf("maskedTOA-dir: %s \n" , masked_toa_refl_home);
    printf("cloudMask-dir: %s \n" , cloud_masked_dir);
    printf("atmmodel_csvfile_name: %s \n" , atmmodel_csvfile);

    //------------------------------------------------------------------------------------------------------------------
    DIR *dirp;
    FILE *fp, *filePtr;
    struct dirent* FileEntryPtr;
    char message[256];
    char idir[256];
    char an_file[128];
    char atm_fname_fullpath[256];

    char toa_an_red_masked_fullpath[256];
    char toa_an_green_masked_fullpath[256];
    char toa_an_blue_masked_fullpath[256];
    char toa_an_NIR_masked_fullpath[256];
    char toa_af_red_masked_fullpath[256];
    char toa_aa_red_masked_fullpath[256];
    char toa_bf_red_masked_fullpath[256];
    char toa_ba_red_masked_fullpath[256];
    char toa_df_red_masked_fullpath[256];
    char toa_da_red_masked_fullpath[256];
    char toa_ca_red_masked_fullpath[256];
    char toa_cf_red_masked_fullpath[256];

    char cloudmask_fname_fullpath[256];
    //char lsmask_atm_fname_fullpath[256];
    //unsigned char lsmask;
    char xfile[256];
    char **atm_file_list = 0;
    char syear[5];  // size to define is: total elements = 4+1 --> +1 for null character that marks the end of the string
    char smonth[3];     // size to define is: total elements = 2+1 --> +1 for null character that marks the end of the string
    char sday[3];   // size to define is: total elements = 2+1 --> +1 for null character that marks the end of the string
    char misr_start_time[32];
    char misr_end_time[32];
    char str[16];
    char *token;
    char *line_in_buff = NULL;
    int* orbitlist;
    int orbit_count;
    int month;
    int day;
    int path;
    int atm_files_list_index = 0;
    int misr_nfiles = 0;
    int total_trainingDS_row_in_mem = 0;
    int i, j, k_misr_date, available_ds_row_index, n, w;
    int img_block;
    int atm_point_in_pixel_key;
    int natm_half_weight = 0;
    int natm_valid = 0;
    //int monthday[12][31] = {0};
    double avg_rms = 0;
    double avg_valid_rms = 0;
    double weight;
    // add all cameras here in program scope
    double anr, ang, anb, annir, af, aa, bf, ba, cf, ca, df, da; // note: cm dtype should be double or uint? inside read_data f(.) it is defined as double and this f(.) is used to read several files
    // double cm; // = -1;
    uint8_t cm; // dtype= maybe to uint8_t??? cause cm is either= 0, 1
    double xlat, xlon, xrms; //, xcam;
    int xcam;
    float fline, fsample;
    int line, sample;

    // int nocloud_pts, cloud_pts, misscloud_pts; //it is for report stats-turned it off 10 Nov 22
    
    int path_x, orbit_x, block_x;
    double weight_x;
    int nocloud_x, cloud_x, misscloud_x;
    size_t slen = 0;
    // ssize_t read;
    ssize_t line_size;
    int ATMnewLine = 0;

    //**********************************************************************************
    // E- set to 1 if we use cloud mask
    int cloudMask_runMode = 1; // 0 == turn off cloud mask


    // this codes runs either of the following, not both at the same time
    int misr_cloudmask = 0;    // to run with MISR cloudmask data products
    int cmcombo_cloudmask = 1; // to run cmcombo mode

    /* -------------------------------------------------------------------------------------------------- */
    /* Get list of all available ATM.csv files in directory */

    printf("\n*********** PROGRAM: ATMRoughness2MISRPixels.c ***********\n\n");
    printf("making list of ATM.csv files...\n");

    dirp = opendir(atm_dir); // create a stream 

    if (dirp)
    {
    	while ((FileEntryPtr = readdir(dirp)) != NULL) 
        { // num of iterations == num of ATM files available == atm_files_list_index == FileEntryPtr is ptr to fileObj, we create it for every iteration = ATM csv file available
		    // if (strstr(FileEntryPtr->d_name, "combine")) continue; // d_name is string of fileName in fileObj
		    // if (strstr(FileEntryPtr->d_name, "SeaIce")) continue;
            if (! strstr(FileEntryPtr->d_name, "ILATM2")) 
            {
                // printf("dirEntry does not start w/ ILATM2, skipping it ... \n");
                continue;
            }
            if (strstr(FileEntryPtr->d_name, ".xml")) 
            {
                // printf("dirEntry ends w/ .xml , skipping it ... \n");
                continue;
            }
            if (! strstr(FileEntryPtr->d_name, ".csv")) 
            {
                // printf("dirEntry does not end w/ .csv , skipping it ... \n");
                continue;
            }

		    if (atm_file_list == 0)
            {
                atm_file_list = (char **) malloc(sizeof(char *));
                if (!atm_file_list)
                {
                    printf("main: couldn't malloc atm_file_list\n");
                    return 0;
                }
            }
            else
            {
                atm_file_list = (char **) realloc(atm_file_list, (atm_files_list_index + 1) * sizeof(char *));
                if (!atm_file_list)
                {
                    printf("getFileList: couldn't realloc atm_file_list\n");
                    return 0;
                }
            }

            atm_file_list[atm_files_list_index] = (char *) malloc(strlen(FileEntryPtr->d_name) + 1);

            if (!atm_file_list[atm_files_list_index]) 
            {
                printf("main: couldn't malloc atm_file_list[%d]\n", atm_files_list_index);
                return 0;
            }

            strcpy(atm_file_list[atm_files_list_index], FileEntryPtr->d_name); // fill the list with available ATM files
            atm_files_list_index ++;
        }
    	closedir (dirp); // close the stream
    }

    else
    {
        strcat(message, "Can't open ATM.csv file\n");
        strcat(message, atm_dir);
        perror (message);
        return EXIT_FAILURE;
    }

    printf("----------------------------------------------------------------------------------------------\n");
    printf("number of ATM.csv (ILATM2) files found= (%d) in direcotry: %s \n" , atm_files_list_index, atm_dir);
    printf("\n");

    /* ---------------------------------------------------------------------------- */


    /* ---------------------------------------------------------------------------- */
    /* start- read each available ATM-file.csv from the list we made in the past section */

    for (int i = 0; i < atm_files_list_index; i++)
    { // i = num of available ATM files in the list

        printf("\n############################## process a new ATM nfile ##############################\n");
        printf("ATM file: (%d/%d) --> %s \n\n" , i+1, atm_files_list_index, atm_file_list[i]);
        
        // printf("syear1: %s \n" , syear);
        // printf("c- sizeof(syear): %lu \n" , sizeof(syear)) ;


        // to set/clean the memory assgined to it before writing into it again. fill the array with null character.
        memset(syear, '\0', sizeof(syear));  
        // printf("syear1: %s \n" , syear);


        strncpy(syear, (atm_file_list[i] + 7), 4); // get year from file name; why not in ptr format?
        // printf("syear2: %s \n" , syear);


        // char* yearCopy = strdup(syear); // Pouya replaced every syear with yearCopy

        // printf(smonth);
        memset(smonth, '\0', sizeof(smonth));
        strncpy(smonth, (atm_file_list[i] + 11), 2); // get month from file name; how?
        // printf("smonth: %s \n" , smonth);

        month = atoi(smonth);

        memset(sday, '\0', sizeof(sday));
        strncpy(sday, (atm_file_list[i] + 13), 2); // get dat from file name
        // printf("sday: %s \n" , sday);

        // extracted this info from each ATM file name
        // printf("date info= yr: %s; mon: %d; day: %s \n", syear, month, sday);

        //--------------------------------------------------------------------------------------------------
        for (k_misr_date  = -1; k_misr_date < 2; k_misr_date++) 
        { // k=days; yesterday (-1) or tmrw (+1) == 0.5 of the ATM overpass; today=o


            // test for only k=0 (MISR images from todat)
            if (k_misr_date == -1 || k_misr_date == 1) 
            {
                /* skip the iteration at this step */
                printf("k= %d, we continue to next k\n", k_misr_date);
                continue; // we skip k= -+1
            } 

            printf("k= %d\n", k_misr_date);


            // printf("\nprocess for k-day: %d \n", k);
            day = atoi(sday) + k_misr_date;
            // printf("day= %d \n" , day);

            // to use for MTK function
            sprintf(misr_start_time, "%s-%02d-%02dT00:00:00Z", syear, month, day); // start time by considering K=+-1 day
            sprintf(misr_end_time, "%s-%02d-%02dT23:59:59Z", syear, month, day); // end time
            
            // printf("ATM time= %s, %s, %s\n", atm_file_list[i], misr_start_time, misr_end_time);
            // printf("ATM period= %s -- to -- %s\n", misr_start_time, misr_end_time);


            if (k_misr_date == 0) weight = 1.0; // for today k=0; w=1 of the ATM overpass in MISR pixel?; test for only k=0 (MISR images from todat)
            else weight = 0.5; // yesterday or tmrw; weight=0.5 of the ATM overpass
            
            //if (monthday[month][day] == 0) {
            status = MtkTimeRangeToOrbitList(misr_start_time, misr_end_time, &orbit_count, &orbitlist); // outputs are orbitCount and list
            // printf("MtkTimeRangeToOrbitList status: %d \n" , status) ;
            // printf("C -> default MTK_SUCCESS: %d \n"  , MTK_SUCCESS) ;

            if (status != MTK_SUCCESS) 
            {
                printf("ERROR: returned from MtkTimeRangeToOrbitList \n");
                return 1;
            }


            /*check orbit list*/
            // int i_orbit;
            // // printf("ATM period= %s -- to -- %s\n", misr_start_time, misr_end_time);
            // // printf("found following MISR orbits for k_day= %d (%d) \n", day, k);

            // int path_number;
            // for (i_orbit = 0; i_orbit < orbit_count; i_orbit++) 
            // {
            //     if (MtkOrbitToPath(orbitlist[i_orbit], &path_number) == MTK_SUCCESS)
            //     {
            //         printf("orbit: %d path: %d \n" , orbitlist[i_orbit], path_number);
            //     }
            // }
            // printf("\nnow process each orbit number...\n\n");

            //----------------------------------------------------------------------------------------

            for (j = 0; j < orbit_count; j++)
            {   // what is orbit_count? orbitCount; Q- orbit during each day? --> comes from MTK, orbits in a specific day
                
                status = MtkOrbitToPath(orbitlist[j], &path); // what is the path for a given orbit == or which path the orbit belong to.
                // printf("MtkOrbitToPath: orbit %d --> path= %d and day= %d \n" , orbitlist[j], path, day);

                if (status != MTK_SUCCESS)
                {
                    printf("status did not match MTK_SUCCESS! \n");
                    return 1;
                }

                //printf("MISR orbit & path in this k: \n");
                //printf("orbit %d goes to path: %d \n", orbitlist[j], path); // MISR: checking orbit and path of MISR
                
                // ATM: create full path of each ATM file(i)
                sprintf(atm_fname_fullpath, "%s/%s", atm_dir, atm_file_list[i]); // i: index of each atm.csv file in list

                //******************************************
                // OPEN EACH ROW OF ATM.csv FILE

                fp = fopen(atm_fname_fullpath, "r"); // create stream = fp for ATM file == open ATM file
                if (!fp)
                {

                    fprintf(stderr, "main: couldn't open ATM file: %s \n", atm_fname_fullpath);
                    return 1;
                }

                //******************************************
                // START PROCESSING EACH ROW OF ATM.csv FILE

                //printf("\n______open ATM fileNum(%d), orbit/path(%d), in day_k(%d), WhileLoop each sample/row info from: %s \n\n", i, path, k, atm_fname_fullpath); // the ATM file
                
                int atm_row_num = 1;
                // now associate each ATM row/sample to 9 MISR surf files
                
                // while ((read = getline(&line_in_buff, &slen, fp)) != -1) 
                while ((line_size = getline(&line_in_buff, &slen, fp)) >= 0 )
                {  // returns number of characters that was read // read each line of each ATM csv

                    /* check ATM.csv row */
                    // printf("ATM row (%d) in file (%d) \n", atm_row_num, i+1);  // E- counts how many rows are read
                    // atm_row_num++;


                    //printf("------------------------------------------\n");
                    //printf("Retrieved line of length \n"); //%zu :\n", read);
                    token = strtok (line_in_buff, ","); // get the 1st token
                    w = 0;

                    // printf ("token: %s\n", token);

                    if (strstr(token, "#"))
                    { // Q- how about this one? this works better than previous one --> skip lines with #
                        // printf("found # in csv file; kday= %d, orbit= %d, skipping the row in csv. \n", k, orbitlist[j]);
                        atm_row_num++;
                        continue;
                    }

//                    if (!strcmp(token, "#")) { // if line starts with #, skip it
//                        printf("found # in csv file; skipping the line in csv \n");
//                        continue; //
//                    }
                    // int ret_num = strcmp(token, "#");
                    // printf("\nreturnNo is: %d \n" , ret_num);

                    // if (strcmp(token, "#") == 0) { // Q- how about this one? --> skip lines with #
                    //     printf("found # in csv file; skipping the row in csv \n");
                    //     continue;
                    // }

              

                    while (token != NULL)
                    { // parse line w/o # until we get to the last word in a line
                        if (w == 1) xlat = atof(token); // ATM lat
                        if (w == 2) xlon = atof(token); // ATM lon
                        if (w == 6) xrms = atof(token); // roughness = target/label (in centemeters)
                        if (w == 10) xcam = atof(token); // Track_Identifier == 0 == nadir ATM camera view; >>>> we are looking for xcam = 0 <<<<
                        token = strtok (NULL, " ,");
                        w++;
                    }

                    // printf("xcam= %d \n" , xcam);

                    if (xcam != 0)
                    { // if not ATM nadir; because we only use nadier camera from ATM
                        // printf("xcam (%d) not nadier (!0), skipping this ATM sample/row.\n" , xcam);
                        atm_row_num++;
                        continue; // to read next ATM row
                    }


                    // do this for xcam == 0, so for each ATM xlat/xlon, we try to find the MISR pixel that ATM falls into 
                    // for a path number, we search if ATM laton falls into an image block
                    // if ok, then means we can use this ATM latlon to label this image block
                    
                    status = MtkLatLonToBls(path, 275, xlat, xlon, &img_block, &fline, &fsample); // try to find an image pixel for an ATM point
                    
                    if (status != MTK_SUCCESS)
                    {
                        // printf("WARNING-1: (Mtk-LatLonToBls) could not find a MISR image&pixel for ATM point @ path= %d, xlat= %f, xlon= %f \n", path, xlat, xlon);

                        if (img_block == -1) 
                        { //Q- why/when img_block returns -1? 
                            // printf("WARNING2: MTK img_block retured -1 from MTK, skippingm! \n");
                            atm_row_num++;
                            continue;  // to next ATM row==location== similar to exception in python
                        }
//                        printf("ERROR for: %f %f %d %f %f", xlat, xlon, img_block, fline, fsample); // Q- why float for line-sample?
//                        return 1;
                    }

                    // else { // turn on for checking if Mtk has returned image pixel for a new ATM point

                    //     printf("\n*** line: (%d) xcam= %d, (MtkLatLonToBls) associated ATM lat-lon: (%lf, %lf) -- to --> MISR b/l/s: %d, %f, %f\n\n" , atm_row_num, xcam, xlat, xlon, img_block, fline, fsample);
                    // }

                    //################################################################################

                    line = rint(fline); // round to int
                    sample = rint(fsample); // Q- why line/sample are float?

                    //printf("ATM lat/lon to MISR pixel: line: %d; sample: %d \n" , line, sample);

                    
                    // printf("preparing input files for: %d, %d, %d \n", path, orbitlist[j], img_block);

                    /* NOTE: now that Mtk has returned an associated Bls for ATM lat-lon, it's time to search for input files: MISR masked_toa files & cloudmask files.: MISR toa masked files based on the extracted info from each ATM row; we should look into these files */


                    // printf("checking masked-toa-refl.dat for path: %d, orbit: %d, block: %d\n", path, orbitlist[j], img_block);
                    
                    //*************************************************
                    // note: here we should consider band names as file name tag (red, green, blue, NIR) that comes from masked.c code- later add g, b , nir
                    // note: do not continue if one cameras was missing
                    // 1- preparing An camera
                    sprintf(toa_an_red_masked_fullpath, "%s/An/masked_toa_refl_P%03d_O%06d_B%03d_an_red.dat", masked_toa_refl_home, path, orbitlist[j], img_block);
                    
                    // check if file is accessible 
                    // if (access(toa_an_red_masked_fullpath, F_OK) == -1){
                    //     // printf("WARNING: input NOT exist: continue to next ATM row: toa-an: %s\n" , toa_an_red_masked_fullpath);
                    //     continue; 
                    // }

                    // 10- preparing An-green camera
                    sprintf(toa_an_green_masked_fullpath, "%s/An/masked_toa_refl_P%03d_O%06d_B%03d_an_green.dat", masked_toa_refl_home, path, orbitlist[j], img_block);
                    
                    // check if file is accessible 
                    // if (access(toa_an_green_masked_fullpath, F_OK) == -1){
                    //     // printf("WARNING: input NOT exist: continue to next ATM row: toa-an: %s\n" , toa_an_green_masked_fullpath);
                    //     continue; 
                    // }

                    // 11- preparing An-blue camera
                    sprintf(toa_an_blue_masked_fullpath, "%s/An/masked_toa_refl_P%03d_O%06d_B%03d_an_blue.dat", masked_toa_refl_home, path, orbitlist[j], img_block);
                    
                    // check if file is accessible 
                    // if (access(toa_an_blue_masked_fullpath, F_OK) == -1){
                    //     // printf("WARNING: input NOT exist: continue to next ATM row: toa-an: %s\n" , toa_an_blue_masked_fullpath);
                    //     continue; 
                    // }

                    // 12- preparing An-nir camera
                    sprintf(toa_an_NIR_masked_fullpath, "%s/An/masked_toa_refl_P%03d_O%06d_B%03d_an_NIR.dat", masked_toa_refl_home, path, orbitlist[j], img_block);
                    
                    // check if file is accessible 
                    // if (access(toa_an_NIR_masked_fullpath, F_OK) == -1){
                    //     // printf("WARNING: input NOT exist: continue to next ATM row: toa-an: %s\n" , toa_an_NIR_masked_fullpath);
                    //     continue; 
                    // }


                    // 2- preparing Cf camera
                    sprintf(toa_cf_red_masked_fullpath, "%s/Cf/masked_toa_refl_P%03d_O%06d_B%03d_cf_red.dat", masked_toa_refl_home, path, orbitlist[j], img_block);
                    
                    // printf("toa cf: %s\n" , toa_cf_masked_fullpath);
                    // check if file is accessible 
                    // if (access(toa_cf_red_masked_fullpath, F_OK) == -1){
                    //     // printf("WARNING: input NOT exist: continue to next ATM row: toa-an: %s\n" , toa_cf_red_masked_fullpath);
                    //     continue; // check if file is accessiblem
                    // }

                    // 3- preparing Ca camera
                    sprintf(toa_ca_red_masked_fullpath, "%s/Ca/masked_toa_refl_P%03d_O%06d_B%03d_ca_red.dat", masked_toa_refl_home, path, orbitlist[j], img_block);
                    
                    // printf("toa ca: %s\n" , toa_ca_masked_fullpath);
                    // check if file is accessible 
                    // if (access(toa_ca_red_masked_fullpath, F_OK) == -1){
                    //     // printf("WARNING: input NOT exist: continue to next ATM row: toa-an: %s\n" , toa_ca_red_masked_fullpath);
                    //     continue; // check if file is accessiblem
                    // }

                    // add all other cameras here**********************

                    // 4- preparing Af camera
                    sprintf(toa_af_red_masked_fullpath, "%s/Af/masked_toa_refl_P%03d_O%06d_B%03d_af_red.dat", masked_toa_refl_home, path, orbitlist[j], img_block);
                    
                    // if (access(toa_af_red_masked_fullpath, F_OK) == -1){
                    //     // printf("WARNING: input NOT exist: continue to next ATM row: toa-an: %s\n" , toa_af_red_masked_fullpath);
                    //     continue; 
                    // }

                    // 5- preparing Aa camera
                    sprintf(toa_aa_red_masked_fullpath, "%s/Aa/masked_toa_refl_P%03d_O%06d_B%03d_aa_red.dat", masked_toa_refl_home, path, orbitlist[j], img_block);
                    
                    // if (access(toa_aa_red_masked_fullpath, F_OK) == -1){
                    //     // printf("WARNING: input NOT exist: continue to next ATM row: toa-an: %s\n" , toa_aa_red_masked_fullpath);
                    //     continue; 
                    // }

                    // 6- preparing Bf camera
                    sprintf(toa_bf_red_masked_fullpath, "%s/Bf/masked_toa_refl_P%03d_O%06d_B%03d_bf_red.dat", masked_toa_refl_home, path, orbitlist[j], img_block);
                    
                    // if (access(toa_bf_red_masked_fullpath, F_OK) == -1){
                    //     // printf("WARNING: input NOT exist: continue to next ATM row: toa-an: %s\n" , toa_bf_red_masked_fullpath);
                    //     continue; 
                    // }

                    // 7- preparing Ba camera
                    sprintf(toa_ba_red_masked_fullpath, "%s/Ba/masked_toa_refl_P%03d_O%06d_B%03d_ba_red.dat", masked_toa_refl_home, path, orbitlist[j], img_block);
                    
                    // if (access(toa_ba_red_masked_fullpath, F_OK) == -1){
                    //     // printf("WARNING: input NOT exist: continue to next ATM row: toa-an: %s\n" , toa_ba_red_masked_fullpath);
                    //     continue; 
                    // }

                    // 8- preparing Df camera
                    sprintf(toa_df_red_masked_fullpath, "%s/Df/masked_toa_refl_P%03d_O%06d_B%03d_df_red.dat", masked_toa_refl_home, path, orbitlist[j], img_block);
                    
                    // if (access(toa_df_red_masked_fullpath, F_OK) == -1){
                    //     // printf("WARNING: input NOT exist: continue to next ATM row: toa-an: %s\n" , toa_df_red_masked_fullpath);
                    //     continue; 
                    // }

                    // 9- preparing Da camera
                    sprintf(toa_da_red_masked_fullpath, "%s/Da/masked_toa_refl_P%03d_O%06d_B%03d_da_red.dat", masked_toa_refl_home, path, orbitlist[j], img_block);
                    
                    // if (access(toa_da_red_masked_fullpath, F_OK) == -1){
                    //     // printf("WARNING: input NOT exist: continue to next ATM row: toa-an: %s\n" , toa_da_red_masked_fullpath);
                    //     continue; 
                    // }






                    //--------- cloud mask here ------------------
                    // 4- now check cloudMask file
                    if (!cloudMask_runMode == 1)
                    { // if cloudMask is off go here
                        printf("c: NOTE: cloudMask is off == we do not use cloud mask anymore! \n");
                    } 
                    else 
                    {

                        if (misr_cloudmask)
                        {
                            // sprintf(cloudmask_fname_fullpath, "%s/An/sdcm_p%03d_o%06d_b%03d_an.dat", cloud_masked_dir, path, orbitlist[j], img_block); // original=lsdcm =? note: to do a test run, i renamed the filename from lsdcm_p* to sdcm_p* 
                            sprintf(cloudmask_fname_fullpath, "%s/cloudmask_P%03d_O%06d_B%03d.msk", cloud_masked_dir, path, orbitlist[j], img_block); // original=lsdcm =? note: to do a test run, i renamed the filename from lsdcm_p* to sdcm_p* 
                        }

                        if (cmcombo_cloudmask)
                        {
                           sprintf(cloudmask_fname_fullpath, "%s/cmcombo_P%03d_O%06d_B%03d.msk", cloud_masked_dir, path, orbitlist[j], img_block); // original=lsdcm =? note: to do a test run, i renamed the filename from lsdcm_p* to sdcm_p* 

                        }


                        // check if file is accessible 
                        if (access(cloudmask_fname_fullpath, F_OK) != 0)  // update access f(.)?
                        {
                            printf("cloudMask NOT exist: %s\n", cloudmask_fname_fullpath);
                            // continue; // check if file is accessible
                            // cloudMask_runMode = 0;
                        }
                        else
                        {
                            printf("c: checking: cloudMask EXIST: %s \n" , cloudmask_fname_fullpath);
                        }
                    }
                    /* --------- cloud mask here [for future?] ------------------
                        what was coming here??? I don't rememebr */




                    /*############################################################################################################*/

                    // printf("*** C: preparing input files for: %d, %d, %d, %d, %d, %f, %f, %f\n", path, orbitlist[j], img_block, line, sample, xlat, xlon, xrms); // for each csv row == label info
                    /* --------------------------------------------------------------------------------------------------- */
                    /* now try to find MISR pixels that each ATM point roughness falls into it and then/in what step? sum all roughness values */

                    /* we assume there was not any previous ATM points in MISR pixel, it's a switch to check if every new ATM point falls inside a previous pixel, reset to zero for every new entry=ATM line */
                    atm_point_in_pixel_key = 0; // to turn on&off

                    if (total_trainingDS_row_in_mem == 0)
                    {
                        // 1st allocate mem- for each row of csv file
                        trainingDS_dataStruct = (atm_dtype*) malloc(sizeof(atm_dtype));
                    }
                    else
                    {      
                        /* check in existing dataset for similar atm point
                            for 2nd total_trainingDS_row_in_mem and the rest */
                        available_ds_row_index = 0; // counter of row in trainingDS_dataStruct 
                        while ((available_ds_row_index < total_trainingDS_row_in_mem) && !atm_point_in_pixel_key) 
                        { // checks new ATM point with previous rows in dataset= all n points inside trainingDS_dataStruct until pixel is found

                            /* first check if there are previous ATM points inside MISR pixel so that we average new [roughness?] value w/ previous values
                                we compare new MISR path,img_block,line,sample (associated with ATM point/row) with every n point available in fileObj;
                                if even one similar MISR pixel is found in dataset, we will sum & update trainingDS_dataStruct with ATM info [rms, npts, var] to previous MISR pixel values in fileObj */
                            if ((trainingDS_dataStruct[available_ds_row_index].path == path) && (trainingDS_dataStruct[available_ds_row_index].img_block == img_block) && (trainingDS_dataStruct[available_ds_row_index].line == line) && (trainingDS_dataStruct[available_ds_row_index].sample == sample) && (trainingDS_dataStruct[available_ds_row_index].weight == weight)) // weight reprsents same day
                            {   // Q- what is this condition? why check to be the same? in same day in same pixel????
                                
                                // printf("FOUND previous ATM points in a MISR pixel: summing with previous ATM values ...\n");
                                //printf("FOUND: ATM in MISR pixel >>> day: (%d), path: (%d), img_block: (%d), line: (%d), sample: (%d)\n\n", k, path, img_block, line, sample);
                                trainingDS_dataStruct[available_ds_row_index].rms += weight * xrms; // sum of weighted ATM roughness in the same pixel?
                                trainingDS_dataStruct[available_ds_row_index].npts += weight; // number of ATM points in the same MISR pixel?
                                trainingDS_dataStruct[available_ds_row_index].var += weight * xrms * xrms; // sum of what???? variance?
                                atm_point_in_pixel_key = 1; // turn on the key- when we find the first ATM point in pixel == turns on here == 1 and skip while-loop
                            }
                            
                            available_ds_row_index++ ; // iterate to next point in file Obj
                        }

                        /* if ATM point was the first point in that MISR pixel (not found in previous MISR pixels so far) 
                            we allocate memory for the new point in trainingDS_dataStruct, basically trainingDS_dataStruct grows a row  */
                        if (!atm_point_in_pixel_key)
                        { // if key is still off- here we check if the key is still off- here we check !0(being off)==1(valid) as a condition
                            // printf("need to add a row in trainingDS_dataStruct for new ATM point! \n");
                            trainingDS_dataStruct = (atm_dtype*) realloc(trainingDS_dataStruct, (total_trainingDS_row_in_mem + 1) * sizeof(atm_dtype));
                            // note: size of buffer is sizeof ptr (trainingDS_dataStruct) * how many ptrs we allocate == how much memory we allocate
                        }
                    }

                    /* check if mem- is allocated */
                    if (!trainingDS_dataStruct)
                    {

                        fprintf(stderr,  "ERROR in main: couldn't malloc/realloc trainingDS_dataStruct\n");
                        return 0;
                    }

                    //************************************************************************************************
                    /* this block runs first:
                            we run this because the new ATM location (xlat/xlon) was not found in previous MISR pixels 
                            and we will add it as a new dataPoint row to trainingDS_dataStruct 
                            this will be the first row entry to the datastructure in memory */
                    
                    if (!atm_point_in_pixel_key)
                    { 
                        //if key is still off == we could not find any ATM point in MISR pixel ==  atm_point_in_pixel_key=0 
                        // printf("FOUND a new/first? ATM point (row/sample), will add it to trainingDS_dataStruct now...\n");
                        trainingDS_dataStruct[total_trainingDS_row_in_mem].path = path;
                        trainingDS_dataStruct[total_trainingDS_row_in_mem].orbit = orbitlist[j];
                        trainingDS_dataStruct[total_trainingDS_row_in_mem].img_block = img_block;
                        trainingDS_dataStruct[total_trainingDS_row_in_mem].line = line;
                        trainingDS_dataStruct[total_trainingDS_row_in_mem].sample = sample;
                        trainingDS_dataStruct[total_trainingDS_row_in_mem].lat = xlat;
                        trainingDS_dataStruct[total_trainingDS_row_in_mem].lon = xlon;
                        trainingDS_dataStruct[total_trainingDS_row_in_mem].weight = weight;
                        trainingDS_dataStruct[total_trainingDS_row_in_mem].cloud = -1; // Q- how to interpret it? is filling value?


                        // printf("using AN file: %s\n", toa_an_masked_fullpath);
                        read_data(toa_an_red_masked_fullpath, line, sample, &anr);
                        read_data(toa_an_green_masked_fullpath, line, sample, &ang);
                        read_data(toa_an_blue_masked_fullpath, line, sample, &anb);
                        read_data(toa_an_NIR_masked_fullpath, line, sample, &annir);
                        // printf("PROBLEM: an= %f\n" , an); // problem here: why all zero?
                        read_data(toa_cf_red_masked_fullpath, line, sample, &cf); // returns value of 1 pixel at a time  // int read_data(char* atm_fname_fullpath, int line, int sample, double* data)
                        // printf("PROBLEM: cf= %f\n" , cf); // problem here: why all zero? cuz all input files were not defined correctly!
                        read_data(toa_ca_red_masked_fullpath, line, sample, &ca);
                        // printf("PROBLEM: ca= %f\n" , ca); // problem here: why all zero?
                        read_data(toa_aa_red_masked_fullpath, line, sample, &aa); // returns value of 1 pixel at a time 
                        read_data(toa_af_red_masked_fullpath, line, sample, &af); // returns value of 1 pixel at a time 
                        read_data(toa_ba_red_masked_fullpath, line, sample, &ba); // returns value of 1 pixel at a time 
                        read_data(toa_bf_red_masked_fullpath, line, sample, &bf); // returns value of 1 pixel at a time 
                        read_data(toa_da_red_masked_fullpath, line, sample, &da); // returns value of 1 pixel at a time 
                        read_data(toa_df_red_masked_fullpath, line, sample, &df); // returns value of 1 pixel at a time 


                        // printf("CHECK all 9 cameras: ca= %f, an= %f, cf= %f\n\n" , ca, an, cf);
                        //***********************************************************
                        //  to process cloud mask files- should change the fields depending on the cloudmask product

                        if (cloudMask_runMode) // if it is turned on...
                        {
                            printf("c: reading cloud mask file... \n");
                            read_data_cloudmask(cloudmask_fname_fullpath, line, sample, &cm); // cloud mask
                            // printf("c: cm is: %d \n", cm); // cm dtype: double FS is %lf shows zero which is wrong, but better to show with %e == scientific E notation to visually check a very small number.



                            /* new algorithm based on Stereo SDCM and RCCM; for both we stract the pixel value of cloudiness from cloudmask product and add it to the cloud field */ 

                            trainingDS_dataStruct[total_trainingDS_row_in_mem].cloud = cm; // we turn off cloud == no cloud


                            //#########################################################################################
                            /* this section is only for ASCM*/

                            // if (cm == 1)
                            // {  // E: clear pixel, so we turn off cloud parameter
                            //     // printf("c: clear pixel (cm=1) \n");
                            //     trainingDS_dataStruct[total_trainingDS_row_in_mem].cloud = 0; // we turn off cloud == no cloud
                            // }
                            // else if (cm == 0)
                            // {  // cloudy pixel, so we turn on cloud
                            
                            //     // printf("c: cloudy pixel (cm=0) \n");
                            //     // if (cm == CMASKED) trainingDS_dataStruct[total_trainingDS_row_in_mem].cloud = 1; // E: turn on cloud
                            //     trainingDS_dataStruct[total_trainingDS_row_in_mem].cloud = 1; // E: turn on cloud == there is cloud, what is CMASKED?
                            // }
                            // else
                            // { // we have filled value from cloudmask.c code that I set to -9, so we set that pixel as noValue. we do not have any other value expect 0 and 1
                            
                            //     trainingDS_dataStruct[total_trainingDS_row_in_mem].cloud = -1; // fill value
                            // }

                            //#########################################################################################

                          
                        }

                        // add all 9 cameras + 4 bands to training dataset here
                        trainingDS_dataStruct[total_trainingDS_row_in_mem].anr = anr;
                        trainingDS_dataStruct[total_trainingDS_row_in_mem].ang = ang;
                        trainingDS_dataStruct[total_trainingDS_row_in_mem].anb = anb;
                        trainingDS_dataStruct[total_trainingDS_row_in_mem].annir = annir;
                        trainingDS_dataStruct[total_trainingDS_row_in_mem].ca = ca;
                        trainingDS_dataStruct[total_trainingDS_row_in_mem].cf = cf;
                        trainingDS_dataStruct[total_trainingDS_row_in_mem].aa = aa;
                        trainingDS_dataStruct[total_trainingDS_row_in_mem].af = af;
                        trainingDS_dataStruct[total_trainingDS_row_in_mem].ba = ba;
                        trainingDS_dataStruct[total_trainingDS_row_in_mem].bf = bf;
                        trainingDS_dataStruct[total_trainingDS_row_in_mem].da = da;
                        trainingDS_dataStruct[total_trainingDS_row_in_mem].df = df;
                        // rest of data fields here
                        trainingDS_dataStruct[total_trainingDS_row_in_mem].npts = weight;
                        trainingDS_dataStruct[total_trainingDS_row_in_mem].rms = weight * xrms;
                        trainingDS_dataStruct[total_trainingDS_row_in_mem].var = weight * xrms * xrms;

                        total_trainingDS_row_in_mem++ ; // increment for every fileObj element
                        // printf("training dataset row in memory updates: %d\n", total_trainingDS_row_in_mem); // similar to: "FOUND a new ATM point (row/sample), will add it to trainingDS_dataStruct now...\n"
                    }

                    atm_row_num++; // new while iteration
                } // while reading each ATM line

                fclose(fp); // fp = pointer to each ATM file closed 

            }   // for each orbit num
        } // for ach K-day
    } // for each ATM line number


    /* end- read all available ATM file from the list we made in the past section */



    // /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    printf("\n************************* preparing output file *************************\n\n");
    // q- what happens here that reduces the rows?

    double max_npts = -1e23;
    double max_rms = -1e23;
    double min_rms = 1e23;

    //fp = fopen(atmfile, "w");
    filePtr = fopen(atmmodel_csvfile, "w"); // create and open a csv file to write into it; returns the ptr
    // printf("%p" , &filePtr);
    // I added here to check opening of csv file.
    if (filePtr == NULL)
    {
       printf("ERROR: Could not open atmmodel_csvfile. Either dirPath or output file has problem. Exiting.\n" );
       exit(1);
    } 
    else
    {
        printf("atmmodel_csvfile (our output) file opened successfully! \n");
    }

    printf("writing data into output file... \n");
    printf("total rows in training dataset (in-mem dataStruct)= %d after checking all ATM files, k days, orbits.\n", total_trainingDS_row_in_mem);
    
    // cloud_pts = 0;
    // nocloud_pts = 0;
    // misscloud_pts = 0;

    cloud_x = 0;
    nocloud_x = 0;
    misscloud_x = 0;
    orbit_x = 0;  // what is this?



    // write this line as the header/1st line of output file- note: no space between commas (,,,) in column labels
    fprintf(filePtr, "path,orbit,img_block,line,sample,firstLat,firstLon,anr,ang,anb,annir,aa,af,ba,bf,ca,cf,da,df,rms,weight,npts,cloud,var\n"); 

    // q- what happens here that reduces the rows?
    //***************************************************************************************
    for (n = 0; n < total_trainingDS_row_in_mem; n++)
    {  // for each row in training dataste- size of rows in trainingDS_dataStruct == num of points= the MISR pixels that ATM found for them 
        
        trainingDS_dataStruct[n].rms /= trainingDS_dataStruct[n].npts; // to calculate averaged weighted roughness from total rms falling inside each MISR pixel
        trainingDS_dataStruct[n].var = sqrt(trainingDS_dataStruct[n].var / trainingDS_dataStruct[n].npts - trainingDS_dataStruct[n].rms * trainingDS_dataStruct[n].rms);

        if (trainingDS_dataStruct[n].anr > 0)
        { // Q- why an camera is checked? can camera be negative? surf refl > 0
        
            natm_valid++; // increment- valid==number of pixels with anr > 0
            avg_valid_rms += trainingDS_dataStruct[n].rms; // total roughness of every valid pixel/element; Q- wby we vheck valid refl value?
            if (trainingDS_dataStruct[n].rms > max_rms) max_rms = trainingDS_dataStruct[n].rms; // Q- can roughness be negative? and minus? // set the max roughness value
            if (trainingDS_dataStruct[n].rms < min_rms) min_rms = trainingDS_dataStruct[n].rms; // set min roughness value
            if (trainingDS_dataStruct[n].weight == 0.5) natm_half_weight++;
        }

        avg_rms += trainingDS_dataStruct[n].rms; // sum with initial value

        if (trainingDS_dataStruct[n].npts > max_npts) max_npts = trainingDS_dataStruct[n].npts; // collect num of points

        // old:
        //fprintf(fp, "%d, %d, %d, %d, %d, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf\n", trainingDS_dataStruct[n].path, trainingDS_dataStruct[n].orbit, trainingDS_dataStruct[n].img_block, trainingDS_dataStruct[n].line, trainingDS_dataStruct[n].sample, trainingDS_dataStruct[n].lat, trainingDS_dataStruct[n].lon, trainingDS_dataStruct[n].an, trainingDS_dataStruct[n].ca, trainingDS_dataStruct[n].cf, trainingDS_dataStruct[n].rms, trainingDS_dataStruct[n].weight, trainingDS_dataStruct[n].npts);
        
        /* E: why check this condition? for cloudy? */ // Ehsan turned off 
        // if ((trainingDS_dataStruct[n].cloud == 0) || (trainingDS_dataStruct[n].cloud == 1) || 
        //     ((trainingDS_dataStruct[n].cloud == -1) && (trainingDS_dataStruct[n].anr > 0) && (trainingDS_dataStruct[n].ca > 0) && (trainingDS_dataStruct[n].cf > 0))) {

        //**************************** useless! turned off ****************************
        // if (orbit_x == 0){ // Q-why zero?
        //     // printf("path, orbit, img_block, weight, cloud, nocloud, misscloud, orbit_x= %d\n", orbit_x);
        //     // printf("orbit_x= %d\n", orbit_x);
        // }

        // int misr_orbit = trainingDS_dataStruct[n].orbit;  
        // printf("misr_orbit= %d \n" , misr_orbit);

        // if (misr_orbit != orbit_x){ // if misr_orbit not zero, GO- Ehsan: useless! turned off
        //     path_x = trainingDS_dataStruct[n].path;
        //     // printf("path_x 3: %d \n" , path_x);

        //     // orbit_x = trainingDS_dataStruct[n].orbit;
        //     // block_x = trainingDS_dataStruct[n].img_block;
        //     // weight_x = trainingDS_dataStruct[n].weight;
        //     // cloud_x = 0;
        //     // nocloud_x = 0;
        //     // misscloud_x = 0;
        // }
        //**************************** useless! turned off ****************************


        // setup cloud variable/ for stats/report

        // if (trainingDS_dataStruct[n].cloud == 0)
        // { // E: no-cloud pixel, cloud var is off==0 based before, count nocloud pixels
        //     nocloud_pts += 1;
        //     // nocloud_x += 1;
        // }
        
        // if (trainingDS_dataStruct[n].cloud == 1)
        // { // E: cloudy pixel, cloud var is on==1, count cloudy pixels
        //     cloud_pts += 1;
        //     // cloud_x += 1;
        // }
        
        // if (trainingDS_dataStruct[n].cloud == -1)
        // { // miss-cloud == case with no CloudMask pixels, and also filling values
        //     misscloud_pts += 1;
        //     // misscloud_x += 1;
        // }
        //########################################


        // (file-print-format) == pointer to atmmodel_csvfile file- writes trainingDS_dataStruct to a file on disc
        fprintf(filePtr, "%d, %d, %d, %d, %d, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %d, %lf\n", 
            trainingDS_dataStruct[n].path, 
            trainingDS_dataStruct[n].orbit, 
            trainingDS_dataStruct[n].img_block, 
            trainingDS_dataStruct[n].line, 
            trainingDS_dataStruct[n].sample, 
            trainingDS_dataStruct[n].lat, 
            trainingDS_dataStruct[n].lon, 
            trainingDS_dataStruct[n].anr, 
            trainingDS_dataStruct[n].ang, 
            trainingDS_dataStruct[n].anb, 
            trainingDS_dataStruct[n].annir, 
            trainingDS_dataStruct[n].aa, 
            trainingDS_dataStruct[n].af, 
            trainingDS_dataStruct[n].ba, 
            trainingDS_dataStruct[n].bf, 
            trainingDS_dataStruct[n].ca, 
            trainingDS_dataStruct[n].cf, 
            trainingDS_dataStruct[n].da, 
            trainingDS_dataStruct[n].df, 
            trainingDS_dataStruct[n].rms, 
            trainingDS_dataStruct[n].weight, 
            trainingDS_dataStruct[n].npts, 
            trainingDS_dataStruct[n].cloud, 
            trainingDS_dataStruct[n].var); // 24 columns
    }

    // comment at the end of the file
    // fprintf(filePtr, "the end of calculations\n");

    // // flush the buffer memory
    // fflush(filePtr);

    //fclose(fp);
    printf("closing the atmmodel file pointer to flush the buffer to disk\n"); 
    fclose(filePtr);            // close pointer to atmmodel.csv & flushes/moves the output from buffer(==temporary storage) to disk


    // // //close memory assigned to training dataset
    // printf("free memory assigned to training dataset here\n");
    free(trainingDS_dataStruct); 
    

    avg_rms /= total_trainingDS_row_in_mem; // Q- why?
    avg_valid_rms /= natm_valid;
    printf("********************************************************\n");
    printf("Number of Total ATM rms points = %d\n", total_trainingDS_row_in_mem);
    printf("Number of Valid (with AN-red positive) ATM rms points = %d\n", natm_valid); // valid???
    printf("Number of Valid ATM rms with weight of 1.0 (ATM overpass today) = %d\n", natm_valid - natm_half_weight);
    printf("Number of Valid ATM rms with weight of 0.5 (ATM overpass yesterday/tomorrow) = %d\n", natm_half_weight);
    printf("Total Average rms = %lf from max_npts= %lf\n", avg_rms, max_npts);
    printf("Average valid rms = %lf\n", avg_valid_rms);
    printf("Max valid rms = %lf\n", max_rms);
    printf("Min valid rms = %lf\n", min_rms);
    printf("\n");

    // /* this is just for stats/report- turned it off */
    // printf("Number of cloudy pixels = %d\n", cloud_pts);
    // printf("Number of nocloud pixels = %d\n", nocloud_pts);
    // printf("Number of missing cloud mask pixels = %d\n", misscloud_pts);


    printf("\n***** FINISHED SUCCESSFULLY!***** \n\n");

    return 0;
}
