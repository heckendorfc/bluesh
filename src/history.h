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

#ifndef HISTORY_H
#define HISTORY_H

#include <string.h>

#define HISTORY_SIZE 256

typedef struct history_t{
	char **line;
	int size;
	int writepos;
	int readpos;
}history_t;

void init_history();
void history_add(char *str);
char* history_prev();
char* history_search_prev(const char *str, const int len);
char* history_next();
char* history_search_next(const char *str, const int len);

#endif
