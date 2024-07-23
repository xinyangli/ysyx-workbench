#ifndef __NEMU_HEADER__
#define __NEMU_HEADER__

#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
#include <gdbstub.h>
#ifdef __cplusplus
}
#endif

int nemu_read_mem(void *args, size_t addr, size_t len, void *val);
int nemu_write_mem(void *args, size_t addr, size_t len, void *val);
void nemu_cont(void *args, gdb_action_t *res);
void nemu_stepi(void *args, gdb_action_t *res);
bool nemu_set_bp(void *args, size_t addr, bp_type_t type);
bool nemu_del_bp(void *args, size_t addr, bp_type_t type);
void nemu_on_interrupt(void *args);
int nemu_read_reg(void *args, int regno, size_t *data);
int nemu_write_reg(void *args, int regno, size_t data);
void nemu_init(void *args);

extern arch_info_t nemu_isa_arch_info;
extern bool nemu_do_difftest;
extern bool nemu_dbg_state_size;

#endif // __NEMU_HEADER__
