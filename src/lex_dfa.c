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

#include <lex.h>
#include <lex_dfa.h>
#include <shell.h>

State *trap=NULL;

void add_path(State *dfa,const char *str,int token){
	int i,j;
	State *o=dfa;
	for(i=0;str[i];i++){
		if(!dfa->out){
			INIT_MEM(dfa->out,SIGMA_SIZE);
			for(j=0;j<SIGMA_SIZE;j++){
				dfa->out[i].state=trap;
			}
		}
		if(!dfa->out[(int)str[i]].state ||
				dfa->out[(int)str[i]].state==o ||
				dfa->out[(int)str[i]].state==trap){
			INIT_MEM(dfa->out[(int)str[i]].state,1);
			dfa->out[(int)str[i]].state->final=TOK_NULL;
			dfa->out[(int)str[i]].state->out=NULL;
		}
		dfa=dfa->out[(int)str[i]].state;
	}
	dfa->final=token;
	dfa->out=NULL;
	//dfa->out=malloc(sizeof(*dfa->out)*SIGMA_SIZE);
	//for(i=0;i<SIGMA_SIZE;i++){
		//dfa->out[i].state=o;
	//}
}

State* generate_operator_dfa(){
	int i;

	State *dfa;
	INIT_MEM(dfa,1);

	dfa->out=NULL;
	dfa->final=0;
	INIT_MEM(dfa->out,SIGMA_SIZE);
	for(i=0;i<SIGMA_SIZE;i++){
		dfa->out[i].state=dfa;
	}

	if(!trap){
		INIT_MEM(trap,1);
		trap->final=TOK_NULL;
		trap->out=NULL;
	}
	/*
	malloc(sizeof(*dfa->out)*SIGMA_SIZE);
	for(i=0;i<SIGMA_SIZE;i++){
		trap->out[i].state=trap;
	}*/

	/*
	malloc(dfa->out,sizeof(*dfa->out)*SIGMA_SIZE);
	for(i=0;i<SIGMA_SIZE;i++){
		dfa->out[i].state=trap;
		dfa->out[i].final=0;
	}
	*/

	//add_path("echo",TOK_B_COMMAND);
	//add_path("cd",TOK_B_COMMAND);

	add_path(dfa,";",TOK_OPERATOR|TOK_SEMICOLON);

	add_path(dfa,"(",TOK_OPERATOR|TOK_OPAR);

	add_path(dfa,")",TOK_OPERATOR|TOK_CPAR);

	add_path(dfa,"&",TOK_OPERATOR|TOK_AMP);
	add_path(dfa,"&&",TOK_OPERATOR|TOK_AMPAMP);

	add_path(dfa,"|",TOK_OPERATOR|TOK_BAR);
	add_path(dfa,"||",TOK_OPERATOR|TOK_BARBAR);

	add_path(dfa,">",TOK_REDIRECT|TOK_GT);
	add_path(dfa,">&",TOK_REDIRECT|TOK_GTAMP);
	add_path(dfa,">>",TOK_REDIRECT|TOK_GTGT);

	add_path(dfa,"<",TOK_REDIRECT|TOK_LT);
	add_path(dfa,"<&",TOK_REDIRECT|TOK_LTAMP);
	add_path(dfa,"<<",TOK_REDIRECT|TOK_LTLT);

	//dfa->out['|'].final=TOK_OPERATOR;
	//dfa->out['\\'].final=TOK_OPERATOR;

	return dfa;
}

State* generate_quote_dfa(){
	int i;

	State *dfa;
	INIT_MEM(dfa,1);

	dfa->out=NULL;
	dfa->final=0;
	INIT_MEM(dfa->out,SIGMA_SIZE);
	for(i=0;i<SIGMA_SIZE;i++){
		dfa->out[i].state=dfa;
	}

	if(!trap){
		INIT_MEM(trap,1);
		trap->final=TOK_NULL;
		trap->out=NULL;
	}

	add_path(dfa,"\"",TOK_QUOTE);
	add_path(dfa,"'",TOK_QUOTE);

	return dfa;
}

State* generate_reserved_dfa(){
	int i;

	State *dfa;
	INIT_MEM(dfa,1);

	dfa->out=NULL;
	dfa->final=0;
	INIT_MEM(dfa->out,SIGMA_SIZE);
	for(i=0;i<SIGMA_SIZE;i++){
		dfa->out[i].state=dfa;
	}

	if(!trap){
		INIT_MEM(trap,1);
		trap->final=TOK_NULL;
		trap->out=NULL;
	}

	add_path(dfa,"set",TOK_RESERVED|TOK_SET);
	//add_path(dfa,"",TOK_RESERVED|TOK_);

	return dfa;
}

State* generate_meta_dfa(){
	int i;

	State *dfa;
	INIT_MEM(dfa,1);

	dfa->out=NULL;
	dfa->final=0;
	INIT_MEM(dfa->out,SIGMA_SIZE);
	for(i=0;i<SIGMA_SIZE;i++){
		dfa->out[i].state=dfa;
	}

	if(!trap){
		INIT_MEM(trap,1);
		trap->final=TOK_NULL;
		trap->out=NULL;
	}

	//add_path(dfa,"!",TOK_META);
	add_path(dfa,"*",TOK_META);
	add_path(dfa,"?",TOK_META);
	add_path(dfa,"[",TOK_META);
	add_path(dfa,"]",TOK_META);

	return dfa;
}
