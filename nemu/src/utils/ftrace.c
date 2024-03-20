#include "debug.h"
#include "macro.h"
#include <assert.h>
#include <common.h>
#include <elf.h>
#include <ftrace.h>

// Put this into another file
#ifdef CONFIG_FTRACE
static vaddr_t ftrace_stack[CONFIG_FTRACE_STACK_SIZE] = {0};
static vaddr_t ftrace_stack_len = 0;
func_t *func_table = NULL;
int func_table_len = 0, func_table_size = 8;
#endif

static int cmp_func_t(const void *a, const void *b) {
  return ((func_t *)a)->start > ((func_t *)b)->start;
}

static const char *get_func_name(vaddr_t addr) {
  int l = 0, r = func_table_len - 1;
  while(l <= r) {
    int mid = (l + r) / 2;
    if(func_table[mid].start <= addr)  l = mid + 1;
    else r = mid - 1;
  }
  // if(func_table[l].start + func_table[l].len < addr) return "???";
  return l == 0 ? "???" : func_table[l - 1].name;
}

void init_elf(const char *path) {
  FILE *elf_file = fopen(path, "rb");
  Elf32_Ehdr header;
  Elf32_Shdr section_header[200], *psh;

  func_table = (func_t *)calloc(func_table_size, sizeof(func_t));
  assert(func_table);

  FAILED_GOTO(failed_nosym, fread(&header, sizeof(Elf32_Ehdr), 1, elf_file) <= 0);
  FAILED_GOTO(failed_nosym, fseek(elf_file, header.e_shoff, SEEK_SET) != 0);
  FAILED_GOTO(failed_nosym, fread(section_header, header.e_shentsize, header.e_shnum, elf_file) <= 0);

  char *shstrtab = calloc(1, section_header[header.e_shstrndx].sh_size);
  FAILED_GOTO(failed_shstrtab, fseek(elf_file, section_header[header.e_shstrndx].sh_offset, SEEK_SET) != 0);
  FAILED_GOTO(failed_shstrtab, fread(shstrtab, section_header[header.e_shstrndx].sh_size, 1, elf_file) <= 0);

  Elf32_Shdr *symtab = NULL, *strtab = NULL;
  for(int i = 0; i < header.e_shnum; i++) {
    psh = section_header + i;
    if (psh->sh_type == SHT_SYMTAB) {
      symtab = psh;
    } else if (psh->sh_type == SHT_STRTAB && strncmp(shstrtab + psh->sh_name, ".strtab", 8) == 0) {
      strtab = psh;
    }
  }

  int sym_length = symtab->sh_size / sizeof(Elf32_Sym);
  Elf32_Sym *sym = calloc(sym_length, sizeof(Elf32_Sym));
  assert(sym);
  FAILED_GOTO(failed, fseek(elf_file, symtab->sh_offset, SEEK_SET) != 0);
  FAILED_GOTO(failed, fread(sym, sizeof(Elf32_Sym), sym_length, elf_file) <= 0);
  
  for(int j = 0; j < sym_length; j++) {
    if(ELF32_ST_TYPE(sym[j].st_info) != STT_FUNC) continue;
    // Only read function type symbol
    func_t *f = &func_table[func_table_len];
    char *func = (char *)malloc(30);
    FAILED_GOTO(failed, fseek(elf_file, strtab->sh_offset + sym[j].st_name, SEEK_SET) != 0);
    FAILED_GOTO(failed, fgets(func, 30, elf_file) <= 0);
    f->start = sym[j].st_value;
    f->len = sym[j].st_size;
    f->name = func;
    ++func_table_len;
    if(func_table_len >= func_table_size) {
      Assert(func_table_size * 2 > func_table_size, "Function table exceed memory limit");
      func_table_size *= 2;
      func_table = realloc(func_table, func_table_size * sizeof(func_t));
      Assert(func_table, "Function table exceed memory limit");
    }
  }
  qsort(func_table, func_table_len, sizeof(func_t), cmp_func_t);
  goto success;

success:
  free(sym);
  free(shstrtab);
  return;

failed:
  free(sym);
failed_shstrtab:
  free(shstrtab);
failed_nosym:
  for(int i = 0; i < func_table_len; i++) {
    func_t *f = &func_table[i];
    if(f->name) { free(f->name); }
  } 
  free(func_table);
  Error("Failed reading elf file");
  return;
}

void ftrace_call(vaddr_t pc, vaddr_t addr) {
  Assert(ftrace_stack_len < CONFIG_FTRACE_STACK_SIZE,
         "Ftrace stack exceed size limit, consider turn off ftrace or increase "
         "FTRACE_STACK_SIZE.");
  ftrace_stack[ftrace_stack_len] = pc + 4;
  Trace("%*s0x%x call [%s@0x%x]", ftrace_stack_len, "", pc, get_func_name(addr),
        addr);
  ftrace_stack_len++;
}

void ftrace_return(vaddr_t pc, vaddr_t addr) {
  --ftrace_stack_len;
  for (; addr != ftrace_stack[ftrace_stack_len] && ftrace_stack_len >= 0;
       ftrace_stack_len--) {
    vaddr_t tco_addr = ftrace_stack[ftrace_stack_len];
    Trace("%*s0x%x <TCO> [%s@0x%x] ", ftrace_stack_len, "", pc, get_func_name(tco_addr), tco_addr);
  }
  Trace("%*s0x%x ret [%s@0x%x]", ftrace_stack_len, "", pc, get_func_name(addr), addr);
}
