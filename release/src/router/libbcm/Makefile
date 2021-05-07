# libbcm
include $(TOP)/.config

all:
	cp -f ./prebuilt/$(BUILD_NAME)/libbcm.so .

install: all
	install -d $(INSTALLDIR)/usr/lib
	install -m 755 libbcm.so $(INSTALLDIR)/usr/lib
	$(STRIP) $(INSTALLDIR)/usr/lib/libbcm.so

clean:
	rm -f *.o *.so
