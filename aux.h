/* Auxiliary functions  */


// get random double in range [min, max]
double random_double(double min, double max); 


// fill NxM matrix with random doubles in range [min, max]
void fill_matrix(double *matrix, int n, int m, int min, int max);


// print NxM matric
void print_matrix(double *matrix, int n, int m);


// when using pthread library functions, since perror does not work for these functions
void my_perror(char *message, int errnum);


