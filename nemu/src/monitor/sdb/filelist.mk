SRCS-y += src/monitor/sdb/addrexp.tag.c src/monitor/sdb/addrexp.yy.c
INC_PATH += src/monitor/sdb
LFLAGS += -DYY_NO_UNPUT -DYY_NO_INPUT
