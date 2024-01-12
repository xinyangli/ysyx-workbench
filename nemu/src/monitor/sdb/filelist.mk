SRCS-y += src/monitor/sdb/addrexp.tag.c src/monitor/sdb/addrexp.yy.c
INC_PATH += src/monitor/sdb
LFLAGS += -DYY_NO_UNPUT -DYY_NO_INPUT

$(BUILD_DIR)/src/monitor/sdb/addr-exp.c: src/monitor/sdb/addr-exp.y
	@echo + YACC $<
	@mkdir -p $(dir $@)
	@$(YACC) $(YFLAGS) --header=$(<:.y=.h) -o $@ $<

$(BUILD_DIR)/src/monitor/sdb/addr-lex.c: src/monitor/sdb/addr-lex.l $(BUILD_DIR)/src/monitor/sdb/addr-exp.c
	@echo + LEX $<
	@mkdir -p $(dir $@)
	@$(LEX) $(LFLAGS) -o $@ $<
