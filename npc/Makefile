VSRC := $(wildcard vsrc/*.v)
CPPSRC := $(addprefix $(PWD)/,$(wildcard csrc/*.cpp))
PREFIX ?= build
OBJDIR := $(PREFIX)/obj

all: $(OBJDIR)
	$(MAKE) -j -C $(OBJDIR) -f Vexample.mk Vexample

sim: all
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@echo "Running" $(OBJDIR)/Vexample "..."
	@echo "================================"
	@$(OBJDIR)/Vexample +trace off

trace: all
	$(call git_commit, "trace RTL") # DO NOT REMOVE THIS LINE!!!
	@$(OBJDIR)/Vexample
	

$(OBJDIR): $(VSRC) $(CPPSRC)
	mkdir -p $(OBJDIR)
	verilator --trace --cc --exe --Mdir $(PWD)/$(OBJDIR) $(VSRC) $(CPPSRC)

include ../Makefile

.PHONY: clean 

clean: 
	$(RM) -r $(OBJDIR)
