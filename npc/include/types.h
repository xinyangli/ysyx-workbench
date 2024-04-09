#ifndef _NPC_TYPES_H__
#define _NPC_TYPES_H__
#include <inttypes.h>

typedef uint32_t word_t;
typedef int32_t sword_t;
static const word_t WORD_T_MAX = UINT32_MAX;
static const sword_t SWORD_T_MAX = INT32_MAX;
static const sword_t SWORD_T_MIN = INT32_MIN;
#define WORD_BYTES 4

#define FMT_WORD "0x%08x"
typedef uint32_t vaddr_t;
typedef uint32_t paddr_t;
#define FMT_ADDR "0x%08x"
typedef uint16_t ioaddr_t;

#ifdef __cplusplus
#include <difftest.hpp>
using CPUState = CPUStateBase<word_t, 32>;
#endif

#endif