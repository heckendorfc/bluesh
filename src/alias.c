#include <alias.h>
#include <shell.h>
#include <lex.h>
#include <variable.h>

TokenList* next_start(TokenList *ptr){
	while(ptr){
		if(ptr->token.type&TOK_OPERATOR)
			return ptr;
		ptr=ptr->next;
	}
	return NULL;
}

char* next_aliased(TokenList **list){
	TokenList *ptr=*list;
	char *val=NULL;

	while(ptr->next){
		if(ptr->next->token.type&TOK_TEXT && is_alias(ptr->next->token.word)){
			val=get_alias(ptr->next->token.word);
			break;
		}
		ptr=next_start(ptr->next);
	}

	*list=ptr;

	return val;
}

void replace_alias(TokenList **list){
	TokenList *end,*ptr=*list;
	char *val;

	while(ptr){
		val=next_aliased(&ptr);
		if(!ptr || !ptr->next || !val)
			break;
		end=ptr->next->next;
		ptr->next->next=NULL;
		free_tokens(ptr->next);
		ptr->next=lex(val);
		if(!ptr->next){
			ptr->next=end;
			break;
		}
		else if(ptr->next->token.type==TOK_NULL){
			TokenList *temp=ptr->next->next;
			ptr->next->next=NULL;
			free_tokens(ptr->next);
			ptr->next=temp;
		}
		while(ptr->next)ptr=ptr->next;
		ptr->next=end;
		ptr=next_start(end);
	}
}
