VERILATOR := verilator

VSRC := $(wildcard vsrc/*.v)
CPPSRC := $(wildcard csrc/*.cpp)
PREFIX ?= ./build
OBJDIR := $(PREFIX)/obj

all: $(OBJDIR)
	$(MAKE) -C $(OBJDIR) -f Vexample.mk

sim: $(OBJDIR)
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@echo "Write this Makefile by your self."

$(OBJDIR): $(VSRC) $(CPPSRC)
	mkdir -p $(OBJDIR)
	$(VERILATOR) $(VSRC:%=--cc %) $(CPPSRC:%=--exe %) --Mdir $(OBJDIR)

include ../Makefile
