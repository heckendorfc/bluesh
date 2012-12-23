#ifndef BUILTIN_H
#define BUILTIN_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <build.h>

typedef struct builtin_t{
	const char *name;
	int (*func)(wordlist_t *);
}builtin_t;

extern builtin_t builtins[];

//int cmd_cd(wordlist_t *arg);

#endif
