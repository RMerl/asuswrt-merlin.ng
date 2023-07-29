#***********************************************************************
# 
# Makefile
#
#***********************************************************************
#OBJS = cosql_utils.o codb_utils.o log.o
#
#CFLAGS += -g -fPIC
#CFLAGS += -I$(TOP)/sqlite
#
#LDFLAGS += -L$(TOP)/sqlite/.libs -lsqlite3
#
#all: libcodb.so
#
#.c.o:
#    $(CC) -c $*.c $(CFLAGS)
#
#libcodb.so: $(OBJS)
#    $(CC) -shared $(OBJS) $(LDFLAGS) -o libcodb.so
#
#.PHONY: clean
#clean:
#    @echo LINUXDIR=$(LINUXDIR)
#    -rm -rf libcodb.so cosql_utils.o codb_utils.o $(OBJS)


CC = gcc

OBJS = main.o
LIB_OBJS = ipaddr.o

LIB_CFLAGS = -g -fPIC 
#CFLAGS += -DPDBG
LDFLAGS = -L. -lipaddr

all: ipaddr ipchker

.c.o:
	$(CC) -c $*.c $(CFLAGS) $(LIB_CFLAGS)

ipchker: $(OBJS)
	$(CC) -o ipchker $(OBJS) $(CFLAGS) $(LDFLAGS)

ipaddr: $(LIB_OBJS)
	$(CC) -shared $(LIB_OBJS) $(LIB_CFLAGS) -o libipaddr.so

.PHONY: clean
clean:
	-rm -rf ipchker libipaddr.so $(OBJS) $(LIB_OBJS)

