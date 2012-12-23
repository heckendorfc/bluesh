#include <builtin.h>
#include <build.h>
#include <shell.h>
#include <variable.h>

int cmd_cd(wordlist_t*);

builtin_t builtins[]={
	{"cd",cmd_cd},
	{NULL,NULL}
};

char** split_colons(char *str){
	int i;
	int count=0;
	char **ret;
	char *start;

	for(i=0;str[i];i++)if(str[i]==':')count++;

	INIT_MEM(ret,count+1);

	start=str;
	for(i=0;str[i];i++){
		if(str[i]==':'){
			str[i]=0;
			if(*start)
				ret[count++]=start;
			i++;
			start=str+i;
		}
	}
	ret[count]=NULL;
	return ret;
}

void builtin_error(const char *str){
	fprintf(stderr,"%s\n",str);
}

void simplify_path(char *path){
	char *p,*q;
	int pathlen=strlen(path);

	p=path;
	while(*p){
		if(p[0]=='.' && p[1]=='/'){
			memmove(p,p+2,pathlen-((p-path)-2));
			pathlen-=2;
			path[pathlen]=0;
		}
		else if(p[0]=='.' && p[1]=='.' && p[2]=='/'){
			q=p-2;
			while(q>path && *q!='/')q--;
			pathlen-=(p-q)+2;
			memmove(q,p+2,pathlen-(q-path));
			path[pathlen]=0;
			p=q;
		}
		else if(p[0]=='/' && p[1]=='/'){
			q=p+1;
			while(*q=='/')q++;
			pathlen-=(q-p)-1;
			memmove(p+1,q,pathlen-(p-path));
			path[pathlen]=0;
		}
		else
			p++;
	}

	if(pathlen>2 && path[pathlen-1]=='/')
		path[pathlen-1]=0;
}

int change_pwd(char *dir){
	int ret;

	if(dir==NULL){
		builtin_error("No such file or directory.");
		return 1;
	}

	ret = chdir(dir);
	if(ret!=0){
		return 1;
	}

	simplify_path(dir);

	set_variable("OLDPWD",get_variable("PWD"));
	set_variable("PWD",dir);

	return 0;
}

char* make_path(const char *path, const char *rel){
	char *ret;
	int pathlen=strlen(path);
	int rellen=strlen(rel);
	int retsize=pathlen+rellen;
	int relstart=0;

	if(rel[0]=='.' && rel[1]=='/'){
		retsize-=2;
		relstart+=2;
	}

	if(path[pathlen-1]!='/')
		retsize++;
	if(rel[rellen-1]!='/')
		retsize++;

	INIT_MEM(ret,retsize+1);

	memcpy(ret,path,pathlen);

	if(path[pathlen-1]!='/'){
		pathlen++;
		ret[pathlen-1]='/';
	}

	memcpy(ret+pathlen,rel+relstart,rellen-relstart);

	if(ret[retsize-1]!='/')
		ret[retsize-1]='/';

	ret[retsize]=0;

	return ret;
}

int set_rel_path(const char *rel){
	char *cdpath;
	char *pwd;
	char *path;
	char **paths;
	char *ptr;
	int ret;

	cdpath=get_variable("CDPATH");
	if(cdpath==NULL){
		// use pwd
		pwd=get_variable("PWD");
		path=make_path(pwd,rel);
		ret=change_pwd(path);
		free(path);
		return ret;
	}
	else{
		cdpath=strdup(cdpath);
		paths=split_colons(cdpath);
		for(ptr=*paths;ptr;ptr++){
			path=make_path(ptr,rel);
			ret=change_pwd(path);
			free(path);
			if(ret==0)
				break;
		}
		free(paths);
		free(cdpath);
		return ret;
	}

}

int cmd_cd(wordlist_t *arg){
	int ret=0;
	char *str=NULL;
	char *ptr;

	if(arg->next==NULL){
		// go home
		str=get_variable("HOME");
		if(!str)
			return 1;

		ret=change_pwd(str);
	}
	else if(arg->next->next==NULL){
		// single dir
		if(arg->next->word[0]=='-' && arg->next->word[1]==0){
			// go to $OLDPWD
			str=get_variable("OLDPWD");
			ptr=strdup(str);
			if(!str){
				builtin_error("No previous directory.");
				return 1;
			}
			if(!ptr)return 1;
			ret=change_pwd(ptr);
			free(ptr);
			if(ret==0)
				printf("%s\n",ptr);
		}
		else{
			if(arg->next->word[0]!='/'){
				ret=set_rel_path(arg->next->word);
			}
			else{
				ret=change_pwd(arg->next->word);
			}
		}
	}
	else if(arg->next->next){
		// two arguments. what does it mean?
		return 1;
	}

	if(ret!=0)
		builtin_error("An error occured while changing directory.");

	return ret;
}
