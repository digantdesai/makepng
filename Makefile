CC = gcc
CFLAGS = -O2 -Werror
LDFLAGS = -L /usr/lib
LIBS = -lpng -lm

PROG = makepng

%.o: $.c $(PROG).h
	$(CC) $(CFLAGS) -c -o $@ $< 

all: $(PROG)

$(PROG): $(PROG).o
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@ $^ 

.PHONY: clean

clean:
	rm -f *.o *~ $(PROG)

rebuild: clean all
