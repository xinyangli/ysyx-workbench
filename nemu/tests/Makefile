TEST_SRCS += tests/expr_test.c

$(OBJ_DIR)/%: %.c $(TEST_OBJS) app
	@echo + CC $<
	@$(CC) $(CFLAGS) -o $@.o -c $< 
	@echo + LD $@
	@$(LD) $(LIBS) $(LDFLAGS) -o $@ $(TEST_OBJS) $@.o
