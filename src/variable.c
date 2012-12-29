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

#include <variable.h>
#include <shell.h>

int loc_tab_cap;
int loc_tab_size;
var_t **loc_tab;

char** split_colons(char *str){
	int i;
	int count=0;
	char **ret;
	char *start;

	for(i=0;str[i];i++)if(str[i]==':')count++;

	INIT_MEM(ret,count+2);

	start=str;
	for(count=i=0;str[i];i++){
		if(str[i]==':'){
			str[i]=0;
			if(*start)
				ret[count++]=start;
			start=str+i+1;
		}
	}

	if(*start)
		ret[count++]=start;

	ret[count]=NULL;
	return ret;
}

void set_variable_simple(char *str){
	putenv(str);
}

void set_variable(const char *name, const char *value){
	setenv(name,value,1);
}

char* get_variable(const char *name){
	return getenv(name);
}

void init_local_table(){
	int i;
	INIT_MEM(loc_tab,INITIAL_LOCTAB_SIZE);
	loc_tab_size=INITIAL_LOCTAB_SIZE;
	loc_tab_cap=0;
	for(i=0;i<loc_tab_size;i++)loc_tab[i]=NULL;
}

uint32_t hash(const char *str){
	uint32_t k;
	const char *p=str;
	for(k=0;*p;p++){
		k += *p;
		k += (k<<10);
		k ^= (k>>6);
	}
	k += (k<<3);
	k ^= (k>>11);
	k += (k<<15);

	return k;
}

void grow_local(){
	var_t **temp=loc_tab;
	int i,j;
	uint32_t k;
	int new_size=2*loc_tab_size;

	INIT_MEM(loc_tab,new_size);
	for(i=0;i<new_size;i++)loc_tab[i]=NULL;

	for(i=0;i<loc_tab_size;i++){
		if(temp[i]==NULL)continue;
		
		k=hash(temp[i]->name);

		for(j=k%new_size;loc_tab[j]!=NULL;j=(j+1+k)%new_size);
		loc_tab[j]=temp[i];
	}
}

int find_local(const char *n, uint32_t k){
	int i;
	int index;

	for(i=0;i<loc_tab_size;i++){
		index=(i+k)%loc_tab_size;
		if(loc_tab[index]==NULL || strcmp(loc_tab[index]->name,n)==0)
			break;
	}

	return index;
}

void set_local(const char *name, const char *value){
	uint32_t k;
	int index;

	if(loc_tab_cap>(loc_tab_size/4)*3)
		grow_local();

	k=hash(name);
	index=find_local(name,k);
	if(loc_tab[index]==NULL){
		INIT_MEM(loc_tab[index],1);
		loc_tab[index]->name=strdup(name);
		loc_tab[index]->value=NULL;
		loc_tab_cap++;
	}
	if(loc_tab[index]->value)
		free(loc_tab[index]->value);
	loc_tab[index]->value=strdup(value);
}

char *get_local(const char *name){
	uint32_t k;
	int index;

	if(!name)return NULL;

	k=hash(name);
	index=find_local(name,k);
	if(loc_tab[index]==NULL){
		fprintf(stderr,"%s: Undefined variable.\n",name);
		return NULL;
	}
	return loc_tab[index]->value;
}
