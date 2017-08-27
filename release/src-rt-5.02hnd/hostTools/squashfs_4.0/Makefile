ifndef Sqlzma
$(error Sqlzma is not defined)
endif

INSTALL_DIR = /usr/local/bin

INCLUDEDIR = .

CFLAGS := -I$(INCLUDEDIR) -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_GNU_SOURCE -O2
ifdef UseDebugFlags
DebugFlags = -g -Wall -Wno-unused-variable -O0 -UNDEBUG
endif
CFLAGS += -I${Sqlzma} -D_REENTRANT -DNDEBUG ${DebugFlags}
LDFLAGS += -L${LzmaAlone} -L${LzmaC}

all: mksquashfs unsquashfs

mksquashfs: mksquashfs.o read_fs.o sort.o swap.o pseudo.o
	$(CC) $(LDFLAGS) mksquashfs.o read_fs.o sort.o swap.o pseudo.o -lunlzma_r -llzma_r -lz -lpthread -lm -lstdc++ -o $@

unsquashfs: unsquashfs.o unsquash-1.o unsquash-2.o unsquash-3.o unsquash-4.o swap.o 
	$(CC) $(LDFLAGS) unsquashfs.o unsquash-1.o unsquash-2.o unsquash-3.o unsquash-4.o swap.o -lunlzma_r -llzma_r -lz -lpthread -lm -lstdc++ -o $@

mksquashfs.o: mksquashfs.c squashfs_fs.h mksquashfs.h global.h sort.h squashfs_swap.h Makefile \
	${Sqlzma}/sqlzma.h ${Sqlzma}/sqmagic.h \
	${LzmaAlone}/liblzma_r.a ${LzmaC}/libunlzma_r.a

read_fs.o: read_fs.c squashfs_fs.h read_fs.h global.h squashfs_swap.h Makefile \
	${Sqlzma}/sqlzma.h ${Sqlzma}/sqmagic.h

sort.o: sort.c squashfs_fs.h global.h sort.h Makefile

pseudo.o: pseudo.o pseudo.h

swap.o: swap.c Makefile

unsquash-1.o: unsquashfs.h unsquash-1.c squashfs_fs.h squashfs_compat.h global.h Makefile

unsquash-2.o: unsquashfs.h unsquash-2.c unsquashfs.h squashfs_fs.h squashfs_compat.h global.h Makefile

unsquash-3.o: unsquashfs.h unsquash-3.c squashfs_fs.h squashfs_compat.h global.h Makefile

unsquash-4.o: unsquashfs.h unsquash-4.c squashfs_fs.h squashfs_swap.h global.h Makefile

unsquashfs.o: unsquashfs.h unsquashfs.c squashfs_fs.h squashfs_swap.h squashfs_compat.h global.h Makefile \
    ${Sqlzma}/sqlzma.h ${Sqlzma}/sqmagic.h \
    ${LzmaAlone}/liblzma_r.a ${LzmaC}/libunlzma_r.a

clean:
	-rm -f *.o mksquashfs unsquashfs

install: mksquashfs unsquashfs
	mkdir -p $(INSTALL_DIR)
	cp mksquashfs $(INSTALL_DIR)
	cp unsquashfs $(INSTALL_DIR)
