VERSION=\"0.2.0\"

CFLAGS=-O0 -ldmalloc -Wall -g -I. -DVERSION=${VERSION}

scis : scis.o lexer.o main.o symtab.o decode_ops.o
	$(CC) scis.o lexer.o main.o symtab.o decode_ops.o -o scis

clean :
	rm -f *.o scis
