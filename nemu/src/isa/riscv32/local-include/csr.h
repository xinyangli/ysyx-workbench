#ifndef __NEMU_CSR_H__
#define __NEMU_CSR_H__
#include <isa-def.h>
#include <types.h>

#define MSTATUS 0x300
#define MISA 0x301
#define MIE 0x304
#define MTVEC 0x305
#define MEPC 0x341
#define MCAUSE 0x342

enum { CauseEnvironmentCallFromMMode = 11 };

typedef uint16_t csr_addr_t;

void init_csr(csr_t csr);

/* macro for setting and clearing csr bits */
#define set_csr_bits(csr, reg, mask)                                           \
  write_csr(csr, reg, read_csr(csr, reg) | (mask))

#define clear_csr_bits(csr, reg, mask)                                         \
  write_csr(csr, reg, read_csr(csr, reg) & ~(mask))

void write_csr(csr_t csr, csr_addr_t csrnum, word_t value);
word_t read_csr(csr_t csr, csr_addr_t csrnum);

#endif // __NEMU_CSR_H__
