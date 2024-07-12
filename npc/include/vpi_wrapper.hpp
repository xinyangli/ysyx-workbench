#ifndef _NPC_VPI_WRAPPER_H_
#define _NPC_VPI_WRAPPER_H_
#include <components.hpp>
#include <vpi_user.h>

template <typename T, std::size_t nr>
class _RegistersVPI : public _RegistersBase<T, nr> {
  std::array<vpiHandle, nr> reg_handles;
  vpiHandle pc_handle;
  T vpi_get(vpiHandle vh) const {
    s_vpi_value v;
    v.format = vpiIntVal;
    vpi_get_value(vh, &v);
    return v.value.integer;
  }
  T fetch_pc(void) const { return vpi_get(pc_handle); }
  T fetch_reg(std::size_t id) const { return vpi_get(reg_handles[id]); }

public:
  _RegistersVPI<T, nr>(const std::string regs_prefix,
                       const std::string pcname) {
    for (int i = 0; i < nr; i++) {
      std::string regname = regs_prefix + std::to_string(i);
      vpiHandle vh = vpi_handle_by_name((PLI_BYTE8 *)regname.c_str(), nullptr);
      if (vh == nullptr) {
        std::cerr << "vpiHandle " << regname.c_str() << " not found"
                  << std::endl;
        exit(EXIT_FAILURE);
      }
      reg_handles[i] = vh;
    }
    pc_handle = vpi_handle_by_name((PLI_BYTE8 *)pcname.c_str(), nullptr);
  }
};

#endif
