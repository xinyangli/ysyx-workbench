INC_PATH += $(if $(CONFIG_TARGET_NATIVE_ELF),$(NEMU_HOME)/mini-gdbstub/include, )

DIRS-y += src/monitor/monitor.c

CXXSRC += src/monitor/gdbstub.cc 
