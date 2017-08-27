#***********************************************************************
#
#  Copyright (c) 2006-2007  Broadcom Corporation
#  All Rights Reserved
#
#***********************************************************************

LIB = libssp.so
OBJS = brcmssp_util.o

# treat all warnings as errors
CFLAGS += -Werror -Wfatal-errors


all: $(LIB)

clean:
	rm -f *.o $(LIB)

install:
	install -D $(LIB) $(INSTALLDIR)/lib/$(LIB)
	cd $(INSTALLDIR)/lib && rm -f libssp_nonshared.so && ln -s $(LIB) libssp_nonshared.so

#
# Implicit rule will make the .c into a .o
# Implicit rule is $(CC) -c $(CPPFLAGS) $(CFLAGS)
# See Section 10.2 of Gnu Make manual
# 
$(LIB): $(OBJS)
	$(CC) -shared $(LDFLAGS) -Wl,--whole-archive,-soname,$(notdir $@) -o $@ $(OBJS) -Wl,--no-whole-archive

