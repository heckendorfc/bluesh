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

#include <edit.h>
#include <complete.h>
#include <shell.h>
#include <history.h>

static char *xbuf;
static char *xend;
static char *cursor;
static char *cbuf=NULL; // used for complete
static int inquote;

int key_backspace();
int key_literal();
int key_escape();
int key_exit();
int key_complete();
int key_done();

enum cc_returns{
	CC_DEFAULT=0,
	CC_EXIT,
	CC_NOECHO,
	CC_DONE,
};

enum cchars{
	CTRL_D=4,
	CTRL_H=8,
	CTRL_I=9,
	CTRL_J=10,
};

struct keybind{
	int (*func)();
	char key;
}defbind[]={
	{key_exit,		CTRL('D')},
	{key_complete,	CTRL('I')},
	{key_done,		CTRL('J')},
	{key_literal,	CTRL('V')},
	{key_backspace,	CTRL('H')},
	{key_backspace,	CTRL('?')},
	{key_escape,	CTRL('[')},
	{NULL,0},
};

void init_prompt(char *s){
	int i;
	cursor=xbuf=xend=s;
	for(i=0;i<XEND_OFFSET;i++)
		xend[i]=' ';
	xend[XEND_OFFSET]=0;
	xend+=XEND_OFFSET;
	//cbuf[0]=0;
	bzero(cbuf,SOURCE_INPUT_SIZE);
	inquote=0;
	printf("bluesh> ");
}

void print_char(char c){
	if(c<32){
		printf("^%c",c+64);
	}
	else{
		printf("%c",c);
	}
}

char get_cpos_char(char *start, char *ptr, char c){
	if(c==' ' || c=='\t'){
		if(inquote || (ptr>start && *(ptr-1)=='\\')){
			return c;
		}
		else{
			return 0;
		}
	}
	else if(c=='\'' || c=='"'){
		if(ptr>start && *(ptr-1)=='\\')
			return c;

		if(inquote && inquote==c){
			inquote=0;
		}
		else if(!inquote){
			inquote=c;
		}
		return c;
	}
	return c;
}

void movecursor(char *pos){
	if(pos<xbuf || pos>xend-XEND_OFFSET || pos==cursor)return;

	if(pos<cursor){
		while(pos<cursor){
			if(*(cursor-1)<32)
				printf("\b");
			printf("\b");
			cursor--;
		}
	}
	else{
		while(pos>cursor){
			print_char(*(cursor++));
		}
	}
}

void find_inquote(){
	char *cpos,*cptr,*xptr;

	cptr=cpos=(cursor-xbuf)+cbuf;
	while(*cptr && cptr>cbuf)cptr--; // Find word start
	if(cptr>cbuf)cptr++;
	inquote=0;
	while(cptr<cpos){
		xptr=(cptr-cbuf)+xbuf;
		*cptr=get_cpos_char(xbuf,xptr,*xptr); // Set inquote for current cursor
		cptr++;
	}
}

void insert_char(char c){
	char *pos=cursor;
	//char *cpos=(cursor-xbuf)+cbuf;

	memmove(pos+1,pos,xend-pos);
	*pos=c;

	//memmove(cpos+1,cpos,xend-pos);
	//*cpos=get_cpos_char(cpos,c);
	
	
	*(xend++)=' ';
	while(cursor<(xend-1)){ // XEND_OFFSET?
		print_char(*(cursor++));
	}
	movecursor(pos+1);
	//print_cbuf();
}

// asdfgh00
void delete_chars(int num){
	int i=0;
	int tail=0;
	char *pos=cursor-num;
	//char *cpos=(pos-xbuf)+cbuf;
	//char c=*pos;

	if(pos<xbuf){
		num=cursor-xbuf;
		pos=xbuf;
	}

	movecursor(pos);

	tail=(xend-pos)-num;
	memmove(pos,pos+num,tail);
	//memmove(cpos,cpos+1,xend-(pos));

	for(i=0;i<num;i++)pos[i+tail]=' ';

	while(cursor<(xend-XEND_OFFSET)){
		print_char(*(cursor++));
	}
	movecursor(pos);

	xend-=num;
	//find_inquote();
	//print_cbuf();
}

void delete_char(){
	if(cursor==xbuf)return;

	delete_chars(1);
}

void insert_chars(char *s, int len){
	int i;

	for(i=0;i<len;i++)
		insert_char(s[i]);
}

void replace_word(char *d,int dlen, char *s, int slen){
	char temp=d[dlen];
	d[dlen]=0;
	if(dlen==slen && strcmp(d,s)==0){
		d[dlen]=temp;
		return;
	}
	d[dlen]=temp;

	if(d+dlen<=xend-XEND_OFFSET)
		movecursor(d+dlen);
	else
		movecursor(xend-XEND_OFFSET);
	delete_chars(dlen);
	insert_chars(s,slen);
}

int getchar_timed(){
	int ret;
	fd_set fdset;
	struct timeval timeout;

	return getchar();

	timeout.tv_sec=1;
	timeout.tv_usec=1000;

	FD_ZERO(&fdset);
	FD_SET(0,&fdset);

	if((ret=select(1,&fdset,NULL,NULL,&timeout))>0)
		return getchar();
	return -1;
}

char* timed_escape(){
	char *ret;
	char c;
	int i=0;
	int timed_ret;

	INIT_MEM(ret,ESC_SEQ_SIZE);

	if((timed_ret=getchar_timed())<0 || timed_ret!='['){
		/* Error/Timeout/Non-sequence. send first char found */
		if(timed_ret<0)
			*ret=(char)getchar();
		else
			*ret=timed_ret;
		ret[1]=0;
		return ret;
	}

	ret[i++]=timed_ret; // '['
	do{
		c=ret[i]=(char)getchar();
		i++;
	}while(c<'@' || c>'~');
	ret[i]=0;

	return ret;
}

int key_literal(){
	char c=(char)getchar();
	int i;
	insert_char(c);
	if(c==27){
		char *sequence = timed_escape();
		for(i=0;sequence[i];i++){
			// TODO: make bulk insert function
			insert_char(sequence[i]);
		}
		free(sequence);
	}

	return CC_NOECHO;
}

int key_backspace(){
	delete_char();
	return CC_NOECHO; 
}

int key_left(){
	movecursor(cursor-1);
	//find_inquote();
	return CC_NOECHO;
}

int key_right(){
	movecursor(cursor+1);
	//find_inquote();
	return CC_NOECHO;
}

int key_up(){
	char *str = history_next();
	if(str)
		replace_word(xbuf,((xend-XEND_OFFSET)-xbuf),str,strlen(str));
	return CC_NOECHO;
}

int key_down(){
	char *str = history_prev();
	if(str)
		replace_word(xbuf,((xend-XEND_OFFSET)-xbuf),str,strlen(str));
	else
		replace_word(xbuf,((xend-XEND_OFFSET)-xbuf),"",0);
	return CC_NOECHO;
}

int key_escape(){
	char *sequence = timed_escape();
	if(sequence[0]=='[' && sequence[1]!=0 && sequence[2]==0){
		switch(sequence[1]){
			case 'D':
				key_left();
				break;
			case 'C':
				key_right();
				break;
			case 'A':
				key_up();
				break;
			case 'B':
				key_down();
				break;
		}
	}
	else if(sequence[0]=='^'){
		movecursor(xbuf);
	}
	else if(sequence[0]=='$'){
		movecursor(xend-XEND_OFFSET);
	}

	free(sequence);

	return CC_NOECHO;
}

int key_exit(){
	return CC_EXIT;
}

/*
	-find substring to complete
	-identify type?
*/
int key_complete(){
	int i;
	char *p=(cursor-xbuf)+cbuf;
	char *cend=(xend-xbuf)+cbuf;
	char *start=p-1;
	char *end=p;
	char *completed;

	if(cursor==xbuf && xend<=cursor+XEND_OFFSET)return CC_NOECHO; // Nothing to complete

	if(p==cbuf)start++;

	inquote=0;
	//memcpy(cbuf,xbuf,xend-xbuf);
	for(i=0;xbuf+i<xend;i++)
		cbuf[i]=get_cpos_char(xbuf,xbuf+i,xbuf[i]);
		//find_inquote(cbuf+i,cbuf[i]);

	//print_cbuf();
	while(*start && start>cbuf)start--;
	if(!*start)start++;

	while(*end && end<cend)end++;

	completed=complete(start,end-start);

	if(completed){
		replace_word((start-cbuf)+xbuf,end-start,completed,strlen(completed));
		free(completed);
	}

	return CC_NOECHO;
}

int key_done(){
	return CC_DONE;
}

int handle_char(char c){
	int i;
	if(c<32 || c>126){
		for(i=0;defbind[i].func;i++){
			if(c==defbind[i].key){
				return defbind[i].func();
			}
		}
		return CC_NOECHO;
	}
	else{
		if(xend-xbuf>SOURCE_INPUT_SIZE)
			return CC_NOECHO;
		//*cursor=c;
		//*(++cursor)=' ';
		//xend++;
	}
	return CC_DEFAULT;
}

void cap_buf(){
	char *ptr=xend-XEND_OFFSET;
	while(ptr>xbuf && (*ptr==' ' || *ptr=='\t'))ptr--;
	if(*ptr!=';' && *ptr!='&'){
		ptr[1]=';';
		ptr++;
	}
	ptr[1]=0;
}

int readline(char *source){
	//int i=0;
	int ret;
	char c;

	if(cbuf==NULL)
		INIT_MEM(cbuf,SOURCE_INPUT_SIZE);

	init_prompt(source);

	while(1){
		c=(char)getchar();
		//print_char(c);
		//continue;

		if(!(ret=handle_char(c))){
			//printf("\n%d %d\n",(cursor-xbuf),xend-xbuf);
			//for(i=0;i<(cursor-xbuf);i++)printf("%c",xbuf[i]);
			//printf("%c",c);
			insert_char(c);
		}
		if(ret==CC_EXIT)break;
		if(ret==CC_DONE){
			cap_buf();
			history_add(xbuf);
			printf("\n");
			return 1;
		}
	}
	return 0;
}

