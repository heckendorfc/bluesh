#include <check.h>
#include <check_shell.h>
#include <builtin.h>
#include <shell.h>

START_TEST(test_split_colons){
	char *ptr=strdup("one:two:::three");
	char **ret=split_colons(ptr);
	fail_unless(strcmp(ret[0],"one")==0 &&
				strcmp(ret[1],"two")==0 &&
				strcmp(ret[2],"three")==0 &&
				ret[3]==NULL);
	free(ptr);
	free(ret);
}END_TEST

START_TEST(test_simplify_path){
	char *ptr=strdup("/one/two/../../one/./twoo///three/../four/");
	simplify_path(ptr);
	fail_unless(strcmp(ptr,"/one/twoo/four")==0);
	free(ptr);
}END_TEST

START_TEST(test_make_path){
	char *a=strdup("/one");
	char *b=strdup("end");
	char *ret=make_path(a,b);
	fail_unless(strcmp(ret,"/one/end/")==0);
	free(ret);
	free(a);
	free(b);
}END_TEST

START_TEST(test_make_path_slashes){
	char *a=strdup("/one/");
	char *b=strdup("end/");
	char *ret=make_path(a,b);
	fail_unless(strcmp(ret,"/one/end/")==0);
	free(ret);
	free(a);
	free(b);
}END_TEST

Suite* builtin_suite(){
	Suite *s = suite_create("Builtin");
	TCase *tc_core = tcase_create("Core");

	tcase_add_test(tc_core,test_split_colons);

	tcase_add_test(tc_core,test_simplify_path);

	tcase_add_test(tc_core,test_make_path);
	tcase_add_test(tc_core,test_make_path_slashes);

	suite_add_tcase(s,tc_core);

	return s;
}
