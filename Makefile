all: memprofiler.so test test_mt

memprofiler.so: linked_list.c memprofiler.c
	gcc -shared -fPIC memprofiler.c linked_list.c -o memprofiler.so -ldl -g

test_mt: test_mt.c
	gcc test_mt.c -o test_mt -lpthread

test: test.c
	gcc test.c -o test 
clean:
	rm memprofiler.so test_mt test
