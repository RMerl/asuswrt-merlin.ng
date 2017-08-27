TARGET   := 
SOURCES := radeapclient.c

ifneq ($(OPENSSL_LIBS),)
SOURCES += ${top_srcdir}/src/main/cb.c ${top_srcdir}/src/main/tls.c
endif

TGT_PREREQS := libfreeradius-radius.a libfreeradius-eap.a
TGT_PRLIBS  := ${OPENSSL_LIBS} ${LIBS}

SRC_INCDIRS  := libeap
