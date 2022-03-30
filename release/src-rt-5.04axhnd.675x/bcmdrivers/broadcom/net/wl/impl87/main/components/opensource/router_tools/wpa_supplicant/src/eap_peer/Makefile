CFLAGS += -DIEEE8021X_EAPOL

LIB_OBJS= \
	eap.o \
	eap_methods.o

include ../lib.rules

install:
	if ls *.so >/dev/null 2>&1; then \
		install -d $(DESTDIR)$(LIBDIR)/wpa_supplicant && \
		cp *.so $(DESTDIR)$(LIBDIR)/wpa_supplicant \
	; fi
