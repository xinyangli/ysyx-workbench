NVBOARD_HOME ?= $(abspath ../nvboard)
PREFIX ?= build
OBJDIR := $(PREFIX)/obj
TARGET := $(OBJDIR)/Vexample

VSRC := $(wildcard vsrc/*.v)
CPPSRCS := $(addprefix $(PWD)/,$(wildcard csrc/*.cpp))
SUBMAKE := $(OBJDIR)/Vexample.mk

VERILATOR_FLAGS := --cc --exe
LDFLAGS += $(shell sdl2-config --libs) -lSDL2_image

all: sim-bin nvboard-bin
	
$(OBJDIR)/Vexample: $(SUBMAKE)
	$(MAKE) -C $(OBJDIR) -f $(notdir $(SUBMAKE)) Vexample

$(SUBMAKE): $(VSRC) $(CPPSRCS) $(OBJDIR)
	verilator $(VERILATOR_FLAGS) $(addprefix -CFLAGS , $(CXXFLAGS)) $(addprefix -LDFLAGS , $(LDFLAGS)) --Mdir $(abspath $(OBJDIR)) $(VSRC) $(CPPSRCS)

$(OBJDIR):
	mkdir -p $(OBJDIR)

sim-bin: VERILATOR_FLAGS += --trace 
sim-bin: $(VSRC) $(CPPSRCS) $(OBJDIR)/Vexample

SRC_AUTO_BIND := $(abspath $(PREFIX)/auto_bind.cpp)
NXDC_FILES := $(abspath constr/top.nxdc)
$(SRC_AUTO_BIND): $(NXDC_FILES)
	NVBOARD_HOME=$(NVBOARD_HOME) python3 $(NVBOARD_HOME)/scripts/auto_pin_bind.py $< $@

nvboard-bin: OBJDIR := $(PREFIX)/nvobj
nvboard-bin: SUBMAKE := $(OBJDIR)/Vexample.mk
# TODO: fix this awkward way to find nvboard.a
nvboard-bin: CPPSRCS := $(addprefix $(PWD)/,$(wildcard csrc_nvboard/*.cpp)) $(SRC_AUTO_BIND) $(NVBOARD_HOME)/build/nvboard.a
nvboard-bin: CXXFLAGS += -I$(NVBOARD_HOME)/include $(shell sdl2-config --cflags) -g

nvboard-bin: $(VSRC) $(CPPSRCS) $(SUBMAKE) $(SRC_AUTO_BIND) $(OBJDIR)/Vexample 


ifneq (,$(wildcard ../Makefile))
include ../Makefile
else
define git_commit	# not in ICS subfolder, no tracing
endef
endif

git_trace_sim: 
	$(call git_commit, "sim RTL")

git_trace_nvboard: 
	$(call git_commit, "nvboard")

.PHONY: clean nvboard sim compile_commands.json 

nvboard: OBJDIR := $(PREFIX)/nvobj
nvboard: nvboard-bin git_trace_nvboard
	@NVBOARD_HOME=$(NVBOARD_HOME) $(OBJDIR)/Vexample

sim: sim-bin git_trace_sim
	@echo "Running" $(OBJDIR)/Vexample "..."
	@echo "================================"
	@$(OBJDIR)/Vexample

compile_commands.json: clean
	bear --output nvboard.json -- $(MAKE) nvboard-bin
	bear --output all.json -- $(MAKE) sim-bin
	jq -s ".[0] + .[1]"  all.json nvboard.json > compile_commands.json
	$(RM) all.json nvboard.json

clean: 
	$(RM) -r $(PREFIX)
