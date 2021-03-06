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

#include <exec.h>
#include <shell.h>
#include <builtin.h>
#include <variable.h>
#include <build.h>
#include <jobs.h>
#include <sig.h>

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
	int glob_flags=flags;
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

int get_var_position(char **v){
	char *p=*v;

	if(*p!='{')return 0;

	(*v)++;

	for(p++;*p && *p!='}';p++);

	return p-(*v);
}

char* replace_string(char *full, char *a, char *b, int a_rep, int a_size){
	int b_size;
	char *ret;

	if(!b){
		// No such variable. fail instead?
		memmove(a,a+a_rep,a_size-a_rep);
		return full;
	}
	else{
		b_size=strlen(b);
		if(b_size<=a_rep){
			memmove(a+b_size,a+a_rep,a_size-a_rep);
			memcpy(a,b,b_size);
			return full;
		}
		else{
			int fullsize=(a-full)+a_size;
			fullsize+=(b_size-a_rep);
			INIT_MEM(ret,fullsize);
			memcpy(ret,full,a-full);
			memcpy(ret+(a-full),b,b_size);
			memcpy(ret+(a-full)+b_size,a+a_rep,a_size-a_rep);
			free(full);
			return ret;
		}
	}
}

int substitute_variables(wordlist_t *a){
	char c;
	char *temp;
	char *ptr,*start;
	int len;
	int word_len;
	int offset;

	while(a){
		word_len=strlen(a->word)+1;
		for(ptr=a->word;*ptr;ptr++){
			if(*ptr=='$' && (ptr==a->word || *(ptr-1)!='\\')){
				offset=ptr-(a->word);
				start=ptr+1;
				len=get_var_position(&start);
				c=start[len];
				start[len]=0;
				temp=get_local(start);
				start[len]=c;
				a->word=replace_string(a->word,ptr,temp,len+3,word_len-offset);
				ptr=(a->word+offset+len+3)-1;
			}
		}
		a=a->next;
	}
	return 0;
}

char** wordlist_to_arglist(command_t *a){
	wordlist_t *ptr=a->args;
	char **ret;

	substitute_variables(ptr);
	ret = glob_wordlist(ptr);

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
			words=simple_glob(r->dest,GLOB_TILDE|GLOB_NOCHECK);
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
		char **args=wordlist_to_arglist(a);
		builtins[index].func(args);
		for(index=0;args[index];index++)
			free(args[index]);
		free(args);
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
		if(a->flags&COM_SEMI){
			int status=0;
			int waitret=0;
			char strret[5];

			do{
				waitret=waitpid(pid,&status,0);
			}while(waitret!=pid && !(waitret==-1 && errno==ECHILD));

			if(WIFEXITED(status)){
				sprintf(strret,"%d",WEXITSTATUS(status));
				set_local("?",strret);
			}
			else if(WIFSIGNALED(status)){
				sprintf(strret,"%d",128+WTERMSIG(status));
				set_local("?",strret);
			}
			else{
				// What now?
			}
		}
		else{
			add_job(pid,a->args->word);
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

		add_job(pid,a->args->word);
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

	while((num=read(fd,buf+offset,bufsize-offset))!=0){
		if(num==-1){
			if(errno==EINTR)
				continue;
			else
				break;
		}
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
	if(num<0){
		fprintf(stderr,"Read error (%d)",errno);
	}
	if(offset>0){
		ptr[offset]=0;
		wp->word=strdup(ptr);
		wp->next=NULL;
	}
	else{
		if(ret->next){
			for(wp=ret;wp->next->next;wp=wp->next);
			free(wp->next);
			wp->next=NULL;
		}
		else{
			free(ret);
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

command_t* execute_for(command_t *start){
	command_t *tail;
	command_t *ptr=start->next;
	wordlist_t *var=start->args;
	wordlist_t *list=start->args->next;
	int i;
	int len;
	char *item;
	int copy=0;
	int orig_sigint=caught_sigint;

	while(ptr && ptr->next && !(ptr->next->flags&COM_ENDFOR)){
		ptr=ptr->next;
	}
	tail=ptr->next;
	ptr->next=NULL;

	substitute_variables(list);
	len=strlen(list->word);

	i=0;
	while(list->word[i] && (list->word[i]=='\n' || list->word[i]==' ' || list->word[i]=='\t'))i++;
	if(!list->word[i])return tail;
	item=list->word+i;

	for(i=0;i<len && orig_sigint==caught_sigint;i++){
		while(i<len && (list->word[i]=='\n' || list->word[i]==' ' || list->word[i]=='\t')){
			copy=1;
			list->word[i]=0;
			i++;
		}
		if(copy){
			copy=0;
			set_local(var->word,item);
			execute_commands(start->next);
			item=list->word+i;
		}
	}
	if(*item){
		set_local(var->word,item);
		execute_commands(start->next);
	}

	return tail;
}

command_t* execute_while(command_t *start){
	command_t *testc;
	command_t *runc;
	command_t *ptr;
	command_t *tail;
	char *testret;
	int orig_sigint=caught_sigint;

	testc=start->next;

	ptr=testc;
	while(ptr && !(ptr->flags&COM_SEMI))ptr=ptr->next;
	runc=ptr->next;
	ptr->next=NULL;

	ptr=runc;
	while(ptr->next && !(ptr->flags&COM_ENDWHILE))ptr=ptr->next;
	tail=ptr->next;
	ptr->next=NULL;

	while(orig_sigint==caught_sigint){
		execute_commands(testc);

		testret=get_local("?");
		if(testret==NULL || (testret[0]!='0' && testret[1]==0))
			break;
		
		execute_commands(runc);
	}

	ptr=testc;
	while(ptr->next)ptr=ptr->next;
	ptr->next=runc;
	while(ptr->next)ptr=ptr->next;
	ptr->next=tail;

	return ptr;

}

int successful_return(char *test){
	if(test==NULL)
		return 0;
	
	if(test[0]=='0' && test[1]==0)
		return 1;

	return 0;
}

command_t* execute_or(command_t *start){
	command_t *ptr=start;
	char *testret;

	start->flags=COM_DEFAULT|COM_SEMI;
	execute_simple(start);

	ptr=start;
	testret=get_local("?");
	if(successful_return(testret)){
		ptr=ptr->next;
		while(ptr->next && !(ptr->flags&COM_SEMI) && !(ptr->flags&COM_BG))
			ptr=ptr->next;
	}

	return ptr;
}

command_t* execute_and(command_t *start){
	command_t *ptr=start;
	char *testret;

	start->flags=COM_DEFAULT|COM_SEMI;
	execute_simple(start);

	ptr=start;
	testret=get_local("?");
	if(!successful_return(testret)){
		ptr=ptr->next;
		while(ptr->next && !(ptr->flags&COM_SEMI) && !(ptr->flags&COM_BG))
			ptr=ptr->next;
	}

	return ptr;
}

wordlist_t* create_command_sub(command_t **start){
	wordlist_t *ret=NULL;
	command_t *ptr=*start;
	int pipefd[2];
	int tmp_flag;

	if(!ptr->next)return NULL;
	
	ptr=ptr->next;
	do{
		switch(ptr->flags&COM_TERM_MASK){
			case COM_SUBST: // single command to substitute
			case COM_SEMI:
				pipe(pipefd);
				make_sub_redirect(ptr,pipefd[1]);
				tmp_flag=ptr->flags;
				ptr->flags=(COM_DEFAULT|COM_BG);
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
		if(!(ptr->flags&COM_SUBST))
			ptr=ptr->next;
		else
			break;
	}while(ptr);

	/* (cmd|null)`(cmd)(cmd)`(args|null); */
	/* (start)	   ...	(ptr) (ptr->next); */
	if(ptr){
		(*start)->next=ptr->next;
		//(*start)->flags=ptr->next->flags;
		(*start)->flags&=COM_BASE_MASK;
		(*start)->flags|=ptr->next->flags&COM_TERM_MASK;
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
		//fprintf(stderr,"EC|%d|%s|\n",ptr->flags,ptr->args?ptr->args->word:"");
		switch(ptr->flags&COM_TERM_MASK){
			//case COM_VAR:
				//execute_set_variables(ptr);
				//break;
			case COM_BG:
			case COM_SEMI:
			//case COM_DEFAULT:
				if(ptr->flags&COM_DEFAULT)
					execute_simple(ptr);
				else if(ptr->flags&COM_VAR)
					execute_set_variables(ptr);
				else if(ptr->flags&COM_FOR)
					ptr=execute_for(ptr);
				else if(ptr->flags&COM_WHILE)
					ptr=execute_while(ptr);
				break;
			case COM_AND:
				ptr=execute_and(ptr);
				break;
			case COM_OR:
				ptr=execute_or(ptr);
				break;
			case COM_PIPE:
				create_pipe(ptr);
				break;
			case COM_SUBST:
				temp_c=ptr;
				temp_wl=create_command_sub(&ptr);
				if(temp_c->flags&COM_DEFAULT){
					append_wordlist(temp_c->args,temp_wl);
					append_wordlist(temp_c->args,ptr->next->args);
					temp_c->redirection=ptr->next->redirection; // Append instead?
					execute_simple(temp_c);
				}
				else if(temp_c->flags&COM_VAR){
					concat_wordlist(temp_c->args,temp_wl);
					append_wordlist(temp_c->args,ptr->next->args);
					execute_set_variables(temp_c);
				}
				ptr=ptr->next;
				break;
		}
		ptr=ptr->next;
	}
	//free_commands(start);
}
