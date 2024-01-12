#include "sys/types.h"
#include <check.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <addrexp.h>
#include <addrexp_lex.h>
#include <assert.h>
#include <time.h>

char buf[65536] = "python3 -c print(";
char *buf_ptr = buf + 17;

void gen(char c) {
  *(buf_ptr++) = c;
}

void gen_num(void) {
  int len = rand() % 8 + 1;
  int base = 10;
  switch(rand() % 2) {
    case 0: base = 10; break;
    case 1: base = 16; break;
    default: assert(0);
  }
  const char *strmap = "0123456789abcdef";
  // TODO: add minus
  if (base == 16) {
    gen('0');
    gen('x');
  }
  while(len--) {
    gen(strmap[rand() % base]);
  }
}

void gen_rand_op(void) {
  switch(rand() % 4) {
    case 0: gen('+'); break;
    case 1: gen('-'); break;
    case 2: gen('*'); break;
    case 3: gen('/'); break;
  }
}

void gen_rand_expr(void) {
  int choice = rand() % 3;
  switch (choice) {
      case 0: gen_num(); break;
      case 1: gen('('); gen_rand_expr(); gen(')'); break;
      default: gen_rand_expr(); gen_rand_op(); gen_rand_expr(); break;
  }
  printf("buf: %s\n", buf);
}

START_TEST(test_test1) {
  for (int i = 0; i < 10; i++) {
    gen_rand_expr();
    yy_scan_string(buf + 18);
    uint32_t addr;
    ck_assert(!yyparse(&addr));
    yylex_destroy();

     /* Open the command for reading. */
    FILE *fp;
    *(buf_ptr++) = ')';
    fp = popen(buf, "r");
    ck_assert(fp != NULL);

    /* Read the output a line at a time - output it. */
    uint32_t reference = 0;
    ck_assert(fscanf(fp, "%u", &reference) == 0);
    ck_assert(addr == reference);

    while(buf_ptr != buf + 18) {
        *(--buf_ptr) = '\0';
    }
  }
} END_TEST

START_TEST(test_test2) {
    ;
} END_TEST

Suite *expr_suite(void) {
    Suite *s;
    TCase *tc_core;

    s = suite_create("Expr test");
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_test1);
    tcase_add_test(tc_core, test_test2);
    suite_add_tcase(s, tc_core);

    return s;
}

int test_main(void) {
    int number_failed;
    Suite *s;
    SRunner *sr;
    srand(time(0));

    s = expr_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

