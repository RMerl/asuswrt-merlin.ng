TARGET = libhostapi.so

all:
	-cp -f prebuilt/$(TARGET) $(TARGET)

clean:
	rm -f $(TARGET)
	rm -f $(INSTALLDIR)/lib/$(TARGET)

install: all
	mkdir -p $(INSTALLDIR)/lib
	install -D $(TARGET) $(INSTALLDIR)/lib/$(TARGET)
	$(STRIP) $(INSTALLDIR)/lib/$(TARGET)
