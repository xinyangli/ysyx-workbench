#ifndef __TYPES_H__
#define __TYPES_H__
#include <inttypes.h>
#include <macro.h>
#if CONFIG_MBASE + CONFIG_MSIZE > 0x100000000ul
#define PMEM64 1
#endif

typedef MUXDEF(CONFIG_ISA64, uint64_t, uint32_t) word_t;
typedef MUXDEF(CONFIG_ISA64, int64_t, int32_t)  sword_t;
static const word_t WORD_T_MAX = MUXDEF(CONFIG_ISA64, UINT64_MAX, UINT32_MAX);
static const sword_t SWORD_T_MAX = MUXDEF(CONFIG_ISA64, INT64_MAX, INT32_MAX);
static const sword_t SWORD_T_MIN = MUXDEF(CONFIG_ISA64, INT64_MIN, INT32_MIN);
#define WORD_BYTES MUXDEF(CONFIG_ISA64, 8, 4)
#define FMT_WORD MUXDEF(CONFIG_ISA64, "0x%016" PRIx64, "0x%08" PRIx32)

typedef word_t vaddr_t;
typedef MUXDEF(PMEM64, uint64_t, uint32_t) paddr_t;
#define FMT_PADDR MUXDEF(PMEM64, "0x%016" PRIx64, "0x%08" PRIx32)
typedef uint16_t ioaddr_t;
#endif