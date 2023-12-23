VERILATOR := verilator

VSRC := $(wildcard vsrc/*.v)
CPPSRC := $(wildcard csrc/*.cpp)

all:
	@echo "Write this Makefile by your self."

sim: obj_dir
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@echo "Write this Makefile by your self."

obj_dir: $(VSRC) $(CPPSRC)
	$(VERILATOR) $(VSRC:%=--cc %) $(CPPSRC:%=--exe %) --Mdir $@

include ../Makefile
