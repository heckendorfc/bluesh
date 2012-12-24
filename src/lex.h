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

#ifndef LEX_H
#define LEX_H

/* Maxumum word length */
#define NUM_CANDIDATE 5

enum SplitPlace{
	SPLIT_BEFORE=1,
	SPLIT_AFTER=2,
};

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <lex_dfa.h>


typedef struct Token{
	char *word;
	int type;
}Token;

typedef struct TokenList{
	Token token;
	struct TokenList *next;
}TokenList;


#ifdef TEST_MODE
#define STATIC
int split_token(TokenList *token, const int start, const int word_i);
int identify(TokenList *token,State *q);
int identify_full(TokenList *token, State *q);
void strip_backslash(Token *token);
TokenList* create_tokens(char *str);
#else
#define STATIC static
#endif

void free_tokens(TokenList *t);
TokenList* lex(const char *str);

#endif
