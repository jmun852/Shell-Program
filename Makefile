CC=gcc
CFLAGS=-g -Wall
LIBFLAGS=-ltermcap -lcurses
CFLAGSO=-g -Wno-parentheses -Wno-format-security

DEF=-DHAVE_CONFIG_H -DRL_LIBRARY_VERSION='"8.1"'
INC=-I./include
LIBLOC=./lib/

all: 	rlbasic 	\
	histexamp 	\
	shell	\
	yosh

%.o : %.c
	$(CC) $(CFLAGSO) $(DEF) $(INC) -c $<

rlbasic : rlbasic.o
	$(CC) $(CFLAGS) -o $@ $< $(LIBLOC)libreadline.a $(LIBFLAGS)

histexamp : histexamp.o
	$(CC) $(CFLAGS) -o $@ $< $(LIBLOC)libhistory.a $(LIBFLAGS)

shell:	shell.o parse.o printing.o parse.h printing.h
	$(CC) $(CFLAGS) -o $@ shell.o parse.o printing.o $(LIBLOC)libreadline.a $(LIBFLAGS)

yosh:	yosh.o parse.o printing.o process.o parse.h printing.h process.h
	$(CC) $(CFLAGS) -o $@ yosh.o parse.o printing.o process.o $(LIBLOC)libreadline.a $(LIBFLAGS)

clean:
	rm -f shell *~
	rm -f pipe
	rm -f *.o
	rm -f rlbasic rlbasic.o
	rm -f histexamp histexamp.o
