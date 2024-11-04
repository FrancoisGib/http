#include "test_parsing.h"

START_TEST(test)
{
   ck_assert_int_eq(4, 5);
}
END_TEST

Suite *parsing_suite(void)
{
   Suite *s;
   TCase *tc_core;

   s = suite_create("Parsing");
   tc_core = tcase_create("First");

   tcase_add_test(tc_core, test);
   suite_add_tcase(s, tc_core);

   return s;
}