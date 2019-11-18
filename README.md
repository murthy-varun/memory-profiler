# memory-profiler
Memory profiler using wrapper around memory allocation functions and utilizing LD_PRELOAD

## Index
 - Development Environment
 - Extraction Instructions
 - Build Instructions
 - Design details
 - Source code structure
 - Test details

-------------------------
Development Environment
-------------------------
GNU/Linux Used for development: Ubuntu 18.04.3 LTS

-------------------------
Extraction Instructions
-------------------------
Download the tarball into local directory

Use the following command to extract
$tar -xzvf memprofiler.tar.gz

-------------------------
Build Instructions
-------------------------
gcc version: gcc version 7.4.0
make version: GNU Make 4.1

commands:
$cd memprofiler
$make

This should build a shared library "memprofiler.so" and a test executable "test_mt"

Note: 
makefile is a very basic
Use command "make" to build
Use command "make clean" to clean

-------------------------
Design details
-------------------------
High level points
1. Using dlsym(RTLD_NEXT, ...) to get the real memory allocation function
2. Not using GCC constructors to intialize function pointers since there is no guarantee that it will be called before the actual functions
3. dlsym calls calloc() internally. In order to break the endless recursion (and to avoid segmentation fault), 
   allocated a static buffer and return it for the first time. 
   Reference: https://elinux.org/images/b/b5/Elc2013_Kobayashi.pdf
4. Used a linked list to store the information required for statistics.
    a. Cannot use the generically available data strcutures (glib hashtable or STL) since they call malloc/calloc/realloc internally
    b. Implemented a rudimentary singly linked list which calls the "real" memory allocation functions.
    c. Linked list is not the most optimal solution. Using Hashtable/s would be the fastest.
        Used Linked list to simplify the solution and since performace was not an issue as mentioned in the problem.

-------------------------
Source code structure
-------------------------
memprofiler.c - implements the wrapper functions and utilities to store and print statistics
linked_list.c/.h - rudimentary singly linked list
test_mt.c - multi-threaded test program
Makefile - basic makefile to created shared library and test executable
readme.txt - this :)

-------------------------
Test details
-------------------------
Tested Ubuntu 18.04.3 LTS

Sample test commands:
1. $LD_PRELOAD=$PWD/memprofiler.so ./test_mt
2. $sudo LD_PRELOAD=$PWD/memprofiler.so find / -name abcdef

Test program overview
1. Has 100 integer pointers
2. Creates 5 threads -  each thread handles a range of pointers. 
   thread1 handles pointers between [0-19],
   thread1 handles pointers between [20-39],
   ...
3. Each thread randomly picks an operation and allocates randomly sized memory or frees up and then sleeps for a random time betwen 0-5 seconds.
4. number of iterations = 20
5. Main thread waits until all threads are finished executing.
6. Main thread cleans up any allocations which are not yet freed up. 
