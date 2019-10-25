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

#define _GNU_SOURCE

#include <stdio.h>
#include <dlfcn.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include "linked_list.h"

/*-----------------------------------------------------------------------------
                                    MACROS
-----------------------------------------------------------------------------*/
//#define LOG_DEBUG
#define LOG_ERROR
#define LOG_INFO

#ifdef LOG_ERROR
#define log_error(format, args...) \
    fprintf(stderr, "ERR:\t"format, ##args)
#else
#define log_error(format, args...)
#endif

#ifdef LOG_INFO
#define log_info(format, args...) \
    fprintf(stderr, format, ##args)
#else
#define log_info(format, args...)
#endif

#ifdef LOG_DEBUG
#define log_debug(format, args...) \
    fprintf(stderr, "DBG:\t"format, ##args)
#else
#define log_debug(format, args...)
#endif

/*-----------------------------------------------------------------------------
                          TYPE DECLARATIONS
-----------------------------------------------------------------------------*/
typedef void* (*orig_malloc_t)(size_t);
typedef void* (*orig_calloc_t)(size_t, size_t);
typedef void* (*orig_realloc_t)(void*, size_t);
typedef void  (*orig_free_t)(void*);


typedef struct {
    size_t  alloc_sz;
    time_t  alloc_time;
} alloc_info_t;

/*-----------------------------------------------------------------------------
                                GLOBALS
-----------------------------------------------------------------------------*/

/* Buffer to resolve calloc and dlsym inter-dependency
 * Used only for the first time */
static __thread int no_hook;
static char alloc_buff[128];

/* Function pointers to store the hooks to original system calls */
static orig_malloc_t orig_malloc = NULL;
static orig_calloc_t orig_calloc = NULL;
static orig_realloc_t orig_realloc = NULL;
static orig_free_t orig_free = NULL;

static pthread_mutex_t alloc_lock = PTHREAD_MUTEX_INITIALIZER;

/* Overall allocations */
static long      overall_num_alloc = 0;
static long long overall_alloc_sz  = 0;

/* Linked List to store current allocations */
static list_node_t *curr_alloc_list = NULL;

/*-----------------------------------------------------------------------------
                          INTERNAL FUNCTIONS
-----------------------------------------------------------------------------*/

static int increment_overall_num_alloc()
{
    pthread_mutex_lock(&alloc_lock);
    overall_num_alloc++;
    pthread_mutex_unlock(&alloc_lock);
    return 0;
}

static int add_overall_alloc_sz(size_t size)
{
    pthread_mutex_lock(&alloc_lock);
    overall_alloc_sz += size;
    pthread_mutex_unlock(&alloc_lock);
    return 0;
}

static int add_curr_alloc_list(void *ptr, size_t size)
{
    list_node_t *node = (list_node_t*)orig_calloc(1, sizeof(list_node_t));
    if (!node) {
        log_error("Could not allocate lined list node for %p\n", ptr);
        return -1;
    }
    alloc_info_t *info
        = (alloc_info_t*)orig_calloc(1, sizeof(alloc_info_t));
    info->alloc_sz = size;
    time(&info->alloc_time);
    node->key = ptr;
    node->val = info;

    log_debug("Adding node:%p\n", ptr);

    pthread_mutex_lock(&alloc_lock);
    list_insert(&curr_alloc_list, node);
    pthread_mutex_unlock(&alloc_lock);
    return 0;
}

static int del_curr_alloc_list(void *ptr)
{
    list_node_t *node = NULL;

    pthread_mutex_lock(&alloc_lock);
    node = list_delete(&curr_alloc_list, ptr);
    pthread_mutex_unlock(&alloc_lock);

    if(!node) {
        log_error("Could not find node:%p\n", ptr);
        return -1;
    }

    log_debug("Deleting node:%p\n", ptr);
    orig_free(node->val);
    orig_free(node);
    return 0;
}

static void print_stats(bool force_print)
{
    static time_t  time_last_printed = 0;
    static time_t  curr_time;
    list_node_t   *current = NULL;
    size_t         ovrl_alloc_sz = 0;
    long           ovrl_num_alloc = 0;
    long long      curr_alloc_sz = 0;
    long           curr_num_alloc = 0;

    /* Print stats if 5 seconds have elapsed since last print
    Or Force print */
    time(&curr_time);
    if((force_print == false) &&(curr_time - time_last_printed) < 5) {
        return;
    }
    time_last_printed = curr_time;

    pthread_mutex_lock(&alloc_lock);
    current = curr_alloc_list;    

    /* Traverse Linked list */
    while(current != NULL) {
        alloc_info_t *info = (alloc_info_t*)current->val;
        curr_num_alloc++;
        curr_alloc_sz += info->alloc_sz;
        current = current->next;
    }

    ovrl_alloc_sz  = overall_alloc_sz;
    ovrl_num_alloc = overall_num_alloc;

    pthread_mutex_unlock(&alloc_lock);

    log_info("\n>>>>>>>>>> %s", ctime(&curr_time));
    log_info("Overall Stats:\n");
    log_info("Overall number of allocations: %ld\n", ovrl_num_alloc);
    log_info("Overall allocation size:%ld \n\n", ovrl_alloc_sz);
    log_info("Current Stats:\n");
    log_info("Current number of allocations:%ld\n", curr_num_alloc);
    log_info("Current allocation size:%lld\n\n\n", curr_alloc_sz);
    //log_info("\nCurrent allocations by size:\n");
    return;
}

/*-----------------------------------------------------------------------------
                          EXTERNAL FUNCTIONS
-----------------------------------------------------------------------------*/
void* malloc(size_t size)
{
    void* ret_ptr = NULL;

    if(orig_malloc == NULL) {
        orig_malloc = (orig_malloc_t)dlsym(RTLD_NEXT, "malloc");
    }

    ret_ptr = orig_malloc(size);
    log_debug("malloc size:%ld ret_ptr:%p\n", size, ret_ptr);
    if(ret_ptr) {
        increment_overall_num_alloc();
        add_overall_alloc_sz(size);
        add_curr_alloc_list(ret_ptr, size);
        print_stats(false);
    }
    return ret_ptr;
}

void* calloc(size_t nmemb, size_t size)
{
    void* ret_ptr = NULL;

    if(no_hook && orig_calloc == NULL) {
        return alloc_buff;
    }

    if(orig_calloc == NULL) {
        no_hook = 1;
        orig_calloc = (orig_calloc_t)dlsym(RTLD_NEXT, "calloc");
        no_hook = 0;
    }

    ret_ptr = orig_calloc(nmemb, size);
    log_debug("calloc size:%ld*%ld ret_ptr:%p\n", nmemb, size, ret_ptr);
    if(ret_ptr) {
        increment_overall_num_alloc();
        add_overall_alloc_sz(nmemb * size);

        add_curr_alloc_list(ret_ptr, size);
        print_stats(false);
    }

    return ret_ptr;
}

void* realloc(void* ptr, size_t size)
{
    void *ret_ptr = NULL;
    size_t curr_size = 0;

    if(orig_realloc == NULL) {
        orig_realloc = (orig_realloc_t)dlsym(RTLD_NEXT, "realloc");
    }

    ret_ptr = orig_realloc(ptr, size);
    log_debug("realloc ptr:%p size:%ld ret_ptr:%p\n", ptr, size, ret_ptr);

    if(ptr) {
        /* get current alloc size */
        pthread_mutex_lock(&alloc_lock);
        alloc_info_t *curr_info = list_find(curr_alloc_list, ptr);
        pthread_mutex_unlock(&alloc_lock);

        del_curr_alloc_list(ptr);
    }
    if(ret_ptr) {
        /* If new size is greater than previous, add the difference
           to overall stats */
        if((curr_size != 0) && (size > curr_size)) {
            add_overall_alloc_sz(size - curr_size);
        }
        add_curr_alloc_list(ret_ptr, size);
    }

    if (ptr || ret_ptr) {
        print_stats(false);
    }

    return ret_ptr;
}

void free(void* ptr)
{
    if(orig_free == NULL) {
        orig_free = (orig_free_t)dlsym(RTLD_NEXT, "free");
    }

    log_debug("free %p\n", ptr);
    if(ptr != alloc_buff) {
        orig_free(ptr);
        if(ptr) {
            del_curr_alloc_list(ptr);            
        }
        print_stats(false);
    }
    return;
}


/*-----------------------------------------------------------------------------
                    GCC constructor and destructor - Unused
-----------------------------------------------------------------------------*/
__attribute__ ((constructor)) void init(void)
{
    log_debug("Memory Profiler Constructor called!!\n");
    return;
}
__attribute__ ((destructor)) void fini(void)
{
    log_debug("Memory Profiler Destructor called!!\n");
    print_stats(true);
    return;
}
