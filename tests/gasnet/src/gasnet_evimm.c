/*!
 * \file gasnet_evimm.c
 *
 * \version     v0.15
 * \date        2016-12-20
 *
 * This example implements Matrix Multiplication on top of GASNet in the same way
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

#include <pthread.h>
#include <gasnet.h>
#include <gasnet_tools.h>

#if GASNET_ALIGNED_SEGMENTS!=1
error;
#endif

#include "mytime.h"

//#define RESERVE_NODE_ZERO

//#define NO_COMPUTE
#define STEP_DEBUG
#define MATRIX_DEBUG
#define FINAL_CHECK

// big blocks
//#define BLOCKSIZE       20
//#define N               400

// small blocks
//#define BLOCKSIZE       3
//#define N               300

//#define BLOCKSIZE       10
//#define N               100


#define BLOCKSIZE       32
#define N               256

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

static gasnet_node_t mynode;
static gasnet_node_t nodes;

int x=0;

/* task ditributed among the cluster nodes */
void matmul(int size, int n, int *A, int *B, int *C, long int *elapsed)
{
    struct timespec time_start, time_end;
    int i, j, k;
    int tmp;

    logmsg("Start matmul %d %p %p %p",x++,A,B,C);
    time_gettime(&time_start);
#ifndef NO_COMPUTE
    for (i = 0; i < size; i++) {
	for (j = 0; j < n; j++) {
	    tmp = 0;
	    for (k = 0; k < n; k++) {
	        tmp += A[i*n + k] * B[k*n + j];
#ifdef MATRIX_DEBUG
	        if (A[i*n + k] != k || B[k*n + j] != j) {
			logmsg("ERROR CLUSTER a[%d][%d] = %d (expected %d) b[%d][%d] = %d (expecetd %d)",
                		i, k, A[i*n + k], k, k, j, B[k*n + j], j);
	        }
#endif
	    }
            C[i*n + j] = tmp;
#ifdef MATRIX_DEBUG
            if (tmp != ((N*(N-1))/2) * j) {
	         logmsg("ERROR CLUSTER c[%d][%d] = %d (expected %d) ",
                		i, j, tmp, (N*(N-1))/2 * j);
            }
#endif
	}
    }
#endif
    time_gettime(&time_end);
    *elapsed = time_diff_msec(&time_end, &time_start);
    logmsg("End matmul   %p %p %p elapse=%ld",A,B,C,*elapsed);
}

// 0 free 1 occupied
static unsigned workers=0;
static inline void w_set_node_busy(gasnet_node_t node) {
    workers|=(1<<node);
}
static inline void w_set_node_free(gasnet_node_t node) {
    workers&=~(1<<node);
}
static inline gasnet_node_t w_get_usable_node() {
    unsigned mask=1;
    //unsigned mask=1<<(nodes-1);
    int i;
    for (i=0;i<nodes;i++) {
    //for (i=nodes-1;i>=0;i--) {
        if ((workers&mask)==0) {
            w_set_node_busy(i);
            return i;
        }
        mask<<=1;
        //mask>>=1;
    }
    return -1;
}

void done(gasnet_token_t token, gasnet_handlerarg_t _elapsed);
void compute(gasnet_token_t token, gasnet_handlerarg_t _size, gasnet_handlerarg_t _n, gasnet_handlerarg_t _A, gasnet_handlerarg_t _B, gasnet_handlerarg_t _C);
void mustexit(gasnet_token_t token);

static gasnet_handlerentry_t table[]={{0,compute},{0,done},{0,mustexit}};
static gasnet_seginfo_t seginfo;
#define REMOTEFUNC_COMPUTE (table[0].index)
#define REMOTEFUNC_DONE (table[1].index)
#define REMOTEFUNC_MUSTEXIT (table[2].index)

#define DEPTR(x) ((void*)(((uint8_t*)(seginfo.addr))+(x)))
#define ENPTR(p) (((uint8_t*)p)-((uint8_t*)(seginfo.addr)))

#define MAXNUMNODES 8

struct {
    int to_post_elab;
    long int *elapsed;
    int *C;
    int len;
    //
    int counter;
} data[MAXNUMNODES];

static gasnett_atomic_t numtodo=gasnett_atomic_init(0);
static int numreq=0;
static int numelab=0;

void done(gasnet_token_t token, gasnet_handlerarg_t _elapsed) {
    gasnet_node_t node;
    gasnet_AMGetMsgSource(token,&node);
    data[node].to_post_elab=1;
    *data[node].elapsed=_elapsed;
    gasnett_atomic_increment(&numtodo,0);
}

void after_done() {
    gasnet_node_t node;
    for (node=0;node<nodes;node++) {
        if (data[node].to_post_elab) {
            if (node!=mynode) {
                gasnet_get(data[node].C,node,data[node].C,data[node].len*sizeof(int));
            }
            data[node].to_post_elab=0;
            data[node].counter++;
            w_set_node_free(node);
            gasnett_atomic_decrement(&numtodo,0);
            numelab++;
            break;
        }
    }
}

void compute(gasnet_token_t token, gasnet_handlerarg_t _size, gasnet_handlerarg_t _n, gasnet_handlerarg_t _A, gasnet_handlerarg_t _B, gasnet_handlerarg_t _C) {
    long int elapsed;
    matmul(_size,_n,DEPTR(_A),DEPTR(_B),DEPTR(_C),&elapsed);
    gasnet_AMReplyShort1(token,REMOTEFUNC_DONE,elapsed);
}

void submit_job(int size, int n, int *A, int *B, int *C, long int *elapsed) {
    gasnet_node_t node;
#ifdef STEP_DEBUG
    logmsg("Submit job! searching free node...");
#endif
    for (;;) {
        node=w_get_usable_node();
        if (node!=-1) break;
        GASNET_BLOCKUNTIL(gasnett_atomic_read(&numtodo,0)!=0);
        after_done();
    }
#ifdef STEP_DEBUG
    logmsg("Submit job to node %d",node);
#endif
    numreq++;
    data[node].C=C;
    data[node].elapsed=elapsed;
    data[node].len=size*n;
    if (node!=mynode) {
#ifdef STEP_DEBUG
    logmsg("Putting data to node %d...",node);
#endif
        gasnet_put(node,A,A,size*n*sizeof(int));
        //gasnet_put(node,B,B,n*n*sizeof(int));
    }
#ifdef STEP_DEBUG
    logmsg("Request remote computation to node %d",node);
#endif            
    gasnet_AMRequestShort5(node,REMOTEFUNC_COMPUTE,size,n,ENPTR(A),ENPTR(B),ENPTR(C));
}

void wait_pending_jobs() {
    while (numreq!=numelab) {
        GASNET_BLOCKUNTIL(gasnett_atomic_read(&numtodo,0)!=0);
        after_done();
    }
}

static int *a, *b, *c;
static long int matmul_time[N];

static int _mustexit=0;

void mustexit(gasnet_token_t token) {
    _mustexit=1;
}

void *worker(void *dummy) {
#ifdef STEP_DEBUG
    logmsg("Starting gasnet polling on node %d...",mynode);
#endif
    GASNET_BLOCKUNTIL(_mustexit==1);
#ifdef STEP_DEBUG
    logmsg("Gasnet polling on node %d end!",mynode);
#endif
    gasnet_exit(0);
    return NULL;
}

void start_worker() {
    pthread_t th;
    pthread_attr_t attr;
#ifdef STEP_DEBUG
    logmsg("Start working thread...");
#endif
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
    pthread_create(&th,&attr,worker,NULL);
    pthread_attr_destroy(&attr);
}


int main (int argc, char *argv[])
{
    struct timespec time_start, time_end;
    int  i, j, k, errors = 0;
    long int total_matmul = 0;
    long int max_matmul = 0;
    long int min_matmul = LONG_MAX;
    long int mean_matmul = 0;
    int ret=0;
    uintptr_t base=0;
    gasnet_node_t no;

    logmsg("Initializing system ....................");
    ret=gasnet_init(&argc,&argv);
    if (ret!=GASNET_OK) {
        logmsg("FATAL during gasnet_init()!");
        return -1;
    }
    ret=gasnet_attach(table,sizeof(table)/sizeof(gasnet_handlerentry_t),(((sizeof(int)*3*N*N+sizeof(long int)*N)/GASNET_PAGESIZE)+1)*GASNET_PAGESIZE  ,base);
    if (ret!=GASNET_OK) {
        logmsg("FATAL during gasnet_attach()!");
        return -1;
    }
    ret=gasnet_getSegmentInfo(&seginfo,1);
    if (ret!=GASNET_OK) {
        logmsg("FATAL during gasnet_getSemnetInfo()!");
        return -1;
    }
    a=seginfo.addr;
    b=(int*)(((uint8_t*)a)+sizeof(int)*N*N);
    c=(int*)(((uint8_t*)b)+sizeof(int)*N*N);
    mynode=gasnet_mynode();
    nodes=gasnet_nodes();
    logmsg("Initializing system done (mynode=%d)!",mynode);

    if (mynode!=0) {
        worker(NULL);
        // never return
    }

    logmsg("Initializing matrices ....................");
    seqinit(N, N, a, b);
    memset(matmul_time,0,sizeof(long int)*N);
    logmsg("Initializing matrices done!");

#ifdef RESERVE_NODE_ZERO
    w_set_node_busy(0);
#else
    start_worker();
#endif
    for (no=1;no<nodes;no++) {
#ifdef STEP_DEBUG
        logmsg("Gasnet put to node %d...",no);
#endif
        
        
        
        /*
        for (i=0; i < N-BLOCKSIZE; i+=BLOCKSIZE) {
            gasnet_put(no,b+i*N,b+i*N,BLOCKSIZE*N*sizeof(int));
        }
        if (i < N) {
            gasnet_put(no,b+i*N,b+i*N,(N-i)*N*sizeof(int));
        }
        */
        gasnet_put(no,b,b,N*N*sizeof(int));



        data[no].counter=0;
#ifdef STEP_DEBUG
    logmsg("Gasnet put to node %d done!",no);
#endif
    }
    data[0].counter=0;

    logmsg("Calculating matrices ....................");
    time_gettime(&time_start);
    for (i=0; i < N-BLOCKSIZE; i+=BLOCKSIZE) {
        //logmsg("N=%d B=%d I=%d",N,BLOCKSIZE,i);
        submit_job(BLOCKSIZE, N,  a + i*N, b, c + i*N, matmul_time + i);
    }
    if (i < N) {
        //logmsg("N=%d B=%d I=%d",N,BLOCKSIZE,i+1);
        submit_job(N-i, N, a + i*N, b, c + i*N, matmul_time + i);
    }
    wait_pending_jobs();
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
		logmsg("ERROR c[%d][%d] = %d (expected %d) a[%d][3] = %d b[0][%d] = %d",
                		i, j, c[i*N + j], k*j, i, a[i*N + 3], j, b[j]);
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
    mean_matmul = total_matmul/(N/BLOCKSIZE);

    logmsg("Min matmul       %8ld msec", min_matmul);
    logmsg("Mean matmul      %8ld msec", mean_matmul);
    logmsg("Max matmul       %8ld msec", max_matmul);
    logmsg("Total matmul     %8ld msec", total_matmul);
    logmsg("Total elab time  %8ld msec", time_diff_msec(&time_end, &time_start));
    //logmsg("Overhead         %8ld msec", time_diff_msec(&time_end, &time_start)-total_matmul/nodes);
    for (no=0;no<nodes;no++) {
        logmsg("node[%d] jobs done %3d", no, data[no].counter);
    }

    fflush(stderr);

    gasnet_exit(errors);
    return errors;
}


