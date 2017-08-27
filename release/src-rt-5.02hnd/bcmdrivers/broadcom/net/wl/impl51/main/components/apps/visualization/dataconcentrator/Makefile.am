# <<Broadcom-WL-IPTag/Proprietary:>>


AUTOMAKE_OPTIONS = foreign

VISSHARED=../shared
SRCDIR = $(BASEDIR)/src

CFLAGS=-Wall -Werror -I$(SRCDIR)/common/include/ -I$(SRCDIR)/include/ -I../include/ \
		-I$(SRCBASE_ROUTER)/json-c/ \
		-I$(SRCBASE_ROUTER)/libxml2/include/ \
		-I$(SQLITE3_DIR)/ \
		-I$(SRCDIR)/../components/shared/ \
		-DVERSION=\"$(VERSION)\" -DPROG="\"$(PACKAGE)\""

LDFLAGS=-L$(SRCBASE_ROUTER)/json-c/.libs \
		-L$(SRCBASE_ROUTER)/libxml2/.libs \
		-lpthread -lxml2 -ljson-c -L$(SRCBASE_ROUTER)/nvram${BCMEX}$(EX7) \
		-L$(INSTALLDIR)/nvram/usr/lib -lnvram
if IS_BCA_HNDROUTER
LDFLAGS += -L$(SRCBASE_ROUTER)/wlcsm -lwlcsm
endif
LDFLAGS += $(EXTRA_LDFLAGS)
LIBS += $(SQLITE3_DIR)/.libs/libsqlite3.a

if ISBCMINTERNAL
CFLAGS  += -DBCMINTERNAL
endif
CFLAGS  += -DBCMDBG

bin_PROGRAMS = vis-dcon
vis_dcon_SOURCES = $(VISSHARED)/vis_xmlshared.c $(VISSHARED)/vis_sock_util.c \
			$(VISSHARED)/vis_shared_util.c vis_synchdb.c \
			vis_utility.c database.c vis_xmlutility.c vis_jsonutility.c \
			vis_sockserver.c vis_dcon_main.c

clean-local:
	rm -f Makefile
	rm -rf autom4te.cache
	rm -f config.h
	rm -f config.log
	rm -f config.status
	rm -f stamp-h1
	rm -rf installbin
