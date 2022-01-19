/* 
by: Ehsan Mosadegh, 29 August 2020

*/
// E- header files
#include <stdio.h> // to use printf 
#include <string.h> // to define string?
#include <pthread.h> // to use gcc? threading capability, cuz Clang does not have multithreading standard
#include <unistd.h> // to use sleep function
#include <stdlib.h> // for malloc()

#include <math.h>
#include <ctype.h>
#include <MisrToolkit.h>
#include <MisrError.h>
#include <dirent.h>





// E- number of threads/CPU cores to use
#define total_threads 10         // Q- how many CPU cores on your machine? max number of CPUs on a board. how many logical processors?





// E- declarations
#define NO_DATA -999999.0
#define BACKGROUND -999998.0
#define FOREGROUND -999997.0
#define TDROPOUT -999996.0
#define MASKED -999995.0
#define LMASKED -999994.0
#define VERBOSE 0



// global variables

typedef struct {  // q- do elements change w/ thread? if not get rid of "thread_iter"; variabloes are diff in each toaFile_DS, cuz each toaFile_DS represents a single diff toaFile
	int toa_file_count;
	int total_toa_files;
	int amtmodel_total_rows;
	int path;
	int orbit;
	int block;
	char outFile_lable[256];  // check size file later
	char ca[256]; // q- how define string w/ char* and strcpy string into it in main()?
	char cf[256];
	char an[256];
	char outDir[256];
	// int valid;
} toaFile_DS;
/* define ptr for our dataStruct */
toaFile_DS* toaFile_struct_ptr[total_threads]; // array of ptrs w/ [size]; q- why define size here? why array of ptrs? ptr should be defibed as global var?


typedef struct {    // E- why not path??? --> 9 var/elements  
	int block;
	int orbit;
	double an;
	double ca;
	double cf;
	double rms;
	float weight;
	float tweight; // E-???
	int ascend; // E- whats up w/ this?
} atmFile_DS;
// define ptr for atmFile_DS dataStruct
atmFile_DS* ATMModel_struct_ptr; // declare an instance/member

int nfiles;
int nlines = 512;
int nsamples = 2048;
MTKt_status status;

int read_data(char *fname, int nline, int nsample, double **data);
void* multithread_task(void* arg_ptr);
int read_bytedata(char *fname, unsigned char **data, int nlines, int nsamples);
int write_data(char *fname, double* data, int nlines, int nsamples);
// int pixel2grid(int path, int block, int line, int sample, double *lat, double*lon, int *r, int *c);
int misrPixel2LatLon(int path, int block, int line, int sample, double* lat, double* lon, int *r, int *c);
char *strsub(char *s, char *a, char *b);

//################################################## main() ############################################################

int main(int argc, char* argv[]) {

    // check number of inouts to this code from commandLine (from python wrapper)
    if (argc == 4) {
        printf("C: OK! received 4 arguments. \n");
    }

    if (argc != 4) { // this might happen later, cos I turnedoff fname[2]
        fprintf(stderr, "Usage: <exe-name> <maskedTOA-AN-dir-dir> <atmmodelCSV-file> <predictedRoughness-dir> \n"); // updated
        // fprintf(stderr, "Usage: TOA3 input-misr-file block band minnaert output-data-file output-image-file-Ehsan--noNeed\n"); old with image
        return 1;
    }
	//------------------------------------------------------------------------------------------------------------------
	/* E: input directories, I kept the history of paths here
	path to An dir files, we use An camera to define file labels for Ca Cf cameras */
	
	// old path on Anne's Linux machine
	// char masked_toa_an_dir[256] = "/home/mare/Nolin/data_2000_2016/2016/Surface3_LandMasked/Jul/An/test_ehsan"; // output of LandMask.c - use masked_surf files instead
	
	// path on my Mac
	// char masked_toa_an_dir[256] = "/Volumes/Ehsanm_DRI/research/MISR/masked_toa_files/masked_toa_refl_ellipsoid_apr2013_day1to16_p1to233_b1to40/An" ; // path to An dir files, we use An camera to define file labels for Ca Cf cameras
	
	// path on Pronghorn cluster
	// char masked_toa_an_dir[256] = "/data/gpfs/assoc/misr_roughness/masked_toa_refl_ellip_apr2013_d1to16_p1to233_b1to40/An" ;
    char masked_toa_an_dir[256];

	// old path on Anne's Linux machine
   // char atmmodel_csvfile[256] = "/home/mare/Projects/MISR/Julienne/IceBridge2016/SeaIce_Jul2016_atmmodel_csvfile2_r025.csv"; // ATM csv file; source from where/
	
	// char atmmodel_csvfile[256] = "/Volumes/Ehsanm_DRI/research/MISR/atmmodel_dir/atmmodel_2013/atmmodel_2013_aug1_16_b1_40_newASCM.csv" ; // ATM csv file; source from where/
	// char atmmodel_csvfile[256] = "/data/gpfs/assoc/misr_roughness/atmmodels/atmmodel_2013_aug1_16_b1_40_newASCM.csv" ;
	char atmmodel_csvfile[256];

	// we don't use this file anymore, we decided to use all blocks, meaning no correction/reversing Cf and Ca cameras.
	// char relAzimuthFile[256] =  "/home/mare/Projects/MISR/Julienne/IceBridge2016/RelativeAzimuth_Jul2016_sorted.txt" ; // we don't need this anymore; source from where?

	
	// outputs 
	// char predicted_roughness_dir[256] = "/Volumes/Ehsanm_DRI/research/MISR/roughness_files/multithreaded_atmmodel_newASCM_test_Path40" ; // MISR roughness; rms file; no "/" at the end
	// char predicted_roughness_dir[256] = "/data/gpfs/assoc/misr_roughness/roughness_2013_apr1to16_p1to233_b1to40" ; // MISR roughness; rms file; no "/" at the end
    char predicted_roughness_dir[256];
    //------------------------------------------------------------------------------------------------------------------
    // fill string from commandLine
    strcpy(masked_toa_an_dir, argv[1]);
    strcpy(atmmodel_csvfile, argv[2]);
    strcpy(predicted_roughness_dir, argv[3]);

    //------------------------------------------------------------------------------------------------------------------
	// hiow define a array of ptrs inside main == local to main()? so that be able to pass the num_threads as arg to main()?
	// int total_threads = 5;
	// toaFile_struct_ptr[total_threads];

	// int* num_threads_ptr; 
	// num_threads_ptr = &total_threads;
	// *num_threads_ptr = 7;
	// printf("c: we are running with %d threads \n" , total_threads);


	/* declare identifiers */
	pthread_t tid [total_threads]; // tid descriptor, q- inside or outside main()? create several tid identifires/descriptors/variables
	int ret1, ret2; // return values
	int toa_file_iter;
	char file_char_label[256];

	/* from old code*/
	DIR* dirp;
	FILE* fPtr_csv;
	FILE* fPtr_rough;
	struct dirent* DirEntryObj; // directory entries

	// other variables
	char command[256];
	char wc_out[256];
	char message[256];
	char an_file[128];  //ascend
	// char roughness_fname[256];
	char ca_fname[256];
	char cf_fname[256];
	char an_fname[256];
	char sblock[4]; // make for 4 elements to include the last null char
	char spath[4];  // make 4 so that last=4 element is null char
	char sorbit[7]; // made 7 elements so that last=7 element is null char
	char *token;
	char *sline = NULL;
	char **toa_an_files_list_ptr = 0;
	int total_toa_an_files = 0;
	int atmmodel_tot_rows = 0;
	int i, j, k, n, w;
	// int r, c, r2, c2;
	double ca, cf, an;
	double xcf, xca, xan, xweight, tweight; // xroughness

	double xdata_distance, xvector_min_len, xrough_nearest;
	// double *an_masked_toa, *cf_masked_toa, *ca_masked_toa;
	// float* roughness_mem_block_ptr;
	double radius;
	double radius_ascend = 0.050; //Jul2016 SeaIce Model
	double radius_descend = 0.010; //Jul2016 SeaIce Model
	//double radius_ascend = 0.010; //AprMay2016 SeaIce Model
	//double radius_descend = 0.020; //AprMay2016 SeaIce Model
	//double radius = 0.015; //2009 SeaIce Model
	//double radius = 0.025; //2010 SeaIce Model
	//double radius = 0.025; //2013 SeaIce Model
	// double lat, lon;
	float lat, lon, xroughness; // note: all were double initially. i changed the stype to save storage
	int path, block, orbit;
	// int blockElements = nlines * nsamples; // blockElements
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


	/* //////////////////////////// reads atmmodel_csvfile csv file //////////////////////////////////////////// */ // ok
	/* reads all rows of atmmodel_csvfile csv file and fills the fileObj= ATMModel_struct_ptr */
	
	fPtr_csv = fopen(atmmodel_csvfile, "r");
	if (!fPtr_csv) {

		fprintf(stderr, "main: couldn't open %s\n", atmmodel_csvfile);
		return 1;
	}

	printf("\nc: atmmodel_csvfile: %s \n", atmmodel_csvfile);

	  /* Get the first line of the file. */
	// line_size = getline(&sline, &slen, fPtr_csv);
	// printf("line_size ptr: %p \n" , &line_size);

	/* Loop through each line of file until we are done with the file. */
	// note: returns number of characters read == line_size = getline(&line_buf, &line_buf_size, fPtr_csv);
	while (getline(&sline, &slen, fPtr_csv) >= 0) { // note: use getline inside loop; reads each row of atmmodel_csvfile.csv ==> getline(&line_in_buffer, &line_in_buffer_size, fPtr_csv=input stream to read each line==stdin)
	
		//printf("num of char read: %zu \n", read); read
		// printf("new row extracted: %s \n", sline);
		
		if (sline[0] == '#') {

			// printf("row started with '#', so we skip the entire line \n");
			continue;
		}

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
			if (column_cnt == 10) xroughness = atof(token);  // based on 0-index == col 11 based on 1-index
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




		/* create ATMModel_struct_ptr here: fill the fileObj with each row of ATM data extracted from previous step */
		if (atmmodel_tot_rows == 0) ATMModel_struct_ptr = (atmFile_DS * ) malloc(sizeof(atmFile_DS));
		else ATMModel_struct_ptr = (atmFile_DS * ) realloc(ATMModel_struct_ptr, (atmmodel_tot_rows + 1) * sizeof(atmFile_DS));

		// printf("final value for ascend= %d \n" , ascend);

		// update elements/variables for a new member
		ATMModel_struct_ptr[atmmodel_tot_rows].orbit = orbit;   
		ATMModel_struct_ptr[atmmodel_tot_rows].block = block;
		ATMModel_struct_ptr[atmmodel_tot_rows].an = xan;
		ATMModel_struct_ptr[atmmodel_tot_rows].ca = xca;
		ATMModel_struct_ptr[atmmodel_tot_rows].cf = xcf;
		ATMModel_struct_ptr[atmmodel_tot_rows].rms = xroughness;
		ATMModel_struct_ptr[atmmodel_tot_rows].weight = xweight;
		ATMModel_struct_ptr[atmmodel_tot_rows].tweight = tweight;
		ATMModel_struct_ptr[atmmodel_tot_rows].ascend = ascend; // note: ascned comes from w=column=15 of ATM csv file.
		atmmodel_tot_rows++;// counter - max will be the max num of rows in atmmodel_csvfile.csv
	}

	// printf("ATMModel_struct_ptr now is: %d \n", ATMModel_struct_ptr->d_name);
	fclose(fPtr_csv);

	printf("c: total rows of ATMModel-DataStruct: %d \n" , atmmodel_tot_rows);

	/* //////////////////////////// reads ATM csv file ///////////////////////////////////////////////////////////// */


	/* //////////////////////////// Get list of MISR Masked files /////////////////////////////////////////////// */ //ok
	printf("\nc: toa dir: %s \n" , masked_toa_an_dir);
	printf("c: make a list from masked TOA An files ...\n"); // == toa_an_files_list_ptr
	
	dirp = opendir(masked_toa_an_dir); // define dir stream == dirp == ptr to that directory
	if (dirp) {     // if ptr available == TRUE
		while ((DirEntryObj = readdir(dirp)) != NULL) {     // read the first item in dir and moves the ptr to next item/file in dir; DirEntryObj == struct==fileObj for each file in dir
			
			if (!strstr(DirEntryObj->d_name, ".dat")) continue; // d_name= fileName, if could not find the pattern ".dat" in this string
			
			if (toa_an_files_list_ptr == 0) {  // if it has not been created yet
				
				toa_an_files_list_ptr = (char **) malloc(sizeof(char *));   // allocate mem- for 1st entry
				if (!toa_an_files_list_ptr) {  // if its reverse was true....
					printf("main: couldn't malloc toa_an_files_list_ptr\n");
					return 0;
				}
			}
			else{
				
				toa_an_files_list_ptr = (char **) realloc(toa_an_files_list_ptr, (total_toa_an_files + 1) * sizeof(char *));
				if (!toa_an_files_list_ptr) { // if not = to 1 == True...
					printf("getFileList: couldn't realloc atm_flist\n");
					return 0;
				}
			}

			toa_an_files_list_ptr[total_toa_an_files] = (char *) malloc(strlen(DirEntryObj->d_name) + 1); // allocate mem-

			if (!toa_an_files_list_ptr[total_toa_an_files]) {
				printf("main: couldn't malloc atm_flist[%d]\n", total_toa_an_files);
				return 0;
			}


			// copy one string to the fileList
			strcpy(toa_an_files_list_ptr[total_toa_an_files], DirEntryObj->d_name);  // fill the toa_an_files_list_ptr with d_name: misr surf files; from DirEntryObj=fileObj gets the fileName
			// printf("FOUND: d_name == TOA file: %s \n", DirEntryObj->d_name);       // d_name is char array inside <dirent.h>
			//printf("file no. %d, %s \n", total_toa_an_files, toa_an_files_list_ptr[total_toa_an_files]);
			total_toa_an_files ++; // counter of num of MISR files found == elements in toa_an_files_list_ptr
		}
		closedir (dirp);
	} 
	else {
		strcat(message, "Can't open ");
		strcat(message, masked_toa_an_dir);
		perror (message);
		return EXIT_FAILURE;
	}
	/* //////////////////////////// Get list of MISR Masked files //////////////////////////////////////////////////// */

	int total_batches = total_toa_an_files/total_threads; // total batches for toa files

	/* //////////////////////////////////////// Ehsan: process toa files ///////////////////////////////////////////// */
	
	printf("c: total toa (AN) files: %d \n" , total_toa_an_files);
	printf("\nc: In main: creating threads \n");
	printf("c: total threads: %d \n" , total_threads);
	printf("c: total batches of files: %d \n" , total_batches);

	// char* toa_list_str = "EhsanMosadeghAnbaran";

	/* define batches of toa input files- we don't input single toa files into alg. anymore, 
	but we use batches of toa files now. w/ batches we'll be able to run in a multi-threading cycle */
	for (int batch_iter = 0; batch_iter < total_batches; batch_iter++){ 

		// define batches of input file from toa list
		printf("***************************************************************************************************\n");
		printf("***************************************************************************************************\n");
		printf("c: start of batch: (%d/%d) \n" , batch_iter+1, total_batches);
		printf("***************************************************************************************************\n");
		printf("*************************************************************************************************\n\n");

		// we create x threads for a batch of input files
		for (int thread_iter = 0 ; thread_iter < total_threads; thread_iter++){ 


			/* extract information from input file names- we extract toa files out of toa.csv list for each batch and by using toa_file_iter */
			toa_file_iter = thread_iter + (batch_iter * total_threads);
			printf("c: toa file iterator (changes in for-loop): %d \n" , toa_file_iter);

			// file_char_label = toa_an_files_list_ptr[toa_file_iter];
			// printf("c: char (future toa-file): %c \n" , file_char_label);

			/* ******************* we find path, orbit, block numbers from file labels ***************************** */

			/* extract path number*/
			if (strstr(toa_an_files_list_ptr[toa_file_iter], "_P")) {   // search for _P in the file name, find _P in each surf file
				// printf("check path num \n");
				memset(spath,0,sizeof(spath)); // empty string before writing to it
				strncpy(spath, strstr(toa_an_files_list_ptr[toa_file_iter], "_P") + 2, 3); // find path
				printf("c: spath: %s\n", spath);
				path = atoi(spath); // find path num from surf file
				// printf("path from MISR: %d \n", path);
			}
			else{
				printf("No path info in file name. \n");
				return 1;
			}
	
			/* extract orbit number*/
			if (strstr(toa_an_files_list_ptr[toa_file_iter], "_O")) {  // check if orbit is found in file name
				// printf("check orbit number \n");
				memset(sorbit,0,sizeof(sorbit)); // empty string before writing to it
				strncpy(sorbit, strstr(toa_an_files_list_ptr[toa_file_iter], "_O") + 2, 6); // get the orbit num from surf file and copy to sorbit
				printf("c: sorbit: %s \n" , sorbit);
				orbit = atoi(sorbit); // find orbit no from surf file
				// printf("orbit: %d \n" , orbit);
			}
			else {
				printf("No orbit info in file name. \n");
				return 1;
			}

			/* extract block number*/
			if (strstr(toa_an_files_list_ptr[toa_file_iter], "_B")) {  // get the block number from surf file name
				memset(sblock,0,sizeof(sblock)); // empty string before writing to it
				strncpy(sblock, strstr(toa_an_files_list_ptr[toa_file_iter], "_B") + 2, 3);
				printf("c: sblock: %s \n", sblock);
				block = atoi(sblock);
				// printf("block: %d \n", block);
			}
			else{
				printf("No block info in file name. \n");
				return 1;
			}

			/* ******************* check if input file exists check if roughness-file exists *********************** */

			// char output_roughness_file [256];
			// char roughness_fullpath [256];

			// sprintf(output_roughness_file, "roughness_toa_refl_P%s_O%s_B%s.dat", spath, sorbit, sblock);
			// printf("c: outfile: %s\n", output_roughness_file);

			// sprintf(roughness_fullpath, "%s/%s", predicted_roughness_dir, output_roughness_file); 
			// printf("c: check if this roughness-toa file exists: %s\n", roughness_fullpath);

			// fPtr_rough = fopen(roughness_fullpath, "r");
			// if (fPtr_rough){
			// 	// fprintf(stderr, "c: main: roughness file exist on disc, continue to next file\n");
			// 	printf("c: main >>> roughness file exist on disc, continue to next file <<<\n");
			// 	fclose(fPtr_rough);
			// 	continue;
			// }

			/* ******************* we use An filenames to define/label Cf and Ca files ***************************** */
			// 

			//printf("misr list file: %s \n", toa_an_files_list_ptr[i]);
			sprintf(an_fname, "%s/%s", masked_toa_an_dir, toa_an_files_list_ptr[toa_file_iter]);      // stores surf fileName into an_fname buffer
			printf("c: an_fname: %s \n", an_fname);


			// do the same thing with ca, copy the same surf file into ca
			sprintf(ca_fname, "%s/%s", masked_toa_an_dir, toa_an_files_list_ptr[toa_file_iter]);
			// printf("ca_fname: %s \n", ca_fname);
			// substitute an with cf in both filelabel and directory label
			strsub(ca_fname, "An", "Ca");       // to rename dir 
			strsub(ca_fname, "_an", "_ca");     // to rename file label
			printf("c: ca_fname: %s \n", ca_fname);


			//printf("misr list file: %s \n", toa_an_files_list_ptr[i]);
			sprintf(cf_fname, "%s/%s", masked_toa_an_dir, toa_an_files_list_ptr[toa_file_iter]); // copy the same surf file into cf
			// printf("cf_fname: %s \n", cf_fname);
			// replace an with cf to create file label
			strsub(cf_fname, "An", "Cf");       // to rename dir // to substitute An dir label to Cf dir label
			strsub(cf_fname, "_an", "_cf");     // to rename file label   // to substitute an filelabel to cf filelabel
			printf("c: cf_fname: %s \n", cf_fname);

			/* ******************* we create dataStruct as the arg- for multithreading f() **************************** */

			/* build struct for each thread_iter */
			// struct contains: path and orbit Num, and 3 file names changes with iterations maybe???
			printf("c: create struct for thread_iter: %d \n" , thread_iter);

			toaFile_struct_ptr[thread_iter] = (toaFile_DS*) malloc(sizeof(toaFile_DS));
			// check if memory allocated
			if (toaFile_struct_ptr[thread_iter] == NULL) {
				printf("ERROR: memory not allocated for toa-DS variable \n");
				return 1;
			}
			// else {
			 printf("c: struct memory allocated successfully for thread_iter: %d \n" , thread_iter);
			// }
		
			/* ******************* we add variables/rows to toa-dataStruct  **************************** */

			/* assing variables to toa dataStruct by dereferencing each by operator: -> */
			printf("c: add variable/row for thread_iter: %d \n" , thread_iter);
			toaFile_struct_ptr[thread_iter]->total_toa_files = total_toa_an_files;
			toaFile_struct_ptr[thread_iter]->toa_file_count = toa_file_iter;
			toaFile_struct_ptr[thread_iter]->path = path;
			toaFile_struct_ptr[thread_iter]->orbit = orbit;
			toaFile_struct_ptr[thread_iter]->block = block;
			toaFile_struct_ptr[thread_iter]->amtmodel_total_rows = atmmodel_tot_rows;
			strcpy(toaFile_struct_ptr[thread_iter]->outFile_lable, toa_an_files_list_ptr[toa_file_iter]); // its string so we shoud copy it
			strcpy(toaFile_struct_ptr[thread_iter]->ca, ca_fname);
			strcpy(toaFile_struct_ptr[thread_iter]->cf, cf_fname);
			strcpy(toaFile_struct_ptr[thread_iter]->an, an_fname);
			strcpy(toaFile_struct_ptr[thread_iter]->outDir, predicted_roughness_dir);
			// strcpy(toaFile_struct_ptr[thread_iter]->valid, 1);

			// if (toaFile_struct_ptr.valid == 0){
			// 	continue;
			// }

			/* ******************* we create threads and pass ptr-to-struct as an arg  **************************** */

			/* create thread for each struct; each struct is a single diff toaFile */
			// printf("run pthread... \n");
			ret1 = pthread_create(&tid[thread_iter], NULL, &multithread_task, (void*) toaFile_struct_ptr[thread_iter]); // called to create threads
			// printf("retruned: %d \n" , !ret);
			if (ret1 != 0) {  // Q- what happens here if multithread_task() returns 1 == error? does this code terminate? or goes to next iteration?
				/* check not-exist true(success)-signal */
				printf("ERROR: pthread_create() failed \n");
				return 1;
			}
		}
		

		/* ******************* we wait for threads to finish  **************************************************** */

		/* wait for thread structs to finish and join us */
		int msg;

		for (int thread_iter = 0; thread_iter < total_threads; thread_iter++) {

			ret2 = pthread_join(tid[thread_iter], (void**) &msg); // current tid == forces main to wait until threads finish // blocks the calling thread until the specified thread terminates.
			// check return of join()
			if (ret2 != 0){
				printf("ERROR: pthread_join() failed \n");
				return 1;
			}
			else{
				printf("c: Waiting for thread %d to finish ********* \n" , thread_iter);
			}
			// printf("tid %d return status: %d \n" , thread_iter, msg); // obtail return status for future debuging
		}

		/* ******************* we free allocated memory to created threads **************************************** */

		/* free memory of struct[thread] */
		for (int thread_iter = 0; thread_iter < total_threads; thread_iter++) {
			// free allocated memory
			free(toaFile_struct_ptr[thread_iter]);
		}
		printf("c: in main(): out of pThread(), batch iteration ended, and we did free allocated memory to all struct[thread] \n");

		// exit threads?
		// pthread_exit(NULL); // to terminate threads, where/how use it???


	}

	/* free all allocated memory here*/
	free(toa_an_files_list_ptr);
	free(ATMModel_struct_ptr);

	// finish main() with success signal
	return 0;
}


// ##################################### multithreaded function as a task #############################################

void* multithread_task(void* arg_ptr) { // function definitions, q- what part of code.c should be parallel or is good for parallel exe.? should be a completely independent part for parallel execution. 

	// declare IDfires
	double *an_masked_toa, *cf_masked_toa, *ca_masked_toa;
	// float* roughness_mem_block_ptr;
	double* roughness_mem_block_ptr;

	int blockElements = nlines * nsamples; // blockElements
	char roughness_fname[256];

	// define ptr to DStruct
	toaFile_DS* inputStruct_ptr = (toaFile_DS*) arg_ptr; // define new ptr to be clear

	// declare IDfiers/variables
	int r2, c2;
	double xcf, xca, xan, tweight, xdata_distance, xvector_min_len, xrough_nearest, xroughness, lat, lon;;
	// float xroughness, lat, lon;

	printf("\n********** processing input (%d/%d): %s \n" , inputStruct_ptr->toa_file_count+1, inputStruct_ptr->total_toa_files, inputStruct_ptr->an);

	// printf("c: reading each MISR image/block from 3 camera files into memory...\n");
   
	/* read MISR input files */
	if (!read_data(inputStruct_ptr->an, nlines, nsamples, &an_masked_toa)) { // we fill an_masked_toa array from: an_fname
		printf("ERROR: check AN read_data: file name available? \n"); // Q- what happens in main() if here returns 1?
		return 1; // return 0 or 1? also, how can this return some signal similar to continue that carries the process to next itteration in the multi-thread loop? can we return continue here?
	}

	if (!read_data(inputStruct_ptr->ca, nlines, nsamples, &ca_masked_toa)) {
		printf("ERROR: check CA read_data: file name available? \n");
		return 1;
	}

	if (!read_data(inputStruct_ptr->cf, nlines, nsamples, &cf_masked_toa)) {
		printf("ERROR: check CF read_data: file name available? \n");
		return 1;
	}

	
	int roughness_mem_block_layers = 5; // 5 for lat, lon,

	roughness_mem_block_ptr = (double *) malloc(roughness_mem_block_layers * nlines * nsamples * sizeof(double)); // allocate mem- for 5 layers of data
	// roughness_mem_block_ptr = (float *) malloc(roughness_mem_block_layers * nlines * nsamples * sizeof(float)); // allocate mem- for 5 layers of data: lat, lon, roughness, what if we change dtype to float?

	double radius = 0.025; // check w/ Anne

	int descend_mode = 1; // Ehsan: check w/ Anne: does it define descending for all blocks?

	// printf("descend_mode now is= %d \n" , descend_mode);

	// printf("inverse descend_mode ~ %d \n" , ~descend_mode);

	// printf("inverse descend_mode ! %d \n" , !descend_mode);


	//////////////////////////////////////////////////////////////////////////////
	/*  */
	printf("c: now processing 3 MISR toa_block.dat files in memory \n");

	for (int r = 0; r < nlines; r++) { // r==row
		for (int c = 0; c < nsamples; c++) { // c==column

			if (!misrPixel2LatLon(inputStruct_ptr->path, inputStruct_ptr->block, r, c, &lat, &lon, &r2, &c2)) { // E- r2 and c2 not used, should I modify misrPixel2LatLon and remove both? I aassume it was used for Land mask and we don't need land mask here anymore
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

			//////////////////////////////////////////////////////////////////////////////
			xroughness = 0;
			tweight = 0;
			xvector_min_len = 1e23; // 22fold of 10

			// use this print to monitor each toa pixel
			// printf("now sort each ATMModel.csv rows for new MISR pixel data_vector= (%d, %d) \n" , r, c);


			for (int n = 0; n < inputStruct_ptr->amtmodel_total_rows; n++) { // we compare each MISR pixel with all ATM.csv rows
			
				// printf("we set ascend to: %d in ATMModel_struct_ptr.\n" , !ATMModel_struct_ptr[n].ascend);


				//if (ATMModel_struct_ptr[n].ascend != ascend) continue;
				//if (~ascend || ((block < 20) && (ATMModel_struct_ptr[n].block < 20)) || ((block >= 20) && (ATMModel_struct_ptr[n].block >= 20))) {

				// check w/ Anne: what is this condition?
				// if ( (~ascend  && ((ATMModel_struct_ptr[n].block < 20) || // || if any == 1 then True...GO
				//      ~ATMModel_struct_ptr[n].ascend)) || 
				//      (ascend && (ATMModel_struct_ptr[n].block >= 20) && (ATMModel_struct_ptr[n].ascend)))  // && == if all 1 then GO

				if (descend_mode) {  // printf("we do this section, without inverting ca/cf cameras. \n");
					/* check w/ Anne - 
					is it necessary to compute this here? or can take out of the for-loop? 
					why they get subtracted from each other?for example: why for AN: (MISR - ATM?) both are MISR TOA refl values! */
					xan = (an_masked_toa[r * nsamples + c] - ATMModel_struct_ptr[n].an);
					xca = (ca_masked_toa[r * nsamples + c] - ATMModel_struct_ptr[n].ca); // Anne: difference is: unknown/unseen/new data - trainign data 
					xcf = (cf_masked_toa[r * nsamples + c] - ATMModel_struct_ptr[n].cf);

					// check w/ Anne - why values are different?
					// printf("an_masked_toa= %f \n" , an_masked_toa[r * nsamples + c]);
					// printf("an_atmmodel= %f \n" , ATMModel_struct_ptr[n].an);

					// printf("cf_atmmodel= %f \n" , ATMModel_struct_ptr[n].cf);
					// printf("an_atmmodel= %f \n" , ATMModel_struct_ptr[n].an);


					// printf("xan= %f \n" , xan);
					// printf("xca= %f \n" , xca);
					// printf("xcf= %f \n" , xcf);
				}
				else {  // maybe turn this section off?
					// is this the correction swection?
					printf("check: we do NOT run this section for inverting cameras. \n");
					xan = (an_masked_toa[r*nsamples + c] - ATMModel_struct_ptr[n].an);
					xca = (cf_masked_toa[r*nsamples + c] - ATMModel_struct_ptr[n].ca);
					xcf = (ca_masked_toa[r*nsamples + c] - ATMModel_struct_ptr[n].cf);
				}

				/***
				xan = (an_masked_toa[r*nsamples + c] - ATMModel_struct_ptr[n].an);
				xca = (ca_masked_toa[r*nsamples + c] - ATMModel_struct_ptr[n].ca);
				xcf = (cf_masked_toa[r*nsamples + c] - ATMModel_struct_ptr[n].cf);
				***/

				// distance as similarity, from each 3 pixel
				xdata_distance = sqrt(xan * xan + xca * xca + xcf * xcf); // E- distance == lenght of toa-refl vector in MISR 3D feature space
				
				// check if data_vector is qualified? // compare each pixel_data_vector w/ all ATM.csv rows?
				if (xdata_distance < radius) {

					xroughness += ATMModel_struct_ptr[n].tweight * ATMModel_struct_ptr[n].rms; // E- label data_vector here; changed xrms == xroughness; check w/ Anne: how about if tweight=0?
					tweight += ATMModel_struct_ptr[n].tweight;

					// sorting here?
					if (xdata_distance < xvector_min_len) {
						
						xvector_min_len = xdata_distance; // E- update min-lengh and lower threshold to closer the range
						xrough_nearest = ATMModel_struct_ptr[n].rms;
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
	
	// we use toa_an.dat file to rename output file
	sprintf(roughness_fname, "%s/%s", inputStruct_ptr->outDir, inputStruct_ptr->outFile_lable);
	strsub(roughness_fname, "_an.dat", ".dat");
	strsub(roughness_fname, "masked", "roughness");  // E: renamed output file

	//fp = fopen(roughness_fname, "wb");
	printf("c: writing roughness block: (%d) \n", inputStruct_ptr->toa_file_count+1);
	printf("%s \n" , roughness_fname);
	
	write_data(roughness_fname, roughness_mem_block_ptr, nlines, nsamples);

	printf("c: *** SUCCESSFULLY FINISHED writing roughness block: (%d) \n", inputStruct_ptr->toa_file_count+1);

	// free allocated memory
	free(an_masked_toa);
	free(ca_masked_toa);
	free(cf_masked_toa);
	free(roughness_mem_block_ptr);

	printf("c: f(pThread) we free allocated mem- to toa-file (%d) & exit its pthread.  \n" , inputStruct_ptr->toa_file_count+1); // " %ld! \n" , (long) inStructPtr+1);
	
	// close all open files
	pthread_exit((void*)0); // terminate when tid completes its work

	// return 0; // no return cuz its void
}


// ##################################### multithreaded function as a task #############################################





//######################################################################################################################

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

//######################################################################################################################

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

//######################################################################################################################

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

//######################################################################################################################

// int pixel2grid(int path, int block, int line, int sample, double* xlat, double* xlon, int* r, int* c)
int misrPixel2LatLon(int path, int block, int line, int sample, double* xlat, double* xlon, int* r, int* c) { // receives path-block-line-sample as pointer(mem-add) and updates/outputs xlat,xlon,r,c in mem-

	int status;
	char *errs[] = MTK_ERR_DESC;
	double lat, lon;
	/* parameters for grid stereographic :              */
	/* MOD44W.A2000055.h14v01.005.2009212173527_MOD44W_250m_GRID.dat    */ 
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
//######################################################################################################################

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
//######################################################################################################################

