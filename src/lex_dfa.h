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

#ifndef LEX_DFA_H
#define LEX_DFA_H

#define SIGMA_SIZE 128

/*
enum LexTokens{
	TOK_NULL=		0x0000,
	TOK_OPERATOR=	0x0001,
	TOK_B_COMMAND=	0x0002,
	TOK_TEXT=		0x0004,
	TOK_QUOTE=		0x0008,
	TOK_META=		0x0010,
	TOK_QUOTE_STR=	0x0020,
	TOK_WHITESPACE= 0x0030,
	TOK_COMMENT=	0x1000,
};
*/

typedef struct State{
	int final;
	struct StateList *out;
}State;

typedef struct StateList{
	State *state;
	//struct StateList *next;
}StateList;

State* generate_operator_dfa();
State* generate_quote_dfa();
State* generate_meta_dfa();
State* generate_reserved_dfa();

#endif
