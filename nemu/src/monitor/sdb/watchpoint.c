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
#include <common.h>
#include <stdio.h>

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  word_t val;
  char *expr;
} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *tail = NULL, *free_ = NULL;
static int wp_count = 0;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

static WP *wp_new() {
  if (free_ == NULL) {
    Error("wp_pool: Watchpoint pool not initialized or is full.");
    return NULL;
  }
  
  WP *ret = free_;
  free_ = free_->next;

  ret->NO = 0;
  ret->next = NULL;
  return ret;
}

static void wp_delete(WP *wp) {
  assert(wp);
  wp->next = free_;
  free_ = wp;
}

int wp_add(char * expr) {
  WP *wp = wp_new();
  if (wp == NULL) {
    Error("watchpoint: Failed to add watchpoint, pool is full.");
    return 1;
  }

  wp->NO = wp_count++;
  if (tail == NULL) {
    head = wp;
    tail = wp;
  } else {
    tail->next = wp;
    tail = wp;
  }
  int len = strlen(expr);
  wp->expr = malloc(len * sizeof(char));
  if (wp->expr == NULL) {
    Error("Failed to allocate memory for expression");
    return 1;
  }
  strncpy(wp->expr, expr, len - 1);
  return 0;
}

int wp_remove_by_number(int number) {
  WP *target_prev;
  // Find previous node of target number
  for (target_prev = head; target_prev != NULL && target_prev->next->NO != number; target_prev = target_prev->next) ;
  if (target_prev == NULL) {
    Error("Watchpoint not found, you can check current watchpoints with `info w`");
    return 1;
  }
  WP *target = target_prev->next;
  target_prev->next = target->next;
  if (target == head) {
    head = target->next;
  } else if (target == tail) {
    tail = target_prev;
  }
  wp_delete(target);
  return 0;
}

static bool wp_check_change(WP* wp) {
  bool success = false;
  word_t result;

  result = parse_expr(wp->expr, &success);
  if (!success) {
    panic("Failed to evaluate expression `%s`", wp->expr);
  } 
  if (result != wp->val) {
    wp->val = result;
    return true;
  }
  return false;
}

/* 
  Check if watchpoint value changed after execution
*/
void wp_eval_all() {
  WP *wp;
  for (wp = head; wp != NULL; wp = wp->next) {
    int prev_val = wp->val;
    if (wp_check_change(wp)) {
      printf("Watchpoint %d: %s\n  %u -> %u\n", wp->NO, wp->expr, prev_val, wp->val);
    }
  }
}
