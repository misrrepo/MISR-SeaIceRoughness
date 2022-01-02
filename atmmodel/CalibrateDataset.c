/* 
Ehsan Jan 15, 2020
originally from dir: /home/mare/Nolin/SeaIce/Code/C
name: atm_to_misr_pixels.c
usage: labels MISR pixels w/ATM roughness data

1- makes a list of all available ATM.csv files
2- 
3- outputs atmmodel

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

typedef struct {
    int path;
    int orbit;
    int img_block;
    int line;
    int sample;
    double npts;
    double lat;
    double lon;
    double an;
    double ca;
    double cf;
    double rms;
    float weight;
    int8_t cloud; // E: maybe change to int8? cuz range={-1,0,1}
    double var;
} atm_dtype;

MTKt_status status;
atm_dtype* ATM_dataStruct;

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

int read_data_cloudmask(char* fname_fullpath, int line, int sample, uint8_t* data) {
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

int main(int argc, char *argv[]) {

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
    // char atmmodel_csvfile[256] = "/Users/ehsanmos/Documents/RnD/MISR_lab/misr_processing_dir/atmmodel_csvfile_July_2011/Ehsan_July_2011_atmmodel.csv" ; // ERROR: how check if a file or directory exists in Clang? check ptr?
    // char atmmodel_csvfile[256] = "/Volumes/Ehsan7757420250/2016/april_2016/atmmodel/atmmodel_april_2016.csv" ; // writes output=atmmodel_csvfile to current dir
    char atmmodel_csvfile[256];
    // char atmmodel_csvfile[256] = "/home/mare/Projects/MISR/Julienne/IceBridge2016/SeaIce_Jul2016_atmmodel_csvfile_cloud_var.csv"; // old
    //char lsmask_dir[256] = "/home/mare/Nolin/SeaIce/LWData/MISR_LandSeaMask";

    //------------------------------------------------------------------------------------------------------------------
    // check number of inouts to this code from commandLine (from python wrapper)
    if (argc == 5) {
        printf("C: OK! received 5 arguments. \n");
    }

    if (argc != 5) { // this might happen later, cos I turnedoff fname[2]
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
    printf("atmmodel-csvfilem: %s \n" , atmmodel_csvfile);

    //------------------------------------------------------------------------------------------------------------------
    DIR *dirp;
    FILE *fp, *filePtr;
    struct dirent* FileEntryPtr;
    char message[256];
    char idir[256];
    char an_file[128];
    char atm_fname_fullpath[256];
    char toa_ca_masked_fullpath[256];
    char toa_cf_masked_fullpath[256];
    char toa_an_masked_fullpath[256];
    char cloudmask_fname_fullpath[256];
    //char lsmask_atm_fname_fullpath[256];
    //unsigned char lsmask;
    char xfile[256];
    char **atm_fileList = 0;
    char syear[5];  // size to define is: total elements = 4+1 --> +1 for null character that marks the end of the string
    char smonth[3];     // size to define is: total elements = 2+1 --> +1 for null character that marks the end of the string
    char sday[3];   // size to define is: total elements = 2+1 --> +1 for null character that marks the end of the string
    char ATMStartTime[32];
    char ATMEndTime[32];
    char str[16];
    char *token;
    char *line_in_buff = NULL;
    int* orbitlist;
    int orbit_count;
    int month;
    int day;
    int path;
    int atm_nfiles_index = 0;
    int misr_nfiles = 0;
    int atm_DS_row = 0;
    int i, j, k, ATM_DStruct_row, n, w;
    int img_block;
    int previous_atm_in_pixel;
    int natm_half_weight = 0;
    int natm_valid = 0;
    //int monthday[12][31] = {0};
    double avg_rms = 0;
    double avg_valid_rms = 0;
    double weight;
    double ca, cf, an; // , cm; // note: cm dtype should be double cuz inside read_data f(.) it is defined as double and this f(.) is used to read several files
    // double cm; // = -1;
    uint8_t cm; // dtype= maybe to uint8_t??? cuz cm is either= 0, 1

    double xlat, xlon, xrms; //, xcam;
    int xcam;
    float fline, fsample;
    int line, sample;
    int nocloud_pts, cloud_pts, misscloud_pts;
    int path_x, orbit_x, block_x;
    double weight_x;
    int nocloud_x, cloud_x, misscloud_x;
    size_t slen = 0;
    // ssize_t read;
    ssize_t line_size;
    int ATMnewLine = 0;

    // E- if we use cloud mask
    int cloudMask_run_stat = 0; // 0 == turn off cloud mask

    /*////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
    /* Get list of all available ATM.csv files available in directory */
    printf("\n*********** PROGRAM: ATMRoughness2MISRPixels.c ***********\n\n");
    printf("making list of ATM.csv files...\n");

    dirp = opendir(atm_dir); // create a streem 
    if (dirp){
    	while ((FileEntryPtr = readdir(dirp)) != NULL) { // num of iterations == num of ATM files available == atm_nfiles_index == FileEntryPtr is ptr to fileObj, we create it for every iteration = ATM csv file available
		    // if (strstr(FileEntryPtr->d_name, "combine")) continue; // d_name is string of fileName in fileObj
		    // if (strstr(FileEntryPtr->d_name, "SeaIce")) continue;
            if (! strstr(FileEntryPtr->d_name, "ILATM2")) {
                // printf("dirEntry does not start w/ ILATM2, skipping it ... \n");
                continue;
            }
            if (strstr(FileEntryPtr->d_name, ".xml")) {
                // printf("dirEntry ends w/ .xml , skipping it ... \n");
                continue;
            }
            if (! strstr(FileEntryPtr->d_name, ".csv")) {
                // printf("dirEntry does not end w/ .csv , skipping it ... \n");
                continue;
            }

		    if (atm_fileList == 0){
                atm_fileList = (char **) malloc(sizeof(char *));
                if (!atm_fileList){
                    printf("main: couldn't malloc atm_fileList\n");
                    return 0;
                }
            }
            else{
                atm_fileList = (char **) realloc(atm_fileList, (atm_nfiles_index + 1) * sizeof(char *));
                if (!atm_fileList){
                    printf("getFileList: couldn't realloc atm_fileList\n");
                    return 0;
                }
            }

            atm_fileList[atm_nfiles_index] = (char *) malloc(strlen(FileEntryPtr->d_name) + 1);

            if (!atm_fileList[atm_nfiles_index]) {
                printf("main: couldn't malloc atm_fileList[%d]\n", atm_nfiles_index);
                return 0;
            }

            strcpy(atm_fileList[atm_nfiles_index], FileEntryPtr->d_name); // fill the list with available ATM files
            atm_nfiles_index ++;
        }
    	closedir (dirp); // close the stream
    }
    else{
        strcat(message, "Can't open ATM.csv file\n");
        strcat(message, atm_dir);
        perror (message);
        return EXIT_FAILURE;
    }

    printf("----------------------------------------------------------------------------------------------\n");
    printf("number of ATM.csv (ILATM2) files found= (%d) \nin direcotry: %s \n" , atm_nfiles_index, atm_dir);
    printf("\n");
    // Get list of available ATM csv files /////////////////////////////////////////////////////////////////////////////


    //for (i = 0; i < atm_nfiles_index; i++) {
	//printf("%d %s\n", i, atm_fileList[i]);
    //}


    /*////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
    /* read all available ATM file from the list we made in the past section */

    for (int i = 0; i < atm_nfiles_index; i++)
    { // i = num of available ATM files in the list
        printf("\n############################## process a new ATM nfile ##############################\n");
        printf("ATM file: (%d/%d) --> %s \n\n" , i+1, atm_nfiles_index, atm_fileList[i]);
        
        // printf("syear1: %s \n" , syear);
        // printf("c- sizeof(syear): %lu \n" , sizeof(syear)) ;

        memset(syear, '\0', sizeof(syear));  // why we do this? to set/clean the memory assgined to it before writing into it again. fill the array with null character.
        // printf("syear1: %s \n" , syear);

        // # Filename:   ILATM2_20110316_124420_smooth_nadir3seg_50pt.csv

        strncpy(syear, (atm_fileList[i] + 7), 4); // get year from file name; why not in ptr format?
        // printf("syear2: %s \n" , syear);


        // char* yearCopy = strdup(syear); // Pouya replaced every syear with yearCopy

        // printf(smonth);
        memset(smonth, '\0', sizeof(smonth));
        strncpy(smonth, (atm_fileList[i] + 11), 2); // get month from file name; how?
        // printf("smonth: %s \n" , smonth);

        month = atoi(smonth);

        memset(sday, '\0', sizeof(sday));
        strncpy(sday, (atm_fileList[i] + 13), 2); // get dat from file name
        // printf("sday: %s \n" , sday);

        // extracted this info from each ATM file name
        // printf("date info= yr: %s; mon: %d; day: %s \n", syear, month, sday);

        //-------------
        for (k  = -1; k < 2; k++) { // k=days; yesterday (-1) or tmrw (+1) == 0.5 of the ATM overpass; today=o

            // printf("\nprocess for k-day: %d \n", k);
            day = atoi(sday) + k;
            // printf("day= %d \n" , day);

            // to use for MTK function
            sprintf(ATMStartTime, "%s-%02d-%02dT00:00:00Z", syear, month, day); // start time
            sprintf(ATMEndTime, "%s-%02d-%02dT23:59:59Z", syear, month, day); // end time
            // printf("ATM time= %s, %s, %s\n", atm_fileList[i], ATMStartTime, ATMEndTime);
            // printf("ATM period= %s -- to -- %s\n", ATMStartTime, ATMEndTime);


            if (k == 0) weight = 1.0; // for today k=0; w=1 of the ATM overpass;
            else weight = 0.5; // yesterday or tmrw; weight=0.5 of the ATM overpass
            //if (monthday[month][day] == 0) {
            status = MtkTimeRangeToOrbitList(ATMStartTime, ATMEndTime, &orbit_count, &orbitlist); // outputs are orbitCount and list
            // printf("MtkTimeRangeToOrbitList status: %d \n" , status) ;
            // printf("C -> default MTK_SUCCESS: %d \n"  , MTK_SUCCESS) ;

            if (status != MTK_SUCCESS) {
                printf("ERROR: returned from MtkTimeRangeToOrbitList \n");
                return 1;
            }

            /*check orbit list*/
            // int i_orbit;
            // // printf("ATM period= %s -- to -- %s\n", ATMStartTime, ATMEndTime);
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

            //---------------------
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
                sprintf(atm_fname_fullpath, "%s/%s", atm_dir, atm_fileList[i]); 

                fp = fopen(atm_fname_fullpath, "r"); // create stream = fp for ATM file == open ATM file
                if (!fp) 
                {
                    fprintf(stderr, "main: couldn't open ATM file: %s \n", atm_fname_fullpath);
                    return 1;
                }


                // START PROCESSING EACH ROW OF ATM.csv FILE

                //printf("\n______open ATM fileNum(%d), orbit/path(%d), in day_k(%d), WhileLoop each sample/row info from: %s \n\n", i, path, k, atm_fname_fullpath); // the ATM file
                int atm_row_num = 1;
                // now associate each ATM row/sample to 3 MISR surf files
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

                    // 1- preparing An camera
                    sprintf(toa_an_masked_fullpath, "%s/An/masked_toa_refl_P%03d_O%06d_B%03d_an.dat", masked_toa_refl_home, path, orbitlist[j], img_block);
                    // check if file is accessible 
                    if (access(toa_an_masked_fullpath, F_OK) == -1) 
                    {
                        // printf("WARNING-1: input NOT exist: continue to next ATM row: toa-an: %s\n" , toa_an_masked_fullpath);
                        continue; 
                    }

                    // 2- preparing Cf camera
                    sprintf(toa_cf_masked_fullpath, "%s/Cf/masked_toa_refl_P%03d_O%06d_B%03d_cf.dat", masked_toa_refl_home, path, orbitlist[j], img_block);
                    // printf("toa cf: %s\n" , toa_cf_masked_fullpath);
                    // check if file is accessible 
                    if (access(toa_cf_masked_fullpath, F_OK) == -1) 
                    {
                        // printf("WARNING-2: input NOT exist: toa_cf_masked_fullpath, continue to next ATM row.\n");
                        continue; // check if file is accessiblem
                    }

                    // 3- preparing Ca camera
                    sprintf(toa_ca_masked_fullpath, "%s/Ca/masked_toa_refl_P%03d_O%06d_B%03d_ca.dat", masked_toa_refl_home, path, orbitlist[j], img_block);
                    // printf("toa ca: %s\n" , toa_ca_masked_fullpath);
                    // check if file is accessible 
                    if (access(toa_ca_masked_fullpath, F_OK) == -1) 
                    {
                        // printf("WARNING-3: input NOT exist: toa_ca_masked_fullpath, continue to next ATM row.\n");
                        continue; // check if file is accessiblem
                    }

                    // 4- now check cloudMask file
                    if (!cloudMask_run_stat) // if cloudMask is off
                    { // go here
                        printf("c: NOTE: cloudMask is off == we do not use cloud mask anymore! \n");
                    } 
                    else 
                    {
                        // sprintf(cloudmask_fname_fullpath, "%s/An/sdcm_p%03d_o%06d_b%03d_an.dat", cloud_masked_dir, path, orbitlist[j], img_block); // original=lsdcm =? note: to do a test run, i renamed the filename from lsdcm_p* to sdcm_p* 
                        sprintf(cloudmask_fname_fullpath, "%s/cloudmask_P%03d_O%06d_B%03d.msk", cloud_masked_dir, path, orbitlist[j], img_block); // original=lsdcm =? note: to do a test run, i renamed the filename from lsdcm_p* to sdcm_p* 
                        // check if file is accessible 
                        if (access(cloudmask_fname_fullpath, F_OK) != 0) 
                        {
                            printf("C: cloudMask NOT exist: %s\n", cloudmask_fname_fullpath);
                            // continue; // check if file is accessible
                            // cloudMask_run_stat = 0;
                        }
                        else
                        {
                            // printf("c: cloudMask EXIST: %s \n" , cloudmask_fname_fullpath);
                        }
                    }

                    // printf("c: preparing input files for: %d, %d, %d, %d, %d, %f, %f, %f\n", path, orbitlist[j], img_block, line, sample, xlat, xlon, xrms); // for each csv row == label info

                    /*////////////////////////////////////////////////////////////////////////////////////////////////*/
                    /* now try to find MISR pixels that each ATM roughness falls into it and then sum all roughness values */

                    /* we assume there is not any previous ATM points in MISR pixel, 
                        a switch to check if every new ATM point falls inside a previous pixel, reset to zero for every new entry=ATM line */
                    previous_atm_in_pixel = 0; 

                    if (atm_DS_row == 0) 
                    {
                        // 1st allocate mem- for each row of csv file
                        ATM_dataStruct = (atm_dtype *) malloc(sizeof(atm_dtype));
                    }
                    else 
                    {      /* for 2nd atm_DS_row and the rest */
                        ATM_DStruct_row = 0; // counter of variables/row in ATM_dataStruct 
                        while ((ATM_DStruct_row < atm_DS_row) && !previous_atm_in_pixel) 
                        { // checks new ATM point with previous rows in data= all n points inside ATM_dataStruct until pixel is found

                            /* first check if there are previous ATM points inside MISR pixel so that we average new value w/ previous values
                                we compare new MISR path,img_block,line,sample (associated with ATM point/row) with every n point available in fileObj;
                                if even one similar MISR pixel was found, we will sum&update ATM info: rms, npts, and other info to previous pixel values in fileObj */
                            if ((ATM_dataStruct[ATM_DStruct_row].path == path) && (ATM_dataStruct[ATM_DStruct_row].img_block == img_block) && (ATM_dataStruct[ATM_DStruct_row].line == line) && (ATM_dataStruct[ATM_DStruct_row].sample == sample) && (ATM_dataStruct[ATM_DStruct_row].weight == weight)) 
                            {   // Q- what is this condition? why check to be the same? in same day in same pixel????
                                
                                // printf("c: FOUND previous ATM points in a MISR pixel: summing with previous ATM values ...\n");
                                //printf(">>> FOUND: ATM in MISR pixel >>> day: (%d), path: (%d), img_block: (%d), line: (%d), sample: (%d)\n\n", k, path, img_block, line, sample);
                                ATM_dataStruct[ATM_DStruct_row].rms += weight * xrms; // sum of weighted ATM roughness in the same pixel?
                                ATM_dataStruct[ATM_DStruct_row].npts += weight; // sum of num of points in the same pixel?
                                ATM_dataStruct[ATM_DStruct_row].var += weight * xrms * xrms; // sum of what???? variance?
                                previous_atm_in_pixel = 1; // when we find the first ATM point in pixel == turns on here == 1 and skip while-loop
                            }
                            
                            ATM_DStruct_row++ ; // iterate to next point in file Obj
                        }

                        /* if ATM point was the first point in that MISR pixel (not found in previous MISR pixels so far) we allocate mem- for the new point in ATM_dataStruct, basically ATM_dataStruct grows a row  */
                        if (!previous_atm_in_pixel) 
                        {
                            // printf("need to add a row in ATM_dataStruct for new ATM point! \n");
                            ATM_dataStruct = (atm_dtype * ) realloc(ATM_dataStruct, (atm_DS_row + 1) * sizeof(atm_dtype));
                        }
                    }

                    /* check if mem- is allocated */
                    if (!ATM_dataStruct) 
                    {
                        fprintf(stderr,  "main: couldn't malloc/realloc ATM_dataStruct\n");
                        return 0;
                    }

                    /* we run this cuz the new ATM xlat/xlon was not found in previous MISR pixels, we add it as new dataPoint row in ATM_dataStruct */
                    if (!previous_atm_in_pixel) { /* if we could not find any MISR pixel associated with ATM point ==  previous_atm_in_pixel=0 */
                        
                        // printf("c: FOUND a new ATM point (row/sample), will add it to ATM_dataStruct\n");
                        ATM_dataStruct[atm_DS_row].path = path;
                        ATM_dataStruct[atm_DS_row].orbit = orbitlist[j];
                        ATM_dataStruct[atm_DS_row].img_block = img_block;
                        ATM_dataStruct[atm_DS_row].line = line;
                        ATM_dataStruct[atm_DS_row].sample = sample;
                        ATM_dataStruct[atm_DS_row].lat = xlat;
                        ATM_dataStruct[atm_DS_row].lon = xlon;
                        ATM_dataStruct[atm_DS_row].weight = weight;
                        ATM_dataStruct[atm_DS_row].cloud = -1; // Q- how to interpret it? is filling value?

                        read_data(toa_cf_masked_fullpath, line, sample, &cf); // returns value of 1 pixel at a time  // int read_data(char* atm_fname_fullpath, int line, int sample, double* data)
                        // printf("PROBLEM: cf= %f\n" , cf); // problem here: why all zero? cuz all input files were not defined correctly!

                        read_data(toa_ca_masked_fullpath, line, sample, &ca);
                        // printf("PROBLEM: ca= %f\n" , ca); // problem here: why all zero?

                        read_data(toa_an_masked_fullpath, line, sample, &an);
                        // printf("PROBLEM: an= %f\n" , an); // problem here: why all zero?

                        // printf("CHECK: ca= %f, an= %f, cf= %f\n\n" , ca, an, cf);

                        //  to process cloud mask files
                        if (cloudMask_run_stat) 
                        {
                            // printf("c: reading cloud mask file... \n");
                            read_data_cloudmask(cloudmask_fname_fullpath, line, sample, &cm); // cloud mask
                            // printf("c: cm is: %d \n", cm); // cm dtype: double FS is %lf shows zero which is wrong, but better to show with %e == scientific E notation to visually check a very small number.

                            if (cm == 1)  // E: clear pixel, so we turn off cloud parameter
                            { 
                                // printf("c: clear pixel (cm=1) \n");
                                ATM_dataStruct[atm_DS_row].cloud = 0; // we turn off cloud == no cloud
                            }
                            else if (cm == 0 )  // cloudy pixel, so we turn on cloud
                            {
                                // printf("c: cloudy pixel (cm=0) \n");
                                // if (cm == CMASKED) ATM_dataStruct[atm_DS_row].cloud = 1; // E: turn on cloud
                                ATM_dataStruct[atm_DS_row].cloud = 1; // E: turn on cloud == there is cloud, what is CMASKED?
                            }
                            else // we have filled value from cloudmask.c code that I set to -9, so we set that pixel as noValue. we do not have any other value expect 0 and 1
                            {
                                ATM_dataStruct[atm_DS_row].cloud = -1; // fill value
                            }
                        }

                        // printf("c: checkpoint2 \n");

                        ATM_dataStruct[atm_DS_row].an = an;
                        ATM_dataStruct[atm_DS_row].ca = ca;
                        ATM_dataStruct[atm_DS_row].cf = cf;
                        ATM_dataStruct[atm_DS_row].npts = weight;
                        ATM_dataStruct[atm_DS_row].rms = weight * xrms;
                        ATM_dataStruct[atm_DS_row].var = weight * xrms * xrms;

                        atm_DS_row++ ; // increment for every fileObj element
                        //printf("atm_DS_row updates.........................\n");
                    }

                    atm_row_num++; // new while iteration
                }

                fclose(fp); // fp = ATM file closed //printf("*** close csv and go to next orbit\n");

            }   // for each orbit num
        }
    }




    // /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    printf("\n************************* preparing output file *************************\n\n");

    double max_npts = -1e23;
    double max_rms = -1e23;
    double min_rms = 1e23;
    //fp = fopen(atmfile, "w");
    filePtr = fopen(atmmodel_csvfile, "w"); // create and open a csv file to write into it; return the ptr
    // printf("%p" , &filePtr);
    // I added here to check opening of csv file.
    if (filePtr == NULL) 
    {
       printf("ERROR: Could not open atmmodel_csvfile. Either dirPath or output file has problem. Exiting.\n" );
       exit(1);
    } 
    else 
    {
        printf("\natmmodel_csvfile (output) file opened successfully! \n");
    }

    printf("\nwriting data into output file... \n");
    printf("\nnumber of variables/rows in ATM_DataStruct= %d after checking all ATM files, k days, orbits.\n", atm_DS_row);
    
    cloud_pts = 0;
    nocloud_pts = 0;
    misscloud_pts = 0;
    cloud_x = 0;
    nocloud_x = 0;
    misscloud_x = 0;
    orbit_x = 0;  // what is this?

    
    fprintf(filePtr, "#path, orbit, img_block, line, sample, lat, lon, an, ca, cf, rms, weight, npts, cloud, var\n"); // write this line as the header/1st line of output file
    // printf("check seg fault-4 \n");

    for (n = 0; n < atm_DS_row; n++) 
    {  // num of points= the MISR pixels that ATM found for them == size of elements in ATM_dataStruct
        
        ATM_dataStruct[n].rms /= ATM_dataStruct[n].npts; // average weighted roughness // Q- ATM_dataStruct is for each what? pixel? or
        ATM_dataStruct[n].var = sqrt(ATM_dataStruct[n].var / ATM_dataStruct[n].npts - ATM_dataStruct[n].rms * ATM_dataStruct[n].rms);

        if (ATM_dataStruct[n].an > 0) 
        {    // Q- why an camera is checked? can camera be negative? surf refl > 0
            
            natm_valid++; // increment
            avg_valid_rms += ATM_dataStruct[n].rms; // sum roughness of every valid pixel/element; Q- wby we vheck valid refl value?
            if (ATM_dataStruct[n].rms > max_rms) max_rms = ATM_dataStruct[n].rms; // Q- can roughness be negative? and minus? // set the max roughness value
            if (ATM_dataStruct[n].rms < min_rms) min_rms = ATM_dataStruct[n].rms; // set min roughness value
            if (ATM_dataStruct[n].weight == 0.5) natm_half_weight++;
        }

        avg_rms += ATM_dataStruct[n].rms; // sum with initial value

        if (ATM_dataStruct[n].npts > max_npts) max_npts = ATM_dataStruct[n].npts; // collect num of points

        //fprintf(fp, "%d, %d, %d, %d, %d, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf\n", ATM_dataStruct[n].path, ATM_dataStruct[n].orbit, ATM_dataStruct[n].img_block, ATM_dataStruct[n].line, ATM_dataStruct[n].sample, ATM_dataStruct[n].lat, ATM_dataStruct[n].lon, ATM_dataStruct[n].an, ATM_dataStruct[n].ca, ATM_dataStruct[n].cf, ATM_dataStruct[n].rms, ATM_dataStruct[n].weight, ATM_dataStruct[n].npts);
        
        /* E: why check this condition? for cloudy? */
        if ((ATM_dataStruct[n].cloud == 0) || 
            (ATM_dataStruct[n].cloud == 1) || 
            ((ATM_dataStruct[n].cloud == -1) && (ATM_dataStruct[n].an > 0) && (ATM_dataStruct[n].ca > 0) && (ATM_dataStruct[n].cf > 0))) {
        
            if (orbit_x == 0) 
            { // Q-why zero?
                // printf("path, orbit, img_block, weight, cloud, nocloud, misscloud, orbit_x= %d\n", orbit_x);
                // printf("orbit_x= %d\n", orbit_x);
            }

            int atm_orbit = ATM_dataStruct[n].orbit;
            // printf("atm_orbit= %d \n" , atm_orbit);

            if (atm_orbit != orbit_x) 
            { // if atm_orbit not zero, GO

                // printf("atm_orbit= %d, orbit_x= %d \n" , atm_orbit, orbit_x);


                // printf("path_x 1: %d \n" , path_x);
                // printf("\n%d, %d, %d, %lf, %d, %d, %d\n", path_x, orbit_x, block_x, weight_x, cloud_x, nocloud_x, misscloud_x); // Q- whats x?
                // printf("path_x 2: %d \n" , path_x);


                path_x = ATM_dataStruct[n].path;
                // printf("path_x 3: %d \n" , path_x);

                orbit_x = ATM_dataStruct[n].orbit;
                block_x = ATM_dataStruct[n].img_block;
                weight_x = ATM_dataStruct[n].weight;
                cloud_x = 0;
                nocloud_x = 0;
                misscloud_x = 0;
            }

            // printf("path_x 4: %d \n" , path_x);


            // setup cloud variable            
            if (ATM_dataStruct[n].cloud == 0) 
            { // E: no-cloud pixel, cloud var is off==0 based before, count nocloud pixels
                nocloud_pts += 1;
                nocloud_x += 1;
            }
            if (ATM_dataStruct[n].cloud == 1) 
            { // E: cloudy pixel, cloud var is on==1, count cloudy pixels
                cloud_pts += 1;
                cloud_x += 1;
            }
            if (ATM_dataStruct[n].cloud == -1) 
            { // miss-cloud == case with no CloudMask pixels, and also filling values
                misscloud_pts += 1;
                misscloud_x += 1;
            }

            // ERROR here
            // file-print-format
            fprintf(filePtr, "%d, %d, %d, %d, %d, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %d, %lf\n", ATM_dataStruct[n].path, ATM_dataStruct[n].orbit, ATM_dataStruct[n].img_block, ATM_dataStruct[n].line, ATM_dataStruct[n].sample, ATM_dataStruct[n].lat, ATM_dataStruct[n].lon, ATM_dataStruct[n].an, ATM_dataStruct[n].ca, ATM_dataStruct[n].cf, ATM_dataStruct[n].rms, ATM_dataStruct[n].weight, ATM_dataStruct[n].npts, ATM_dataStruct[n].cloud, ATM_dataStruct[n].var); // 14

            // printf("check seg fault-2 \n");

            path_x = ATM_dataStruct[n].path;
            orbit_x = ATM_dataStruct[n].orbit;
            block_x = ATM_dataStruct[n].img_block;
            weight_x = ATM_dataStruct[n].weight;
        }
    }

    //fclose(fp);
    fclose(filePtr);
    avg_rms /= atm_DS_row; // Q- why?
    avg_valid_rms /= natm_valid;
    printf("********************************************************\n");
    printf("Number of Total ATM rms points = %d\n", atm_DS_row);
    printf("Number of Valid ATM rms points = %d\n", natm_valid);
    printf("Number of Valid ATM rms with weight of 1.0  = %d\n", natm_valid - natm_half_weight);
    printf("Number of Valid ATM rms with weight of 0.5  = %d\n", natm_half_weight);
    printf("Total Average rms = %lf from max_npts= %lf\n", avg_rms, max_npts);
    printf("Average valid rms = %lf\n", avg_valid_rms);
    printf("Max valid rms = %lf\n", max_rms);
    printf("Min valid rms = %lf\n", min_rms);
    printf("\n");
    printf("Number of cloud points = %d\n", cloud_pts);
    printf("Number of nocloud points = %d\n", nocloud_pts);
    printf("Number of missing cloud mask pts = %d\n", misscloud_pts);

    printf("\n***** FINISHED SUCCESSFULLY!***** \n\n");

    return 0;
}
