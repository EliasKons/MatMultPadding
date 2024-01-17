#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <malloc.h>
#include "timer.h"
#include "aux.h"

int m, n, p, thread_count, doubles_per_cache_line, cells_per_thread;
double *A, *B, *Y, time1, time2;

void *mult_matrix_parallel(void *arg) {
    int rank = *(int *)arg, local_m, first_row, last_row, l1, l2;

    local_m = m / thread_count;
    first_row = rank * local_m;
    if (rank + 1 == thread_count) { // Last thread in pool
        last_row = m - 1;
    }
    else {                       
        last_row = (rank + 1) * local_m - 1;
    }

    l1 = rank * (cells_per_thread + doubles_per_cache_line); // start of thread-cells
    for (int i = first_row; i <= last_row; i++) {
        l2 = l1 + (i - first_row) * p; // specific row
        for (int j = 0; j < p; j++) {
            Y[l2 + j] = 0.0;
            for (int k = 0; k < n; k++) {
                Y[l2 + j] += A[i*n + k] * B[k*p + j];
            }
        }
    }

    free(arg);
    pthread_exit(NULL);
}



int main(int argc, char **argv) {
    long cache_line = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    int min = 1, max = 10, err, *rank;
    double start, finish;
    char *output;
    size_t total_size, size_needed;
    pthread_t *thread_pool;
    FILE *fp;

    srand(time(NULL));

    
    // Checking input parameters

	if (argc != 6) {
		fprintf(stderr, "main [m] [n] [p] [thread count] [output file]\n");
		exit(EXIT_FAILURE);
	}

	m = atoi(argv[1]);
	n = atoi(argv[2]);
	p = atoi(argv[3]);
    thread_count = atoi(argv[4]);
    output = argv[5];

    if (m <= 0 || n <= 0 || p <= 0) {
        fprintf(stderr, "Matrix dimensions must be non-zero positive integers!\n");
        exit(EXIT_FAILURE);
    }
    else if (thread_count <= 0) {
        fprintf(stderr, "Number of threads must be a non-zero positive integer!\n");
        exit(EXIT_FAILURE);
    }

    // Initialization of matrices

    GET_TIME(start);

    A = malloc((m * n) * sizeof(double));
    B = malloc((n * p) * sizeof(double));

    // padding when necessary to avoid false-sharing
    // global 'columns' variable is the columns Y will have. It may not always be equal to p, due to padding
    cells_per_thread = (m * p) / thread_count; 

    if(m / thread_count == 0) {
        thread_count = 1;
        cells_per_thread = m*p;
    }


    doubles_per_cache_line = cache_line / sizeof(double);
    total_size = thread_count * (cells_per_thread + doubles_per_cache_line);
    size_needed = total_size * sizeof(double);
    Y = memalign(cache_line, size_needed);

    fill_matrix(A, m, n, min, max);
    fill_matrix(B, n, p, min, max);

    GET_TIME(finish); 
    time1 = finish - start;

    // Matrix multiplication 

    if (thread_count == 1 || (m / thread_count) == 0) { // single thread 
        GET_TIME(start);
        for (int i = 0; i < m; i++) {
            for (int j = 0; j < p; j++) {
                Y[i*p + j] = 0.0;
                for (int k = 0 ; k < n ; k++) {
                    Y[i*p + j] += A[i*n + k] * B[k*p + j];
                }
            }
        }
        GET_TIME(finish);
    }
    else { // multi-thread
        GET_TIME(start);
        thread_pool = malloc(thread_count * sizeof(pthread_t));

        for (int i = 0; i < thread_count; i++) {
            rank = malloc(sizeof(int)); *rank = i;
            err = pthread_create(&thread_pool[i], NULL, mult_matrix_parallel, (void *)rank);
            if (err != 0) {
                my_perror("pthread_create failed", err);
                exit(EXIT_FAILURE);
            }
        }

        for (int i = 0; i < thread_count; i++) {
            err = pthread_join(thread_pool[i], NULL);
            if (err != 0) {
                my_perror("pthread_join failed", err);
                exit(EXIT_FAILURE);
            }
        }
        free(thread_pool);
        

        GET_TIME(finish); 
    }

    time2 = finish - start;

    // writing to a csv file

    fp = fopen(output, "a");

    if (fp == NULL) {
        perror(output);
        exit(EXIT_FAILURE);
    }
    fprintf(fp, "%e, %e, %d, %d, %d, %d\n",time1, time2, thread_count, m, n, p);

    // freeing all memory 
    
    free(A);
    free(B);
    free(Y);
    fclose(fp);

    exit(EXIT_SUCCESS);
}