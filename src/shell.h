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

#ifndef SHELL_H
#define SHELL_H

#include <sys/stat.h>

#include <lex.h>
#include <build.h>
#include <y.tab.h>

#define TOK_NULL 0
#define TOK_REDIRECT	0x010000
#define TOK_OPERATOR	0x020000
#define TOK_RESERVED	0x040000
#define TOKEN_MASK		0x00FFFF

#define INIT_MEM(array, size) if(!(array=malloc(sizeof(*(array))*(size))))exit(1);

extern TokenList *tlist;
extern command_t *start_command;

#endif
