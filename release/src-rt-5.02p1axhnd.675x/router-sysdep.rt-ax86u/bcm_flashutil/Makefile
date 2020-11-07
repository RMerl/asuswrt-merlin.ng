LIB := libbcm_flashutil.so

OBJS := bcm_flashutil.o bcm_flashutil_nand.o \
        bcm_imgutil.o bcm_imgutil_api.o bcm_imgcombo.o bcm_imgif.o
ifeq ($(strip $(DESKTOP_LINUX)),)
OBJS += bcm_flashutil_emmc.o  bcm_imgif_nand.o bcm_imgif_emmc.o 
endif

CFLAGS += -Werror -Wfatal-errors -Wno-attributes -I$(HND_SRC)/router-sysdep/bcm_boardctl/ -I$(HND_SRC)/userspace/public/include/ -I$(HND_SRC)/userspace/public/include/linux/ -I$(HND_SRC)/router-sysdep/bcm_util/ -I$(HND_SRC)/router-sysdep/gen_util/ -I$(INC_BRCMSHARED_PUB_PATH)/../flash -I$(INC_BRCMSHARED_PUB_PATH)/../include/bcm963xx/

CFLAGS += -DCHIP_FAMILY_ID_HEX=0x$(BRCM_CHIP)

# These are public API headers needed by other apps which want to call this lib.
# Private/internal headers should not be listed here.
HEADERS := bcm_flashutil.h


all: $(LIB)

LIB_INSTALL_DIR       := $(BCM_FSBUILD_DIR)/public/lib
HEADER_INSTALL_DIR    := $(BCM_FSBUILD_DIR)/public/include

install: all
	mkdir -p $(LIB_INSTALL_DIR)
	install -p -t $(LIB_INSTALL_DIR) $(LIB)
	mkdir -p $(INSTALLDIR)/lib
	install -t $(INSTALLDIR)/lib $(LIB)
	mkdir -p $(HEADER_INSTALL_DIR)
	install -p -m 444 -t $(HEADER_INSTALL_DIR) $(addprefix $(mdir)/,$(HEADERS))
ifneq ($(strip $(BRCM_NO_AUTOCOMMIT_IMAGE)),)
	@echo "INSTALLING COMMIT AFTER REBOOT SCRIPT (first stage build)"
	mkdir -p $(ETC_INSTALL_DIR)/init.d
	mkdir -p $(ETC_INSTALL_DIR)/rc3.d
	install -p -m 0755 -t $(ETC_INSTALL_DIR)/init.d $(mdir)/commit.sh
	(cd $(ETC_INSTALL_DIR)/rc3.d; rm -f S68commit; ln -s ../init.d/commit.sh S68commit)
endif

clean:
	rm -f *.o *.d $(LIB)
	rm -f $(LIB_INSTALL_DIR)/$(LIB)
	rm -f $(addprefix $(HEADER_INSTALL_DIR)/,$(HEADERS))
	rm -f $(ETC_INSTALL_DIR)/rc3.d/S68commit
	rm -f $(ETC_INSTALL_DIR)/init.d/commit.sh

$(LIB): $(OBJS)
	$(CC) -shared $(BCM_LD_FLAGS) -Wl,--whole-archive,-soname,$(notdir $@) \
	      -o $@ $(OBJS) -Wl,--no-whole-archive


# Set VPATH because we want to compile in a seperate dir than source.
name := $(lastword $(MAKEFILE_LIST))
mdir := $(realpath $(dir $(name)))
VPATH=$(mdir)

# Generate and use dependencies.
CFLAGS += -MD
-include $(OBJS:%.o=%.d)
