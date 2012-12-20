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

#include <build.h>
#include <shell.h>

/* substitute vars if needed
 * escape words in quotes if needed
 * glob
 */

void free_wordchain(wordchain_t *w){
	wordchain_t *p;
	if(!w)return;
	for(p=w->next;w;){
		free(w);
		w=p;
		if(p)p=p->next;
	}
}

wordchain_t* make_word(wordchain_t *word, char *piece, int flags){
	int wordlen;
	wordchain_t *ptr=word;
	if(piece && *piece){
		if(!word){
			INIT_MEM(word,1);
			ptr=word;
			word->word=NULL;
			word->flags=0;
			word->next=NULL;
			wordlen=0;
		}
		else{
			for(ptr=word;ptr->next;ptr=ptr->next);
			//wordlen=strlen(ptr->word);
			INIT_MEM(ptr->next,1);
			ptr=ptr->next;
			ptr->next=NULL;
		}
		//piecelen=strlen(piece);
		ptr->word=piece;
		//ptr->word=realloc(ptr->word,wordlen+piecelen+1);
		ptr->flags=flags;
		//strcat(ptr->word,piece);
		//free(piece);
	}
	return word;
}

/* default:
	subs allowed
   ":
	no subs, vars allowed
   ':
    no subs, no vars
*/
void wordcat(char *word, int *wordsize, wordchain_t *wc){
	int i;
	char *wp=word;

	while(*wp)wp++;
	if(wc->flags&(WORD_DEFAULT)){
		for(i=0;wc->word[i];i++){
			*(wp++)=wc->word[i];
		}
		*wp=0;
	}
	else if(wc->flags&(WORD_SQUOT|WORD_DQUOT)){
		for(i=0;wc->word[i];i++){
			switch(wc->word[i]){
				case '\\':
				case '*':
				case '?':
				case '[':
					*(wp++)='\\';
					break;
				case '$':
					if(wc->flags&WORD_SQUOT)
						*(wp++)='\\';
					break;
			}
			*(wp++)=wc->word[i];
		}
		*wp=0;
	}
	else
		fprintf(stderr,"Warning: invalid word flags (%s)(%d)!\n",wc->word,wc->flags);
}

char* merge_wordchain(wordchain_t *word){
	wordchain_t *wc_ptr;
	int wordsize=0;
	char *ret;

	for(wc_ptr=word;wc_ptr;wc_ptr=wc_ptr->next){
		wordsize+=strlen(wc_ptr->word);
	}

	wordsize=2*wordsize+1;
	INIT_MEM(ret,wordsize);
	*ret=0;
	for(wc_ptr=word;wc_ptr;wc_ptr=wc_ptr->next){
		wordcat(ret,&wordsize,wc_ptr);
	}

	return ret;
}

wordlist_t* append_wordlist(wordlist_t *a, wordlist_t *b){
	if(!a){
		if(b){
			return b;
		}
		return NULL;
	}
	while(a->next){
		a=a->next;
	}
	a->next=b;
	return a;
}

wordlist_t* make_word_list(wordlist_t *wl, wordchain_t *word){
	wordlist_t *wl_ptr;
	if(!wl){
		INIT_MEM(wl,1);
		wl_ptr=wl;
		wl->next=NULL;
	}
	else{
		for(wl_ptr=wl;wl_ptr->next;wl_ptr=wl_ptr->next);
		INIT_MEM(wl_ptr->next,1);
		wl_ptr=wl_ptr->next;
		wl_ptr->next=NULL;
	}

	wl_ptr->word=merge_wordchain(word);

	free_wordchain(word);
	
	return wl;
}

redirect_t* make_redirect(int type, wordchain_t *fd, wordchain_t *dest){
	redirect_t *ret;
	char *word;

	INIT_MEM(ret,1);
	//ret->wc_fd=fd;
	//ret->wc_dest=dest;
	ret->flags=0;
	ret->next=NULL;


	if(fd!=NULL){
		word=merge_wordchain(fd);
		if(*word>='0' && *word<='9')
			ret->fd=strtol(word,NULL,10);
		free(word);
	}
	else
		ret->fd=-1;
	ret->dest=merge_wordchain(dest);
	ret->d_flag=REDIR_DEST_STR;

	free_wordchain(fd);
	free_wordchain(dest);

	switch(type){
		case TOK_LT:
			ret->flags = O_RDONLY;
			if(ret->fd<0)ret->fd=0;
			break;

		case TOK_GT:
			ret->flags = O_TRUNC | O_WRONLY | O_CREAT;
			if(ret->fd<0)ret->fd=1;
			break;

		case TOK_GTAMP:
			ret->flags = O_WRONLY;
			if(ret->fd<0 || ret->fd>9)ret->fd=1;
			if(*ret->dest>='0' && *ret->dest<='9' && ret->dest[1]=='\0'){
				char *temp = ret->dest;
				INIT_MEM(ret->dest,10); /* /dev/fd/?\0 */
				sprintf(ret->dest,"/dev/fd/%c",*temp);
				free(temp);
			}
			else{
				char *temp = ret->dest;
				INIT_MEM(ret->dest,10); /* /dev/fd/?\0 */
				sprintf(ret->dest,"/dev/fd/%d",ret->fd);
				free(temp);
			}
			break;

		case TOK_GTGT:
			ret->flags = O_APPEND | O_WRONLY | O_CREAT;
			if(ret->fd<0)ret->fd=1;
			break;

		default:
			fprintf(stderr,"Warning: using default redirect flags\n"); break;
	}

	return ret;
}

command_t* make_command(wordlist_t *wl, redirect_t *redirect){
	command_t *ret;

	INIT_MEM(ret,1);
	ret->args=wl;
	ret->redirection=redirect;
	ret->flags=0;
	ret->next=0;
	ret->exec.infd=ret->exec.outfd=-1;
	ret->exec.pid=0;

	return ret;
}

command_t* append_command(command_t *a, command_t *b){
	command_t *temp=a;

	while(a->next)a=a->next;
	a->next=b;

	return temp;
}

void append_command_flags(command_t *a, const int flags){
	while(a->next)a=a->next;
	a->flags=flags;
}
