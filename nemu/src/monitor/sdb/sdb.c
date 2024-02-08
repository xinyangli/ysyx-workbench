/***************************************************************************************
 * Copyright (c) 2014-2022 Zihao Yu, Nanjing University
 *
 * NEMU is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan
 *PSL v2. You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 *KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 *NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 * See the Mulan PSL v2 for more details.
 ***************************************************************************************/

#include "sdb.h"
#include "common.h"
#include "sys/types.h"
#include <addrexp.h>
#include <addrexp_lex.h>
#include <cpu/cpu.h>
#include <errno.h>
#include <isa.h>
#include <memory/vaddr.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <stdint.h>

static int is_batch_mode = false;

// command handlers
static int cmd_help(char *args);
static int cmd_c(char *args);
static int cmd_q(char *args);
static int cmd_w(char *args);
static int cmd_x(char *args);
static int cmd_si(char *args);
static int cmd_info(char *args);
static int cmd_info_r(char *args);
static int cmd_info_w(char *args);

static struct CommandTable {
  const char *name;
  const char *description;
  int (*handler)(char *);
  struct CommandTable *subcommand;
  int nr_subcommand;
} cmd_info_table[] =
    {
        {"r", "List all registers and their contents", cmd_info_r, NULL, 0},
        {"w", "Status of specified watchpoints", cmd_info_w, NULL, 0},
},
  cmd_table[] = {
      {"help", "Display information about all supported commands", cmd_help,
       NULL, 0},
      {"c", "Continue the execution of the program", cmd_c, NULL, 0},
      {"q", "Exit NEMU", cmd_q, NULL, 0},
      {"x", "Examine content of physical memory address", cmd_x, NULL, 0},
      {"w", "Break when expression is changed", cmd_w, NULL, 0},
      {"si", "Execute next [n] program line", cmd_si, NULL, 0},
      {"info", "Print information of registers or watchpoints", cmd_info,
       cmd_info_table, ARRLEN(cmd_info_table)},
};

#define NR_CMD ARRLEN(cmd_table)

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin.
 */
static char *rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("\e[1;34m(nemu)\e[0m ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

/* Extract Integer from a string. Can handle hex, binary and decimal numbers.
 * Print error if meet any error.
 * Return `UINTMAX_MAX` if the string is invalid or number exceed the limit of
 * uint.
 */
static word_t parse_uint(const char *arg, bool *success) {
  if (arg == NULL) {
    puts("Invalid uint argument.");
    *success = false;
    return 0;
  }
  int base = 10;
  int token_length = strnlen(arg, 34);
  if (token_length > 2) {
    if (arg[0] == '0' && (arg[1] == 'b' || arg[1] == 'B')) {
      base = 2;
      arg = arg + 2;
    } else if (arg[0] == '0' && (arg[1] == 'x' || arg[1] == 'X')) {
      base = 16;
      arg = arg + 2;
    }
  }
  char *endptr;
  uintmax_t n = strtoumax(arg, &endptr, base);
  if (errno == ERANGE || n > WORD_T_MAX) {
    printf("%s exceed the limit of uint\n", arg);
    *success = false;
    return 0;
  } else if (arg == endptr) {
    puts("Invalid uint argument.");
    *success = false;
    return 0;
  } else if (n > WORD_T_MAX) {
    *success = false;
    return WORD_T_MAX;
  } else {
    *success = true;
    return n;
  }
}

word_t parse_expr(const char *arg, bool *success) {
  if (arg == NULL) {
    puts("Invalid expr argument.");
    *success = false;
    return 0;
  } else {
    word_t res;
    yy_scan_string(arg);
    *success = !yyparse(&res);
    yylex_destroy();
    return res;
  }
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  nemu_state.state = NEMU_QUIT;
  return -1;
}

/* Single stepping
 * <step>: execute <step> step
 */
static int cmd_si(char *args) {
  char *arg = strtok(NULL, " ");
  if (arg == NULL) {
    cpu_exec(1);
  } else {
    bool res = false;
    word_t n = parse_uint(arg, &res);
    if (!res)
      goto wrong_usage;
    cpu_exec(n);
  }
  return 0;

wrong_usage:
  printf("Invalid argument for command si: %s\n", args);
  printf("Usage: si [N: uint]\n");
  return 0;
}

static int cmd_info_r(char *args) {
  isa_reg_display();
  return 0;
}

static int cmd_info_w(char *args) {
  printf("Not implemented");
  return 0;
}

static int cmd_w(char *args) {
  char *expr = strtok(NULL, " ");
  wp_add(expr);
  return 0;
}

static int cmd_x(char *args) {
  char *arg = strtok(NULL, " ");
  bool res = false;
  word_t n = parse_uint(arg, &res);
  if (!res)
    goto wrong_usage;
  // No deliminter here, just pass all the remain argument to `parse_expr()`
  arg = strtok(NULL, "");
  word_t start_addr = parse_expr(arg, &res);
  if (!res)
    goto wrong_usage;
  start_addr = start_addr & ~(WORD_BYTES - 1);
  for (vaddr_t vaddr = start_addr; vaddr < start_addr + n; vaddr += WORD_BYTES) {
    word_t value = vaddr_read(vaddr, WORD_BYTES);
    printf("\e[1;34m" FMT_PADDR "\e[0m"
           "  " FMT_WORD "\n",
           vaddr, value);
  }
  return 0;

wrong_usage:
  printf("Invalid argument for command x: %s\n", arg);
  printf("Usage: x [N: uint] [EXPR: <expr>]\n");
  return 0;
}

static int cmd_info(char *args) {
  char *arg = strtok(NULL, " ");
  int i;
  if (arg == NULL) {
    goto wrong_usage;
    return 0;
  }
  for (i = 0; i < ARRLEN(cmd_info_table); i++) {
    if (strcmp(arg, cmd_info_table[i].name) == 0) {
      cmd_info_table[i].handler(args);
      return 0;
    }
  }

wrong_usage:
  printf("Invalid argument for command info: %s\n", args);
  printf("Usage: info [r | w]\n");
  return 0;
}

static int cmd_help_print(char *args, struct CommandTable *cur_cmd_table,
                          int cur_nr_cmd) {
  int i;
  char *arg = strtok(NULL, " ");
  if (arg == NULL) {
    return -1;
  } else {
    for (i = 0; i < cur_nr_cmd; i++) {
      if (strcmp(arg, cur_cmd_table[i].name) == 0) {
        printf("%s ", cur_cmd_table[i].name);
        if (cmd_help_print(arg, cur_cmd_table[i].subcommand,
                           cur_cmd_table[i].nr_subcommand) == -1) {
          printf("-- %s\n", cur_cmd_table[i].description);
        }
        return 0;
      }
    }
    return -1;
  }
}

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i++) {
      printf("%s -- %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  } else {
    for (i = 0; i < NR_CMD; i++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s ", cmd_table[i].name);
        if (cmd_help_print(args, cmd_table[i].subcommand,
                           cmd_table[i].nr_subcommand) == -1) {
          printf("-- %s\n", cmd_table[i].description);
          // Print available subcommands
          for (int j = 0; j < cmd_table[i].nr_subcommand; j++) {
            struct CommandTable *sub_cmd_table = cmd_table[i].subcommand;
            printf("  > %s -- %s\n", sub_cmd_table[j].name,
                   sub_cmd_table[j].description);
          }
        }
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() { is_batch_mode = true; }

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL;) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) {
      continue;
    }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) {
          return;
        }
        break;
      }
    }

    if (i == NR_CMD) {
      printf("Unknown command '%s'\n", cmd);
    }
  }
}

void init_sdb() {
  // /* Compile the regular expressions. */
  // init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
