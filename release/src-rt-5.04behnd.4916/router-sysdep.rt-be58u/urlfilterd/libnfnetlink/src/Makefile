LIB  = libnfnetlink.a
OBJS = iftable.o rtnl.o libnfnetlink.o 
CFLAGS += -I../include $(BRCM_WERROR_CFLAGS) -D_GNU_SOURCE

all: $(LIB)

clean: 
	rm -f $(LIB) *.o

$(LIB): $(OBJS)
	$(AR) -rcs $@ $(OBJS)
	mv $(LIB) ../
