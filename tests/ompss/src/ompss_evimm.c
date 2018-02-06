/*!
 * \file ompss_evimm.c
 *
 * \version     v1.0
 * \date        2016-02-13
 *
 * This example implements Matrix Multiplication on top of OmpSS in the same way
 * of MM on JUMP.
 *
 * Copyright (C) 2016, Evidence Srl.
 * Terms of use are as specified in COPYING
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>

#include "time.h"

//#define NO_COMPUTE
//#define MATRIX_DEBUG
#define FINAL_CHECK

#define DEFBLOCKSIZE       25
#define DEFN               300

static int BLOCKSIZE=DEFBLOCKSIZE;
static int N=DEFN;

#define logmsg(msg, ...) {\
    struct timespec _t1;\
    time_t _secs,_min;\
    clock_gettime(CLOCK_REALTIME_COARSE,&_t1);\
    _secs=_t1.tv_sec%60;\
    _min=(_t1.tv_sec/60)%60;\
    fprintf(stderr,"[%02d:%02d.%06d]: " msg "\n", (int)_min, (int)_secs, (int)(_t1.tv_nsec/1000), ##__VA_ARGS__);\
}

/*
 * Each matrix cell is filled with column index. (0...N-1)
 * In this way, the correct output after matrix multiplication
 * is c[i][j]=K*j where K=(N-1)*N/2
 */
/* XXX: the init is made by the master node like JUMP implementation */
//#pragma omp target copy_deps
//#pragma omp task out([size]A, [size]B)
void seqinit(int size, int n, int *A, int *B)
{
    int i,j;

    for (i = 0; i < size; i++) {
	for (j = 0; j < n; j++) {
	    A[i*n + j] = j;
	    B[i*n + j] = j;
	}
    }
}

/* task ditributed among the cluster nodes */
#pragma omp target device(smp) copy_deps
#pragma omp task in([size*n]A, [n*n]B) out([size*n]C, [1]elapsed)
void matmul(int size, int n, int *A, int *B, int *C, long int *elapsed)
{
    struct timespec time_start, time_end;
    int i, j, k;
    int tmp;

    logmsg("Start matmul %p %p %p",A,B,C);
    time_gettime(&time_start);
#ifndef NO_COMPUTE
    for (i = 0; i < size; i++) {
	for (j = 0; j < n; j++) {
	    tmp = 0;
	    for (k = 0; k < n; k++) {
	        tmp += A[i*n + k] * B[k*n + j];
#ifdef MATRIX_DEBUG
	        if (A[i*n + k] != k || B[k*n + j] != j) {
			logmsg("ERROR CLUSTER a[%d][%d] = %d b[%d][%d] = %d b[%d][%d] = %d (size=%d n=%d)",
                		i, k, A[i*n + k], k, j, B[k*n + j], k, j+1, B[k*n + j + 1], size, n);
	        }
#endif
	    }
            C[i*n + j] = tmp;
#ifdef MATRIX_DEBUG
            if (tmp != ((n*(n-1))/2) * j) {
	         logmsg("ERROR CLUSTER c[%d][%d] = %d (expected %d) a[%d][3] = %d b[0][%d] = %d",
                		i, j, tmp, (n*(n-1))/2 * j, i, A[i*n + 3], j, B[j]);
            }
#endif
	}
    }
#endif
    time_gettime(&time_end);
    *elapsed = time_diff_msec(&time_end, &time_start);
    logmsg("End matmul   %p %p %p elapse=%ld",A,B,C,*elapsed);
}

static int *a, *b, *c;
static long int *matmul_time;

int main (int argc, char *argv[])
{
    struct timespec time_start, time_end;
    int  i, j, k, errors = 0;
    long int total_matmul = 0;
    long int max_matmul = 0;
    long int min_matmul = LONG_MAX;
    long int mean_matmul = 0;

    logmsg("Initializing system ....................");
    if (argc>1) {
        BLOCKSIZE=atoi(argv[1]);
        if (argc>2) {
            N=atoi(argv[2]);
        }
    }
    logmsg("block size:%d matrix size:%d",BLOCKSIZE,N);
    if (N<BLOCKSIZE) {
        logmsg("bad sizes");
        exit(-1);
    }
    a=malloc(sizeof(int)*N*N);
    b=malloc(sizeof(int)*N*N);
    c=malloc(sizeof(int)*N*N);
    matmul_time=malloc(sizeof(long int)*N);
    memset(matmul_time,0,sizeof(long int)*N);
    logmsg("Initializing system done!");

    logmsg("Initializing matrices ....................");
    seqinit(N, N, a, b);
    logmsg("Initializing matrices done!");

    logmsg("Calculating matrices ....................");
    time_gettime(&time_start);
    for (i=0; i < N-BLOCKSIZE; i+=BLOCKSIZE) {
        matmul(BLOCKSIZE, N,  a + i*N, b, c + i*N, matmul_time + i);
    }
    if (i < N) {
        matmul(N-i, N, a + i*N, b, c + i*N, matmul_time + i);
    }
    #pragma omp taskwait
    time_gettime(&time_end);
    logmsg("Calculating matrices done!");

#ifndef NO_COMPUTE
#ifdef FINAL_CHECK
    /*
     * check results
     *
     * expected c[i][j] == K*j  where K = (N-1)*N/2,
     * because a[i][j] = b[i][j] = j
     */
    k = (N*(N-1))/2;
    for (i=0; i < N; i++) {
        for (j=0; j < N; j++) {
            if (c[i*N + j] != k*j) {
                errors++;
#ifdef MATRIX_DEBUG
		logmsg("ERROR c[%d][%d] = %d (expected %d) a[%d][3] = %d b[0][%d] = %d",
                		i, j, c[i*N + j], k*j, i, a[i*N + 3], j, b[j]);
#endif
            }
        }
    }
    if (errors != 0)
	logmsg("NOTE: Found %d ERRORS!", errors);
#endif
#endif

    for (i=0; i < N-BLOCKSIZE; i+=BLOCKSIZE) {
        if (matmul_time[i]>=0) {
            total_matmul += matmul_time[i];
            max_matmul = (max_matmul > matmul_time[i]) ? max_matmul : matmul_time[i];
            min_matmul = (min_matmul < matmul_time[i]) ? min_matmul : matmul_time[i];
        }
    }
    if (i<N) {
        if (matmul_time[i]>=0) {
            total_matmul += matmul_time[i];
            max_matmul = (max_matmul > matmul_time[i]) ? max_matmul : matmul_time[i];
            min_matmul = (min_matmul < matmul_time[i]) ? min_matmul : matmul_time[i];
        }
    }
    mean_matmul = total_matmul/(N/BLOCKSIZE);

    logmsg("Min matmul       %8ld msec", min_matmul);
    logmsg("Mean matmul      %8ld msec", mean_matmul);
    logmsg("Max matmul       %8ld msec", max_matmul);
    logmsg("Total matmul     %8ld msec", total_matmul);
    logmsg("Total elab time  %8ld msec", time_diff_msec(&time_end, &time_start));
    fflush(stderr);

    #pragma omp taskwait
    return errors;
}


