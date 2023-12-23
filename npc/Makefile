VSRC := $(wildcard vsrc/*.v)
CPPSRC := $(addprefix $(PWD)/,$(wildcard csrc/*.cpp))
PREFIX ?= build
OBJDIR := $(PREFIX)/obj
SUBMAKE := $(OBJDIR)/Vexample.mk
VERILATOR_FLAGS := --cc --exe

all: sim

sim: VERILATOR_FLAGS += --trace
sim: $(VSRC) $(CPPSRC) $(SUBMAKE)
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@echo "Running" $(OBJDIR)/Vexample "..."
	@echo "================================"
	@$(OBJDIR)/Vexample
	
$(OBJDIR)/Vexample: $(SUBMAKE)
	$(MAKE) -C $(OBJDIR) -f $(notdir $(SUBMAKE)) Vexample
	
$(SUBMAKE): $(VSRC) $(CPPSRC) $(OBJDIR)
	verilator $(VERILATOR_FLAGS) --Mdir $(PWD)/$(OBJDIR) $(VSRC) $(CPPSRC)

$(OBJDIR):
	mkdir -p $(OBJDIR)

include ../Makefile

.PHONY: clean 

clean: 
	$(RM) -r $(OBJDIR)
