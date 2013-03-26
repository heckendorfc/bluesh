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

#include <jobs.h>
#include <shell.h>

job_t *jobs=NULL;
int jobfinished=0;

void add_job(pid_t pid){
	/*
	job_t *ptr=jobs;
	INIT_MEM(jobs,1);
	jobs->next=ptr;
	jobs->pid=pid;
	*/
	job_t *ptr=jobs;
	if(ptr){
		while(ptr->next)
			ptr=ptr->next;
		INIT_MEM(ptr->next,1);
		ptr->next->next=NULL;
		ptr->next->pid=pid;
	}
	else{
		INIT_MEM(jobs,1);
		jobs->next=NULL;
		jobs->pid=pid;
	}
}

STATIC
void delete_job(pid_t pid){
	job_t *ptr=jobs;
			
	if(jobs==NULL)return;

	if(jobs->pid==pid){
		ptr=jobs->next;
		free(jobs);
		jobs=ptr;
		return;
	}

	while(ptr->next){
		if(ptr->next->pid==pid){
			job_t *temp=ptr->next->next;
			free(ptr->next);
			ptr->next=temp;
			return;
		}
		ptr=ptr->next;
	}
}

void waitjobs(){
	pid_t ret;
	int status;
	job_t *ptr;

	while(jobfinished){
		ptr=jobs;
		while(ptr){
			ret=waitpid(ptr->pid,&status,WNOHANG);
			if(ret==ptr->pid){
				delete_job(ptr->pid);
				break;
			}
			ptr=ptr->next;
		}
		jobfinished--;
	}
}

void sigchld_handler(int signal){
	if((jobfinished++)==0){
		waitjobs();
	}
}

