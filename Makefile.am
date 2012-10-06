EXTRA_DIST = lexer.h
bin_PROGRAMS = scis
scis_SOURCES = decode_ops.c scis.c lexer.c resource.c sci0.c sci11.c symtab.c

lexer.c: lexer.l
	$(LEX) --header-file=lexer.h -o $@ $<
lexer.h: lexer.c
