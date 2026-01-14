#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <time.h>

// Threshold to switch from parallel tasks to serial execution.
// Prevents task creation overhead for small subarrays.
#define MINWORTH 10000

/**
 * Standard Merge function.
 * Uses a pre-allocated temporary buffer (tmp) passed as an argument
 * to avoid expensive malloc/free calls inside the recursive loop.
 */
void merge(int *a, int *tmp, int left, int mid, int right) {
    // Copy data to temp array.
    // Safe in parallel because tasks operate on disjoint memory regions.
    for (int i = left; i <= right; i++) {
        tmp[i] = a[i];
    }

    int i = left;       // Pointer for left subarray
    int j = mid + 1;    // Pointer for right subarray
    int k = left;       // Pointer for sorted array

    while (i <= mid && j <= right) {
        if (tmp[i] <= tmp[j]) {
            a[k++] = tmp[i++];
        } else {
            a[k++] = tmp[j++];
        }
    }

    // Copy remaining elements
    while (i <= mid)   a[k++] = tmp[i++];
    while (j <= right) a[k++] = tmp[j++];
}

/**
 * Serial Merge Sort.
 * Used for small problem sizes to maximize CPU efficiency.
 */
void mergeSort_serial(int *a, int *tmp, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergeSort_serial(a, tmp, left, mid);
        mergeSort_serial(a, tmp, mid + 1, right);
        merge(a, tmp, left, mid, right);
    }
}

/**
 * Parallel Merge Sort using OpenMP Tasks.
 * Implements a "Divide and Conquer" strategy.
 */
void mergeSort_parallel(int *a, int *tmp, int left, int right) {
    if (left < right) {
        // Optimization: Fallback to serial sort if the subarray is small.
        // This avoids the overhead of managing tasks for trivial work.
        if ((right - left) < MINWORTH) {
            mergeSort_serial(a, tmp, left, right);
            return;
        }

        int mid = left + (right - left) / 2;

        // Spawn tasks for the left and right subarrays.
        // 'shared' clause is used for arrays as they are allocated in main scope.
        #pragma omp task shared(a, tmp)
        mergeSort_parallel(a, tmp, left, mid);

        #pragma omp task shared(a, tmp)
        mergeSort_parallel(a, tmp, mid + 1, right);

        // Wait for both sub-tasks to complete before merging.
        #pragma omp taskwait
        merge(a, tmp, left, mid, right);
    }
}

/**
 * Utility: Checks if the array is sorted in ascending order.
 */
int is_sorted(int *a, int n) {
    for (int i = 1; i < n; i++)
        if (a[i-1] > a[i])
            return 0;
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <size> <serial|parallel> <threads>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    int is_parallel = (strcmp(argv[2], "parallel") == 0);
    int threads = atoi(argv[3]);

    if (is_parallel) {
        omp_set_num_threads(threads);
    }

    // Allocate memory
    int *a = malloc(n * sizeof(int));
    int *tmp = malloc(n * sizeof(int)); // Pre-allocated buffer for merge operations

    if (a == NULL || tmp == NULL) {
        fprintf(stderr, "Memory allocation failed!\n");
        return 1;
    }

    // Deterministic random initialization
    srand(42); 
    for (int i = 0; i < n; i++) {
        a[i] = rand();
    }

    printf("Sorting %d elements (%s) with %d threads...\n", 
           n, is_parallel ? "Parallel" : "Serial", is_parallel ? threads : 1);

    double start = omp_get_wtime();
    
    if (is_parallel) {
        // Start parallel region
        #pragma omp parallel
        {
            // Only one thread starts the recursion (Root task)
            #pragma omp single
            {
                mergeSort_parallel(a, tmp, 0, n - 1);
            }
        }
    } else {
        mergeSort_serial(a, tmp, 0, n - 1);
    }
    
    double end = omp_get_wtime();

    printf("Sorted: %s\n", is_sorted(a, n) ? "YES" : "NO");
    printf("Execution Time: %.6f seconds\n", end - start);

    free(a);
    free(tmp);
    return 0;
}