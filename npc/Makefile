NVBOARD_HOME ?= $(abspath ../nvboard)
PREFIX ?= build
OBJDIR := $(PREFIX)/obj
TARGET := $(OBJDIR)/Vexample

VSRC := $(wildcard vsrc/*.v)
CPPSRCS := $(addprefix $(PWD)/,$(wildcard csrc/*.cpp))
SUBMAKE := $(OBJDIR)/Vexample.mk

VERILATOR_FLAGS := --cc --exe
LDFLAGS += $(shell sdl2-config --libs) -lSDL2_image

all: sim

sim: VERILATOR_FLAGS += --trace 
sim: $(VSRC) $(CPPSRCS) $(OBJDIR)/Vexample git_trace
	@echo "Running" $(OBJDIR)/Vexample "..."
	@echo "================================"
	@$(OBJDIR)/Vexample
	
$(OBJDIR)/Vexample: $(SUBMAKE)
	$(MAKE) -C $(OBJDIR) -f $(notdir $(SUBMAKE)) Vexample

$(SUBMAKE): $(VSRC) $(CPPSRCS)
	mkdir -p $(OBJDIR)
	verilator $(VERILATOR_FLAGS) $(addprefix -CFLAGS , $(CXXFLAGS)) $(addprefix -LDFLAGS , $(LDFLAGS)) --Mdir $(abspath $(OBJDIR)) $(VSRC) $(CPPSRCS)

$(SRC_AUTO_BIND): $(NXDC_FILES)
	NVBOARD_HOME=$(NVBOARD_HOME) python3 $(NVBOARD_HOME)/scripts/auto_pin_bind.py $< $@

ifneq (,$(wildcard ../Makefile))
include ../Makefile
else
define git_commit	# not in ICS subfolder, no tracing
endef
endif

git_trace: 
	$(call git_commit, "sim RTL")

.PHONY: clean nvboard

nvboard: OBJDIR := $(PREFIX)/nvobj
nvboard: SUBMAKE := $(OBJDIR)/Vexample.mk

nvboard: NXDC_FILES = $(abspath constr/top.nxdc)
nvboard: SRC_AUTO_BIND := $(abspath $(PREFIX)/auto_bind.cpp)

# TODO: fix this awkward way to find nvboard.a
nvboard: CPPSRCS := $(addprefix $(PWD)/,$(wildcard csrc_nvboard/*.cpp)) $(SRC_AUTO_BIND) $(NVBOARD_HOME)/build/nvboard.a
nvboard: CXXFLAGS += -I$(NVBOARD_HOME)/include $(shell sdl2-config --cflags) -g

nvboard: $(VSRC) $(CPPSRCS) $(SUBMAKE) $(SRC_AUTO_BIND) $(OBJDIR)/Vexample 
	@NVBOARD_HOME=$(NVBOARD_HOME) $(OBJDIR)/Vexample

clean: 
	$(RM) -r $(PREFIX)
