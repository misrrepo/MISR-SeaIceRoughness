// Ehsan Mosadegh 10 Nov 2019-Jan 2020
// notes:
// to-do: 
/* notes:
- ascend is not included in columns of atmmodel.csv, so zero is just a filling value

*/

// E- header files
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <MisrToolkit.h>
#include <MisrError.h>
#include <dirent.h>

// E- declarations
#define NO_DATA -999999.0
#define BACKGROUND -999998.0
#define FOREGROUND -999997.0
#define TDROPOUT -999996.0
#define MASKED -999995.0
#define LMASKED -999994.0
#define VERBOSE 0

// E- global variables
typedef struct // E- why not path??? --> 9 var/elements
{
    int block;
    int orbit;
    double an;
    double ca;
    double cf;
    double rms;
    float weight;
    float tweight; // E-???
    int ascend; // E- whats up w/ this?
} atm_type;
atm_type* ATMModel_DataStruct; // declare an instance/member

MTKt_status status;

int nfiles;

int nlines = 512;
int nsamples = 2048;

int read_misr_data(char *fname, int nline, int nsample, double **data);
int read_bytedata(char *fname, unsigned char **data, int nlines, int nsamples);
int write_data(char *fname, double* data, int nlines, int nsamples);
// int pixel2grid(int path, int block, int line, int sample, double *lat, double*lon, int *r, int *c);
int misrPixel2LatLon(int path, int block, int line, int sample, double *lat, double*lon, int *r, int *c);
char *strsub(char *s, char *a, char *b);

//#####################################################################################################

int read_data(char *fname, int nline, int nsample, double **data)
{
    FILE *f;

    f = fopen(fname, "r"); // f = stream; ptr that is opened and shows where data is stored in memory;
    if (!f) {
        fprintf(stderr,  "read_data: couldn't open %s\n", fname);
        return 0;
    }
	
    *data = (double *) malloc(nlines * nsamples * sizeof(double));
    if (!*data) {
        fprintf(stderr,  "read_data: couldn't malloc data\n");
        return 0;
    }
	// read data from stream = f --> *data;  f is the pointer to a FILE object that specifies an input stream.
    if (fread(*data, sizeof(double), nlines * nsamples, f) != nlines * nsamples) { // check see if number of elements read= initial number of elements
        fprintf(stderr,  "read_data: couldn't read data in %s\n", fname);
        return 0;
    }

    fclose(f);
    return 1;
}

//#####################################################################################################

int read_bytedata(char *fname, unsigned char **data, int nlines, int nsamples)
{
    FILE *f;

    f = fopen(fname, "rb");
    if (!f)
	{
	    printf("read_bytedata: couldn't open %s\n", fname);
	    return 0;
	}
	
    *data = (unsigned char *) malloc(nlines * nsamples * sizeof(unsigned char));
    if (!*data)
	{
	    printf("read_bytedata: couldn't malloc data\n");
	    return 0;
	}
	
    if (fread(*data, sizeof(unsigned char), nlines * nsamples, f) != nlines * nsamples)
	{
	    printf("read_bytedata: couldn't read data %s\n", fname);
	    return 0;
	}

    //fprintf(stderr, "read_bytedata: %s\n", fname);
	
    fclose(f);
    return 1;
}

//#####################################################################################################

int write_data(char *fname, double* data, int nlines, int nsamples)
{
    FILE *f;

    f = fopen(fname, "wb");
        if (!f) {
    	fprintf(stderr, "write_data_1: couldn't open %s\n", fname);
    	return 0;
    }
	
    if (fwrite(data, sizeof(double), 3 * nlines * nsamples, f) != 3 * nlines * nsamples) 
    {
	   fprintf(stderr, "write_data_2: couldn't write data\n");
	   return 0;
    }
	
    fclose(f);
    return 1;
}

//#####################################################################################################

// int pixel2grid(int path, int block, int line, int sample, double* xlat, double* xlon, int* r, int* c)
int misrPixel2LatLon(int path, int block, int line, int sample, double* xlat, double* xlon, int* r, int* c) { // receives path-block-line-sample as pointer(mem-add) and updates/outputs xlat,xlon,r,c in mem-

    int status;
    char *errs[] = MTK_ERR_DESC;
    double lat, lon;
    /* parameters for grid stereographic : 				*/
    /* MOD44W.A2000055.h14v01.005.2009212173527_MOD44W_250m_GRID.dat	*/ 
    //printf("lat before: %d \n", lat);
    status = MtkBlsToLatLon(path, 275, block, line * 1.0, sample * 1.0, &lat, &lon); // returns lat/lon of a pixel
    //printf("lat after: %d \n", lat);

    if (status != MTK_SUCCESS) 
    	{
    	printf("misrPixel2LatLon: MtkBlsToLatLon failed!!!, status = %d (%s)\n", status, errs[status]);
    	return 0;
    	}
    	
    if (VERBOSE) printf("misrPixel2LatLon: lat = %.6f, lon = %.6f\n", lat, lon);

    double psize = 10./12000.;
    double lon0 = -130.0;
    double lat0 = 90.0;

    // out
    *xlat = lat; // updates value at xlat == lat
    *xlon = lon; // updates value at xlon == lon
    *c = rint((lon - lon0)/psize); // Q- what is c?
    *r = rint(-(lat - lat0)/psize); // Q- what is r?

    return 1;
}
//#####################################################################################################

char *strsub(char *s, char *a, char *b) {
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

int main(int argc, char *argv[]) {
    DIR* dirp;
    FILE* fp;
    struct dirent* DirEntryObj; // directory entries
    
    // inputs
    // char masked_toa_an_dir[256] = "/home/mare/Nolin/data_2000_2016/2016/Surface3_LandMasked/Jul/An/test_ehsan"; // output of LandMask.c - use masked_surf files instead
    // // path to An dir files, we use An camera to define file labels for Ca Cf cameras
    char masked_toa_an_dir[256] = "/data/gpfs/assoc/misr_roughness/2016/july_2016/masked_toa_refl_test/An" ;



   // char atmmodel_csvfile[256] = "/home/mare/Projects/MISR/Julienne/IceBridge2016/SeaIce_Jul2016_atmmodel_csvfile2_r025.csv"; // ATM csv file; source from where/
    // // ATM csv file; source from where/
    char atmmodel_csvfile[256] = "/data/gpfs/assoc/misr_roughness/2016/july_2016/atmmodel/atmmodel_july_2016_k_zero_npts_larger5.csv" ;



    // we don't use this file anymore, we decided to use all blocks, meaning no correction/reversing Cf and Ca cameras.
    // char relAzimuthFile[256] =  "/home/mare/Projects/MISR/Julienne/IceBridge2016/RelativeAzimuth_Jul2016_sorted.txt" ; // we don't need this anymore; source from where?

    
    // outputs 
    // // MISR roughness; rms file; no "/" at the end
    char predicted_roughness_dir[256] = "/data/gpfs/assoc/misr_roughness/2016/july_2016/predict_roughness_k_zero_npts_test/npts_5" ;



    
    // other variables
    char command[256];
    char wc_out[256];
    char message[256];
    char an_file[128];  //ascend
    char roughness_fname[256];
    char ca_fname[256];
    char cf_fname[256];
    char an_fname[256];
    char sblock[4]; // make for 4 elements to include the last null char
    char spath[4];  // make 4 so that last=4 element is null char
    char sorbit[7]; // made 7 elements so that last=7 element is null char
    char *token;
    char *sline = NULL;
    char **misr_an_files_list = 0;
    int toa_an_files = 0;
    int atmmodel_DS_rows = 0;
    int i, j, k, n, w;
    int r, c, r2, c2;
    double ca, cf, an;
    double xcf, xca, xan, xroughness, xweight, tweight;
    double xdata_distance, xvector_min_len, xrough_nearest;
    double *an_masked_toa, *cf_masked_toa, *ca_masked_toa;
    double* roughness_mem_block_ptr;
    double radius;
    double radius_ascend = 0.050; //Jul2016 SeaIce Model
    double radius_descend = 0.010; //Jul2016 SeaIce Model
    //double radius_ascend = 0.010; //AprMay2016 SeaIce Model
    //double radius_descend = 0.020; //AprMay2016 SeaIce Model
    //double radius = 0.015; //2009 SeaIce Model
    //double radius = 0.025; //2010 SeaIce Model
    //double radius = 0.025; //2013 SeaIce Model
    double lat, lon;
    int path, block, orbit;
    int blockElements = nlines * nsamples; // blockElements
    size_t slen = 0;
    ssize_t line_size;

    int l = 0;
    int start_orbit, end_orbit;
    int relAz_nlines;
    int *raz_table;
    int block1, block2;
    int block12;
    int ascend = 0;
    //unsigned char *mask;
    //int gridlines = 24000;
    //int gridsamples = 84000;

    printf("c: input/toa dir: %s \n" , masked_toa_an_dir);
    printf("c: input/atmmodel csv file: %s \n" , atmmodel_csvfile);
    printf("c: output roughness dir: %s \n" , predicted_roughness_dir);

    //printf("Reading /home/mare/Nolin/SeaIce/ArcticTiles.dat ...\n");
    //if (!read_bytedata("/home/mare/Nolin/SeaIce/ArcticTiles.dat", &mask, gridlines, gridsamples)) return 1;

    /* //////////////////////////// process azimuth file //////////////////////////////////////////////////////////// */

    // sprintf(command, "wc -l %s", relAzimuthFile); // stoes command
    // fp = popen(command, "r"); // popen == pipe-open; starts a stream, fp is pointer to a stream
    // if (fp == NULL) {
	   // printf("ERROR: Failed to run command\n" );
	   // exit(1);
    // }

    // //char return_fgets[200];     // Ehsan

    // /* Read the output a line at a time - output it. */
    // //return_fgets = fgets(wc_out, sizeof(wc_out)-1, fp);
    // //printf("fgets got: %s" , return_fgets);
    // while (fgets(wc_out, sizeof(wc_out)-1, fp) != NULL) { // reads line from fp, stores in wc_out
    // 	//printf("wc_out: %s \n", wc_out);
    // 	token = strtok(wc_out, " "); // searches wc_out for tockens delimited by ""; token=no of lines in file
    // 	//printf("wordsare: %s \n", token);
    // 	w = 0;
    // 	if (w == 0) relAz_nlines = atoi(token); // no of lines in relAzimuthFile file
    // 	//printf("AznLines= %d \n", relAz_nlines);
    // }
    // /* close */
    // pclose(fp);

    // /*  E- reads azimuth file  */
    // fp = fopen(relAzimuthFile, "r"); // opens file, stores it somewhere and returns the mem-add
    // if (!fp) {
    // 	fprintf(stderr, "main: couldn't open %s\n", relAzimuthFile);
    // 	return 1;
    // }

    // l = 0;
    // //printf("file ptr is: %p \n" , fp);
    // while ( (read = getline(&sline, &slen, fp)) != -1) { // read is the return characters+ "\n" at the end, if -1 == EOF
    //     //printf("no of characters read from each line: %d \n" , read); // read is the return characters+\n, if -1 == EOF
    //     //printf("sline is: %s \n" , sline);
    // 	token = strtok(sline, " "); // breaks/parse line to its token in each line in a loop
    // 	w = 0;
    // 	//printf("word1 is: %s \n" , token); //  the 1st word, cos not in loop
    // 	while ((l == 0) && (token != NULL)) { // NULL at the end of each sentence
    // 		if (w == 1) start_orbit = atoi(token);
    //     	token = strtok (NULL, " "); // gets tokens in a sentence in a loop
    //     	//printf("word: %s \n" , token); // gets token in a rel azimuth in a loop
    // 		w++;
    // 	}
    // 	while ((l == relAz_nlines - 1) && (token != NULL)) {
    // 	    if (w == 1) end_orbit = atoi(token);  // is end_orbit the last block number for asceding path?
    //     	token = strtok (NULL, " ");
    // 	    w++;
    // 	}
    // 	l++;
    // }


    // printf("start_orbit= %d  end_orbit= %d\n", start_orbit, end_orbit);
    // fseek(fp, 0, SEEK_SET);

    // raz_table = (int *) malloc((end_orbit - start_orbit + 1) * sizeof(int));
    // while ((read = getline(&sline, &slen, fp)) != -1) { // read= no of char read from each line; fp=stream
    // 	token = strtok(sline, " ");
    // 	w = 0; // what is w? is it counter?
    // 	//printf(" w2 is: %d \n" , w);
    // 	//printf("word2 is: %s \n" ,  token); // gets token in a rel. azimuth in a loop
    // 	while (token != NULL) { // the end of file EOF
    // 	    if (w == 1) orbit = atoi(token);
    // 	    if (w == 3) block12 = 100*atoi(token); // col4*100
    // 	    if (w == 4) block12 += atoi(token); // 100col5 + col4; gets updated here
    //     	token = strtok (NULL, " "); // updates words and w in next line
    // 	    w++;
    // 	}
    // 	raz_table[orbit - start_orbit] = block12;
    //     //printf("block12: %d \n", block12);
    // }
    // fclose(fp);

    /* //////////////////////////// process azimuth file //////////////////////////////////////////////////////////// */

    /* //////////////////////////// reads atmmodel_csvfile csv file ///////////////////////////////////////////////////////////// */ // ok
    /* reads all rows of atmmodel_csvfile csv file and fills the fileObj= ATMModel_DataStruct */
    
    fp = fopen(atmmodel_csvfile, "r");
    if (!fp) {

    	fprintf(stderr, "main: couldn't open %s\n", atmmodel_csvfile);
    	return 1;
    }

    printf("\nprocessing atmmodel_csvfile: %s \n\n", atmmodel_csvfile);

      /* Get the first line of the file. */
    // line_size = getline(&sline, &slen, fp);
    // printf("line_size ptr: %p \n" , &line_size);

    /* Loop through each line of file until we are done with the file. */
    // note: returns number of characters read == line_size = getline(&line_buf, &line_buf_size, fp);
    while (getline(&sline, &slen, fp) >= 0) { // note: use getline inside loop; reads each row of atmmodel_csvfile.csv ==> getline(&line_in_buffer, &line_in_buffer_size, fp=input stream to read each line==stdin)
    
        //printf("num of char read: %zu \n", read); read
        // printf("new row extracted: %s \n", sline);
        
        if (sline[0] == '#') {

            // printf("row started with '#', so we skip the entire line \n");
            continue;
        }

        //printf("read row: %d \n" , atmmodel_DS_rows);
        int column_cnt = 0;
        //printf("w is: %d \n" , w);
    	token = strtok (sline, ","); // moves ptr to first token
    	// printf("first token: %s \n\n" , token);

      	while (token != NULL) {// NULL = end of line or EOF; Q- how about w=0==path?

        	// printf ("select based on w = %d and token = %s \n", w, token);
    	    if (column_cnt == 1) orbit = atof(token);
    	    if (column_cnt == 2) block = atof(token);
    	    if (column_cnt == 7) xan = atof(token); // xan comes from ATMmodel file
    	    if (column_cnt == 8) xca = atof(token); // misr cam
    	    if (column_cnt == 9) xcf = atof(token); // misr cam
    	    if (column_cnt == 10) xroughness = atof(token);
    	    if (column_cnt == 13) xweight = atof(token); // w from k_day; is cloud in ATM code ???
    	    if (column_cnt == 14) tweight = atof(token); // Q- ??? - is it var?

    	    if (column_cnt == 15) {
                // printf("ascend col is: %s \n" , token);
                ascend = atoi(token); // E: ??? - there is no col=15 (index = 16) in the csv file for ascend, so I set it to zero
            }

        	token = strtok (NULL, ","); // whats this?
        	//printf("word updates: %s \n" , token);
        	//printf("\n");
    	    column_cnt++; // increases inside while-loop
      	}




        /* create ATMModel_DataStruct here: fill the fileObj with each row of ATM data extracted from previous step */
    	if (atmmodel_DS_rows == 0) ATMModel_DataStruct = (atm_type * ) malloc(sizeof(atm_type));
    	else ATMModel_DataStruct = (atm_type * ) realloc(ATMModel_DataStruct, (atmmodel_DS_rows + 1) * sizeof(atm_type));

        // printf("final value for ascend= %d \n" , ascend);

    	// update elements/variables for a new member
    	ATMModel_DataStruct[atmmodel_DS_rows].orbit = orbit;
    	ATMModel_DataStruct[atmmodel_DS_rows].block = block;
    	ATMModel_DataStruct[atmmodel_DS_rows].an = xan;
    	ATMModel_DataStruct[atmmodel_DS_rows].ca = xca;
    	ATMModel_DataStruct[atmmodel_DS_rows].cf = xcf;
    	ATMModel_DataStruct[atmmodel_DS_rows].rms = xroughness;
    	ATMModel_DataStruct[atmmodel_DS_rows].weight = xweight;
    	ATMModel_DataStruct[atmmodel_DS_rows].tweight = tweight;
    	ATMModel_DataStruct[atmmodel_DS_rows].ascend = ascend; // note: ascned comes from w=column=15 of ATM csv file.
    	atmmodel_DS_rows++;// counter - max will be the max num of rows in atmmodel_csvfile.csv
    }

    // printf("ATMModel_DataStruct now is: %d \n", ATMModel_DataStruct->d_name);
    fclose(fp);

    printf("c: total rows of ATMModel-DataStruct: %d \n" , atmmodel_DS_rows);

    /* //////////////////////////// reads ATM csv file ///////////////////////////////////////////////////////////// */


    /* //////////////////////////// Get list of MISR Masked files /////////////////////////////////////////////// */ //ok

    printf("c: make a list from masked TOA An files ...\n"); // == misr_an_files_list
    
    dirp = opendir(masked_toa_an_dir); // define dir stream == dirp == ptr to that directory
    if (dirp) {     // if ptr available == TRUE
    	while ((DirEntryObj = readdir(dirp)) != NULL) {     // read the first item in dir and moves the ptr to next item/file in dir; DirEntryObj == struct==fileObj for each file in dir
            
            if (!strstr(DirEntryObj->d_name, ".dat")) continue; // d_name= fileName, if could not find the pattern ".dat" in this string
            
            if (misr_an_files_list == 0) {  // if it has not been created yet
                
                misr_an_files_list = (char **) malloc(sizeof(char *));   // allocate mem- for 1st entry
                if (!misr_an_files_list) {  // if its reverse was true....
                    printf("main: couldn't malloc misr_an_files_list\n");
                    return 0;
                }
            }
            else {
                
                misr_an_files_list = (char **) realloc(misr_an_files_list, (toa_an_files + 1) * sizeof(char *));
                if (!misr_an_files_list) { // if not = to 1 == True...
                    printf("getFileList: couldn't realloc atm_flist\n");
                    return 0;
                }
            }

            misr_an_files_list[toa_an_files] = (char *) malloc(strlen(DirEntryObj->d_name) + 1); // allocate mem-

            if (!misr_an_files_list[toa_an_files]) {
                printf("main: couldn't malloc atm_flist[%d]\n", toa_an_files);
                return 0;
            }


            // copy one string to the fileList
            strcpy(misr_an_files_list[toa_an_files], DirEntryObj->d_name);  // fill the misr_an_files_list with d_name: misr surf files; from DirEntryObj=fileObj gets the fileName
            // printf("FOUND: d_name == TOA file: %s \n", DirEntryObj->d_name);       // d_name is char array inside <dirent.h>
            //printf("file no. %d, %s \n", toa_an_files, misr_an_files_list[toa_an_files]);
            toa_an_files ++; // counter of num of MISR files found == elements in misr_an_files_list
    	}
    	closedir (dirp);
    } 
    else {
        strcat(message, "Can't open ");
        strcat(message, masked_toa_an_dir);
        perror (message);
        return EXIT_FAILURE;
    }
    /* //////////////////////////// Get list of MISR Masked files ///////////////////////////////////////////// */


	/* //////////////////////////// main algorithm: Process each MISR TOA file ///////////////////////////////////////////////////// */

    // extract P & O info
    printf("c: MISR (An) files found = %d\n" , toa_an_files);
    printf("c: Processing each MISR toa_an.dat file ...\n");

    for (i = 0; i < toa_an_files; i++) { // i= loop for each num of surf MISR files found


        // define a task as thread = f() here?  ?????????????

        








        printf("\n*********************** processing input (%d/%d): %s ***********************\n\n" , i+1, toa_an_files, misr_an_files_list[i]);

        /* ******************* find the path and orbit from file labels ***************************** */

        if (strstr(misr_an_files_list[i], "_P")) {   // search for _P in the file name, find _P in each surf file
            // printf("check path num \n");
            strncpy(spath, strstr(misr_an_files_list[i], "_P") + 2, 3); // find path
            // spath[3] = 0; // Q- why? fills the 3 or 4 element?
            path = atoi(spath); // find path num from surf file
            // printf("path from MISR: %d \n", path);
        }
        else {
            printf("No path info in file name\n");
            return 1;
        }
        //if (path != 78) continue;
        //if (path != 83) continue;

        // if ((path < 167) || (path > 240))  // check w/ Anne
        // {
        //     printf("we don't need this path number! Continue to the next....\n");
        //     continue; // E- what is this path region? our domain?
        // }

        //printf("now ... \n");
        //printf("processing MISR surf file: %s%s \n", masked_toa_an_dir, misr_an_files_list[i]); // check the full path of surf file

        if (strstr(misr_an_files_list[i], "_O")) {  // check if orbit is found in file name
            // printf("check orbit number \n");
            strncpy(sorbit, strstr(misr_an_files_list[i], "_O") + 2, 6); // get the orbit num from surf file and copy to sorbit
            // sorbit[6] = 0; // Q- why?
            // printf("orbit2: %s \n" , sorbit);
            orbit = atoi(sorbit); // find orbit no from surf file
            // printf("orbit: %d \n" , orbit);
        }
        else {
            printf("No orbit info in file name\n");
            return 1;
        }


        // fix file/direcotory labeling here ???????????????????????????????????????????????????????????????????????????????????????????????


        /* ******************* we use An filenames to label Cf and Ca files ***************************** */

        //printf("misr list file: %s \n", misr_an_files_list[i]);
        sprintf(an_fname, "%s/%s", masked_toa_an_dir, misr_an_files_list[i]);      // stores surf fileName into an_fname buffer
        printf("an_fname: %s \n", an_fname);


        // do the same thing with ca, copy the same surf file into ca
        sprintf(ca_fname, "%s/%s", masked_toa_an_dir, misr_an_files_list[i]);
        // printf("ca_fname: %s \n", ca_fname);

        // substitute an with cf in both filelabel and directory label
        strsub(ca_fname, "An", "Ca");       // to rename dir 
        strsub(ca_fname, "_an", "_ca");     // to rename file label
        printf("ca_fname: %s \n", ca_fname);



        //printf("misr list file: %s \n", misr_an_files_list[i]);
        sprintf(cf_fname, "%s/%s", masked_toa_an_dir, misr_an_files_list[i]); // copy the same surf file into cf
        // printf("cf_fname: %s \n", cf_fname);

        // replace an with cf to create file label
        strsub(cf_fname, "An", "Cf");       // to rename dir // to substitute An dir label to Cf dir label
        strsub(cf_fname, "_an", "_cf");     // to rename file label   // to substitute an filelabel to cf filelabel
        printf("cf_fname: %s \n", cf_fname);

        // fix acceess here ???????????????????????????????????????????????????????????????????????????????????????????????
        // need a function to check if ca_fname and other files are accessible -- Ehsan




        // check if cf_fname is accessible
        //if (access(cf_fname, F_OK) == -1) continue;	// check if file is acessible, returns 0

       

        /* up to here we run to extract info to build struct
    

    
        then after here we create a cunction for each thread and inout the struct for eahc iteration

        */





        // ???????????????????????????????????????????????????????????????????????????????????????????????????????????

        // check if file is accessible, returns 0
        //if (access(ca_fname, F_OK) == -1) continue; // check if file exists, returns 0

        printf("c: reading each MISR image block from 3 camera files into memory...\n");
       
        /* read MISR input files */
        if (!read_data(an_fname, nlines, nsamples, &an_masked_toa)) { // we fill an_masked_toa array from: an_fname
            printf("ERROR: check AN read_data: file name available??? \n");
            return 1; // return 0 or 1????
        } 

        if (!read_data(ca_fname, nlines, nsamples, &ca_masked_toa)) {
            printf("ERROR: check CA read_data: file name available??? \n");
            return 1;
        }

        if (!read_data(cf_fname, nlines, nsamples, &cf_masked_toa)) {
            printf("ERROR: check CF read_data: file name available??? \n");
            return 1;
        }

        
        int roughness_mem_block_layers = 5; // 5 for lat, lon, 

        roughness_mem_block_ptr = (double *) malloc(roughness_mem_block_layers * nlines * nsamples * sizeof(double)); // allocate mem- for 5 layers of data
        // roughness_mem_block_ptr = (float *) malloc(roughness_mem_block_layers * nlines * nsamples * sizeof(float)); // allocate mem- for 5 layers of data: lat, lon, roughness, what if we change dtype to float?

        if (strstr(misr_an_files_list[i], "_B")) {  // get the block number from surf file name
            strncpy(sblock, strstr(misr_an_files_list[i], "_B") + 2, 3);
            // sblock[3] = 0;
            block = atoi(sblock);
            // printf("block: %d \n", block);
        }

        else {
            printf("No block info in file name. \n");
            return 1;
        }



        // // from relative azimush file/table -- turned off
        // block1 = raz_table[orbit - start_orbit] /= 100; // result
        // block2 = raz_table[orbit - start_orbit] %= 100; // remainder
        // printf("b1: %d; b2: %d \n", block1, block2);
        // radius = 0.025; //

        // //radius = radius_descend;
        // //if ((block < 20) || ((block >= block1) && (block <= block2))) {
        // if ((block >= block1) && (block <= block2)) { // E- why this condition?
        //     ascend = 1;  // what does this mean?
        //     printf("ascend block");
        //     //radius = radius_ascend;
        // }




        radius = 0.025; // check w/ Anne



        int descend_mode = 1; // Ehsan: check w/ Anne: does it define descending for all blocks?

        // printf("descend_mode now is= %d \n" , descend_mode);

        // printf("inverse descend_mode ~ %d \n" , ~descend_mode);

        // printf("inverse descend_mode ! %d \n" , !descend_mode);


        //////////////////////////////////////////////////////////////////////////////
        /*  */
        printf("c: now processing 3 MISR toa_block.dat files in memory \n");

        for (r = 0; r < nlines; r++) { // r==row
            for (c = 0; c < nsamples; c++) { // c==column

                if (!misrPixel2LatLon(path, block, r, c, &lat, &lon, &r2, &c2)) { // E- r2 and c2 not used, should I modify misrPixel2LatLon and remove both? I aassume it was used for Land mask and we don't need land mask here anymore
                    printf("ERROR from misrPixel2LatLon.\n");
                    return 1; // error return 0 or 1?
                } // pass-by-reference: &lat-&lon-&r2-&c2 
                
                // start from here ******************************************************************

                // fill mem-locations for lat- lon
                roughness_mem_block_ptr[1 * blockElements + r * nsamples + c] = lat; // fill 2nd layer of mem-block
                roughness_mem_block_ptr[2 * blockElements + r * nsamples + c] = lon; // fill 3rd layer of mem-block
                //roughness_mem_block_ptr[3 * blockElements + r*nsamples + c] = 90.0 - r2/1200.0 ;
                //roughness_mem_block_ptr[4 * blockElements + r*nsamples + c] = -130.0 + c2/1200.0;



                if (an_masked_toa[r * nsamples + c] < 0) {  // E: why skip each pixel here?

                    roughness_mem_block_ptr[r * nsamples + c] = an_masked_toa[r * nsamples + c];
                        //roughness_mem_block_ptr[3*blockElements + r*nsamples + c] = an_masked_toa[r*nsamples + c];;
                    continue;
                }

                if (ca_masked_toa[r*nsamples + c] < 0) {

                    roughness_mem_block_ptr[r*nsamples + c] = ca_masked_toa[r * nsamples + c];
                        //roughness_mem_block_ptr[3*blockElements + r*nsamples + c] = ca_masked_toa[r*nsamples + c];;
                    continue;
                }

                if (cf_masked_toa[r*nsamples + c] < 0) {

                    roughness_mem_block_ptr[r*nsamples + c] = cf_masked_toa[r * nsamples + c];
                        //roughness_mem_block_ptr[3*blockElements + r*nsamples + c] = cf_masked_toa[r*nsamples + c];;
                    continue;
                }

                /***  E: for LasndMask part??
                if (r2 >= 0 && r2 < gridlines && c2 >= 0 && c2 < gridsamples) {
                        k = c2 + r2 * gridsamples;
                        if (mask[k] == 1){
                        roughness_mem_block_ptr[r*nsamples + c] = LMASKED;
                        //roughness_mem_block_ptr[3 * blockElements + r*nsamples + c] = LMASKED;
                    continue;
                        }
                }
                ***/

                //////////////////////////////////////////////////////////////////////////////
                xroughness = 0;
                tweight = 0;
                xvector_min_len = 1e23; // 22fold of 10

                // use this print to monitor each toa pixel
                // printf("now sort each ATMModel.csv rows for new MISR pixel data_vector= (%d, %d) \n" , r, c);


                // MAYBE MULTI- THREAD here? define a function for a task for this section???





                for (n = 0; n < atmmodel_DS_rows; n++) // we compare each MISR pixel with all ATM.csv rows
                {
                    // printf("we set ascend to: %d in ATMModel_DataStruct.\n" , !ATMModel_DataStruct[n].ascend);


                    //if (ATMModel_DataStruct[n].ascend != ascend) continue;
                    //if (~ascend || ((block < 20) && (ATMModel_DataStruct[n].block < 20)) || ((block >= 20) && (ATMModel_DataStruct[n].block >= 20))) {

                    // check w/ Anne: what is this condition?
                    // if ( (~ascend  && ((ATMModel_DataStruct[n].block < 20) || // || if any == 1 then True...GO
                    //      ~ATMModel_DataStruct[n].ascend)) || 
                    //      (ascend && (ATMModel_DataStruct[n].block >= 20) && (ATMModel_DataStruct[n].ascend)))  // && == if all 1 then GO

                   if (descend_mode)  // printf("we do this section, without inverting ca/cf cameras. \n");
                   {

                        /* check w/ Anne - 
                        is it necessary to compute this here? or can take out of the for-loop? 
                        why they get subtracted from each other?for example: why for AN: (MISR - ATM?) both are MISR TOA refl values! */
                        xan = (an_masked_toa[r * nsamples + c] - ATMModel_DataStruct[n].an);
                        xca = (ca_masked_toa[r * nsamples + c] - ATMModel_DataStruct[n].ca); // difference is: unknown/unseen/new data - trainign data 
                        xcf = (cf_masked_toa[r * nsamples + c] - ATMModel_DataStruct[n].cf);

                        // check w/ Anne - why values are different?
                        // printf("an_masked_toa= %f \n" , an_masked_toa[r * nsamples + c]);
                        // printf("an_atmmodel= %f \n" , ATMModel_DataStruct[n].an);

                        // printf("cf_atmmodel= %f \n" , ATMModel_DataStruct[n].cf);
                        // printf("an_atmmodel= %f \n" , ATMModel_DataStruct[n].an);


                        // printf("xan= %f \n" , xan);
                        // printf("xca= %f \n" , xca);
                        // printf("xcf= %f \n" , xcf);
                    }
                    else // maybe turn this section off?
                    {    // is this the correction swection?
                        printf("check: we do NOT run this section for inverting cameras. \n");
                        xan = (an_masked_toa[r*nsamples + c] - ATMModel_DataStruct[n].an);
                        xca = (cf_masked_toa[r*nsamples + c] - ATMModel_DataStruct[n].ca);
                        xcf = (ca_masked_toa[r*nsamples + c] - ATMModel_DataStruct[n].cf);
                    }

                    /***
                    xan = (an_masked_toa[r*nsamples + c] - ATMModel_DataStruct[n].an);
                    xca = (ca_masked_toa[r*nsamples + c] - ATMModel_DataStruct[n].ca);
                    xcf = (cf_masked_toa[r*nsamples + c] - ATMModel_DataStruct[n].cf);
                    ***/

                    // distance as similarity, from each 3 pixel
                    xdata_distance = sqrt(xan * xan + xca * xca + xcf * xcf); // E- distance == lenght of toa-refl vector in MISR 3D feature space
                    
                    // check if data_vector is qualified? // compare each pixel_data_vector w/ all ATM.csv rows?
                    if (xdata_distance < radius) 
                    {

                        xroughness += ATMModel_DataStruct[n].tweight * ATMModel_DataStruct[n].rms; // E- label data_vector here; changed xrms == xroughness; check w/ Anne: how about if tweight=0?
                        tweight += ATMModel_DataStruct[n].tweight;

                        // sorting here?
                        if (xdata_distance < xvector_min_len) {
                            
                            xvector_min_len = xdata_distance; // E- update min-lengh and lower threshold to closer the range
                            xrough_nearest = ATMModel_DataStruct[n].rms;
                        }
                    }

                } // E- end of for-loop to check all ATM.csv rows, // we compare each data_vector with all ATM.csv rows --> will find xrough_nearest


                if (xroughness == 0) { // signal that it is inside radius?
                    // printf("checkP-1 \n");
                    xroughness = xrough_nearest;
                }
                else { // check w/ Anne
                    // printf("checkP-2 \n");
                    xroughness /= tweight;
                }
                
                roughness_mem_block_ptr[r * nsamples + c] = xroughness; // fill 1st layer of mem-block for each MISR pixel
                //roughness_mem_block_ptr[3*blockElements + r*nsamples + c] = tweight;
            
            }
        } // end processing 3 MISR block files
        
        sprintf(roughness_fname, "%s/%s", predicted_roughness_dir, misr_an_files_list[i]);
        strsub(roughness_fname, "_an.dat", ".dat");
        // strsub(roughness_fname, "surf", "rms");
        strsub(roughness_fname, "masked", "roughness");  // E: renamed output file

        //strsub(roughness_fname, "surf", "rms");
        //fp = fopen(roughness_fname, "wb");
        printf("c: writing roughness block: %d \n", i+1);
        printf("%s \n" , roughness_fname);
        
        write_data(roughness_fname, roughness_mem_block_ptr, nlines, nsamples);

        printf("c: FINISHED writing roughness block: %d \n", i+1);

        free(an_masked_toa);
        free(ca_masked_toa);
        free(cf_masked_toa);
        free(roughness_mem_block_ptr);

    }
/* //////////////////////////// Process each MISR file ///////////////////////////////////////////////////// */

    return 0;
}


// NOTES:
// || if any == 1 then True...GO
// note= !A reverses the logical state of A (0->1 or 1->0)
// && == if all 1 then GO


