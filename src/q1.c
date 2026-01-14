#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

/**
 * Initializes a polynomial with random non-zero integers [-5, 5].
 */
void init_poly(int *poly, int n) {
    for (int i = 0; i <= n; i++) {
        int r = (rand() % 11) - 5; 
        poly[i] = (r == 0) ? 1 : r;
    }
}

/**
 * Serial Polynomial Multiplication.
 * Complexity: O(N^2)
 */
void serial_mult(int *A, int *B, long long *C, int n) {
    for (int i = 0; i <= 2 * n; i++) C[i] = 0;
    
    for (int i = 0; i <= n; i++) {
        for (int j = 0; j <= n; j++) {
            C[i + j] += (long long)A[i] * B[j];
        }
    }
}

/**
 * Parallel Polynomial Multiplication using Diagonal Iteration.
 * 
 * Strategy: Instead of iterating over input indices (i, j), we iterate over 
 * the result index 'k' (where k = i + j). 
 * Benefit: Each C[k] is computed independently by a single thread, eliminating 
 * race conditions without the need for expensive atomic operations.
 */
void parallel_mult(int *A, int *B, long long *C, int n) {
    // 'guided' schedule is used because work distribution is uneven:
    // middle elements require more computations than edge elements.
    #pragma omp parallel for schedule(guided)
    for (int k = 0; k <= 2 * n; k++) {
        long long sum = 0;
        
        // Determine valid range for 'i' such that:
        // 0 <= i <= n  AND  0 <= (k - i) <= n
        int start_i = (k > n) ? k - n : 0;
        int end_i = (k < n) ? k : n;

        for (int i = start_i; i <= end_i; i++) {
            sum += (long long)A[i] * B[k - i];
        }
        C[k] = sum;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <degree n> <threads>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    int threads = atoi(argv[2]);

    omp_set_num_threads(threads);
    srand(time(NULL));

    // Allocate memory (using long long for result to prevent overflow)
    int *A = malloc((n + 1) * sizeof(int));
    int *B = malloc((n + 1) * sizeof(int));
    long long *C_serial = malloc((2 * n + 1) * sizeof(long long));
    long long *C_parallel = malloc((2 * n + 1) * sizeof(long long));

    // --- Initialization ---
    double start = omp_get_wtime();
    init_poly(A, n);
    init_poly(B, n);
    double end = omp_get_wtime();
    printf("Initialization Time: %f sec\n", end - start);

    // --- Serial Execution ---
    start = omp_get_wtime();
    serial_mult(A, B, C_serial, n);
    end = omp_get_wtime();
    printf("Serial Execution Time: %f sec\n", end - start);

    // --- Parallel Execution ---
    start = omp_get_wtime();
    parallel_mult(A, B, C_parallel, n);
    end = omp_get_wtime();
    printf("Parallel Execution Time (%d threads): %f sec\n", threads, end - start);

    // --- Verification ---
    int correct = 1;
    for (int i = 0; i <= 2 * n; i++) {
        if (C_serial[i] != C_parallel[i]) {
            correct = 0;
            printf("Verification FAILED at index %d: Serial=%lld, Parallel=%lld\n", i, C_serial[i], C_parallel[i]);
            break;
        }
    }
    if (correct) printf("Verification: SUCCESS\n");

    free(A); free(B); free(C_serial); free(C_parallel);
    return 0;
}