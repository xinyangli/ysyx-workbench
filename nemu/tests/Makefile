TEST_SRCS += tests/expr_test.c

$(OBJ_DIR)/tests/%.o: tests/%.c
	@echo + CC $<
	@$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) $(CFLGAS)

$(OBJ_DIR)/tests/%: $(OBJ_DIR)/test/%.o $(OBJS)
	@echo + LD $<
	@$(LD) -e test_main -o $@ $^ $(LDFLAGS) $(CFLGAS)
