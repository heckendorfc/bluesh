#include <check.h>
#include <check_shell.h>
#include <variable.h>
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

Suite* variable_suite(){
	Suite *s = suite_create("Variable");
	TCase *tc_core = tcase_create("Core");

	tcase_add_test(tc_core,test_split_colons);

	suite_add_tcase(s,tc_core);

	return s;
}
