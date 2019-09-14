qca-swconfig.ipq40xx: libnl-tiny-0.1
	$(MAKE) -C qca-swconfig.ipq40xx

qca-swconfig.ipq40xx-clean:
	$(MAKE) -C qca-swconfig.ipq40xx clean

qca-swconfig.ipq40xx-install: qca-swconfig.ipq40xx
	install -D qca-swconfig.ipq40xx/swconfig $(INSTALLDIR)/qca-swconfig.ipq40xx/usr/sbin/swconfig
	$(STRIP) $(INSTALLDIR)/qca-swconfig.ipq40xx/usr/sbin/swconfig
