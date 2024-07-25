#include <csr.h>

void init_csr(csr_t csr) { memset(csr, 0, sizeof(word_t)); }

void write_csr(csr_t csr, csr_addr_t csrnum, word_t value) {
  printf("write csr: 0x%hx %d\n", csrnum, value);
  switch (csrnum) {
  default:
    csr[csrnum] = value;
  }
}

word_t read_csr(csr_t csr, csr_addr_t csrnum) {
  switch (csrnum) {
  // TODO: Implement csr read checks
  default:
    return csr[csrnum];
  }
}
