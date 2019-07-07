
SOURCE=../..
VERSION:=5.0.1dr1

source:	prepare
	cd build/strongswan-$(VERSION) && debuild -S

prepare: clean
	mkdir build
	make -C $(SOURCE) dist-gzip
	mv $(SOURCE)/strongswan-$(VERSION).tar.gz build/strongswan_$(VERSION).orig.tar.gz
	tar -zxv -C build -f build/strongswan_$(VERSION).orig.tar.gz
	dch -b -v $(VERSION)
	cp -R debian build/strongswan-$(VERSION)

clean:
	rm -Rf build

.PHONY:	source prepare clean
