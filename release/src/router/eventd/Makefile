# Linux eventd Makefile

include $(TOP)/.config

include $(TOP)/.config

CFLAGS	+= -I. -I$(TOP)/shared -I$(SRCBASE)/include ${WLAN_StdIncPathA} -I$(SRCBASE)/common/include -Wall -Werror
CFLAGS	+= $(if $(WLAN_ComponentIncPath),$(WLAN_ComponentIncPath),$(addprefix -I,$(wildcard $(SRCBASE)/shared/bcmwifi/include)))
#CFLAGS	+= -g -DDEBUG
CFLAGS	+= -s -O2
LDFLAGS += -L$(TOP)/nvram${BCMEX}${EX7} -lnvram ${EXTRA_NV_LDFLAGS}
LDFLAGS += -L$(TOP)/shared -L$(INSTALLDIR)/shared/usr/lib -lshared
LDFLAGS += $(EXTRA_LDFLAGS)


vpath %.c $(SRCBASE)/shared/

ifeq ($(wildcard $(SRCBASE)/router/eventd/*.c),)
all:
	-cp -f prebuilt/eventd eventd
else
all: eventd
endif

OBJS = eventd_main.o

install: all
	install -d $(INSTALLDIR)/usr/sbin/
	install eventd $(INSTALLDIR)/usr/sbin/
	$(STRIP) $(INSTALLDIR)/usr/sbin/eventd

clean:
	rm -f *.o *.a *.so eventd

${OBJS}: eventd.h

eventd: bcmutils.o ${OBJS}
	$(CC) $(CFLAGS) $^ -o $@  $(LDFLAGS)
