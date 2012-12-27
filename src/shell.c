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

#include <shell.h>
#include <exec.h>
#include <edit.h>
#include <variable.h>
#include <history.h>

int yyparse();
TokenList *tlist;

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
		ptr=tlist=lex(source);
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
	init_history();
	shell();
	return 0;
}
#endif
