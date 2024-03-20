#ifndef __FUNC_DEF_H__
#define __FUNC_DEF_H__
#include <common.h>

#ifdef CONFIG_FTRACE
typedef struct {
  vaddr_t start;
  vaddr_t len;
  char * name;
} func_t;

extern func_t *func_table;
void ftrace_call(vaddr_t);
void ftrace_return(vaddr_t);
// const char *get_func_name(vaddr_t addr);
#endif

#endif