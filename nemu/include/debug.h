/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdio.h>
#include <utils.h>
#include <macro.h>

IFDEF(CONFIG_ITRACE, void log_itrace_print());

#if (CONFIG_LOG_LEVEL >= 4)
#define Trace(format, ...) \
    _Log("[TRACE] " format "\n", ## __VA_ARGS__)
#else
#define Trace(format, ...)
#endif

#if (CONFIG_LOG_LEVEL >= 3)
#define Log(format, ...) \
    _Log(ANSI_FMT("[INFO] %s:%d %s() ", ANSI_FG_BLUE) format "\n", \
        __FILE__, __LINE__, __func__, ## __VA_ARGS__)
#else
#define Log(format, ...)
#endif

#if (CONFIG_LOG_LEVEL >= 2)
#define Warning(format, ...) \
    _Log(ANSI_FMT("[WARNING] %s:%d %s() ", ANSI_FG_YELLOW) format "\n", \
        __FILE__, __LINE__, __func__, ## __VA_ARGS__)
#else
#define Warning(format, ...)
#endif

#if (CONFIG_LOG_LEVEL >= 1)
#define Error(format, ...) \
    _Log(ANSI_FMT("[ERROR] %s:%d %s() ", ANSI_FG_RED) format "\n", \
        __FILE__, __LINE__, __func__, ## __VA_ARGS__)
#else 
#define Error(format, ...)
#endif

#define Assert(cond, format, ...) \
  do { \
    if (!(cond)) { \
      MUXDEF(CONFIG_TARGET_AM, printf(ANSI_FMT(format, ANSI_FG_RED) "\n", ## __VA_ARGS__), \
        (fflush(stdout), fprintf(stderr, ANSI_FMT(format, ANSI_FG_RED) "\n", ##  __VA_ARGS__))); \
      IFNDEF(CONFIG_TARGET_AM, extern FILE* log_fp; fflush(log_fp)); \
      IFDEF(CONFIG_ITRACE, log_itrace_print()); \
      extern void assert_fail_msg(); \
      assert_fail_msg(); \
      assert(cond); \
    } \
  } while (0)

#define panic(format, ...) Assert(0, format, ## __VA_ARGS__)

#define TODO() panic("please implement me")

#endif
