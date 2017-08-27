TARGET		:= rlm_sql.a
SOURCES		:= rlm_sql.c sql.c

SRC_CFLAGS	:= $(rlm_sql_CFLAGS)
TGT_LDLIBS	:= $(rlm_sql_LDLIBS)
