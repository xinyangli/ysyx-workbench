#include <common.h>
#include <elf.h>

// Put this into another file
#ifdef CONFIG_FTRACE
// static vaddr_t g_function_stack[CONFIG_FTRACE_STACK_SIZE] = {0};
#endif

#define FAILED_GOTO(tag, exp) do {if((exp)) goto tag;} while(0)

void init_elf(const char *path) {
  FILE *elf_file = fopen(path, "rb");
  Elf32_Ehdr header;
  Elf32_Shdr section_header[200], *psh;
  FAILED_GOTO(failed_nosym, fread(&header, sizeof(Elf32_Ehdr), 1, elf_file) <= 0);
  FAILED_GOTO(failed_nosym, fseek(elf_file, header.e_shoff, SEEK_SET) != 0);
  printf("%hu %hu %u\n", header.e_shentsize, header.e_shnum, header.e_shoff);
  FAILED_GOTO(failed_nosym, fread(section_header, header.e_shentsize, header.e_shnum, elf_file) <= 0);

  Elf32_Shdr *symtab = NULL, *strtab = NULL;
  for(int i = 0; i < header.e_shnum; i++) {
    psh = section_header + i;
    if (psh->sh_type == SHT_SYMTAB) {
      symtab = psh;
    } else if (psh->sh_type == SHT_STRTAB) {
      strtab = psh;
    }
  }


  int sym_length = symtab->sh_entsize / sizeof(Elf32_Sym);
  Elf32_Sym *sym = calloc(sym_length, sizeof(Elf32_Sym));
  FAILED_GOTO(failed, fseek(elf_file, symtab->sh_offset, SEEK_SET) != 0);
  FAILED_GOTO(failed, fread(&sym, sizeof(Elf32_Sym), sym_length, elf_file) <= 0);
  for(int j = 0; j < sym_length; j++) {
    if(ELF32_ST_TYPE(sym[j].st_info) != STT_FUNC) continue;
    // Only read function type symbol
    char func[30] = "";
    FAILED_GOTO(failed, fseek(elf_file, sym[j].st_name + strtab->sh_offset, SEEK_SET) != 0);
    FAILED_GOTO(failed, fgets(func, 30, elf_file) <= 0);
    puts(func);
  }
  return;
failed:
  free(sym);
failed_nosym:
  Error("Failed reading elf file");
}
