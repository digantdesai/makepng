CC = gcc
CFLAGS = -O2 -Werror
LDFLAGS = -L /usr/lib
LIBS = -lpng -lm -lgcrypt

PROG = makepng

%.o: $.c $(PROG).h
	$(CC) $(CFLAGS) -c -o $@ $<

all: $(PROG)

debug: CFLAGS +=-g -DDEBUG
debug: $(PROG)

$(PROG): $(PROG).o validate.o encode-decode.o utils.o
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@ $^

.PHONY: clean

clean:
	rm -f *.o *~ $(PROG) *.log test/*~

rebuild: clean all
rebuild_debug: clean debug
