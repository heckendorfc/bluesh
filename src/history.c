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

#include <history.h>
#include <shell.h>

history_t *history;

void init_history(){
	int i;

	INIT_MEM(history,1);
	INIT_MEM(history->line,HISTORY_SIZE);
	history->size=0;
	history->writepos=0;
	history->readpos=0;

	for(i=0;i<HISTORY_SIZE;i++)history->line[i]=NULL;
}

void history_add(char *str){
	if(history->size>0 && strcmp(history->line[history->writepos-1],str)==0)
		return; // don't add duplicates back to back

	if(history->line[history->writepos]){
		free(history->line[history->writepos]);
	}

	history->line[history->writepos++]=strdup(str);

	history->readpos=history->writepos;

	if(history->writepos>=HISTORY_SIZE)
		history->writepos=0;
	if(history->size<HISTORY_SIZE)
		history->size++;
}

int get_prevpos(){
	int ret;

	if(history->readpos<0)
		return -1;

	ret=history->readpos+1;

	if(history->size==HISTORY_SIZE){
		if(ret>=HISTORY_SIZE)
			ret=0;
		if(ret==history->writepos)
			return -1;
	}
	else{
		if(ret>=history->writepos)
			return -1;
	}
	return ret;
}

// More recent
char* history_prev(){
	char *ret;
	int toread;

	toread=get_prevpos();

	if(toread<0){
		history->readpos=toread;
		return NULL;
	}

	ret=history->line[toread];
	history->readpos=toread;

	return ret;
}

int get_nextpos(){
	int ret;

	if(history->readpos<0)
		return history->writepos-1;

	ret=history->readpos-1;
	if(history->size==HISTORY_SIZE){
		if(ret<0)
			ret=HISTORY_SIZE-1;
		if(ret==history->writepos)
			return -1;
	}
	else{
		if(ret<0)
			return -1;
	}
	return ret;
}

// Older
char* history_next(){
	char *ret;
	int toread=get_nextpos();

	if(toread<0)
		return NULL;

	ret=history->line[toread];
	history->readpos=toread;

	return ret;
}

