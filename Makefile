CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

f-cc: $(OBJS)
	$(CC) -o f-cc $(OBJS) $(LDFLAGS)

$(OBJS): f-cc.h

test: f-cc
	./test.sh

clean:
	rm -f f-cc *.o *~ tmp*

.PHONY: test clean