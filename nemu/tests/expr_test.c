#include "sys/types.h"
#include "unistd.h"
#include <check.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <addrexp.h>
#include <addrexp_lex.h>
#include <assert.h>
#include <time.h>

char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
const int buf_start_pos = 0;
char *buf_ptr = buf + buf_start_pos;
static char *code_format =
"#include <stdio.h>\n"
"#include <stdint.h>\n"
"int main() { "
"  uint32_t result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";


void gen(char c) {
  *(buf_ptr++) = c;
}

void gen_num(void) {
  uint32_t num = rand() % 100;
  int len = 0;
  switch(rand() % 2) {
    case 0:
      len = snprintf(buf_ptr, 100, "%u", num);
      break;
    case 1:
      len = snprintf(buf_ptr, 100, "0x%x", num);
      break;
    default: assert(0);
  }
  buf_ptr += len;
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
  if (buf_ptr - buf > 2000) {
    choice = 0;
  } 
  switch (choice) {
      case 0: gen_num(); break;
      case 1: gen('('); gen_rand_expr(); gen(')'); break;
      default: gen_rand_expr(); gen(' '); gen_rand_op(); gen(' '); gen_rand_expr(); break;
  }
}

START_TEST(test_expr_random_100) {
  srand(time(0) + _i * 100);
  gen_rand_expr();
  yy_scan_string(buf + buf_start_pos);
  uint32_t addr;
  ck_assert(!yyparse(&addr));
  yylex_destroy();

  sprintf(code_buf, code_format, buf);

  FILE *fp = fopen("/tmp/.code.c", "w");
  ck_assert(fp != NULL);
  fputs(code_buf, fp);
  fclose(fp);

  int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
  ck_assert_msg(!ret, "system ret: %d, error: %s", ret, strerror(ret));

  fp = popen("/tmp/.expr", "r");
  ck_assert(fp != NULL);

  uint32_t reference;
  ret = fscanf(fp, "%u", &reference);
  ck_assert(ret == 1);
  pclose(fp);
  // fprintf(stderr, "\n\tbuf = %s\n\taddr = %u, reference = %u", buf, addr, reference);

  ck_assert_msg(addr == reference, "\n\tbuf = %s\n\taddr = %u, reference = %u\n", buf, addr, reference);

  while(buf_ptr != buf + buf_start_pos) {
      *(--buf_ptr) = '\0';
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

    tcase_add_loop_test(tc_core, test_expr_random_100, 0, 100);
    tcase_add_test(tc_core, test_test2);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void) {
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = expr_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
