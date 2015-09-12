CC = gcc
CFLAGS = -Werror -O3
LDFLAGS = -L /usr/lib
LIBS = -lpng

%.o: $.c
	$(CC) -c -o $@ $< $(CFLAGS)

gpg2png: gpg2png.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS) $(LDFLAGS)

.PHONY: clean

clean:
	rm -f *.0 *~ gpg2png 
