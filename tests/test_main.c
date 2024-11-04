#include <check.h>
#include <stdlib.h>
#include "test_parsing.h"

int main(void)
{
   int number_failed;
   Suite *s, *s2;
   SRunner *sr;

   s = parsing_suite();
   s2 = parsing_suite();
   sr = srunner_create(s);
   srunner_add_suite(sr, s2);

   srunner_run_all(sr, CK_NORMAL);
   number_failed = srunner_ntests_failed(sr);
   srunner_free(sr);
   return (number_failed == 0) ? 0 : 1;
}