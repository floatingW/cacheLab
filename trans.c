/* 
 * Wei Fu
 * 961028
 * 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

#define B1 8
#define B2 4
#define B3 16 

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp, i1, j1;
    if(M == 32) {
        for (i = 0; i < N; i+=B1) {
            for (j = 0; j < M; j+=B1) {
                for(i1 = i; i1 < i+B1; i1++) {
                    for(j1 = j; j1 < j+B1; j1++) {
                        tmp = A[i1][j1];
                        B[j1][i1] = tmp;
                    }
                }
            }
        }    
    }

    if(M == 64) {
        for (i = 0; i < N; i+=B2) {
            for (j = 0; j < M; j+=B2) {
                for(i1 = i; i1 < i+B2; i1++) {
                    for(j1 = j; j1 < j+B2; j1++) {
                        tmp = A[i1][j1];
                        B[j1][i1] = tmp;
                    }
                }
            }
        }    
    }

    if(M == 61) {
        for (i = 0; i < N; i+=B3) {
            for (j = 0; j < M; j+=B3) {
                for(i1 = i; i1 < i+B3 && i1 < N; i1++) {
                    for(j1 = j; j1 < j+B3 && j1 < M; j1++) {
                        tmp = A[i1][j1];
                        B[j1][i1] = tmp;
                    }
                }
            }
        }    
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

