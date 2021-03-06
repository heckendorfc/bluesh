/*
Copyright (c) 2013, Christian Heckendorf <heckendorfc@gmail.com>

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

#include <shell.h>
#include <exec.h>
#include <edit.h>
#include <variable.h>
#include <history.h>
#include <sig.h>
#include <alias.h>

int yyparse();
TokenList *tlist;

void run_rc(){
	TokenList *ptr;
	const int path_size=512;
	char *home;
	char *line;
	char path[path_size];
	FILE *rcfd;
	int i;

	home=get_variable("HOME");

	if(strlen(home)+strlen(RC_FILENAME)>path_size)
		return;

	sprintf(path,"%s/.%s",home,RC_FILENAME);

	if((rcfd=fopen(path,"r"))==NULL)
		return;

	INIT_MEM(line,SOURCE_INPUT_SIZE);

	while(fgets(line,SOURCE_INPUT_SIZE,rcfd)){
		for(i=0;i<SOURCE_INPUT_SIZE && line[i] && line[i]!='\n';i++);
		if(i==0)continue;
		line[i]=0;
		tlist=lex(line);
		replace_alias(&tlist);
		ptr=tlist;
		start_command=NULL;

		if(yyparse()==0){
			execute_commands(start_command);
			free_commands(start_command);
		}

		free_tokens(ptr);
	}

	free(line);
	fclose(rcfd);
}

void* setterm(){
	struct termios *orig;
	struct termios new;

	INIT_MEM(orig,1);
	tcgetattr(0,orig);
	new=*orig;
	new.c_lflag&=(~ICANON);
	new.c_lflag&=(~ECHO);
	new.c_lflag&=(~ISIG);
	new.c_lflag&=(~IEXTEN);
	new.c_cc[VTIME]=0;
	new.c_cc[VMIN]=1;
	tcsetattr(0,TCSANOW,&new);
	
	return orig;
}

void unsetterm(void *term){
	struct termios *t = (struct termios*)term;
	tcsetattr(0,TCSANOW,t);
	free(t);
}

void shell(){
	char *source;
	TokenList *ptr;
	void *t = setterm();

	INIT_MEM(source,SOURCE_INPUT_SIZE+2);

	while(readline(source)){
		tlist=lex(source);
		replace_alias(&tlist);
		ptr=tlist;
		start_command=NULL;

		if(yyparse()==0){
			unsetterm(t);
			execute_commands(start_command);
			free_commands(start_command);
			t=setterm();
		}

		free_tokens(ptr);
	}
	unsetterm(t);
}

#ifndef TEST_MODE
int main(){
	init_local_table();
	init_alias_table();
	init_history();
	set_signals();
	run_rc();
	shell();
	return 0;
}
#endif
