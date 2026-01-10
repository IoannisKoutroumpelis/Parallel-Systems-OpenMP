#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

//Minimum value that is worth using tasks
#define MINWORTH 10000

//Merge function
void merge(int *a, int left, int mid, int right) {
    int i, j, k;
    int n1 = mid - left +1;
    int n2 = right -mid;

    //Temp arrays
    int L[n1], R[n2];

    //copy data to temp arrays
    for (i = 0; i < n1; i++)
        L[i] = a[left + i];

    for (j = 0; j < n2; j++)
        R[j] = a[mid+1 + j];

    //Merge temp arrays back to a[left ... right]
    i = 0;
    j = 0;
    k = left;
    while (i < n1 && j < n2) {
        if(L[i] <= R[j]){
            a[k] = L[i];
            i++;
        } else {
            a[k] = R[j];
            j++;
        }
        k++;
    }

    // Copy the remaining elements of L[],
    // if there are any
    while (i < n1) {
        a[k] = L[i];
        i++;
        k++;
    }

    // Copy the remaining elements of R[],
    // if there are any
    while (j < n2) {
        a[k] = R[j];
        j++;
        k++;
    }
}

void mergeSort_serial(int *a, int left, int right){
    
    if (left < right) {
        int mid = left + (right - left) / 2;

        // Sort first and second halves
        mergeSort_serial(a, left, mid);
        mergeSort_serial(a, mid + 1, right);

        merge(a, left, mid, right);
    }
}

void mergeSort_parallel (int *a, int left, int right) {
    if(left < right) {
        int mid = left + (right - left) / 2;

        #pragma omp task if(right - left > MINWORTH)
        mergeSort_parallel(a, left, mid);
    
        #pragma omp task if(right - left > MINWORTH)
        mergeSort_parallel(a, mid+1, right);

        #pragma omp taskwait
        merge(a, left, mid, right);
    }
}

int is_sorted(int *a, int n) {
    for (int i = 1; i < n; i++)
        if (a[i-1] > a[i])
            return 0;
    
    return 1;
}


int main(int argc, char *argv[]) {
    if( argc != 4) {
        printf("Usage: %s <size> <serial|parallel> <threads>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);

    //if parallel == 1, parallel execution, else serial
    int parallel = strcmp(argv[2], "parallel") == 0;
    int threads = atoi(argv[3]);

    int *a = malloc(n * sizeof(int));

    //fill array a with random numbers
    srand(0);
    for (int i = 0; i < n; i++)
        a[i] = rand();

    double start = omp_get_wtime();
    if (parallel) {
            mergeSort_parallel(a, 0, n-1);
    } else {
        mergeSort_serial(a, 0, n-1);
    }
    double end = omp_get_wtime();

    printf("Sorted: %s\n", is_sorted(a, n) ? "YES" : "NO");
    printf("Time: %.6f seconds\n", end - start);

    free(a);
    return 0;
}