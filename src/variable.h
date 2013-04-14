/*
Copyright (c) 2012, Christian Heckendorf <heckendorfc@gmail.com>

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.
*/

#ifndef VARIABLE_H
#define VARIABLE_H

#include <stdlib.h>
#include <stdint.h>

#define INITIAL_LOCTAB_SIZE 32
#define INITIAL_ALIASTAB_SIZE 32

typedef struct var_t{
	char *name;
	char *value;
}var_t;

void parse_variable(char *s, char **n, char **v);
char** split_colons(char *str);

void set_variable_simple(char *str);
void set_variable(const char *name, const char *value);
char* get_variable(const char *name);

void init_local_table();
char *get_local(const char *name);
void set_local(const char *name, const char *value);

void init_alias_table();
int is_alias(const char *name);
char *get_alias(const char *name);
void set_alias(const char *name, const char *value);
void unset_alias(const char *name);

#endif
