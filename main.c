#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "rtclock.h"
#include "mmm.h"

// shared  globals
unsigned int mode; // 1 for Parallel, 2 for sequential
unsigned int size, num_threads;
// A and B are the matrices to multiply, seq and par are the respective outputs
double **A, **B, **SEQ_MATRIX, **PAR_MATRIX;

int main(int argc, char *argv[]) {

	// parse input
	char *ptr;
	if(argc <= 1){
		// too few parameters
		printf("Too few parameters\n");
		printf("./mmm P <threads> <size>\n");
		return 0;
	}
	else if(strcmp(argv[1], "P") == 0){
		// parallel
		if(argc != 4){
			printf("Incorrect number of parameters\n");
			printf("./mmm P <threads> <size>\n");
			return 0;
		} else{
			mode = 1;
			num_threads = strtol(argv[2], &ptr, 10);
			size = strtol(argv[3], &ptr, 10);
		}
		
	} else if (strcmp(argv[1], "S") == 0){
		// sequential
		if(argc != 3){
			printf("Incorrect number of parameters\n");
			printf("./mmm S <size>\n");
			return 0;
		} else{
			mode = 2;
			size = strtol(argv[2], &ptr, 10);
		}
		
	} else{
		// invalid input
		printf("Invalid Input\n");
		return 0;
	}

	// initialize my matrices
	mmm_init();

	// clocks
	double seq_clockstart = 0, seq_clockend = 0, par_clockstart = 0, par_clockend = 0, seq_time = 0, par_time = 0;
	
	for(int i = 0; i < 5; i++){ // begin looping! first iteration is thrown away :(

		// sequential timing
		seq_clockstart = rtclock();	// start the clock
		mmm_seq();
		seq_clockend = rtclock(); // stop the clock
		seq_time += seq_clockend - seq_clockstart;

	
		// parallel stuff
		if(mode == 1){
	
			par_clockstart = rtclock();

			// threads :(
			thread_args *t_args = (thread_args*) malloc(num_threads * sizeof(thread_args));
			int units = size * size;
			int units_p_thread = units / num_threads;
			
			if(units_p_thread == 0){
				units_p_thread = 1;
			}

			int units_assigned = 0;
			int row = 0;
			int col = 0;
			
			for(int i = 0; i < num_threads; i++){ // setting vals
			
				t_args[i].tid = i;
				t_args[i].start_row = row;
				t_args[i].start_col = col;


				units_assigned += units_p_thread;

				if(units_assigned >= units){
					row = size - 1;
					col = size;
				} else {
					
					row = (units_assigned-1) / size;
					col = units_assigned % size;
				}
				

				t_args[i].end_row = row;
				t_args[i].end_col = col;
			}

			// create threads
			pthread_t *threads = (pthread_t*) malloc(num_threads * sizeof(pthread_t));
			for(int i = 0; i < num_threads; i++){
				pthread_create(&threads[i], NULL, mmm_par, &t_args[i]);
			}

			// join threads
			for(int i = 0; i < num_threads; i++){
				pthread_join(threads[i], NULL);
			}

			par_clockend = rtclock(); // end clock

			// free
			free(threads);
			free(t_args);
			threads = NULL;
			t_args = NULL;

		}
		
	
		if(i == 0){ // throw away first round of tests
			seq_time = 0;
			par_time = 0;
		}

		// reset matrices to multiply again teehee
		mmm_reset(PAR_MATRIX);
		mmm_reset(SEQ_MATRIX);
	}

	// average the times
	par_time /= 4;
	seq_time /= 4;

	if(mode == 1){ // parallel printout stuffs
		printf("mode: Parallel\nthread count: %d\nsize: %d\n", 1, size);

		printf("Parallel time taken (average of 4 runs): %.6f sec\n", par_time);
		printf("Sequential time taken (average of 4 runs): %.6f sec\n", seq_time);


		printf("Error between parallel and sequential: %f\n", mmm_verify());
		printf("Speedup of Paralell: %f\n", seq_time / par_time);

	} else{ // sequential printout stuffs
		printf("mode: sequential\nthread count: %d\nsize: %d\n", num_threads, size);
		printf("Sequential time taken (average of 4 runs): %.6f sec\n", seq_time);
	}
	
	

	// free everything
	mmm_freeup();

	return 0;
}

