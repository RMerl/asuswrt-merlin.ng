SRCROOT = ..
VERSION = $(shell cat ../version)

-include ../MCONFIG
include ../MRULES

OBJS = tftpsubs.$(O)
LIB  = libcommon.a

all: $(LIB)

$(LIB): $(OBJS)
	-rm -f $(LIB)
	$(AR) $(LIB) $(OBJS)
	$(RANLIB) $(LIB)

$(OBJS): tftpsubs.h

install:

clean:
	rm -f *.o *.obj *.exe $(LIB)

distclean: clean
	rm -f *~
