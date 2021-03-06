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

%{
#include <stdio.h>
#include <lex.h>
#include <build.h>
int yylex();
int yyerror();

command_t *start_command;
int charsub=0;
int wp_flag=0;
%}

%token TOK_TEXT TOK_QUOTE TOK_META TOK_QUOTE_STR TOK_WHITESPACE
%token TOK_WORD
%token TOK_SEMICOLON TOK_OPAR TOK_CPAR TOK_AMP TOK_AMPAMP TOK_BACKTICK
%token TOK_BAR TOK_BARBAR TOK_GT TOK_GTAMP TOK_GTGT TOK_LT TOK_LTAMP TOK_LTLT
%token TOK_SET TOK_FOR TOK_WHILE TOK_IN TOK_OCBRACE TOK_CCBRACE

%union{
	int number;
	char *word;
	wordchain_t *wc;
	wordlist_t *wl;
	redirect_t *redirect;
	command_t *command;
}

%start start_command

%nonassoc TOK_WHITESPACE TOK_TEXT TOK_QUOTE
%right TOK_BAR
%left TOK_META

%%

word_piece:	TOK_TEXT
			{
				wp_flag=WORD_DEFAULT;
				$$.word=$1.word;
			}
	|	TOK_QUOTE TOK_QUOTE_STR TOK_QUOTE
			{
				wp_flag=(*($1.word)=='"'?WORD_DQUOT:WORD_SQUOT);
				$$.word=$2.word;
			}
/*
	|	TOK_META
			{
				printf("word META\n");
				if(charsub==0 && *($1.word)=='['){
					charsub=1;
				}
				else if(charsub==1 && *($1.word)==']'){
					charsub=0;
				}
				else
					wp_flag=find_meta_flag($1.word);
					$$=$1;
			}
*/
	;

word:	word_piece word
			{ $$.wc = make_word($2.wc,$1.word,wp_flag); }
	|	word_piece
			{ $$.wc = make_word(NULL,$1.word,wp_flag); }
	;

words:	word
			{ $$.wl = make_word_list(NULL,$1.wc); }
	|	words TOK_WHITESPACE word
			{ $$.wl = make_word_list($1.wl,$3.wc); }
	;

redirect:	TOK_LT TOK_WHITESPACE word
			{ $$.redirect = make_redirect(TOK_LT,NULL,$3.wc); }
	|	word TOK_LT TOK_WHITESPACE word
			{ $$.redirect = make_redirect(TOK_LT,$1.wc,$4.wc); }
	|	TOK_LTAMP TOK_WHITESPACE word
			{ $$.redirect = make_redirect(TOK_LTAMP,NULL,$3.wc); }
	|	word TOK_LTAMP TOK_WHITESPACE word
			{ $$.redirect = make_redirect(TOK_LTAMP,$1.wc,$4.wc); }
	|	TOK_LTLT TOK_WHITESPACE word
			{ $$.redirect = make_redirect(TOK_LTLT,NULL,$3.wc); }
	|	word TOK_LTLT TOK_WHITESPACE word
			{ $$.redirect = make_redirect(TOK_LTLT,$1.wc,$4.wc); }
	|	TOK_GT TOK_WHITESPACE word
			{ $$.redirect = make_redirect(TOK_GT,NULL,$3.wc); }
	|	word TOK_GT TOK_WHITESPACE word
			{ $$.redirect = make_redirect(TOK_GT,$1.wc,$4.wc); }
	|	TOK_GTAMP TOK_WHITESPACE word
			{ $$.redirect = make_redirect(TOK_GTAMP,NULL,$3.wc); }
	|	word TOK_GTAMP TOK_WHITESPACE word
			{ $$.redirect = make_redirect(TOK_GTAMP,$1.wc,$4.wc); }
	|	TOK_GTGT TOK_WHITESPACE word
			{ $$.redirect = make_redirect(TOK_GTGT,NULL,$3.wc); }
	|	word TOK_GTGT TOK_WHITESPACE word
			{ $$.redirect = make_redirect(TOK_GTGT,$1.wc,$4.wc); }
	;

redirect_list:	redirect
	|	redirect_list TOK_WHITESPACE redirect
			{ ($3.redirect)->next=($1.redirect); $$.redirect=$3.redirect; }
	;

command:	words
			{ $$.command=make_command($1.wl,NULL); }
	|	words TOK_WHITESPACE redirect_list
			{ $$.command=make_command($1.wl,$3.redirect); }
	;

pipecommand:	command TOK_BAR
			{ $1.command->flags=COM_PIPE; $$=$1; }
	|	command
			{ $1.command->flags=COM_DEFAULT; $$=$1; }
	;

pipeline:	pipeline pipecommand
			{ $$.command=append_command($1.command,$2.command); }
	|	pipecommand
			{ $$.command=$1.command; }
	;

start_command:	command_line
			{ start_command=$1.command; $$=$1; }
	;

command_line:	command_line command_line1
			{ $$.command=append_command($1.command,$2.command); }
	|	command_line1
			{ $$=$1; }
	;

command_line1:	command_line2 TOK_AMP
			{ append_command_flags($1.command,COM_BG); $$=$1; }
	|	command_line2 TOK_SEMICOLON
			{ append_command_flags($1.command,COM_SEMI); $$=$1; }
	|	command_line2 TOK_AMPAMP
			{ append_command_flags($1.command,COM_AND); $$=$1; }
	|	command_line2 TOK_BARBAR
			{ append_command_flags($1.command,COM_OR); $$=$1; }
	|	command_line2 TOK_BACKTICK
			{ append_command_flags($1.command,COM_SUBST); $$=$1; }
	;

command_line2:	pipeline
			{ $$=$1; }
	|	TOK_SET words
			{ $$.command=make_command($2.wl,NULL); $$.command->flags=COM_VAR; }
	|	TOK_FOR words TOK_IN words TOK_OCBRACE command_line TOK_CCBRACE
			{ 
				$$.command=make_for_command($2.wl,$4.wl,$6.command);
				$$.command->flags=(COM_SEMI|COM_FOR);
				append_command($$.command,make_command(NULL,NULL));
				append_command_flags($$.command,COM_ENDFOR);
			}
	|	TOK_WHILE pipeline TOK_OCBRACE command_line TOK_CCBRACE
			{
				$$.command=make_while_command($2.command,$4.command);
			}
	|	redirect_list
			{ $$.command=make_command(NULL,$1.redirect); $$.command->flags=COM_DEFAULT; }
	|	/* nothing */
			{ $$.command=make_command(NULL,NULL); $$.command->flags=COM_DEFAULT; }
	;
