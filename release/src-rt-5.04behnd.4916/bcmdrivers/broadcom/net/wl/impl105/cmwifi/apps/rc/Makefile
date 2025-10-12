#***********************************************************************
#
#  Copyright (c) 2006  Broadcom Corporation
#  All Rights Reserved
#
#  <:license-private
#
#***********************************************************************/
CFLAGS += -Werror $(USR_CFLAGS)

ifneq ($(strip $(BCA_CPEROUTER)),)
LDFLAGS += $(CMWIFI_RC_LDFLAGS)
endif

EXECUTABLE := wifi_setup

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

OBJS = main.o

all: $(OBJS)
	$(CC) -o $(EXECUTABLE) main.o $(LDFLAGS)

install: all
	install -m 755 $(EXECUTABLE) $(INSTALL_DIR)/sbin
	$(STRIP) $(INSTALL_DIR)/sbin/$(EXECUTABLE)

	# create symbolic link wifi_setup.sh for backward compability
	# better to be removed to avoid confusion
	ln -sf $(EXECUTABLE) $(INSTALL_DIR)/sbin/wifi_setup.sh
	ln -sf $(EXECUTABLE) $(INSTALL_DIR)/sbin/rc
	ln -sf $(EXECUTABLE) $(INSTALL_DIR)/sbin/erase

clean:
	rm -f *.o $(EXECUTABLE)
