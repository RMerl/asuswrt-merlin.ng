PREFIX   = /usr/local
LIBDIR   = $(PREFIX)/lib
DESTDIR  = 
CFLAGS   = -Os -Wall -Werror --std=gnu99 -g3 -Wmissing-declarations $(EXTRACFLAGS)
CFLAGS  += -ffunction-sections -fdata-sections
LDFLAGS  = $(EXTRALDFLAGS)
LDFLAGS += -Wl,--gc-sections
LIBS     = -Wl,--as-needed -lrt

ifneq ($(JSON),)
CFLAGS += -I$(JSON)
JSON_LIB := -L$(JSON) -ljson
else ifneq ($(JSONC),)
CFLAGS += -DJSONC -I$(JSONC)
JSON_LIB := -L$(JSONC)/.libs -ljson-c
endif

TARGETS = libubox.a libubox.so
libubox_SRCS := avl.c avl-cmp.c blob.c blobmsg.c uloop.c usock.c ustream.c ustream-fd.c vlist.c utils.c safe_list.c runqueue.c md5.c kvlist.c ulog.c base64.c
libubox_LIBS :=

ifneq ($(JSON_LIB),)
TARGETS += libblobmsg_json.a libblobmsg_json.so
libblobmsg_json_SRCS := blobmsg_json.c
libblobmsg_json_LIBS := -L. -lubox $(JSON_LIB)

TARGETS += json_script.a json_script.so
json_script_SRCS := json_script.c
json_script_LIBS := -L. -lubox
endif

all: $(TARGETS)

clean:
	rm -f *.o *.a *.so $(TARGETS)

install: all install-common
	$(STRIP) $(DESTDIR)$(LIBDIR)/*

install-common:
	$(INSTALL) -d $(DESTDIR)$(LIBDIR)
	for i in $(TARGETS); do \
		$(INSTALL) -m 755 $i $(DESTDIR)$(LIBDIR) \
	done

.SECONDARY:
%.o: %.c *.h Makefile
	@echo "[CC] $@"
	@$(CC) $(CFLAGS) -fPIC -o $@ -c $<

.SECONDEXPANSION:
%.a: OBJS = $($*_SRCS:.c=.o)
%.a: $$(OBJS)
	@echo "[AR] $@"
	@$(AR) cru $@ $^
	@$(RANLIB) $@

.SECONDEXPANSION:
%.so: OBJS = $($*_SRCS:.c=.o)
%.so: $$(OBJS)
	@echo "[LD] $@"
	@$(CC) -shared $(LDFLAGS) -o $@ $^ $($*_LIBS) $(LIBS)

.PHONY : all clean install install-common dummy
