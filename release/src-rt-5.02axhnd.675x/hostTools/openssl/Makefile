

all: bin/openssl



src/Configure: openssl-*gz
	mkdir -p bin obj src
	cd src && tar --strip-components=1 -xf ../$<

obj/Makefile: src/Configure 
	cd obj && ../src/Configure no-shared linux-x86_64

bin/openssl: obj/Makefile
	cd obj && make -j16 && cp apps/openssl ../bin/

clean:
	rm -rf bin obj src

