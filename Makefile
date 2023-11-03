CFLAGS=-std=c11 -g -static

tokenizer: tokenizer.c

test: tokenizer
		./test.sh

clean:
		rm -f tokenizer *.o *~ tmp*

.PHONY: test clean
