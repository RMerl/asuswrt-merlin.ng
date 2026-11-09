PREFIX   = /usr/local
SBINDIR  = $(PREFIX)/sbin
DESTDIR  = 
CFLAGS   = -Os -ggdb -Wall -Werror --std=gnu99 -Wmissing-declarations $(EXTRACFLAGS) # -Wno-enum-conversion 
CFLAGS  += -ffunction-sections -fdata-sections
LDFLAGS  = $(EXTRALDFLAGS)
LDFLAGS += -Wl,--gc-sections

ifneq ($(DEBUG_PACKET),)
CFLAGS += -DDEBUG_PACKET
endif

ifneq ($(DEBUG),)
CFLAGS += -DDEBUG -g3
endif

CFLAGS += -I$(LIBUBOX)
LIBS += -L$(LIBUBOX) -lubox -lblobmsg_json
ifneq ($(JSON),)
CFLAGS += -I$(JSON)
LIBS += -L$(JSON) -ljson
else ifneq ($(JSONC),)
CFLAGS += -DJSONC -I$(JSONC)
LIBS += -L$(JSONC)/.libs -ljson-c
endif

TARGETS = uqmi
SOURCES = main.c dev.c commands.c qmi-message.c mbim.c

SERVICES := ctl dms nas pds wds wms wda uim
service_sources = $(foreach service,$(SERVICES),qmi-message-$(service).c)
service_headers = $(service_sources:.c=.h)
SOURCES += $(service_sources)

all: uqmi

clean:
	rm -f *.o $(TARGETS) $(service_sources) $(service_headers) qmi-errors.c

install: all install-common
	$(STRIP) $(DESTDIR)$(SBINDIR)/*

install-common:
	$(INSTALL) -d $(DESTDIR)$(SBINDIR)
	$(INSTALL) -m 755 uqmi $(DESTDIR)$(SBINDIR)

gen-headers: $(service_headers)
	@true
.PHONY: gen-headers

qmi-message-%.h: data/qmi-service-%.json data/gen-header.pl data/gen-common.pm
	@data/gen-header.pl $*_ $< > $@

qmi-message-%.c: data/qmi-service-%.json data/gen-code.pl data/gen-common.pm
	@data/gen-code.pl $*_ $< > $@

qmi-message-%.o: CFLAGS += -Wno-unused

gen-errors: qmi-errors.c
	@true
.PHONY: gen-errors

qmi-errors.c: qmi-errors.h data/gen-error-list.pl
	@data/gen-error-list.pl $< > $@

uqmi: gen-headers gen-errors $(SOURCES:.c=.o)
	@echo "[LD] $@"
	@$(CC) $(LDFLAGS) -o $@ $(SOURCES:.c=.o) $(LIBS)

.SECONDARY:
%.o: %.c *.h Makefile
	@echo "[CC] $@"
	@$(CC) $(CFLAGS) -o $@ -c $<

.PHONY : all clean install install-common dummy
