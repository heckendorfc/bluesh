SRC=../src
SOURCE=check_main.c ${SRC}/lex.c check_lex.c ${SRC}/lex_dfa.c

default: build
	./run_test

build:
	clang -Wall -g -O0 -DTEST_MODE=1 -I. -I${SRC} -I/usr/local/include -L/usr/local/lib -o run_test ${SOURCE} -lcheck
