CC = gcc
CFLAGS = -O2
LDFLAGS = -L /usr/lib
LIBS = -lpng -lm

%.o: $.c
	$(CC) $(CFLAGS) -c -o $@ $< 

all: gpg2png

gpg2png: gpg2png.o
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@ $^ 

.PHONY: clean

clean:
	rm -f *.o *~ gpg2png 

rebuild: clean all
