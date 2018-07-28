include ../common.mak
include $(SRCBASE)/.config

OBJS = usb_info.o disk_io_tools.o disk_initial.o disk_share.o
PROGS = libdisk.so write_smb_conf test_of_var_files
ifneq ($(BUILD_NAME),RT-N10U)
PROGS += test_disk1 test_disk2
endif
PROGS += test_share
#PROGS += test_share2
ifeq ($(RTCONFIG_WEBDAV),y)
PROGS += write_webdav_conf
endif

ifeq ($(BCM_MMC),y)
CFLAGS += -DBCM_MMC
endif
CFLAGS += -fPIC $(EXTRACFLAGS)
CFLAGS += -I. -I$(TOP)/shared -I$(SRCBASE)/include -I$(KERNEL_HEADER_DIR) -Wall
#CFLAGS += $(PRIVATE_EXTRACFLAGS)
LDFLAGS_SO = -L$(TOP)/nvram${BCMEX}${EX7} -lnvram -L$(TOP)/shared -lshared
ifeq ($(HND_ROUTER),y)
LDFLAGS_SO += -L$(TOP)/wlcsm -lwlcsm
LDFLAGS_SO += -lgcc_s
endif
LDFLAGS += $(LDFLAGS_SO) -L. -ldisk
ifeq ($(RTCONFIG_BCMARM),y)
CFLAGS += -I$(SRCBASE)/shared/bcmwifi/include -DTYPEDEF_FLOAT_T
CFLAGS += -I$(SRCBASE)/common/include
LDFLAGS += -lgcc_s
endif

ifeq ($(RTCONFIG_QTN),y)
LDFLAGS += -L$(TOP)/libqcsapi_client -lqcsapi_client
endif

ifeq ($(RTCONFIG_QCA),y)
LDFLAGS += -ldl
endif

ifeq ($(RTCONFIG_PERMISSION_MANAGEMENT),y)
CFLAGS += -I$(TOP)/sqlCipher
CFLAGS += -I$(TOP)/PMS_DBapis
LDFLAGS += -L$(TOP)/openssl -lcrypto
LDFLAGS += -L$(TOP)/sqlCipher/.libs -lsqlcipher
LDFLAGS += -L$(TOP)/PMS_DBapis -lpms_sql
endif

#SHARE_STATIC=y

all: $(PROGS)

#libdisk.so: $(OBJS) $(TOP)/shared/shutils.o
libdisk.so: $(OBJS)
ifeq ($(HND_ROUTER),y)
	$(CC) $(LDFLAGS_SO) -shared -o $@ $^
else
	$(LD) $(LDFLAGS_SO) -shared -o $@ $^
endif

write_smb_conf: write_smb_conf.o libdisk.so
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

ifeq ($(RTCONFIG_WEBDAV),y)
write_webdav_conf: write_webdav_conf.o libdisk.so
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
endif

test_of_var_files: libdisk.so test_share
	ln -sf test_share test_of_var_files

test_disk1: test_disk1.o libdisk.so
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

test_disk2: test_disk2.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

ifeq ($(SHARE_STATIC),y)
test_share: test_share.o disk_share.o
	@echo Got Static.
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
else
test_share: test_share.o libdisk.so
	@echo Got lib.
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
endif

infos:
	@echo INSTALLDIR=$(INSTALLDIR)
	install -D libdisk.so $(INSTALLDIR)/lib/libdisk.so

install: all
	$(STRIP) $(PROGS)
ifneq ($(BUILD_NAME),RT-N10U)
	install -D test_disk1 $(INSTALLDIR)/sbin/test_disk1
	install -D test_disk2 $(INSTALLDIR)/sbin/test_disk2
endif
	install -D test_share $(INSTALLDIR)/sbin/test_share

	install -D libdisk.so $(INSTALLDIR)/lib/libdisk.so
	install -D write_smb_conf $(INSTALLDIR)/sbin/write_smb_conf
ifeq ($(RTCONFIG_WEBDAV), y)
	install -D write_webdav_conf $(INSTALLDIR)/sbin/write_webdav_conf
endif
	cd $(INSTALLDIR)/sbin && ln -sf test_share test_of_var_files

	# test
	cd $(INSTALLDIR)/sbin && ln -sf test_share get_folder_list
	cd $(INSTALLDIR)/sbin && ln -sf test_share get_all_folder
	cd $(INSTALLDIR)/sbin && ln -sf test_share get_var_file_name

	cd $(INSTALLDIR)/sbin && ln -sf test_share initial_folder_list
	cd $(INSTALLDIR)/sbin && ln -sf test_share initial_var_file
	cd $(INSTALLDIR)/sbin && ln -sf test_share initial_all_var_file
	cd $(INSTALLDIR)/sbin && ln -sf test_share test_of_var_files
	cd $(INSTALLDIR)/sbin && ln -sf test_share create_if_no_var_files
	cd $(INSTALLDIR)/sbin && ln -sf test_share modify_if_exist_new_folder

	cd $(INSTALLDIR)/sbin && ln -sf test_share get_permission
	cd $(INSTALLDIR)/sbin && ln -sf test_share set_permission

	cd $(INSTALLDIR)/sbin && ln -sf test_share add_account
	cd $(INSTALLDIR)/sbin && ln -sf test_share del_account
	cd $(INSTALLDIR)/sbin && ln -sf test_share mod_account
	cd $(INSTALLDIR)/sbin && ln -sf test_share test_if_exist_account
ifeq ($(RTCONFIG_PERMISSION_MANAGEMENT),y)
	cd $(INSTALLDIR)/sbin && ln -sf test_share add_group
	cd $(INSTALLDIR)/sbin && ln -sf test_share del_group
	cd $(INSTALLDIR)/sbin && ln -sf test_share mod_group
	cd $(INSTALLDIR)/sbin && ln -sf test_share test_if_exist_group
else
	cd $(INSTALLDIR)/sbin && ln -sf test_share get_account_list
endif

	cd $(INSTALLDIR)/sbin && ln -sf test_share add_folder
	cd $(INSTALLDIR)/sbin && ln -sf test_share del_folder
	cd $(INSTALLDIR)/sbin && ln -sf test_share mod_folder
	cd $(INSTALLDIR)/sbin && ln -sf test_share test_if_exist_share

	cd $(INSTALLDIR)/sbin && ln -sf test_share how_many_layer

clean:
	rm -f *.o $(PROGS) write_webdav_conf
