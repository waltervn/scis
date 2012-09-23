VERSION=\"0.3.0\"

CFLAGS=-O0 -ldmalloc -Wall -g -I. -DVERSION=${VERSION}

scis : scis.o lexer.o main.o symtab.o decode_ops.o scis.h symtab.h
	$(CC) scis.o lexer.o main.o symtab.o decode_ops.o -o scis

clean :
	rm -f *.o scis
