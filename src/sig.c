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

#include <sig.h>
#include <jobs.h>

int caught_sigint;

void sigint_handler(int sig){
	caught_sigint++;
}

void set_signal(int sig, void(*handler)(int)){
	struct sigaction sa;

	sa.sa_handler=handler;
	sa.sa_flags=0;
	sigemptyset(&sa.sa_mask);
	
	sigaction(sig,&sa,NULL);
}

void set_signals(){
	caught_sigint=0;

	set_signal(SIGCHLD,sigchld_handler);
	set_signal(SIGINT,sigint_handler);
}
