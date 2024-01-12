TEST_SRCS += tests/expr_test.c

$(OBJ_DIR)/%: %.c $(OBJS)
	@echo + CC $<
	@$(CC) $(CFLAGS) -Wl,-e,test_main -o $@ $< $(LDFLAGS) $(CFLGAS)
	@$(CC) $(CFLAGS) -Wl,-e,test_main -o $@ $^ $(LDFLAGS) $(CFLGAS)

$(OBJ_DIR)/%: %.c $(OBJS)
	@echo + CC $<
	@$(CC) $(CFLAGS) -Wl,-e,test_main -o $@ $< $(LDFLAGS) $(CFLGAS)
	@$(CC) $(CFLAGS) -Wl,-e,test_main -o $@ $^ $(LDFLAGS) $(CFLGAS)
