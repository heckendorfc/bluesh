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

#ifndef BUILD_H
#define BUILD_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#define WORD_DEFAULT	1
#define WORD_SQUOT		2
#define WORD_DQUOT		4

#define COM_DEFAULT 	1
#define COM_PIPE 		2
#define COM_BG 			4
#define COM_VAR 		8
#define COM_SUBST		16

#define REDIR_DEST_STR 	1
#define REDIR_DEST_INT 	2

typedef struct wordchain_t{
	char *word;
	int flags;
	struct wordchain_t *next;
}wordchain_t;

typedef struct wordlist_t{
	char *word;
	struct wordlist_t *next;
}wordlist_t;

typedef struct redirect_t{
	//wordchain_t *wc_fd;
	//wordchain_t *wc_dest;
	int fd;
	union {
		char *dest;
		int dfd;
	};
	int d_flag;
	int flags;
	struct redirect_t *next;
}redirect_t;

typedef struct command_t{
	wordlist_t *args;
	redirect_t *redirection;
	int flags;
	struct{
		int pid;
		int infd;
		int outfd;
	}exec;
	struct command_t *next;
}command_t;

wordchain_t* make_word(wordchain_t *word, char *piece, int flags);
wordlist_t* make_word_list(wordlist_t *wl, wordchain_t *word);
wordlist_t* append_wordlist(wordlist_t *a, wordlist_t *b);
redirect_t* make_redirect(int type, wordchain_t *in, wordchain_t *out);
command_t* make_command(wordlist_t *wl, redirect_t *redirect);
command_t* append_command(command_t *a, command_t *b);
void append_command_flags(command_t *a, const int flags);

#endif
