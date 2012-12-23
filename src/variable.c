#include <variable.h>

void set_variable_simple(char *str){
	putenv(str);
}

void set_variable(const char *name, const char *value){
	setenv(name,value,1);
}

char* get_variable(const char *name){
	return getenv(name);
}
