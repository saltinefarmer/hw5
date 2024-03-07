#ifndef MMM_H_
#define MMM_H_

// shared globals
extern unsigned int mode;
extern unsigned int size, num_threads;
extern double **A, **B, **SEQ_MATRIX, **PAR_MATRIX;

void mmm_init();
void mmm_reset(double **);
void mmm_freeup();
void mmm_seq();
void* mmm_par(void *);
double mmm_verify();

typedef struct thread_args {

    int tid;
    int start_row;
    int start_col;
    int end_row;
    int end_col;


} thread_args;

#endif /* MMM_H_ */
