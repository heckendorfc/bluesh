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

#ifndef JOBS_H
#define JOBS_H

#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>

typedef struct job_t{
	struct job_t *next;
	pid_t pid;
}job_t;

extern job_t *jobs;
extern int jobfinished;

#ifdef TEST_MODE
#define STATIC
void delete_job(pid_t pid);
#else
#define STATIC static
#endif

void add_job(pid_t pid);
void waitjobs();
void sigchld_handler(int signal);

#endif
