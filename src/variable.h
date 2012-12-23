#ifndef VARIABLE_H
#define VARIABLE_H

#include <stdlib.h>

void set_variable_simple(char *str);
void set_variable(const char *name, const char *value);
char* get_variable(const char *name);

#endif
