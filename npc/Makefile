NVBOARD_HOME ?= $(abspath ../nvboard)
PREFIX ?= build
CHISEL_VDIR := $(PREFIX)/chisel

CPPSRCS := $(addprefix $(PWD)/,$(wildcard csrc/*.cpp))

VERILATOR_FLAGS := --cc --exe
LDFLAGS += $(shell sdl2-config --libs) -lSDL2_image

CHISEL_TOP_PACKAGE := learning
CHISEL_TOP_MODULE := Main
CHISEL_TARGET := verilog

SUBDIRS := obj nvobj
SUBDIRS := $(addprefix $(PREFIX),$(SUBDIRS))

SUBMAKE := $(OBJDIR)/V$(CHISEL_TOP_MODULE).mk

# Pretty printing
MAKEFLAGS += --no-print-directory
GREEN := \e[32m
NC := \e[0m
define colorize
	printf '$(GREEN)'$(1)'$(NC) $(2)\n'
endef

all: sim-bin nvboard-bin

$(SUBDIRS):%: %/V$(CHISEL_TOP_MODULE).mk
	verilator $(VERILATOR_FLAGS) $(addprefix -CFLAGS , $(CXXFLAGS)) $(addprefix -LDFLAGS , $(LDFLAGS)) --Mdir $(abspath $(OBJDIR)) $(CHISEL_VSRC) $(CPPSRCS)
	
$(OBJDIR)/V$(CHISEL_TOP_MODULE): $(SUBMAKE)
	@$(call colorize,"SUBMAKE",$^)
	$(MAKE) -s -C $(dir $@) -f $< $(notdir $@)

$(SUBMAKE): $(CPPSRCS) $(OBJDIR) chisel-src
	@$(call colorize,"VERILATOR",$@)
	verilator $(VERILATOR_FLAGS) $(addprefix -CFLAGS , $(CXXFLAGS)) $(addprefix -LDFLAGS , $(LDFLAGS)) --Mdir $(abspath $(OBJDIR)) $(CHISEL_VSRC) $(CPPSRCS)

$(OBJDIR):
	mkdir -p $@

$(CHISEL_VDIR)/filelist.f: $(wildcard src/main/scala/*.scala)
	@$(call colorize,"CIRCT",$^)
	sbt --error "runMain circt.stage.ChiselMain --module $(CHISEL_TOP_PACKAGE).$(CHISEL_TOP_MODULE) --split-verilog --target $(CHISEL_TARGET) -td $(CHISEL_VDIR)"

compile_commands.json: clean
	$(MAKE) $(CHISEL_VDIR)/filelist.f
	$(RM) compile_commands.json
	bear --append -- $(MAKE) nvboard-bin
	bear --append -- $(MAKE) sim-bin

.PHONY: clean nvboard sim nvboard-bin sim-bin git_trace_sim git_trace_nvboard

SRC_AUTO_BIND := $(abspath $(PREFIX)/auto_bind.cpp)
NXDC_FILES := $(abspath constr/top.nxdc)
$(SRC_AUTO_BIND): $(NXDC_FILES)
	NVBOARD_HOME=$(NVBOARD_HOME) python3 $(NVBOARD_HOME)/scripts/auto_pin_bind.py $< $@

nvboard-bin: OBJDIR := $(PREFIX)/nvobj
nvboard-bin: SUBMAKE := $(OBJDIR)/V$(CHISEL_TOP_MODULE).mk
# TODO: fix this awkward way to find nvboard.a
nvboard-bin: CPPSRCS := $(addprefix $(PWD)/,$(wildcard csrc_nvboard/*.cpp)) $(SRC_AUTO_BIND) $(NVBOARD_HOME)/build/nvboard.a
nvboard-bin: CXXFLAGS += -I$(NVBOARD_HOME)/include $(shell sdl2-config --cflags)
nvboard-bin: $(CPPSRCS) $(SUBMAKE) $(SRC_AUTO_BIND) $(OBJDIR)/V$(CHISEL_TOP_MODULE)
	@echo $(SUBMAKE) $(OBJDIR)

sim-bin: VERILATOR_FLAGS += --trace 
sim-bin: $(CPPSRCS) $(OBJDIR)/V$(CHISEL_TOP_MODULE)

chisel-src: $(CHISEL_VDIR)/filelist.f
	$(eval CHISEL_VSRC := $(wildcard $(CHISEL_VDIR)/*.sv))
	@echo "GENERATED: $(CHISEL_VSRC)"

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

nvboard: OBJDIR := $(PREFIX)/nvobj
nvboard: nvboard-bin git_trace_nvboard
	@echo "Running NVBoard ..."
	@echo "================================"
	@NVBOARD_HOME=$(NVBOARD_HOME) $(OBJDIR)/V$(CHISEL_TOP_MODULE)

sim: sim-bin git_trace_sim
	@echo "Running verilator sim ..."
	@echo "================================"
	@$(OBJDIR)/V$(CHISEL_TOP_MODULE)

clean: 
	$(RM) -r $(PREFIX)

$(V).SILENT:
