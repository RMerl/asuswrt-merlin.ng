include ../common.mak

ifeq ($(CONFIG_REALTEK),y)
CFLAGS += -DREALTEK
else ifneq ($(or $(findstring linux-3.10,$(LINUXDIR))),)
ifeq ($(CONFIG_ALPINE),y)
CFLAGS += -Wall -Os $(EXTRACFLAGS) -I$(SRCBASE) -I$(SRCBASE)/include -I$(LINUXDIR)/include -I$(KDIR)/include -I.
else ifeq ($(CONFIG_LANTIQ),y)
CFLAGS += -Wall -Os $(EXTRACFLAGS) -I$(SRCBASE) -I$(SRCBASE)/include -I$(LINUXDIR)/include -I$(KDIR)/include -I.
else
CFLAGS += -Wall -Os $(EXTRACFLAGS) -I$(SRCBASE) -I$(SRCBASE)/include -I.
endif
else
CFLAGS += -Wall -Os $(EXTRACFLAGS) -I$(SRCBASE) -I$(SRCBASE)/include $(if $(CONFIG_LINUX3_14),-I$(KDIR)/include,-I$(LINUXDIR)/include) $(if $(CONFIG_BCMWL5),-I$(LINUXDIR)/arch/arm/include -I$(LINUXDIR)/arch/arm/plat-brcm/include) -I.
endif

ifeq ($(CONFIG_LINUX26),y)
CFLAGS += -DSCSI_SG
endif
ifeq ($(CONFIG_ALPINE),y)
CFLAGS += -DRTCONFIG_ALPINE
endif
ifeq ($(CONFIG_LANTIQ),y)
CFLAGS += -DRTCONFIG_LANTIQ
endif


TARGETS = scsi-start

all: $(TARGETS)

scsi-start:	scsi-start.c
	@$(CC) -o $@ $^ $(CFLAGS)

scsi-idle:	scsi-idle.c
	@$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f *.o $(TARGETS) *~

install:	all
	@install -d $(INSTALLDIR)/usr/sbin
	@install -D scsi-start $(INSTALLDIR)/usr/sbin/scsi-start
	@$(STRIP) -s $(INSTALLDIR)/usr/sbin/scsi-start

	# scsi-stop is a symbolic link to scsi-start
	cd $(INSTALLDIR)/usr/sbin && \
		ln -sf scsi-start scsi-stop
