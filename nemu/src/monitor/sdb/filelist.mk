SRCS-y += src/monitor/sdb/addr-exp.c src/monitor/sdb/addr-lex.c
INC_PATH += src/monitor/sdb
LFLAGS += -DYY_NO_UNPUT -DYY_NO_INPUT

/home/xin/repo/ysyx-workbench/nemu/build/obj-riscv32-nemu-interpreter/src/monitor/sdb/addr-exp.c: src/monitor/sdb/addr-exp.y
	@echo + YACC $<
	@mkdir -p $(dir $@)
	@$(YACC) $(YFLAGS) --header=$(<:.y=.h) -o $@ $<

/home/xin/repo/ysyx-workbench/nemu/build/obj-riscv32-nemu-interpreter/src/monitor/sdb/addr-lex.c: src/monitor/sdb/addr-lex.l /home/xin/repo/ysyx-workbench/nemu/build/obj-riscv32-nemu-interpreter/src/monitor/sdb/addr-exp.c
	@echo + LEX $<
	@mkdir -p $(dir $@)
	@$(LEX) $(LFLAGS) -o $@ $<
