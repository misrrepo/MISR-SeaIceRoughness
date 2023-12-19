#include <stdio.h> // to use printf 
#include <string.h> // to define string?
#include <pthread.h> // to use gcc? threading capability, cuz Clang does not have multithreading standard
#include <unistd.h> // to use sleep function
#include <stdlib.h> // for malloc()

// int main()
// {
//  char name[6]=""; //= "ehsan"; if we initialize it with "empty" there won't be junk character inside string

//  // strcpy(name, "ehsan"); // initialize later! we should use strcpy()

//  printf("hello, my name is %s! \n" , name);

//  for (int i=0; i<sizeof(name); i++)
//  {
//      printf("junk character: %c \n" , name[i]);
//  }


//  return 0;

// }

// =====================================================================================================================

// #define total_threads 5 // q- how many threads for my mac? how many logical processors?  how many for HPC? number of cpu cores on HPC?


// void* test_func(void* inStructPtr) { // function definitions, q- what part of code.c should be parallel or is good for parallel exe.? should be a completely independent part for parallel execution. 

// 	char name[6] = "ehsan";

// 	// int tid;
// 	// tid = (int) threadid;

// 	// 
// 	// some calculations here...

// 	printf("starting %s, tid num: %ld \n" , name, (long) inStructPtr+1);
// 	sleep(5);
// 	printf("finishing tid: %ld! \n" , (long) inStructPtr+1);
// 	// close all open files

// 	pthread_exit((void*)0); // terminate when tid completes its work

// 	// return 0;
// }


// int main() {

// 	pthread_t tid[total_threads]; // tid descriptor, create several tid identifires/descriptors/variables
// 	int ret1, ret2;

// 	printf("In main: creating threads \n");

// 	long file_num;
// 	int step=0;
// 	int eh_total_toa_files = 20;
// 	int total_batches = eh_total_toa_files/total_threads;

// 	printf("total cycles: %d \n" , total_batches);

// 	while(step<total_batches) {

// 		for (int thread_iter=0 ; thread_iter < total_threads; thread_iter++) {
// 			file_num = step * total_threads + thread_iter;
// 			// printf("file_num: %ld \n" , file_num);
// 			ret1 = pthread_create(&tid[file_num], NULL, &test_func, (void*) file_num); // called to create threads
// 			// printf("retruned: %d \n" , !ret);
// 			if (ret1 != 0) { // check not-exist true(success)-signal
// 				printf("ERROR: pthread_create() failed \n");
// 				return 1;
// 			}
// 		}

// 		int msg;
// 		for (int thread_iter=0; thread_iter<total_threads; thread_iter++) {
// 			ret2 = pthread_join(tid[thread_iter], (void**) &msg); // current tid == forces main to wait until threads finish // blocks the calling thread until the specified thread terminates.
// 			if (ret2 != 0){
// 				printf("ERROR: pthread_join() failed \n");
// 				return 1;
// 			}
// 			printf("tid %d return status: %d \n" , thread_iter, msg); // obtail return status for future debuging
// 		}

// 		step++;
// 	}
	
//     pthread_exit(NULL); // to terminate threads, where/how use it???

	
// 	return 0;
// }

// =====================================================================================================================


#define total_threads 5 		// q- how many threads for my mac? how many logical processors?  how many for HPC? number of cpu cores on HPC?


void* test_func(void* arg_ptr);

// struct shgould be defined as a global variable
// toa_dtype {
// 		int thread_id;
// 		int path;
// 		int orbit;
// 		char* ca;
// 		char* cf;
// 		char* an;
// 	}; // toa_dtype;

typedef struct {
		int toa_file_count;
		char toa_file_char;
		int path;
		int orbit;
		char ca[100]; // q- how define string w/ char* and strcpy string into it in main()?
		char cf[100];
		char an[100];
} toa_dtype;
// define ptr for our DS
toa_dtype* ptr_to_toa_struct[total_threads]; // array of ptrs w/ [size]; q- why define size here? why array of ptrs? ptr should be defibed as global var?



int main() {
	/* declare identifiers */
	pthread_t tid [total_threads]; // tid descriptor, create several tid identifires/descriptors/variables
	int ret1, ret2; // return values
	int eh_total_toa_files = 20;
	int total_batches = eh_total_toa_files/total_threads;
	int toa_index;
	char file_char_label;


	printf("In main: creating threads \n");
	printf("total threads: %d \n" , total_threads);
	printf("total input files batches: %d \n" , total_batches);

	char* toa_list_str = "EhsanMosadeghAnbaran";

	/* define batches of toa input files */
	for (int batch_iter = 0; batch_iter < total_batches; batch_iter++) { 

		// define batches of input file from toa list
		printf("***********************************\n");
		printf("batch_iter: (%d) \n\n" , batch_iter);

		// create threads
		for (int thread_iter = 0 ; thread_iter < total_threads; thread_iter++) { 


			/* extract information from file names*/






			toa_index = thread_iter + (batch_iter * total_threads);
			printf("toa-file-index: %d \n" , toa_index);

			file_char_label = toa_list_str[toa_index];
			printf("char (future toa-file): %c \n" , file_char_label);








			/* build struct for each thread_iter */
			// struct contains: path and orbit Num, and 3 file names changes with iterations maybe???
			printf("create struct for thread_iter: %d \n" , thread_iter);

			ptr_to_toa_struct[thread_iter] = (toa_dtype*) malloc(sizeof(toa_dtype));
			// check if mem allocated
			if (ptr_to_toa_struct[thread_iter] == NULL) {
				printf("ERROR: memory not allocated for DS variable \n");
				return 1;
			}
			// else {
			// 	printf("struct memory allocated successfully for thread_iter: %d \n" , thread_iter);
			// }
		

			/* assing variables to toa dataStruct by dereferencing each by operator: -> */
			printf("add variable/row for thread_iter: %d \n" , thread_iter);
			ptr_to_toa_struct[thread_iter]->toa_file_count = toa_index;
			ptr_to_toa_struct[thread_iter]->toa_file_char = file_char_label;
			ptr_to_toa_struct[thread_iter]->path = thread_iter+10;
			ptr_to_toa_struct[thread_iter]->orbit = thread_iter+100;
			strcpy(ptr_to_toa_struct[thread_iter]->ca, "ca_camera");
			strcpy(ptr_to_toa_struct[thread_iter]->cf, "cf_camera");
			strcpy(ptr_to_toa_struct[thread_iter]->an, "an_camera");



			/* create thread for each struct */
			// printf("run pthread... \n");
			ret1 = pthread_create(&tid[thread_iter], NULL, &test_func, (void*) ptr_to_toa_struct[thread_iter]); // called to create threads
			// printf("retruned: %d \n" , !ret);
			if (ret1 != 0) { 
				/* check not-exist true(success)-signal */
				printf("ERROR: pthread_create() failed \n");
				return 1;
			}
		}

		/* wait for thread structs to finish and join us */
		int msg;
		for (int thread_iter = 0; thread_iter < total_threads; thread_iter++) {

			ret2 = pthread_join(tid[thread_iter], (void**) &msg); // current tid == forces main to wait until threads finish // blocks the calling thread until the specified thread terminates.
			// check return of join()
			if (ret2 != 0){
				printf("ERROR: pthread_join() failed \n");
				return 1;
			}
			// printf("tid %d return status: %d \n" , thread_iter, msg); // obtail return status for future debuging
		}

		// free() // memory?
		// exit threads 
	    // pthread_exit(NULL); // to terminate threads, where/how use it???


	}
	// finish main() with success signal
	return 0;
}

void* test_func(void* arg_ptr) { // function definitions, q- what part of code.c should be parallel or is good for parallel exe.? should be a completely independent part for parallel execution. 

	// char name[6] = "ehsan";

	// int tid;
	// tid = (int) threadid;
	toa_dtype* inStruct_ptr = (toa_dtype*) arg_ptr; // define new ptr to be more clear
	
	printf("f(.): toa file count: %d \n" , inStruct_ptr->toa_file_count);
	printf("f(.): toa file char: %c \n" , inStruct_ptr->toa_file_char);

	printf("f(.): orbit: %d \n" , inStruct_ptr->orbit);
	printf("f(.): path: %d \n" , inStruct_ptr->path);
	sleep(10);
	printf("f(.): finishing tid, exiting pthread for file count: %d \n" , inStruct_ptr->toa_file_count); // " %ld! \n" , (long) inStructPtr+1);
	// close all open files
	pthread_exit((void*)0); // terminate when tid completes its work

	// return 0; // no return since its void
}



