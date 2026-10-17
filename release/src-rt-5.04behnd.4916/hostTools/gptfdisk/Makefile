
APP=gptfdisk-1.0.9

all: .patched
	make -C ${APP}
	ln -sf ${APP}/sgdisk sgdisk

.patched: .extracted
	patch -d ${APP} -p0 < nosync-regular-file.patch && touch .patched

.extracted: ${APP}.tar.gz
	tar xzf ${APP}.tar.gz && touch .extracted


clean:
	rm -rf ${APP}
	rm -f sgdisk
	rm -f .extracted
	rm -f .patched

