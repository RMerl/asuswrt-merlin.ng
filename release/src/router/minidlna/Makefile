# $Id: Makefile,v 1.31.2.2 2012/11/05 07:01:21 jmaggard Exp $
# MiniDLNA project
# http://sourceforge.net/projects/minidlna/
# (c) 2008-2009 Justin Maggard
# for use with GNU Make
# To install use :
# $ DESTDIR=/dummyinstalldir make install
# or :
# $ INSTALLPREFIX=/usr/local make install
# or :
# $ make install
#
#CFLAGS = -Wall -O -D_GNU_SOURCE -g -DDEBUG
#CFLAGS = -Wall -g -Os -D_GNU_SOURCE

ifeq ($(MSIPK),y)
IPKINC = $(shell pwd)/../APP-IPK/MediaServer-ipk/opt/include
TOP = $(shell pwd)/..
else
include ../common.mak
endif

LDFLAGS = -ffunction-sections -fdata-sections -Wl,--gc-sections

#CFLAGS = -Wall -g -O3 -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 \

ifneq ($(MSIPK),y)
CFLAGS += -Wall -g -Os -D_GNU_SOURCE $(EXTRACFLAGS) -D_FILE_OFFSET_BITS=64 \
	-ffunction-sections -fdata-sections \
	-I$(TOP)/flac/include -I$(TOP)/sqlite -I$(TOP)/jpeg \
	-I$(TOP)/libexif -I$(TOP)/libid3tag -I$(TOP)/libogg/include \
	-I$(TOP)/libvorbis/include \
	-I$(TOP)/ffmpeg \

ifneq ($(STATIC),1)
LDFLAGS += -L$(TOP)/zlib -L$(TOP)/sqlite/.libs -L$(TOP)/jpeg -L$(TOP)/libvorbis/lib/.libs \
	-L$(TOP)/libogg/src/.libs -L$(TOP)/libexif/libexif/.libs -L$(TOP)/flac/src/libFLAC/.libs \
	-L$(TOP)/ffmpeg/libavutil -L$(TOP)/ffmpeg/libavcodec -L$(TOP)/ffmpeg/libavformat \
	-L$(TOP)/libid3tag/.libs
ifeq ($(or $(RTAC68U),$(RTAX82U)),y)
LDFLAGS += -L$(TOP_PLATFORM)/nvram${BCMEX} -lnvram -L$(TOP)/shared -lshared
endif
else
LDFLAGS += -static
endif
else
ifeq ($(PLATFORM),i686)
EXTRACFLAGS = -DLINUX26 -DCONFIG_BCMWL5 -pipe -DBCMWPA2 -funit-at-a-time -Wno-pointer-sign \
-DLINUX_KERNEL_VERSION=132630 -DMS_IPK -DMS_I686
else
EXTRACFLAGS = -DLINUX26 -DCONFIG_BCMWL5 -pipe -DBCMWPA2 -funit-at-a-time -Wno-pointer-sign \
-mtune=mips32 -mips32 -DLINUX_KERNEL_VERSION=132630 -DMS_IPK
endif
CFLAGS = -Wall -g -Os -D_GNU_SOURCE $(EXTRACFLAGS) -D_FILE_OFFSET_BITS=64 \
	-ffunction-sections -fdata-sections \
	-I$(TOP)/ffmpeg-0.5/libavutil -I$(TOP)/ffmpeg-0.5/libavcodec -I$(TOP)/ffmpeg-0.5/libavformat \
	-I$(TOP)/ffmpeg-0.5/libswscale -I$(TOP)/ffmpeg-0.5 \
	-I$(TOP)/flac/include -I$(TOP)/sqlite -I$(TOP)/jpeg \
	-I$(TOP)/libexif -I$(TOP)/libid3tag -I$(TOP)/libogg/include \
	-I$(TOP)/libvorbis/include

LDFLAGS += -L$(TOP)/zlib -L$(TOP)/sqlite/.libs -L$(TOP)/jpeg/.libs -L$(TOP)/libvorbis/lib/.libs \
	-L$(TOP)/libogg/src/.libs -L$(TOP)/libexif/libexif/.libs -L$(TOP)/flac/src/libFLAC/.libs \
	-L$(TOP)/APP-IPK/MediaServer-ipk/asusffmpeg_1.0.0.5/opt/lib \
	-L$(TOP)/libid3tag/.libs
endif

#STATIC_LINKING: CFLAGS += -DSTATIC
#STATIC_LINKING: LDFLAGS = -static
RM = rm -f
INSTALL = install

ifeq ($(RTCONFIG_BCMARM),y)
CFLAGS += -I$(SRCBASE)/shared/bcmwifi/include -DTYPEDEF_FLOAT_T
endif

ifneq ($(MSIPK),y)
CFLAGS += -I$(TOP)/shared

ifeq ($(RTAC68U), y)
CFLAGS += -I$(SRCBASE)/include
endif

ifeq ($(RTN65U), y)
CFLAGS += -DRTN56U
endif

ifeq ($(RTAC66U), y)
CFLAGS += -DRTN66U
endif

ifeq ($(MS_LIMIT), 1)
CFLAGS += -DMS_LIMIT
endif
endif

ifeq ($(RPAC68U), y)
	CFLAGS += -DRPAC68U
endif

INSTALLPREFIX ?= $(DESTDIR)/usr
SBININSTALLDIR = $(INSTALLPREFIX)/sbin
ETCINSTALLDIR = $(DESTDIR)/etc

BASEOBJS = minidlna.o upnphttp.o upnpdescgen.o upnpsoap.o \
	   upnpreplyparse.o minixml.o \
	   getifaddr.o process.o upnpglobalvars.o \
	   options.o minissdp.o uuid.o upnpevents.o \
	   sql.o utils.o metadata.o scanner.o monitor.o \
	   tivo_utils.o tivo_beacon.o tivo_commands.o \
	   containers.o \
	   avahi.o tagutils/tagutils.o \
	   playlist.o image_utils.o albumart.o log.o clients.o

ALLOBJS = $(BASEOBJS) $(LNXOBJS)

#LIBS = -lpthread -lexif -ljpeg -lsqlite3 -lavformat -lavutil -lavcodec -lid3tag -lFLAC -logg -lvorbis
#STATIC_LINKING: LIBS = -lvorbis -logg -lm -lsqlite3 -lpthread -lexif -ljpeg -lFLAC -lm -lid3tag -lz -lavformat -lavutil -lavcodec -lm

ifneq ($(MSIPK),y)
ifeq ($(STATIC),1)
LIBS = -lpthread -ldl -lm \
	$(TOP)/libvorbis/lib/.libs/libvorbis.a \
	$(TOP)/libogg/src/.libs/libogg.a \
	$(TOP)/sqlite/.libs/libsqlite3.a \
	$(TOP)/libexif/libexif/.libs/libexif.a \
	$(TOP)/jpeg/libjpeg.a \
	$(TOP)/flac/src/libFLAC/.libs/libFLAC.a \
	$(TOP)/libid3tag/.libs/libid3tag.a \
	$(TOP)/zlib/libz.a \
	$(TOP)/ffmpeg/libavformat/libavformat.a \
	$(TOP)/ffmpeg/libavcodec/libavcodec.a \
	$(TOP)/ffmpeg/libavutil/libavutil.a
else
LIBS += -lpthread -ldl -lm -lvorbis -logg -lsqlite3 -lexif -ljpeg -lFLAC -lid3tag -lz -lavformat -lavcodec -lavutil
endif

else
LIBS += -lpthread -lm -lvorbis -logg -lsqlite3 -lexif -ljpeg -lFLAC -lid3tag_ms -lz \
	$(TOP)/libid3tag/.libs/libid3tag_ms.so \
	$(TOP)/APP-IPK/MediaServer-ipk/asusffmpeg_1.0.0.5/opt/lib/libavformat_asus.so \
	$(TOP)/APP-IPK/MediaServer-ipk/asusffmpeg_1.0.0.5/opt/lib/libavcodec_asus.so \
	$(TOP)/APP-IPK/MediaServer-ipk/asusffmpeg_1.0.0.5/opt/lib/libavutil_asus.so
ifeq ($(PLATFORM),i686)
LIBS += -lrt
endif
endif

TESTUPNPDESCGENOBJS = testupnpdescgen.o upnpdescgen.o

EXECUTABLES = minidlna testupnpdescgen

.PHONY:	all clean distclean install depend

all:	$(EXECUTABLES)

clean:
	$(RM) $(ALLOBJS)
	$(RM) $(EXECUTABLES)
	$(RM) testupnpdescgen.o
	$(RM) osver.h

distclean: clean
#	$(RM) config.h

install-conf:
	$(INSTALL) -d $(ETCINSTALLDIR)
	$(INSTALL) --mode=0644 minidlna.conf $(ETCINSTALLDIR)

install:	minidlna
	$(INSTALL) -D minidlna $(INSTALLDIR)/usr/sbin/minidlna
	$(STRIP) $(INSTALLDIR)/usr/sbin/minidlna

minidlna:	$(BASEOBJS) $(LNXOBJS)
	@echo Linking $@
	@$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(BASEOBJS) $(LNXOBJS) $(LIBS)


testupnpdescgen:	$(TESTUPNPDESCGENOBJS)
	@echo Linking $@
	@$(CC) $(CFLAGS) -o $@ $(TESTUPNPDESCGENOBJS)

config.h:	genconfig.sh
#	./genconfig.sh

depend:	config.h
	makedepend -f$(MAKEFILE_LIST) -Y \
	$(ALLOBJS:.o=.c) $(TESTUPNPDESCGENOBJS:.o=.c) 2>/dev/null

# DO NOT DELETE

minidlna.o: config.h upnpglobalvars.h minidlnatypes.h
minidlna.o: upnphttp.h upnpdescgen.h minidlnapath.h getifaddr.h upnpsoap.h
minidlna.o: options.h minissdp.h process.h upnpevents.h log.h
upnphttp.o: config.h upnphttp.h upnpdescgen.h minidlnapath.h upnpsoap.h
upnphttp.o: upnpevents.h image_utils.h sql.h log.h sendfile.h
ifeq ($(MSIPK),y)
upnphttp.o: icons_ipk.c
else
upnphttp.o: icons.c
endif
upnpdescgen.o: config.h upnpdescgen.h minidlnapath.h upnpglobalvars.h upnpdescstrings.h
upnpdescgen.o: minidlnatypes.h upnpdescstrings.h log.h
upnpsoap.o: config.h upnpglobalvars.h minidlnatypes.h log.h utils.h sql.h
upnpsoap.o: upnphttp.h upnpsoap.h upnpreplyparse.h getifaddr.h log.h
upnpreplyparse.o: upnpreplyparse.h minixml.h log.h
minixml.o: minixml.h
getifaddr.o: getifaddr.h log.h
process.o: process.h config.h log.h
upnpglobalvars.o: config.h upnpglobalvars.h upnpdescstrings.h
upnpglobalvars.o: minidlnatypes.h
options.o: options.h config.h upnpglobalvars.h
options.o: minidlnatypes.h
minissdp.o: config.h upnpdescstrings.h minidlnapath.h upnphttp.h
minissdp.o: upnpglobalvars.h minidlnatypes.h minissdp.h log.h
upnpevents.o: config.h upnpevents.h minidlnapath.h upnpglobalvars.h
upnpevents.o: minidlnatypes.h upnpdescgen.h log.h uuid.h
uuid.o: uuid.h
testupnpdescgen.o: config.h upnpdescgen.h
scanner.o: upnpglobalvars.h metadata.h utils.h sql.h scanner.h log.h playlist.h scanner_sqlite.h
metadata.o: upnpglobalvars.h metadata.h albumart.h utils.h sql.h log.h
albumart.o: upnpglobalvars.h albumart.h utils.h image_utils.h sql.h log.h
containers.o: containers.h
tagutils/tagutils.o: tagutils/tagutils-asf.c tagutils/tagutils-flc.c tagutils/tagutils-plist.c
tagutils/tagutils.o: tagutils/tagutils-aac.c tagutils/tagutils-asf.h tagutils/tagutils-flc.h tagutils/tagutils-mp3.c tagutils/tagutils-wav.c
tagutils/tagutils.o: tagutils/tagutils-ogg.c tagutils/tagutils-aac.h tagutils/tagutils.h tagutils/tagutils-mp3.h tagutils/tagutils-ogg.h log.h
playlist.o: playlist.h
inotify.o: inotify.h playlist.h
image_utils.o: image_utils.h
tivo_utils.o: config.h tivo_utils.h
tivo_beacon.o: config.h tivo_beacon.h tivo_utils.h
tivo_commands.o: config.h tivo_commands.h tivo_utils.h utils.h
utils.o: utils.h
sql.o: sql.h
log.o: log.h
clients.o: clients.h

.SUFFIXES: .c .o

.c.o:
	@echo Compiling $*.c
	@$(CC) $(CFLAGS) -o $@ -c $< && exit 0;\
		echo "The following command failed:" 1>&2;\
		echo "$(CC) $(CFLAGS) -o $@ -c $<";\
		$(CC) $(CFLAGS) -o $@ -c $< &>/dev/null
