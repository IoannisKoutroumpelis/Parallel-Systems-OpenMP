#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>

//Compressed Sparse Row (CSR) format structure
typedef struct {
    int *values;        // Non-zero values
    int *col_indices;   // Column indices for values
    int *row_ptr;       // Row offsets (size n_rows + 1)
    int num_non_zeros;  // Total non-zero elements
    int n_rows;         // Number of rows (N)
} CSRMatrix;

CSRMatrix create_csr_serial(int *dense, int n) {
    CSRMatrix csr;
    csr.n_rows = n;
    csr.row_ptr = (int *)malloc((n + 1) * sizeof(int));

    // Pass 1: count non-zeros per row
    int total_nnz = 0;
    for (int i = 0; i < n; i++) {
        int count = 0;
        for (int j = 0; j < n; j++) {
            if (dense[i * n + j] != 0)
                count++;
        }
        csr.row_ptr[i] = count;
        total_nnz += count;
    }

    // Pass 2: prefix sum
    int sum = 0;
    for (int i = 0; i < n; i++) {
        int temp = csr.row_ptr[i];
        csr.row_ptr[i] = sum;
        sum += temp;
    }
    csr.row_ptr[n] = sum;

    csr.num_non_zeros = total_nnz;
    csr.values = (int *)malloc(total_nnz * sizeof(int));
    csr.col_indices = (int *)malloc(total_nnz * sizeof(int));

    // Pass 3: fill CSR arrays
    for (int i = 0; i < n; i++) {
        int pos = csr.row_ptr[i];
        for (int j = 0; j < n; j++) {
            int val = dense[i * n + j];
            if (val != 0) {
                csr.values[pos] = val;
                csr.col_indices[pos] = j;
                pos++;
            }
        }
    }

    return csr;
}


void spmv_csr_serial(CSRMatrix *csr, int *x, int *y, int iterations) {
    int n = csr->n_rows;

    for (int iter = 0; iter < iterations; iter++) {
        for (int i = 0; i < n; i++) {
            int sum = 0;
            int start = csr->row_ptr[i];
            int end = csr->row_ptr[i + 1];

            for (int k = start; k < end; k++) {
                sum += csr->values[k] * x[csr->col_indices[k]];
            }
            y[i] = sum;
        }

        // y γίνεται x για την επόμενη επανάληψη
        if (iterations > 1 && iter < iterations - 1) {
            for (int k = 0; k < n; k++)
                x[k] = y[k];
        }
    }
}


void dense_mult_serial(int *matrix, int *x, int *y, int n, int iterations) {
    for (int iter = 0; iter < iterations; iter++) {
        for (int i = 0; i < n; i++) {
            int sum = 0;
            for (int j = 0; j < n; j++) {
                sum += matrix[i * n + j] * x[j];
            }
            y[i] = sum;
        }

        if (iterations > 1 && iter < iterations - 1) {
            for (int k = 0; k < n; k++)
                x[k] = y[k];
        }
    }
}



//Generates a dense matrix with a specified sparsity percentage
void generate_dense(int *matrix, int n, int sparsity_percent) {
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < n * n; i++) {
        if ((rand() % 100) < sparsity_percent) {
            matrix[i] = 0;
        } else {
            matrix[i] = (rand() % 10) + 1;
        }
    }
}


//Converts a dense matrix to CSR format in parallel
//Algorithm follows a 3-pass approach typical in HPC libraries
CSRMatrix create_csr_parallel(int *dense, int n) {
    CSRMatrix csr;
    csr.n_rows = n;
    csr.row_ptr = (int *)malloc((n + 1) * sizeof(int));

    // Pass 1: Count non-zero elements per row (Parallel)
    // No race conditions: each thread writes to a distinct row_ptr[i].
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < n; i++) {
        int count = 0;
        for (int j = 0; j < n; j++) {
            if (dense[i * n + j] != 0) count++;
        }
        csr.row_ptr[i] = count;
    }

    // Pass 2: Prefix Sum / Cumulative Scan (Sequential)
    // Determines the starting index for each row in the values array.
    int total_nnz = 0;
    int temp;
    for (int i = 0; i < n; i++) {
        temp = csr.row_ptr[i];
        csr.row_ptr[i] = total_nnz;
        total_nnz += temp;
    }
    csr.row_ptr[n] = total_nnz;
    csr.num_non_zeros = total_nnz;

    // Allocate memory for CSR data arrays
    csr.values = (int *)malloc(total_nnz * sizeof(int));
    csr.col_indices = (int *)malloc(total_nnz * sizeof(int));

    // Pass 3: Populate values and column indices (Parallel)
    // Safe because row_ptr provides exact offsets for each thread.
    #pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < n; i++) {
        int current_pos = csr.row_ptr[i];
        for (int j = 0; j < n; j++) {
            int val = dense[i * n + j];
            if (val != 0) {
                csr.values[current_pos] = val;
                csr.col_indices[current_pos] = j;
                current_pos++;
            }
        }
    }

    return csr;
}

//Sparse Matrix-Vector Multiplication (SpMV) using CSR.
//Computes y = A * x.
void spmv_csr_parallel(CSRMatrix *csr, int *x, int *y, int iterations) {
    int n = csr->n_rows;
    for (int iter = 0; iter < iterations; iter++) {
        // Use dynamic scheduling to handle load imbalance caused by
        // varying number of non-zeros per row.
        #pragma omp parallel for schedule(dynamic, 64)
        for (int i = 0; i < n; i++) {
            int sum = 0;
            int start = csr->row_ptr[i];
            int end = csr->row_ptr[i + 1];
            for (int k = start; k < end; k++) {
                sum += csr->values[k] * x[csr->col_indices[k]];
            }
            y[i] = sum;
        }
        
        // Copy y to x for next iteration (simulate iterative solver step)
        if (iterations > 1 && iter < iterations - 1) {
             #pragma omp parallel for
             for(int k=0; k<n; k++) x[k] = y[k];
        }
    }
}

//Baseline Dense Matrix-Vector Multiplication.
void dense_mult_parallel(int *matrix, int *x, int *y, int n, int iterations) {
    for (int iter = 0; iter < iterations; iter++) {
        #pragma omp parallel for default(none) private(iter) shared(matrix, x, y, n) schedule(static)
        for (int i = 0; i < n; i++) {
            int sum = 0;
            for (int j = 0; j < n; j++) {
                sum += matrix[i * n + j] * x[j];
            }
            y[i] = sum;
        }
        
        if (iterations > 1 && iter < iterations - 1) {
             #pragma omp parallel for
             for(int k=0; k<n; k++) x[k] = y[k];
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("Usage: %s <N> <Sparsity %%> <Iterations> <Threads>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    int sparsity = atoi(argv[2]);
    int iterations = atoi(argv[3]);
    int threads = atoi(argv[4]);

    omp_set_num_threads(threads);
    srand(time(NULL));

    // Allocation
    int *dense_matrix = (int *)malloc(n * n * sizeof(int));
    int *x = (int *)malloc(n * sizeof(int));
    int *y = (int *)malloc(n * sizeof(int));
    
    for(int i=0; i<n; i++) x[i] = 1;

    generate_dense(dense_matrix, n, sparsity);
    printf("Matrix Size: %dx%d, Sparsity: %d%%, Threads: %d\n", n, n, sparsity, threads);

    // 1. CSR Parallel Construction
    double start = omp_get_wtime();
    CSRMatrix csr = create_csr_parallel(dense_matrix, n);
    double end = omp_get_wtime();
    printf("CSR Construction parallel: %f sec\n", end - start);

    // 2. CSR Serial Construction
    for(int i=0; i<n; i++) x[i] = 1;
    double startS = omp_get_wtime();
    CSRMatrix csr_serial = create_csr_serial(dense_matrix, n);
    double endS = omp_get_wtime();
    printf("CSR Construction serial: %f sec\n", endS - startS);

    // 3. Serial SpMV
    for(int i=0; i<n; i++) x[i] = 1;
    startS = omp_get_wtime();
    spmv_csr_serial(&csr_serial, x, y, iterations);
    endS = omp_get_wtime();
    printf("SpMV CSR serial: %f sec\n", endS - startS);

    // 3. Parallel SpMV Execution (CSR)
    for(int i=0; i<n; i++) x[i] = 1; // Reset vector
    start = omp_get_wtime();
    spmv_csr_parallel(&csr, x, y, iterations);
    end = omp_get_wtime();
    printf("SpMV (CSR) parallel:       %f sec\n", end - start);

    // 4. Dense Execution (Baseline)
    for(int i=0; i<n; i++) x[i] = 1; // Reset vector
    start = omp_get_wtime();
    dense_mult_parallel(dense_matrix, x, y, n, iterations);
    end = omp_get_wtime();
    printf("Dense Mult Time:       %f sec\n", end - start);

    // Cleanup
    free(dense_matrix); free(x); free(y);
    free(csr.values); free(csr.col_indices); free(csr.row_ptr);

    return 0;
}