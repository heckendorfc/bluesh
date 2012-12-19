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

#include <complete.h>
#include <exec.h>
#include <shell.h>

char keep(char *p, int *inquote, int *escape){
	if(*p=='\\'){
		if(!*escape && !*inquote){
			*escape=1;
			if(p[1]=='*' || p[1]=='?' || p[1]=='[')
				return 1;
			return 0;
		}
		else{
			*escape=0;
			return 1;
		}
	}

	if(*p=='\'' || *p=='"'){
		if(*escape){
			*escape=0;
			return 1;
		}

		if(*inquote && *inquote!=*p){
			return 1;
		}
		else if(*inquote==*p){
			*inquote=0;
			return 0;
		}
		else if(!*inquote){
			*inquote=*p;
			return 0;
		}
	}

	*escape=0;
	return 1;
}

int strip_quotes(char *s, int len){
	int i,j;
	int inquote=0;
	int escape=0;
	int ret=len;
	for(i=0;i<len;i++){
		if(keep(s+i,&inquote,&escape)==0){
			for(j=i;j<len;j++)
				s[j]=s[j+1];
			ret--;
		}
	}
	return ret;
}

int common_prefix(char *a, char *b){
	int i;
	for(i=0;a[i] && b[i];i++)
		if(a[i]!=b[i])return i;
	return i;
}

char* complete(char *s, int len){
	char *search;
	char temp=s[len];
	char **results;
	char *ret=NULL;
	int i;
	int prefix=0;
	int test=0;
	int newlen;

	INIT_MEM(search,len+2);
	memcpy(search,s,len);

	newlen=strip_quotes(search,len);

	search[newlen]='*';
	search[newlen+1]=0;

	s[len]=0;
	fprintf(stderr,"COMPLETE|%s\n",s);
	s[len]=temp;

	results=simple_glob(search);
	if(results[0]){
		prefix=strlen(results[0]);
		for(i=1;results[i];i++){
			test=common_prefix(results[0],results[i]);
			prefix=minimum(prefix,test);
		}
		INIT_MEM(ret,prefix+1);
		memcpy(ret,results[0],prefix);
		ret[prefix]=0;
	}

	free(search);
	for(i=0;results[i];i++)free(results[i]);
	free(results);

	return ret;
}
