include ../Config

ifeq ($(IP_CONFIG_SETNS),y)
	CFLAGS += -DHAVE_SETNS
endif

# brcm: comment out.
# CFLAGS += -fPIC

UTILOBJ = utils.o rt_names.o ll_types.o ll_proto.o ll_addr.o \
	inet_proto.o namespace.o json_writer.o \
	names.o color.o

NLOBJ=libgenl.o ll_map.o libnetlink.o

# brcm: rename libutil.a to libipr2util.a
all: libnetlink.a libipr2util.a

libnetlink.a: $(NLOBJ)
	$(AR) rcs $@ $(NLOBJ)

libipr2util.a: $(UTILOBJ) $(ADDLIB)
	$(AR) rcs $@ $(UTILOBJ) $(ADDLIB)

install:

clean:
	rm -f $(NLOBJ) $(UTILOBJ) $(ADDLIB) libnetlink.a libipr2util.a

