/*
MIT License

Copyright (c) 2019 Varun Murthy (varun.tk@gmail.com)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

#define MAX_NUM_PTRS   100
#define NUM_OPERATIONS 4
#define MAX_SLEEP_TIME 4

//#define DEBUG

#ifdef DEBUG
#define log(format, args...) \
    fprintf(stderr, format, ##args)
#else
#define log(format, args...)
#endif

typedef enum {
    MALLOC,
    CALLOC,
    REALLOC,
    FREE,
    MAX_OPER
} mem_oper_t;

typedef enum {
    BYTES,
    KILOBYTES,
    MEGABYTES,
    MAX_MEMSIZE
} mem_size_t;

typedef enum {
    THREAD1,
    THREAD2,
    THREAD3,
    THREAD4,
    THREAD5,
    NUM_THREADS
} thread_t;

static int *ptr[MAX_NUM_PTRS] = {NULL};
static uint32_t  size_arr[MAX_MEMSIZE] = {1, 1024, 1024*1024};

void common_handler(thread_t thrd)
{
    uint16_t  num_iter = 20;    
    uint8_t   ptr_indx = 0;
    long      size  = 0;
    long      nmemb  = 0;
    uint8_t   ptr_indx_base = 0;

    switch(thrd) {
        case THREAD1:
            ptr_indx_base = 0;
            break;
        case THREAD2:
            ptr_indx_base = 20;
            break;
        case THREAD3:
            ptr_indx_base = 40;
            break;
        case THREAD4:
            ptr_indx_base = 60;
            break;
        case THREAD5:
            ptr_indx_base = 80;
            break;
    }

    srand(time(NULL));
    while(num_iter != 0) {        
        uint8_t  operation;
        uint8_t  retry = 100;
        uint16_t random = rand();

        operation = random%MAX_OPER;

        switch(operation) {
            case MALLOC:
                ptr_indx = rand()%(MAX_NUM_PTRS/NUM_THREADS)+ptr_indx_base;
                if(!ptr[ptr_indx]) {
                    size = size_arr[(rand()%MAX_MEMSIZE)] * (rand()%1000);
                    ptr[ptr_indx] = (int*)malloc(size*sizeof(int));
                    log("Allocating \tptr[%d]:%p \tsize:%ld\n",
                        ptr_indx, ptr[ptr_indx], size);
                }
                break;

            case CALLOC:
                ptr_indx = rand()%(MAX_NUM_PTRS/NUM_THREADS)+ptr_indx_base;
                if(!ptr[ptr_indx]) {
                    nmemb = rand()%20;
                    size = size_arr[(rand()%MAX_MEMSIZE)] * (rand()%1000);
                    ptr[ptr_indx] = (int*)calloc(nmemb, size*sizeof(int));
                    log("Allocating \tptr[%d]:%p \tsize:%ld*%ld\n",
                        ptr_indx, ptr[ptr_indx], nmemb, size);
                }
                break;

            case REALLOC:
                ptr_indx = rand()%(MAX_NUM_PTRS/NUM_THREADS)+ptr_indx_base;
                if(ptr[ptr_indx]) {
                    int *prev_ptr = ptr[ptr_indx];
                    size = size_arr[(rand()%MAX_MEMSIZE)] * (rand()%1000);
                    ptr[ptr_indx] = (int*)realloc(ptr[ptr_indx], size*sizeof(int));
                    log("Re-Allocating \tptr[%d]:%p \tsize:%ld \tp_ptr:%p\n",
                        ptr_indx, ptr[ptr_indx], size, prev_ptr);
                }
                break;

            case FREE:
                retry = 100;
                while(retry > 0) {
                    ptr_indx = rand()%(MAX_NUM_PTRS/NUM_THREADS)+ptr_indx_base;
                    if (ptr[ptr_indx]) {
                        break;
                    }
                    retry--;
                }

                if(ptr[ptr_indx]) {
                    log("Freeing \tptr[%d]:%p\n", ptr_indx, ptr[ptr_indx]);
                    free(ptr[ptr_indx]);
                    ptr[ptr_indx] = NULL;
                }
                break;
        }
        sleep(rand()%5);
        num_iter--;
    }
}

void* thread1(void *arg)
{
    common_handler(THREAD1);
    return NULL;
}

void* thread2(void *arg)
{
    common_handler(THREAD2);
    return NULL;
}

void* thread3(void *arg)
{
    common_handler(THREAD3);
    return NULL;
}

void* thread4(void *arg)
{
    common_handler(THREAD4);
    return NULL;
}

void* thread5(void *arg)
{
    common_handler(THREAD5);
    return NULL;
}

int main()
{
    bool       free_all = false;
    uint8_t    ptr_indx = 0;
    pthread_t  t1, t2, t3, t4, t5;
    int ret = 0;

    do {
        ret = pthread_create(&t1, NULL, thread1, NULL);
        if(ret != 0) {
            break;
        }
        ret = pthread_create(&t2, NULL, thread1, NULL);
        if(ret != 0) {
            break;
        }
        ret = pthread_create(&t3, NULL, thread1, NULL);
        if(ret != 0) {
            break;
        }
        ret = pthread_create(&t4, NULL, thread1, NULL);
        if(ret != 0) {
            break;
        }
        ret = pthread_create(&t5, NULL, thread1, NULL);
        if(ret != 0) {
            break;
        }
    } while(0);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
    pthread_join(t4, NULL);
    pthread_join(t5, NULL);

    if(free_all) {
        /* cleaning up all pending memory */
        log("\n\nCLEANING UP \n\n");
        for(ptr_indx = 0; ptr_indx < MAX_NUM_PTRS; ptr_indx++) {
            if(ptr[ptr_indx]) {
                log("Freeing memory \tptr[%d]:%p\n", ptr_indx, ptr[ptr_indx]);
                free(ptr[ptr_indx]);
                ptr[ptr_indx] = NULL;
            }
        }
    }
    sleep(5);

	/* call print stats once before the test ends 
		Note: free(NULL) is safe and ignored as per C standard
	*/
	free(NULL);

    return 0;
}

