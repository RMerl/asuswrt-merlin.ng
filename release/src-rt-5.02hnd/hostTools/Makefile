ifndef KERNEL_DIR
KERNEL_DIR = ../kernel/linux
endif
ifndef BRCM_BOARD
BRCM_BOARD = bcm963xx
endif
ifndef TARGETS_DIR
TARGETS_DIR = ../targets
endif
ifndef SHARED_DIR
SHARED_DIR = ../shared
endif
ifndef INC_BRCMBOARDPARMS_PATH
INC_BRCMBOARDPARMS_PATH = $(SHARED_DIR)/opensource/boardparms
endif

# host tools should not use cross compiler
unexport AR AS LD CC CPP CXX NM STRIP SSTRIP OBJCOPY OBJDUMP RANLIB

INCLUDEDIR = -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I.

CONFIG_SHELL := $(shell if [ -x "$$BASH" ]; then echo $$BASH; \
	else if [ -x /bin/bash ]; then echo /bin/bash; \
	else echo sh; fi ; fi)
TOPDIR	:= $(shell /bin/pwd)
SSHD_DIR = $(BUILD_DIR)/userspace/public/apps/dropbear/sshd
GENKEY_DIR = $(SSHD_DIR)/genkey
HOST_GENKEY = $(TOPDIR)/host_genkey
CC = gcc
STRIP = strip
RSA_KEY = $(TARGETS_DIR)/fs.src/etc/rsa_host_key
###DSS_KEY = $(TARGETS_DIR)/fs.src/etc/dss_host_key
export SSHD_DIR HOST_GENKEY CC STRIP

#WARNINGS= -Wall

#export DEFS		= -DDEBUG
CFLAGS_NOARCH	= $(DEFS) $(WARNINGS) -O2 -DGNU $(INCLUDEDIR)
CFLAGS		= $(CFLAGS_NOARCH) 
ifeq ($(strip $(BUILD_SQUASH_HIGH)),y)
	CFLAGS += -DBUILD_SQUASH_HIGH
endif
ifeq ($(strip $(BUILD_SQUASH_NORMAL)),y)
	CFLAGS += -DBUILD_SQUASH_NORMAL
endif
ifeq ($(strip $(BUILD_SQUASH_LOW)),y)
	CFLAGS += -DBUILD_SQUASH_LOW
endif
ifeq ($(strip $(BRCM_IKOS)),y)
	CFLAGS += -DCONFIG_BRCM_IKOS
endif
export CFLAGS


BUILD_SET = build_mkfs build_cmplzma build_lz4cmp build_imageutil fakeroot build_secbtutils perlmods scons build_cms2bbf


.PHONY: $(BUILD_SET) nvram_files
all: $(BUILD_SET)


build_imageutil: bcmImageBuilder version addvtoken BcmFsTag gencrc32 nvram_files

bcmImageBuilder: bcmImageBuilder.c
	$(CC) $(CFLAGS) -o bcmImageBuilder bcmImageBuilder.c
version:
	cp -f ../router/shared/version.h .
	cp -f ../router/shared/rtconfig.h .
ifneq ($(wildcard ./prebuilt/$(BUILD_NAME)/addvtoken),)
addvtoken:
	@-cp -f ./prebuilt/$(BUILD_NAME)/addvtoken .
else
addvtoken: addvtoken.c
	rm -rf addvtoken
	$(CC) $(CFLAGS) -o addvtoken addvtoken.c
endif
gencrc32: gencrc32.c
	$(CC) $(CFLAGS) -o gencrc32 gencrc32.c 

perlmods:
	make -C CBC
	make -C CPAN

scons:
	make -C scons

nvram_files:
	$(CC) -U__cplusplus $(CFLAGS) -E createimg_binary_convert.c  |sed -e 's/^#.*//' > nvram.h
	grep "for_script" nvram.h > nvram_defaults.h

build_mkfs: build_xz-5.0.3 mksquashfs

ifeq ($(strip $(BRCM_KERNEL_ROOTFS)),squashfs)
ifneq ($(strip $(BRCM_KERNEL_AUXFS_JFFS2)),)
build_mkfs: mkjffs2
endif
endif

ifeq ($(strip $(BRCM_KERNEL_ROOTFS)),all)
build_mkfs: mkjffs2 build_mtdutils
endif

SQUASHFS_VER="4.2"
SQUASHFS_DIR=$(TOPDIR)/squashfs_4.2
XZ_DIR=$(TOPDIR)/xz-5.0.3
SECBT_DIR=$(TOPDIR)/SecureBootUtils
export LzmaAlone=$(TOPDIR)/lzma457/CPP/7zip/Compress/LZMA_Alone

JFFS2_DIR = $(TOPDIR)/jffs2
MTD_UTILS_DIR = $(HOSTTOOLS_DIR)/mtd-utils-20090606

LZ4_DIR = $(TOPDIR)/lz4-r127

untar_lzma457:
	if [ ! -e lzma457/lzma.txt ]; then \
	echo Untarring lzma 4.57 source...; \
	(tar xkfj lzma457.tar.bz2 2> /dev/null || true); \
	fi

mklzma_cmd: untar_lzma457
	@echo building lzma host tool ...
	$(MAKE) -C $(LzmaAlone) -f makefile.gcc
	cp -f $(LzmaAlone)/lzma lzmacmd

config_xz-5.0.3:
	if [ ! -e xz-5.0.3 ]; then \
	echo Untarring xz-5.0.3 source...; \
	(tar xkjf xz-5.0.3.tar.bz2 2> /dev/null || true); \
	fi 
	if [ ! -e xz-5.0.3/Makefile ]; then \
	(cd xz-5.0.3; ./configure --enable-shared=no --disable-assembler) ;\
	fi 

build_xz-5.0.3: config_xz-5.0.3
	echo Building xz-5.0.3 ...
	$(MAKE) -C $(XZ_DIR) -f Makefile

build_secbtutils:
	@echo Building secure boot utils ...
	make -C $(SECBT_DIR) -f Makefile

mksquashfs: build_xz-5.0.3
	@echo building mksqushfs host tool ...
	$(MAKE) -C $(SQUASHFS_DIR)
	mv $(SQUASHFS_DIR)/mksquashfs mksquashfs

mkcramfs:  cramfs/mkcramfs.cpp $(LZMAOBJS)
	@if [ ! -e /usr/include/zlib.h ]; then \
          echo *****You need to install zlib-devel package to build mkcramfs! ; \
          echo *****You may find it in your Linux distribution CDs. ; \
	  exit ;\
        fi
	g++ $(CFLAGS) -I$(KERNEL_DIR)/include -c cramfs/mkcramfs.cpp
	g++  -o mkcramfs mkcramfs.o $(LZMAOBJS) -lz -lm

mkjffs2:
	@echo building mkjffs2 host tool ...
	$(MAKE) -C $(JFFS2_DIR)
	mv jffs2/mkfs.jffs2 .

build_cmplzma: mklzma_cmd cmplzma.cpp
	g++ $(CFLAGS) -c cmplzma.cpp
	g++  -o cmplzma cmplzma.o -lm

untar_lz4hc: 
	if [ ! -e $(LZ4_DIR) ]; then \
	echo Untarring lz4-r127 source...; \
	(tar xzf r127.tar.gz 2> /dev/null || true); \
	fi

build_lz4cmp: untar_lz4hc lz4comp.c

	gcc lz4comp.c $(LZ4_DIR)/lib/lz4hc.c -o lz4cmp.out -I $(LZ4_DIR)/lib 
	chmod +x lz4cmp.out

build_mtdutils:
	@if [ ! -e $(MTD_UTILS_DIR) -o mtd-utils-20090606.patch -nt $(MTD_UTILS_DIR)/.tar_complete -o mtd-utils_20090606.orig.tar.gz -nt $(MTD_UTILS_DIR)/.tar_complete ]; then \
	rm -rf $(MTD_UTILS_DIR); \
	echo Untarring mtd-utils source...; \
	tar xf mtd-utils_20090606.orig.tar.gz 2> /dev/null || exit 1; \
	patch -p1 -s < mtd-utils-20090606.patch || exit 1; \
	touch $(MTD_UTILS_DIR)/.tar_complete; \
	fi
	@echo building mtd-utils host tool ...
	make -C $(MTD_UTILS_DIR) BUILDDIR=$(MTD_UTILS_DIR) WITHOUT_XATTR=1

build_pinmuxcheck:
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63138_ _63381_63148_6848_4908_6858_),)
	@echo building PinMuxCheck ...
	make -C PinMuxCheck
	./PinMuxCheck/bptest || exit 1
endif

build_cms2bbf:
	make -C cms2bbf


$(LZMAOBJS): %.o: %.cpp
	g++ -c $(CFLAGS) $^ -o $@

export FAKEROOT_DIR = $(TOPDIR)/fakeroot
export FAKEROOT_TAR = fakeroot_1.20.2.orig.tar.bz2

$(FAKEROOT_DIR)/Makefile:
	mkdir -p $(FAKEROOT_DIR)
	(tar -jxf $(FAKEROOT_TAR) --strip 1 -C $(FAKEROOT_DIR) 2> /dev/null || true)
	(patch -p1 -s -d $(FAKEROOT_DIR) < fakeroot_1.20.2.patch || true)
	cd $(FAKEROOT_DIR); CFLAGS="$(CFLAGS_NOARCH)" ac_cv_func_capset=0 ./configure --prefix=$(HOSTTOOLS_DIR)/local_install --bindir=$(HOSTTOOLS_DIR)/local_install

fakeroot: $(FAKEROOT_DIR)/Makefile
	cd $(FAKEROOT_DIR); make; make install


######################################################################
#
# This section deals with doxygen
#
# Note that if you want to change the settings in the config file,
# (Doxyfile), you should do it in this makefile.
# Any modifications to the config file itself will be
# wiped out next time you make the build_doxygen target.
#
######################################################################

doxygen:
	tar -jxf doxygen.tar.bz2
	cd doxygen; ./configure


build_doxygen: doxygen
	cd doxygen; make; \
	cd bin; rm -f Doxyfile; \
	./doxygen -g > /dev/null; \
	sed -i 's,\(PROJECT_NAME[ \t]*=\),\1 CMS,' Doxyfile; \
	sed -i 's,\(OUTPUT_DIRECTORY[ \t]*=\),\1 $(BUILD_DIR)/docs/doxygen,' Doxyfile; \
	sed -i 's,\(JAVADOC_AUTOBRIEF[ \t]*=\)[ \tA-Z]*,\1 YES,' Doxyfile; \
	sed -i 's,\(OPTIMIZE_OUTPUT_FOR_C[ \t]*=\)[ \tA-Z]*,\1 YES,' Doxyfile; \
	sed -i 's,\(WARN_NO_PARAMDOC[ \t]*=\)[ \tA-Z]*,\1 YES,' Doxyfile; \
	sed -i 's,\(ENUM_VALUES_PER_LINE[ \t]*=\)[ \t0-9]*,\1 1,' Doxyfile; \
	sed -i 's,\(INPUT[ \t]*=\),\1 $(BUILD_DIR)/userspace/public/include,' Doxyfile; \
	sed -i 's,\(INPUT[ \t]*=\),\1 $(BUILD_DIR)/userspace/private/include,' Doxyfile; \
	sed -i 's,\(INPUT[ \t]*=\),\1 $(BUILD_DIR)/userspace/private/libs/cms_core,' Doxyfile; \
	sed -i 's,\(RECURSIVE[ \t]*=\)[ \tA-Z]*,\1 YES,' Doxyfile; \
	sed -i 's,\(GENERATE_LATEX[ \t]*=\)[ \tA-Z]*,\1 NO,' Doxyfile; \
	sed -i 's,\(PREDEFINED[ \t]*=\)[ \tA-Z]*,\1 MDM_SHARED_MEM CMS_MEM_DEBUG,' Doxyfile;

# if we want to generate documentation for the unittests 
#sed -i 's,\(INPUT[ \t]*=\),\1 $(BUILD_DIR)/unittests,' Doxyfile; 

######################################################################
#
# Cleaning rules
#
######################################################################

clean:
	rm -f bcmImageBuilder addvtoken version.h mksquashfs mkcramfs mkfs.jffs2 lzmacmd cmplzma $(HOST_GENKEY) BcmFsTag gencrc32 nvram.h nvram_defaults.h
	rm -fr *.o core
	rm -fr $(GENKEY_DIR)/*.o
	make -C $(SQUASHFS_DIR) clean
	make -C $(JFFS2_DIR) clean
	if [ -e $(XZ_DIR)/Makefile ]; then make -C $(XZ_DIR) distclean; fi
	make -C $(SECBT_DIR) clean
	if [ -e $(LzmaAlone)/makefile.gcc ]; then make -C $(LzmaAlone) -f makefile.gcc clean; fi
	if [ -e $(FAKEROOT_DIR)/Makefile ];  then make -C $(FAKEROOT_DIR) distclean; fi
	rm -f $(FAKEROOT_DIR)/fakeroot
	find $(FAKEROOT_DIR) -maxdepth 1 -mindepth 1 ! -name 'configure' -exec rm -r {} \;
	make -C libcreduction clean
	if [ -e $(MTD_UTILS_DIR) ];  then make -C $(MTD_UTILS_DIR) BUILDDIR=$(MTD_UTILS_DIR) clean; fi
	make -C PinMuxCheck clean
	if [ -e lz4-r127 ];then rm -rf lz4-r127; fi
	if [ -e lz4cmp.out ];  then rm lz4cmp.out; fi
	if [ -e libelf-0.8.13 ]; then rm -rf libelf-0.8.13 ; fi
	if [ -e prelink ]; then rm -rf prelink; fi

clean_doxygen:
	rm -fr doxygen

export LD_LIBRARY_PATH := $(HOSTTOOLS_DIR)/libelf-0.8.13/prefix/lib/:$(LD_LIBRARY_PATH)

$(HOSTTOOLS_DIR)/prelink/src/execstack:
	tar -xzpf libelf-0.8.13.tar.gz && \
	cd libelf-0.8.13 && mkdir -p $(HOSTTOOLS_DIR)/libelf-0.8.13/prefix \
	&& ./configure --prefix=$(HOSTTOOLS_DIR)/libelf-0.8.13/prefix && \
	make all install;
	tar -xpjf prelink.tar.bz2 && \
	cd prelink &&  \
	LD_LIBRARY_PATH=$(HOSTTOOLS_DIR)/libelf-0.8.13/prefix/lib/ LDFLAGS="-L$(HOSTTOOLS_DIR)/libelf-0.8.13/prefix/lib/" CPPFLAGS="-I$(HOSTTOOLS_DIR)/libelf-0.8.13/prefix/include -I$(HOSTTOOLS_DIR)/libelf-0.8.13/prefix/include/libelf -I$(HOSTTOOLS_DIR)/libelf-0.8.13/prefix/" CFLAGS="-I$(HOSTTOOLS_DIR)/libelf-0.8.13/prefix/include -I$(HOSTTOOLS_DIR)/libelf-0.8.13/prefix/include/libelf" ./configure --enable-static  && cd src && make execstack


build_execstack: $(HOSTTOOLS_DIR)/prelink/src/execstack

