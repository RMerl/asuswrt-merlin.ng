# Makefile for systems with GNU tools
include ../common.mak

#CC 	=	gcc
#INSTALL	=	install
IFLAGS  = -idirafter dummyinc
#CFLAGS = -g
#CFLAGS	=	-O2 -fPIE -fstack-protector --param=ssp-buffer-size=4 \
#	-Wall -W -Wshadow -Werror -Wformat-security \
#	-D_FORTIFY_SOURCE=2 \
#	#-pedantic -Wconversion
#
#LIBS	=	`./vsf_findlibs.sh`
#LINK	=	-Wl,-s
#LDFLAGS	=	-fPIE -pie -Wl,-z,relro -Wl,-z,now

CFLAGS	+= -O2 -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections -W -Wshadow \
	#-pedantic -Wconversion
CFLAGS  += -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE=1 -D_LARGEFILE64_SOURCE=1 -D_LARGE_FILES=1
CFLAGS  += -I$(SRCBASE)/shared/bcmwifi/include -DTYPEDEF_FLOAT_T
LDFLAGS = -ffunction-sections -fdata-sections -Wl,--gc-sections $(if $(QCA),$(EXTRALDFLAGS))

LIBS	=	-lcrypt
LINK	=

CFLAGS	+=	-I../shared
ifeq ($(RTCONFIG_FTP_SSL),y)
ifneq ($(MUSL32)$(MUSL64),)
LIBS	=	-pthread
else
LIBS	=	-lnsl -pthread
endif
OPENSSLDIR = ../openssl
OPENSSLINC = $(OPENSSLDIR)/include
LIBS	+=	-L$(OPENSSLDIR)/ -lssl -lcrypto
CFLAGS	+=	-I$(OPENSSLINC)
endif

CFLAGS  += 	-I$(SRCBASE)/include -I$(TOP)/shared -I$(TOP)/libdisk
LDFLAGS  +=	-L$(TOP_PLATFORM)/nvram${BCMEX}${EX7} -lnvram -L$(TOP)/shared -lshared -L$(TOP)/libdisk -ldisk
ifeq ($(RTCONFIG_BCM_502L07P2),y)
LDFLAGS += -L$(TOP_PLATFORM)/ethctl_lib -lethctl
endif
ifeq ($(RTCONFIG_HND_ROUTER),y)
LDFLAGS += -L$(TOP_PLATFORM)/wlcsm$(EXHND) -lwlcsm
endif
ifeq ($(RTCONFIG_BCMARM),y)
CFLAGS += -I$(SRCBASE)/common/include
LDFLAGS += -lgcc_s
endif

ifeq ($(HND_ROUTER_BE_4916), y)
CFLAGS+= -fcommon
endif

ifeq ($(RTCONFIG_QTN),y)
LDFLAGS += -L$(TOP)/libqcsapi_client -lqcsapi_client
endif

ifeq ($(RTCONFIG_AMAS),y)
LDFLAGS += -L$(TOP)/json-c/.libs -ljson-c
ifeq ($(RTCONFIG_QCA),y)
LDFLAGS += -lm
endif
endif

ifeq ($(RTCONFIG_PERMISSION_MANAGEMENT),y)
ifeq ($(RTCONFIG_QCA),y)
LDFLAGS += -lpthread -ldl
endif
CFLAGS += -I$(TOP)/sqlCipher
CFLAGS += -I$(TOP)/PMS_DBapis
LDFLAGS += -L$(TOP)/$(OPENSSLDIR) -lcrypto
LDFLAGS += -L$(TOP)/sqlCipher/.libs -lsqlcipher
LDFLAGS += -L$(TOP)/PMS_DBapis -lpms_sql
endif

ifeq ($(RTCONFIG_TRUSTZONE),y)
CFLAGS += -I$(TOP)/ateed/
LDFLAGS += -L$(TOP)/ateed/ -latee
endif

ifeq ($(BUILD_NAME),$(filter $(BUILD_NAME),RT-BE82M GS-BE18000 GS-BE12000 GS7_PRO GT7))
LDFLAGS += -L$(TOP)/libhostapi -lhostapi
endif

OBJS	=	main.o utility.o prelogin.o ftpcmdio.o postlogin.o privsock.o \
		tunables.o ftpdataio.o secbuf.o ls.o \
		postprivparent.o logging.o str.o netstr.o sysstr.o strlist.o \
    banner.o filestr.o parseconf.o secutil.o \
    ascii.o oneprocess.o twoprocess.o privops.o standalone.o hash.o \
    tcpwrap.o ipaddrparse.o access.o features.o readwrite.o opts.o \
    ssl.o sslslave.o ptracesandbox.o ftppolicy.o sysutil.o sysdeputil.o \
    seccompsandbox.o


.c.o:
	$(CC) -c $*.c $(CFLAGS) $(IFLAGS)

vsftpd: $(OBJS) 
	$(CC) -o vsftpd $(OBJS) $(LINK) $(LDFLAGS) $(LIBS)

install:
	if [ -x /usr/local/sbin ]; then \
		$(INSTALL) -m 755 vsftpd /usr/local/sbin/vsftpd; \
	else \
		$(INSTALL) -m 755 vsftpd /usr/sbin/vsftpd; fi
	if [ -x /usr/local/man ]; then \
		$(INSTALL) -m 644 vsftpd.8 /usr/local/man/man8/vsftpd.8; \
		$(INSTALL) -m 644 vsftpd.conf.5 /usr/local/man/man5/vsftpd.conf.5; \
	elif [ -x /usr/share/man ]; then \
		$(INSTALL) -m 644 vsftpd.8 /usr/share/man/man8/vsftpd.8; \
		$(INSTALL) -m 644 vsftpd.conf.5 /usr/share/man/man5/vsftpd.conf.5; \
	else \
		$(INSTALL) -m 644 vsftpd.8 /usr/man/man8/vsftpd.8; \
		$(INSTALL) -m 644 vsftpd.conf.5 /usr/man/man5/vsftpd.conf.5; fi
	if [ -x /etc/xinetd.d ]; then \
		$(INSTALL) -m 644 xinetd.d/vsftpd /etc/xinetd.d/vsftpd; fi

clean:
	rm -f *.o *.swp vsftpd

