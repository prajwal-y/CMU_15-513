/* 
 * Name: Prajwal Yadapadithaya
 * Andrew ID: pyadapad
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
#include "contracts.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. The REQUIRES and ENSURES from 15-122 are included
 *     for your convenience. They can be removed if you like.
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;
    int ii, jj;
    int temp, temp1, temp2, temp3, temp4, temp5, temp6, temp7;

    REQUIRES(M > 0);
    REQUIRES(N > 0);

    if (M == 32 && N == 32) {
        temp1 = 8;
        temp2 = 8;
    }
    else if (M == 61 && N == 67) {
        temp1 = 16;
        temp2 = 16;
    }
    if(M == 32 || M == 61)
    for(ii = 0; ii < N; ii = ii + temp1) {
        for(jj = 0; jj < M; jj = jj + temp2) {
            for (i = ii; (i < (ii + temp1) && i < N); i++) {
                for (j = jj; (j < (jj + temp2) && j < M); j++) {
                    if(i != j)
                        B[j][i] = A[i][j];
                    else {
                        temp = A[i][j];
                        temp3 = i;
                    }
                }
                if (ii == jj)
                    B[temp3][temp3] = temp;
            }
        }
    }
    else if (M == 64) {
    for(ii = 0; ii < N; ii = ii + 8) {
        for(jj = 0; jj < M; jj = jj + 8) {
	  
	  //Blocks causing conflict misses (Eg,(4,0), (12,8), (20,16) and so on.
	  //Trying to use temporary buffer variables for as many as possible.
          temp = A[ii][jj + 4];
          temp1 = A[ii][jj + 5];
          temp2 = A[ii][jj + 6];
          temp3 = A[ii][jj + 7];
          j = A[ii + 1][jj + 7];

          for (i=0; i<8; i++) {
	    temp4 = A[ii + i][jj];
            temp5 = A[ii + i][jj + 1];
            temp6 = A[ii + i][jj + 2];
            temp7 = A[ii + i][jj + 3];

            B[jj][ii + i] = temp4;
            B[jj + 1][ii + i] = temp5;
            B[jj + 2][ii + i] = temp6;
            B[jj + 3][ii + i] = temp7; 
          }
                
          for (i=1; i<8; i++) {
            temp4 = A[ii + i][jj + 4];
            temp5 = A[ii + i][jj + 5];
            temp6 = A[ii + i][jj + 6];
            if(i!=1)
              temp7 = A[ii + i][jj + 7];

            B[jj + 4][ii + i] = temp4;
            B[jj + 5][ii + i] = temp5;
            B[jj + 6][ii + i] = temp6;
            if(i!=1)
              B[jj + 7][ii + i] = temp7;
          }

          B[jj + 4][ii] = temp;
          B[jj + 5][ii] = temp1;
          B[jj + 6][ii] = temp2;
          B[jj + 7][ii] = temp3;
          B[jj + 7][ii + 1] = j;

        }
    }
    }

    ENSURES(is_transpose(M, N, A, B));

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

    int i, j;

    REQUIRES(M > 0);
    REQUIRES(N > 0);

    for(i =0; i < M; i++) {
        for(j = 0; j < N; j++)
            B[j][i] = A[i][j];
    }

    ENSURES(is_transpose(M, N, A, B));

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

