SOURCE=lex.c lex_dfa.c y.tab.c shell.c build.c exec.c edit.c complete.c builtin.c variable.c

default: build

build:
	yacc -t -d parse.y
	clang -Wall -g -O0 -I. -I/usr/local/include -L/usr/local/lib -o bluesh ${SOURCE}

clean:
	rm *.o bluesh
