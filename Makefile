CFLAGS=-std=c11 -g -static

xcc: xcc.c

test: xcc
		./test.sh

clean:
		rm -f 9cc *.o *~ tmp*

.PHONY: test clean
