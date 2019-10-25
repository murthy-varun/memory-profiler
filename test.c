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

#define MAX_NUM_PTRS   100
#define NUM_OPERATIONS 4
#define MAX_SLEEP_TIME 4

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

int main(int argc, char *argv[])
{
	uint16_t  num_iter = 20;
	int      *ptr[MAX_NUM_PTRS] = {NULL};
	uint32_t  size_arr[MAX_MEMSIZE] = {1, 1024, 1024*1024};
	uint8_t   ptr_indx = 0;
	long      size  = 0;
	long      nmemb  = 0;
	bool      free_all = false;

	if(argc > 1) {
		free_all = (atoi(argv[1]) != 0);
	}

	srand(time(NULL));
	while(num_iter != 0) {		
		uint8_t  operation;
		uint8_t  retry = 100;
		uint16_t random = rand();
		//fprintf(stderr, "Random:%u\n", random);

		operation = random%MAX_OPER;
		//fprintf(stderr, "Operation:%u\n", operation);

		switch(operation) {
			case MALLOC:
				ptr_indx = rand()%MAX_NUM_PTRS;
				if(!ptr[ptr_indx]) {
					size = size_arr[(rand()%MAX_MEMSIZE)] * (rand()%1000);
					ptr[ptr_indx] = (int*)malloc(size*sizeof(int));
					fprintf(stderr, "Allocating \tptr[%d]:%p \tsize:%ld\n",
							ptr_indx, ptr[ptr_indx], size);
				}
				break;

			case CALLOC:
				ptr_indx = rand()%MAX_NUM_PTRS;
				if(!ptr[ptr_indx]) {
					nmemb = rand()%20;
					size = size_arr[(rand()%MAX_MEMSIZE)] * (rand()%1000);
					ptr[ptr_indx] = (int*)calloc(nmemb, size*sizeof(int));
					fprintf(stderr, "Allocating \tptr[%d]:%p \tsize:%ld*%ld\n",
							ptr_indx, ptr[ptr_indx], nmemb, size);
				}
				break;

			case REALLOC:
				ptr_indx = rand()%MAX_NUM_PTRS;
				if(ptr[ptr_indx]) {
					int *prev_ptr = ptr[ptr_indx];
					size = size_arr[(rand()%MAX_MEMSIZE)] * (rand()%1000);
					ptr[ptr_indx] = (int*)realloc(ptr[ptr_indx], size*sizeof(int));
					fprintf(stderr, "Re-Allocating \tptr[%d]:%p \tsize:%ld \tp_ptr:%p\n",
							ptr_indx, ptr[ptr_indx], size, prev_ptr);
				}
				break;

			case FREE:
				retry = 100;
				while(retry > 0) {
					ptr_indx = rand()%MAX_NUM_PTRS;
					if (ptr[ptr_indx]) {
						break;
					}
					retry--;
				}

				if(ptr[ptr_indx]) {
					fprintf(stderr, "Freeing \tptr[%d]:%p\n",
							ptr_indx, ptr[ptr_indx]);
					free(ptr[ptr_indx]);
					ptr[ptr_indx] = NULL;
				}
				break;
		}
		sleep(rand()%5);
		num_iter--;
	}

	if(free_all) {
		/* cleaning up all pending memory */
		//fprintf(stderr, "\n\nCLEANING UP \n\n");
		for(ptr_indx = 0; ptr_indx < MAX_NUM_PTRS; ptr_indx++) {
			if(ptr[ptr_indx]) {
				//fprintf(stderr, "Freeing memory \tptr[%d]:%p\n", ptr_indx, ptr[ptr_indx]);
				free(ptr[ptr_indx]);
				ptr[ptr_indx] = NULL;
			}
		}
	}
	sleep(5);

	return 0;
}

