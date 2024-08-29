CFLAGS=-std=c11 -g -static

f-cc: first-cc.c
	cc -o f-cc first-cc.c

test: f-cc
	./test.sh

clean:
	rm -f f-cc *.o *~ tmp*

.PHONY: test clean