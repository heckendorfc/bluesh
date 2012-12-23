#include <check.h>
#include <check_shell.h>
#include <lex.h>
#include <lex_dfa.h>
#include <shell.h>

START_TEST(test_split_one){
	TokenList token;
	TokenList *tok=&token;
	int split;
	tok->token.word=strdup("cmd>one");
	tok->token.type=TOK_OPERATOR;
	tok->next=NULL;
	split=split_token(tok,3,4);
	fail_unless(split==(SPLIT_BEFORE|SPLIT_AFTER));
	fail_unless(strcmp(tok->token.word,"cmd")==0);
	fail_unless(strcmp(tok->next->token.word,">")==0);
	fail_unless(strcmp(tok->next->next->token.word,"one")==0);
	fail_unless(tok->token.type==TOK_NULL && tok->next->token.type==TOK_OPERATOR && tok->next->next->token.type==TOK_NULL);
	free(tok->token.word);
}END_TEST

START_TEST(test_split_one_nofront){
	TokenList token;
	TokenList *tok=&token;
	int split;
	tok->token.word=strdup(">one");
	tok->token.type=TOK_OPERATOR;
	tok->next=NULL;
	split=split_token(tok,0,1);
	fail_unless(split==(SPLIT_AFTER));
	fail_unless(strcmp(tok->token.word,">")==0);
	fail_unless(strcmp(tok->next->token.word,"one")==0);
	fail_unless(tok->token.type==TOK_OPERATOR && tok->next->token.type==TOK_NULL);
	free(tok->token.word);
}END_TEST

START_TEST(test_split_one_noback){
	TokenList token;
	TokenList *tok=&token;
	int split;
	tok->token.word=strdup("cmd>");
	tok->token.type=TOK_OPERATOR;
	tok->next=NULL;
	split=split_token(tok,3,4);
	fail_unless(split==(SPLIT_BEFORE));
	fail_unless(strcmp(tok->token.word,"cmd")==0);
	fail_unless(strcmp(tok->next->token.word,">")==0);
	fail_unless(tok->token.type==TOK_NULL && tok->next->token.type==TOK_OPERATOR);
	free(tok->token.word);
}END_TEST

START_TEST(test_split_two){
	TokenList token;
	TokenList *tok=&token;
	int split;
	tok->token.word=strdup("cmd>>one");
	tok->token.type=TOK_OPERATOR;
	tok->next=NULL;
	split=split_token(tok,3,5);
	fail_unless(split==(SPLIT_BEFORE|SPLIT_AFTER));
	fail_unless(strcmp(tok->token.word,"cmd")==0);
	fail_unless(strcmp(tok->next->token.word,">>")==0);
	fail_unless(strcmp(tok->next->next->token.word,"one")==0);
	fail_unless(tok->token.type==TOK_NULL && tok->next->token.type==TOK_OPERATOR && tok->next->next->token.type==TOK_NULL);
	free(tok->token.word);
}END_TEST

START_TEST(test_identify_double_op){
	State *dfa = generate_operator_dfa();
	TokenList token;
	TokenList *tok=&token;
	tok->token.word=strdup("cmd>>one<<tw\\|o");
	//tok->token.word=strdup("cmd>>one");
	tok->token.type=TOK_NULL;
	tok->next=NULL;
	identify(tok,dfa);
	//fail_unless(split==(SPLIT_BEFORE|SPLIT_AFTER));
	//fprintf(stderr,"WORDS:\n%s\n%s\n%s\n%s\n%s\n",	tok->token.word,
												//tok->next->token.word,
												//tok->next->next->token.word,
												//tok->next->next->next->token.word,
												//tok->next->next->next->next->token.word);
	fail_unless(strcmp(tok->token.word,"cmd")==0);
	fail_unless(strcmp(tok->next->token.word,">>")==0);
	fail_unless(strcmp(tok->next->next->token.word,"one")==0);
	fail_unless(strcmp(tok->next->next->next->token.word,"<<")==0);
	fail_unless(strcmp(tok->next->next->next->next->token.word,"tw\\|o")==0);
	fail_unless(tok->token.type==TOK_NULL && tok->next->token.type==(TOK_REDIRECT|TOK_GTGT) && 
				tok->next->next->token.type==TOK_NULL && tok->next->next->next->token.type==(TOK_REDIRECT|TOK_LTLT) &&
				tok->next->next->next->next->token.type==TOK_NULL);
	free(tok->token.word);
}END_TEST

START_TEST(test_identify_consecutive_op){
	State *dfa = generate_operator_dfa();
	TokenList token;
	TokenList *tok=&token;
	tok->token.word=strdup("cmd>>>two");
	tok->token.type=TOK_NULL;
	tok->next=NULL;
	identify(tok,dfa);
	fail_unless(strcmp(tok->token.word,"cmd")==0);
	fail_unless(strcmp(tok->next->token.word,">>")==0);
	fail_unless(strcmp(tok->next->next->token.word,">")==0);
	fail_unless(strcmp(tok->next->next->next->token.word,"two")==0);
	fail_unless(tok->token.type==TOK_NULL && tok->next->token.type==(TOK_REDIRECT|TOK_GTGT) && 
				tok->next->next->token.type==(TOK_REDIRECT|TOK_GT) && tok->next->next->next->token.type==TOK_NULL);
	free(tok->token.word);
}END_TEST

START_TEST(test_strip_backslash){
	Token token;
	token.word=strdup("one\\two\\\\three\\");
	strip_backslash(&token);
	fail_unless(strcmp(token.word,"onetwo\\three")==0);
	free(token.word);
}END_TEST

START_TEST(test_create_tokens){
	TokenList token;
	TokenList *tok=&token;
	char arg[]="one\"'te\\xt'\"two";
	tok->next=create_tokens(arg);
	fail_unless(tok->next->token.type==TOK_NULL &&
				tok->next->next->token.type==TOK_QUOTE &&
				tok->next->next->next->token.type==TOK_QUOTE_STR &&
				tok->next->next->next->next->token.type==TOK_QUOTE &&
				tok->next->next->next->next->next->token.type==TOK_NULL);
}END_TEST

START_TEST(test_lex){
	TokenList *tok=lex("*>'text'");
	fail_unless(tok->next->token.type==TOK_TEXT &&
				tok->next->next->token.type==(TOK_REDIRECT|TOK_GT) &&
				tok->next->next->next->token.type==TOK_WHITESPACE &&
				tok->next->next->next->next->token.type==TOK_QUOTE &&
				tok->next->next->next->next->next->token.type==TOK_QUOTE_STR &&
				tok->next->next->next->next->next->next->token.type==TOK_QUOTE);
}END_TEST

Suite* lex_suite(){
	Suite *s = suite_create("Lex");
	TCase *tc_core = tcase_create("Core");

	tcase_add_test(tc_core,test_split_one);
	tcase_add_test(tc_core,test_split_two);
	tcase_add_test(tc_core,test_split_one_nofront);
	tcase_add_test(tc_core,test_split_one_noback);

	tcase_add_test(tc_core,test_identify_double_op);
	tcase_add_test(tc_core,test_identify_consecutive_op);

	tcase_add_test(tc_core,test_strip_backslash);

	tcase_add_test(tc_core,test_create_tokens);

	tcase_add_test(tc_core,test_lex);

	suite_add_tcase(s,tc_core);

	return s;
}
