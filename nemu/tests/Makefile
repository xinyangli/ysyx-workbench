TEST_SRCS += tests/expr_test.c
YACC = bison
INC_PATH += $(NEMU_HOME)/src/isa/$(GUEST_ISA)/local-include

$(OBJ_DIR)/%: %.c $(TEST_OBJS) app
	@mkdir -p $(dir $@)
	@echo + CC $<
	@$(CC) $(CFLAGS) -o $@.o -c $< 
	@echo + LD $@
	@$(LD) $(LIBS) $(LDFLAGS) -o $@ $(TEST_OBJS) $@.o
