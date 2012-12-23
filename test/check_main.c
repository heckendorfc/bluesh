#include <stdio.h>
#include <stdlib.h>
#include <check.h>
#include <check_shell.h>

static Suite* master_suite(){
	Suite *s=suite_create("Master");
	return s;
}

int main(){
	int number_failed;
	SRunner *sr = srunner_create(master_suite());

	srunner_add_suite(sr,lex_suite());
	srunner_add_suite(sr,builtin_suite());

	srunner_set_log(sr,"run.log");
	srunner_set_fork_status (sr,CK_NOFORK);
	srunner_run_all (sr, CK_NORMAL);
	number_failed = srunner_ntests_failed (sr);
	srunner_free(sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
