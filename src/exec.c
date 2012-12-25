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
#include <builtin.h>
#include <variable.h>

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

char** simple_glob(char *w, int flags){
	glob_t g;
	int glob_flags=GLOB_TILDE|GLOB_NOCHECK|flags;
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
		
		if(r->d_flag==REDIR_DEST_STR){
			words=simple_glob(r->dest,0);
			if(words[0]==NULL){
				return;
			}

			fd = open(words[0],r->flags,0666);
		}
		else{
			fd=r->dfd;
		}

		dup2(fd,r->fd);
		close(fd);

		if(r->d_flag==REDIR_DEST_STR){
			for(;*words;words++)free(*words);
			free(words);
		}
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

int find_builtin(wordlist_t *w){
	int i;

	if(!w) return -1;

	for(i=0;builtins[i].func!=NULL;i++){
		if(strcmp(builtins[i].name,w->word)==0)
			return i;
	}

	return -1;
}

void execute_simple(command_t *a){
	int pid;
	int index=0;

	if(!a || !a->args)return;

	if((index=find_builtin(a->args))>=0){
		builtins[index].func(a->args);
		return;
	}

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

wordlist_t* get_pipe_output(int fd){
	wordlist_t *ret,*wp;
	const int bufsize=2048;
	char *buf;
	char *ptr;
	int num;
	int i;
	int offset;
	int copy=0;

	INIT_MEM(ret,1);
	INIT_MEM(buf,bufsize+1);
	offset=0;
	wp=ret;
	ret->next=NULL;

	while((num=read(fd,buf+offset,bufsize-offset))>0){
		ptr=buf;
		for(i=offset;i<num+offset;i++){
			while(buf[i]=='\n' || buf[i]==' ' || buf[i]=='\t'){
				copy=1;
				buf[i]=0;
				i++;
			}
			if(copy){
				wp->word=strdup(ptr);
				INIT_MEM(wp->next,1);
				wp=wp->next;
				wp->next=NULL;
				ptr=buf+i;
				copy=0;
			}
		}
		offset=(num+offset)-(ptr-buf);
		if(offset>0)
			memmove(buf,ptr,offset);
	}
	if(offset>0){
		ptr[offset]=0;
		wp->word=strdup(ptr);
		wp->next=NULL;
	}
	else{
		if(wp->next){
			for(wp=ret;wp->next->next;wp=wp->next);
			free(wp->next);
			wp->next=NULL;
		}
		else{
			free(wp);
			ret=wp=NULL;
		}
	}
	
	free(buf);

	return ret;
}

void make_sub_redirect(command_t *c, int fd){
	redirect_t *ret;

	INIT_MEM(ret,1);
	ret->next=c->redirection;
	ret->fd=1;
	ret->dfd=fd;
	ret->d_flag=REDIR_DEST_INT;

	c->redirection=ret;
}

void parse_variable(char *s, char **n, char **v){
	*n=s;

	while(*s && *s!='=')s++;
	if(*s=='='){
		*s=0;
		*v=s+1;
	}
	else
		*v=s;
}

void execute_set_variables(command_t *c){
	wordlist_t *w = c->args;
	char *n,*v;
	char *t;

	while(w){
		parse_variable(w->word,&n,&v);
		//fprintf(stderr,"VAR|%s|%s|\n",n,v);
		set_local(n,v);
		//t=get_local(n);
		//fprintf(stderr,"V2|%s|\n",t!=NULL?t:"");
		w=w->next;
	}
}

wordlist_t* create_command_sub(command_t **start){
	wordlist_t *ret=NULL;
	command_t *ptr=*start;
	int pipefd[2];
	int tmp_flag;

	if(!ptr->next)return NULL;
	
	ptr=ptr->next;
	do{
		switch(ptr->flags){
			case COM_SUBST: // single command to substitute
			case COM_DEFAULT:
				pipe(pipefd);
				make_sub_redirect(ptr,pipefd[1]);
				tmp_flag=ptr->flags;
				ptr->flags=COM_BG;
				execute_simple(ptr);
				close(pipefd[1]);
				ret=append_wordlist(ret,get_pipe_output(pipefd[0]));
				close(pipefd[0]);
				ptr->flags=tmp_flag;
				break;
			case COM_PIPE:
				create_pipe(ptr);
				break;
		}
		if(ptr->flags!=COM_SUBST)
			ptr=ptr->next;
	}while(ptr && ptr->flags!=COM_SUBST);

	if(ptr){
		(*start)->next=ptr->next;
		(*start)->flags=ptr->next->flags;
	}
	// else syntax error. TODO: what to do?

	return ret;
}

void execute_commands(command_t *start){
	command_t *ptr;
	command_t *temp_c;
	ptr=start;
	wordlist_t *temp_wl;
	while(ptr){
		fprintf(stderr,"EC|%d|%s|\n",ptr->flags,ptr->args?ptr->args->word:"");
		switch(ptr->flags){
			case COM_VAR:
				execute_set_variables(ptr);
				break;
			case COM_BG:
			case COM_DEFAULT:
				execute_simple(ptr);
				break;
			case COM_PIPE:
				create_pipe(ptr);
				break;
			case COM_SUBST:
				temp_c=ptr;
				temp_wl=create_command_sub(&ptr);
				append_wordlist(temp_c->args,temp_wl);
				append_wordlist(temp_c->args,ptr->next->args);
				execute_simple(temp_c); // TODO: if var, do var rather than exec_simple
				ptr=ptr->next;
				break;
		}
		ptr=ptr->next;
	}
	free_commands(start);
}
