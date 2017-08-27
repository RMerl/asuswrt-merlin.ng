EXEC = mt-daapd
OBJS = configfile.o main.o mp3-scanner.o query.o \
       strcasestr.o daap.o dynamic-art.o mDNS.o \
       redblack.o strsep.o daap-proto.o err.o mDNSPosix.o \
       parser.o rend-unix.o rend-posix.o uici.o lexer.o mDNSUNP.o\
       playlist.o restart.o webserver.o db-gdbm.o

CFLAGS += -DWITH_GDBM 
CFLAGS += -DNOT_HAVE_SA_LEN -DSETPGRP_VOID -DVERSION=\"0.2.4.2\" -DPACKAGE=\"mt-daapd\" -I./

ifneq ($(MSIPK),y)
CFLAGS += -I$(TOP)/libgdbm -I$(TOP)/libid3tag -I$(TOP)/zlib
ifeq ($(HND_ROUTER),y)
CFLAGS := $(shell echo $(CFLAGS)| sed 's/-O2//g')
CFLAGS += -DHAVE_CONFIG_H
endif
else
CFLAGS += -I$(SRCDIR)/libgdbm -I$(SRCDIR)/libid3tag -I$(SRCDIR)/zlib
endif


LDLIBS = -lpthread -ldl

ifneq ($(MSIPK),y)
ifeq ($(STATIC),1)
LDLIBS += $(TOP)/libgdbm/.libs/libgdbm.a $(TOP)/libid3tag/.libs/libid3tag.a $(TOP)/zlib/libz.a
else
LDLIBS += -L$(TOP)/zlib -lz -L$(TOP)/libgdbm/.libs/ -lgdbm -L$(TOP)/libid3tag/.libs/ -lid3tag
endif
else
LDLIBS += -L$(SRCDIR)/libid3tag/.libs/ -lid3tag_ms -L$(SRCDIR)/libgdbm/.libs/ -lgdbm -L$(SRCDIR)/zlib -lz
LDFLAGS = -L$(CURRENT)/opt/lib -Wl,-rpath,/opt/lib -Wl,-rpath-link,$(CURRENT)/opt/lib
endif

all: $(OBJS)
	$(CC) -o $(EXEC) $(OBJS) $(LDFLAGS) $(LDLIBS)

romfs:
	$(STRIP) $(EXEC) 
	install -d $(INSTALLDIR)/usr/sbin
	install $(EXEC) $(INSTALLDIR)/usr/sbin

clean:
	rm -f $(EXEC) *.o
