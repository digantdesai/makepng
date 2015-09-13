CC = gcc
CFLAGS = -O2 -Werror
LDFLAGS = -L /usr/lib
LIBS = -lpng -lm

%.o: $.c
	$(CC) $(CFLAGS) -c -o $@ $< 

all: makePng

makePng: makePng.o
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@ $^ 

.PHONY: clean

clean:
	rm -f *.o *~ makePng

rebuild: clean all
