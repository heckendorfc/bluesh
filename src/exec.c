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

#include <exec.h>
#include <shell.h>

void free_redirection(redirect_t *r){
	redirect_t *p;
	if(!r)return;
	for(p=r->next;r;){
		free(r);
		r=p;
		if(p)p=p->next;
	}
}

void free_wordlist(wordlist_t *w){
	wordlist_t *p;
	if(!w)return;
	for(p=w->next;w;){
		free(w->word);
		free(w);
		w=p;
		if(p)p=p->next;
	}
}

void free_commands(command_t *c){
	command_t *p;
	if(!c)return;
	for(p=c->next;c;){
		free_wordlist(c->args);
		free_redirection(c->redirection);
		free(c);
		c=p;
		if(p)p=p->next;
	}
}

void set_variable(char *name, char *value){
}

void variable_command(){
}

int experr(const char *epath, int eerrno){
	fprintf(stderr,"error: %d on %s\n",eerrno,epath);
	return 0;
}

char** copy_glob_list(glob_t *g){
	int i;
	char **ret;

	INIT_MEM(ret,g->gl_pathc+1);

	for(i=0;i<g->gl_pathc;i++){
		ret[i]=strdup(g->gl_pathv[i]);
	}
	ret[i]=NULL;

	return ret;
}

char** simple_glob(char *w){
	glob_t g;
	int glob_flags=GLOB_TILDE|GLOB_NOCHECK;
	char **ret;

	glob(w,glob_flags,&experr,&g);
	
	ret=copy_glob_list(&g);

	return ret;
}

char** glob_wordlist(wordlist_t *w){
	glob_t g;
	wordlist_t *ptr=w;
	char **ret;
	int glob_flags=GLOB_TILDE|GLOB_NOCHECK;

	while(ptr){
		glob(ptr->word,glob_flags,&experr,&g);
		ptr=ptr->next;
		glob_flags|=GLOB_APPEND;
	}

	ret=copy_glob_list(&g);

	globfree(&g);
	return ret;
}

char** wordlist_to_arglist(command_t *a){
	wordlist_t *ptr=a->args;
	char **ret = glob_wordlist(ptr);

/*
	char **rp=ret;
	for(rp=ret;*rp;rp++)printf("RP|%s\n",*rp);
	exit(20);
	return ret;

	for(i=0;ptr;i++)ptr=ptr->next;
	args=malloc(sizeof(char*)*i+1);

	i=0;
	for(ptr=a->args;ptr;ptr=ptr->next)
		args[i++]=ptr->word;
	args[i]=NULL;

	return args;
*/
	return ret;
}

void redirect_recursive(redirect_t *r,int depth){
	int fd;
	char **words;
	if(r && depth<MAX_REDIRECT){
		redirect_recursive(r->next,depth+1);
		
		words=simple_glob(r->dest);
		if(words[0]==NULL){
			return;
		}

		fd = open(words[0],r->flags,0666);
		dup2(fd,r->fd);
		close(fd);

		for(;*words;words++)free(*words);
		free(words);
	}
}

void redirect_io(command_t *a){
	/* Redirects are stored in reverse order.
 	 * 	Call them backwards: */
	redirect_recursive(a->redirection,0);
}

void printexecerror(const char *arg){
	fprintf(stderr,"%s: %s.\n",arg,strerror(errno));
}

void execute_simple(command_t *a){
	int pid;

	pid=fork();

	if(pid==0){ /* Child */
		char **args=wordlist_to_arglist(a);
		//fprintf(stderr,"SHELL|%s\n",args[0]);
		redirect_io(a);
		if(a->exec.infd>=0){ /* Take from pipe */
			dup2(a->exec.infd,0);
			close(a->exec.infd);
		}
		if(execvp(args[0],args))
			printexecerror(args[0]);
		exit(1);
	}
	else if(pid>0){ /* Parent */
		if(a->exec.infd>=0){ /* Close tail of pipe */
			close(a->exec.infd);
		}
		if(a->flags&COM_DEFAULT){
			int status;
			waitpid(pid,&status,0);
		}
	}
	else{
		fprintf(stderr,"fork() error\n");
	}
}

void create_pipe(command_t *a){
	int pipefd[2];
	int pid;

	pipe(pipefd);

	pid=fork();

	a->exec.outfd=pipefd[1];
	a->next->exec.infd=pipefd[0];
	
	if(pid==0){ /* Child */
		char **args=wordlist_to_arglist(a);
		redirect_io(a);
		dup2(a->exec.outfd,1);
		if(a->exec.infd>=0){
			dup2(a->exec.infd,0);
			close(a->exec.infd);
		}
		close(pipefd[0]);
		close(pipefd[1]);
		//execvp(a->args->word,args);
		if(execvp(args[0],args))
			printexecerror(args[0]);
		exit(1);
	}
	else if(pid>0){ /* Parent */
		a->exec.pid=pid;

		close(a->exec.outfd);
		if(a->exec.infd>=0){
			close(a->exec.infd);
		}
	}
	else{
		fprintf(stderr,"fork() error\n");
	}
}

void execute_commands(command_t *start){
	command_t *ptr=start;
	ptr=start;
	while(ptr){
		//fprintf(stderr,"SHELL|%s (%d)\n",ptr->args->word,ptr->flags);
		switch(ptr->flags){
			case COM_DEFAULT:
				execute_simple(ptr);
				break;
			case COM_PIPE:
				create_pipe(ptr);
				break;
		}
		ptr=ptr->next;
	}
	free_commands(start);
}
