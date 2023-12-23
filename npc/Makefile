VSRC := $(wildcard vsrc/*.v)
CPPSRC := $(wildcard csrc/*.cpp)
PREFIX ?= .
OBJDIR := $(PREFIX)/obj

all: $(OBJDIR)
	$(MAKE) -j -C $(OBJDIR) -f Vexample.mk Vexample

sim: all
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@echo "Write this Makefile by your self."
	$(OBJDIR)/Vexample
	

$(OBJDIR): $(VSRC) $(CPPSRC)
	mkdir -p $(OBJDIR)
	verilator --cc --exe --Mdir $(PWD)/$(OBJDIR) $(VSRC) $(CPPSRC)

include ../Makefile

.PHONY: clean 

clean: 
	$(RM) -r $(OBJDIR)
