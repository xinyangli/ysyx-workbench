#ifndef _NPC_TYPES_H__
#define _NPC_TYPES_H__
#include <inttypes.h>

typedef uint32_t word_t;
typedef int32_t sword_t;
static const word_t WORD_T_MAX = UINT32_MAX;
static const sword_t SWORD_T_MAX = INT32_MAX;
static const sword_t SWORD_T_MIN = INT32_MIN;
#define WORD_BYTES 4
#define REG_COUNT 32

#define FMT_WORD "0x%08x"
typedef uint32_t vaddr_t;
typedef uint32_t paddr_t;
#define FMT_ADDR "0x%08x"
typedef uint16_t ioaddr_t;

#ifdef __cplusplus
#include <string>
#include <map>

const std::map<std::string, int> riscv32_regs_by_name{
    {"$0", 0},  {"ra", 1},  {"sp", 2},   {"gp", 3},   {"tp", 4},  {"t0", 5},
    {"t1", 6},  {"t2", 7},  {"s0", 8},   {"s1", 9},   {"a0", 10}, {"a1", 11},
    {"a2", 12}, {"a3", 13}, {"a4", 14},  {"a5", 15},  {"a6", 16}, {"a7", 17},
    {"s2", 18}, {"s3", 19}, {"s4", 20},  {"s5", 21},  {"s6", 22}, {"s7", 23},
    {"s8", 24}, {"s9", 25}, {"s10", 26}, {"s11", 27}, {"t3", 28}, {"t4", 29},
    {"t5", 30}, {"t6", 31}};
#endif

#endif