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
int alias_tab_cap;
int alias_tab_size;
var_t **alias_tab;
var_t deleted;

void parse_variable(char *s, char **n, char **v){
	*n=s;

	while(*s && *s!='=')s++;
	if(*s=='='){
		*s=0;
		*v=s+1;
	}
	else
		*v=s;
}

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

void init_alias_table(){
	int i;
	INIT_MEM(alias_tab,INITIAL_ALIASTAB_SIZE);
	alias_tab_size=INITIAL_ALIASTAB_SIZE;
	alias_tab_cap=0;
	for(i=0;i<alias_tab_size;i++)alias_tab[i]=NULL;
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

void grow_local(var_t ***tab,int *size){
	var_t **temp=*tab;
	int i,j;
	uint32_t k;
	int new_size=2*(*size);

	INIT_MEM(*tab,new_size);
	for(i=0;i<new_size;i++)(*tab)[i]=NULL;

	for(i=0;i<*size;i++){
		if(temp[i]==NULL || temp[i]==&deleted)continue;
		
		k=hash(temp[i]->name);

		for(j=k%new_size;(*tab)[j]!=NULL;j=(j+1+k)%new_size);
		(*tab)[j]=temp[i];
	}

	*size=new_size;
}

int find_local(const char *n, uint32_t k, var_t **tab, int size){
	int i;
	int index;
	int ret=-1;

	for(i=0;i<size;i++){
		index=(i+k)%size;
		if(tab[index]==&deleted){
			if(ret<0)
				ret=index;
			continue;
		}
		if(tab[index]==NULL){
			if(ret<0)
				ret=index;
			break;
		}
		if(strcmp(tab[index]->name,n)==0)
			return index;
	}

	return ret;
}

void set_local_var(const char *name, const char *value, var_t **tab, int *size, int *cap){
	uint32_t k;
	int index;

	if(*cap>(*size/4)*3)
		grow_local(&tab,size);

	k=hash(name);
	index=find_local(name,k,tab,*size);
	if(tab[index]==NULL || tab[index]==&deleted){
		INIT_MEM(tab[index],1);
		tab[index]->name=strdup(name);
		tab[index]->value=NULL;
	}
	if(tab[index]==NULL){
		(*cap)++;
	}
	if(tab[index]->value)
		free(tab[index]->value);
	tab[index]->value=strdup(value);
}

void unset_local_var(const char *name, var_t **tab, int *size, int *cap){
	uint32_t k;
	int index;

	k=hash(name);
	index=find_local(name,k,tab,*size);
	if(tab[index]==NULL || tab[index]==&deleted){
		return;
	}
	(*cap)--;
	free(tab[index]);
	tab[index]=&deleted;
}

void set_local(const char *name, const char *value){
	set_local_var(name,value,loc_tab,&loc_tab_size,&loc_tab_cap);
}

void set_alias(const char *name, const char *value){
	set_local_var(name,value,alias_tab,&alias_tab_size,&alias_tab_cap);
}

void unset_alias(const char *name){
	unset_local_var(name,alias_tab,&alias_tab_size,&alias_tab_cap);
}

char *get_local_var(const char *name, var_t **tab, int size){
	uint32_t k;
	int index;

	if(!name)return NULL;

	k=hash(name);
	index=find_local(name,k,tab,size);
	if(tab[index]==NULL || tab[index]==&deleted){
		fprintf(stderr,"%s: Undefined variable.\n",name);
		return NULL;
	}
	return tab[index]->value;
}

char *get_local(const char *name){
	return get_local_var(name,loc_tab,loc_tab_size);
}

int is_alias(const char *name){
	uint32_t k;
	int index;

	if(!name)return 0;

	k=hash(name);
	index=find_local(name,k,alias_tab,alias_tab_size);

	return (alias_tab[index]==NULL || alias_tab[index]==&deleted)?0:1;
}

char *get_alias(const char *name){
	return get_local_var(name,alias_tab,alias_tab_size);
}

