CC = gcc
CFLAGS = -O2 -Werror
LDFLAGS = -L /usr/lib
LIBS = -lpng -lm -lgflags

%.o: $.c
	$(CC) $(CFLAGS) -c -o $@ $< 

all: makepng

makepng: makepng.o
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@ $^ 

.PHONY: clean

clean:
	rm -f *.o *~ makepng

rebuild: clean all
