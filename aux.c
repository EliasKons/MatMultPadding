#include <stdio.h>
#include <stdlib.h>
#include <string.h>


double random_double(double min, double max) {
    return ((double)rand() / (double)RAND_MAX) * (max - min) + min;
}


void fill_matrix(double *matrix, int n, int m, int min, int max) {
    for (int i = 0; i < n; i++){
        for (int j = 0; j < m; j++) {
            matrix[i * m + j] = random_double((double)min, (double)max);
        }
    }
}


void print_matrix(double *matrix, int n, int m) {
    for (int i = 0; i < n; i++){
        for (int j = 0; j < m; j++){
            printf("%.5f  ", matrix[i * m + j]);
        }
        printf("\n");
    }
}


void my_perror(char *message, int errnum) {
    fprintf(stderr, "%s: %s\n",message, strerror(errnum));
}
