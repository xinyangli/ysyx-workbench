VSRC := $(wildcard vsrc/*.v)
CPPSRC := $(addprefix $(PWD)/,$(wildcard csrc/*.cpp))
CPPSRC += $(SRC_AUTO_BIND)
NXDC_FILES = $(abspath constr/top.nxdc)

PREFIX ?= build
OBJDIR := $(PREFIX)/obj
SUBMAKE := $(OBJDIR)/Vexample.mk
VERILATOR_FLAGS := --cc --exe
NVBOARD_HOME ?= $(abspath ../nvboard)

all: sim

sim: VERILATOR_FLAGS += --trace
sim: $(VSRC) $(CPPSRC) $(OBJDIR)/Vexample
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@echo "Running" $(OBJDIR)/Vexample "..."
	@echo "================================"
	@$(OBJDIR)/Vexample
	
$(OBJDIR)/Vexample: $(SUBMAKE)
	$(MAKE) -C $(OBJDIR) -f $(notdir $(SUBMAKE)) Vexample
	
$(SUBMAKE): $(VSRC) $(CPPSRC) $(OBJDIR)
	verilator $(VERILATOR_FLAGS) \
		--Mdir $(abspath $(OBJDIR)) $(VSRC) $(CPPSRC)
# $(addprefix -CFLAGS , $(CXXFLAGS)) $(addprefix -LDFLAGS , $(LDFLAGS)) \ 

$(OBJDIR):
	mkdir -p $(OBJDIR)

SRC_AUTO_BIND = $(abspath $(PREFIX)/auto_bind.cpp)
$(SRC_AUTO_BIND): $(NXDC_FILES)
	python3 $(NVBOARD_HOME)/scripts/auto_pin_bind.py $^ $@

include ../Makefile

.PHONY: clean 

clean: 
	$(RM) -r $(OBJDIR)
