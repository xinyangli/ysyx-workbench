VSRC := $(wildcard vsrc/*.v)
CPPSRC := $(addprefix $(PWD)/,$(wildcard csrc/*.cpp))
PREFIX ?= build
OBJDIR := $(PREFIX)/obj
SUBMAKE := $(OBJDIR)/Vexample.mk
VERILATOR_FLAGS := --cc --exe

all: sim

sim: $(OBJDIR) $(VSRC) $(CPPSRC) $(SUBMAKE)
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@echo "Running" $(OBJDIR)/Vexample "..."
	@echo "================================"
	@$(OBJDIR)/Vexample

trace: VERILATOR_FLAGS += --trace
trace: $(OBJDIR) $(VSRC) $(CPPSRC) $(SUBMAKE)
	$(call git_commit, "trace RTL") # DO NOT REMOVE THIS LINE!!!
	@$(OBJDIR)/Vexample
	
$(SUBMAKE): $(VSRC) $(CPPSRC)
	verilator $(VERILATOR_FLAGS) --Mdir $(PWD)/$(OBJDIR) $(VSRC) $(CPPSRC)

$(OBJDIR): $(VSRC) $(CPPSRC)
	mkdir -p $(OBJDIR)
	verilator $(VERILATOR_FLAGS) --Mdir $(PWD)/$(OBJDIR) $(VSRC) $(CPPSRC)

include ../Makefile

.PHONY: clean 

clean: 
	$(RM) -r $(OBJDIR)
