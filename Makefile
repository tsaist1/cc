CFLAGS=-std=c11 -g -static -Wall
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

tcc: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

$(OBJS): tcc.h

test: tcc
		./test.sh

clean:
		rm -f tcc *.o *~ tmp*

.PHONY: test clean
