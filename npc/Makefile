NVBOARD_HOME ?= $(abspath ../nvboard)
PREFIX ?= build
OBJDIR := $(PREFIX)/obj

VSRC := $(wildcard vsrc/*.v)
SRC_AUTO_BIND := $(abspath $(PREFIX)/auto_bind.cpp)
CPPSRCS := $(addprefix $(PWD)/,$(wildcard csrc/*.cpp))
# TODO: fix this ugly way to find nvboard.a
CPPSRCS += $(SRC_AUTO_BIND) $(NVBOARD_HOME)/build/nvboard.a
NXDC_FILES = $(abspath constr/top.nxdc)
SUBMAKE := $(OBJDIR)/Vexample.mk

VERILATOR_FLAGS := --cc --exe
CXXFLAGS += -I$(NVBOARD_HOME)/include $(shell sdl2-config --cflags) -g
LDFLAGS += $(shell sdl2-config --libs) -lSDL2_image

all: sim nvboard

sim: VERILATOR_FLAGS += --trace 
sim: $(VSRC) $(CPPSRCS) $(OBJDIR)/Vexample git_trace
	@echo "Running" $(OBJDIR)/Vexample "..."
	@echo "================================"
	@$(OBJDIR)/Vexample

nvboard: OBJDIR := $(PREFIX)/nvobj
nvboard: CPPSRCS := $(addprefix $(PWD)/,$(wildcard csrc_nvboard/*.cpp)) $(SRC_AUTO_BIND) $(NVBOARD_HOME)/build/nvboard.a
nvboard: $(VSRC) $(CPPSRCS) $(OBJDIR)/Vexample
	@NVBOARD_HOME=$(NVBOARD_HOME) $(OBJDIR)/Vexample

	
$(OBJDIR)/Vexample: $(SUBMAKE)
	$(MAKE) -C $(OBJDIR) -f $(notdir $(SUBMAKE)) Vexample
	
$(SUBMAKE): $(VSRC) $(CPPSRCS) $(OBJDIR)
	verilator $(VERILATOR_FLAGS) $(addprefix -CFLAGS , $(CXXFLAGS)) $(addprefix -LDFLAGS , $(LDFLAGS)) --Mdir $(abspath $(OBJDIR)) $(VSRC) $(CPPSRCS)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(SRC_AUTO_BIND): $(NXDC_FILES) $(OBJDIR)
	NVBOARD_HOME=$(NVBOARD_HOME) python3 $(NVBOARD_HOME)/scripts/auto_pin_bind.py $< $@


ifneq (,$(wildcard ../Makefile))
include ../Makefile
endif

ifeq (,$(wildcard ../Makefile))
define git_commit
endef
endif

git_trace: 
	$(call git_commit, "sim RTL")


.PHONY: clean 

clean: 
	$(RM) -r $(PREFIX)
