#include "macro.h"
#include "sys/types.h"
#include <unistd.h>
#include <assert.h>
#include <check.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <addrexp.h>
#include <addrexp_lex.h>
#include <isa.h>
#include <reg.h>

char buf[65536] = {}, ref_buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
const int buf_start_pos = 0;
char *buf_ptr = buf + buf_start_pos, *ref_buf_ptr = ref_buf;
static char *code_format = "#include <stdio.h>\n"
                           "#include <stdint.h>\n"
                           "int main() { "
                           "  uint32_t result = %s; "
                           "  printf(\"%%u\", result); "
                           "  return 0; "
                           "}";

void gen(char c) {
  *(buf_ptr++) = c;
  *(ref_buf_ptr++) = c;
}

void gen_num(void) {
  uint32_t num = rand();
  int len = 0, ref_len = 0;
  switch (rand() % 3) {
  case 0:
    len = snprintf(buf_ptr, 100, "%u", num);
    ref_len = snprintf(ref_buf_ptr, 100, "%uU", num);
    break;
  case 1:
    len = snprintf(buf_ptr, 100, "0x%x", num);
    ref_len = snprintf(ref_buf_ptr, 100, "%uU", num);
    break;
  case 2:
    len = snprintf(buf_ptr, 100, "%d", num);
    ref_len = snprintf(ref_buf_ptr, 100, "%d", num);
    break;
  default:
    assert(0);
  }
  buf_ptr += len;
  ref_buf_ptr += ref_len;
}

void gen_rand_op(void) {
  switch (rand() % 4) {
  case 0:
    gen('+');
    break;
  case 1:
    gen('-');
    break;
  case 2:
    gen('*');
    break;
  case 3:
    gen('/');
    break;
  }
}

void gen_rand_expr(void) {
  int choice = rand() % 3;
  if (buf_ptr - buf > 2000) {
    choice = 0;
  }
  switch (choice) {
  case 0:
    gen_num();
    break;
  case 1:
    gen('(');
    gen_rand_expr();
    gen(')');
    break;
  default:
    gen_rand_expr();
    gen(' ');
    gen_rand_op();
    gen(' ');
    gen_rand_expr();
    break;
  }
}

START_TEST(test_expr_random_100) {
  srand(time(0) + _i * 100);
  gen_rand_expr();

  sprintf(code_buf, code_format, ref_buf);

  FILE *fp = fopen("/tmp/.code.c", "w");
  ck_assert(fp != NULL);
  fputs(code_buf, fp);
  fclose(fp);

  int ret =
      system("gcc /tmp/.code.c -Werror=div-by-zero -o /tmp/.expr 2>/dev/null");
  if (ret == 256) {
    // Probably devide by zero. Skip
    goto clean_up;
  }
  ck_assert_msg(!ret, "system ret: %d, error: %s", ret, strerror(ret));

  fp = popen("/tmp/.expr", "r");
  ck_assert(fp != NULL);

  uint32_t reference;
  ret = fscanf(fp, "%u", &reference);
  ck_assert(ret == 1);
  pclose(fp);
  //   fprintf(stderr, "\n\tbuf = %s\n\taddr = %u, reference = %u", buf, addr,
  //   reference);

  yy_scan_string(buf + buf_start_pos);
  uint32_t addr;
  ck_assert(!yyparse(&addr));
  yylex_destroy();

  ck_assert_msg(addr == reference,
                "\n\tbuf = %s\n\t(addr = %u) != (reference = %u)\n", buf, addr,
                reference);

clean_up:
  while (buf_ptr != buf + buf_start_pos) {
    *(--buf_ptr) = '\0';
  }
  while (ref_buf_ptr != ref_buf) {
    *(--ref_buf_ptr) = '\0';
  }
}
END_TEST

struct {
  const char *expr;
  uint32_t reference;
} exprs[] = {
    {"-1", 0xFFFFFFFFU},
    {"-0x1", 0xFFFFFFFFU},
    {"0--1", 0x1},
    {"0--0x1", 0x1},
}, reg_exprs[] = {
    {"$ra", 0x0},
};
START_TEST(test_expr_negative_operand) {
  yy_scan_string(exprs[_i].expr);
  uint32_t addr;
  ck_assert(!yyparse(&addr));
  yylex_destroy();

  ck_assert_msg(addr == exprs[_i].reference,
                "\n\texpr = %s\n\t(addr = %u) != (reference = %u)\n", exprs[_i].expr,
                addr, exprs[_i].reference);
}
END_TEST

extern const char *regs[];
START_TEST(test_expr_plain_register) {
  int i, j, result;
  char buf[50] = {};
  uint32_t value;
  // NOTE: need to fix this if want to support more arch
  buf[0] = '$';
  for (i = 1; i < 32; i++) {
    ck_assert(strcpy(buf + 1, regs[i]));
    gpr(i) = i;
    yy_scan_string(buf);
    result = yyparse(&value);
    ck_assert_msg(result == 0, "expr = %s\n", buf);
    yylex_destroy();

    ck_assert(value == i);
    for (j = 1; j < 10; j++) {
      buf[i] = '\0';
    }
  }

}
END_TEST

START_TEST(test_expr_register) {
  yy_scan_string(reg_exprs[_i].expr);
  uint32_t value;
  ck_assert(!yyparse(&value));
  yylex_destroy();

  ck_assert_msg(value == reg_exprs[_i].reference,
                "\n\texpr = %s\n\t(addr = %u) != (reference = %u)\n", reg_exprs[_i].expr,
                value, reg_exprs[_i].reference);
}
END_TEST

Suite *expr_suite(void) {
  Suite *s;
  TCase *tc_core;

  s = suite_create("Expr test");
  tc_core = tcase_create("Core");

  tcase_add_loop_test(tc_core, test_expr_random_100, 0, 20);
  tcase_add_loop_test(tc_core, test_expr_negative_operand, 0,
                      sizeof(exprs) / sizeof(exprs[0]));
  tcase_add_loop_test(tc_core, test_expr_register, 0,
                      sizeof(reg_exprs) / sizeof(reg_exprs[0]));
  tcase_add_test(tc_core, test_expr_plain_register);
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
