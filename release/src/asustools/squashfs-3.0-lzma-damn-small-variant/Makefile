INCLUDEDIR = .
LZMAPATH = ./lzma/C/7zip/Compress/

CFLAGS := -O2 -I$(INCLUDEDIR) -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LZMA_PARAMS -D_THREAD_SAFE

all: mksquashfs-lzma unsquashfs-lzma

mksquashfs-lzma: mksquashfs.o read_fs.o sort.o
	make -C $(LZMAPATH)/LZMA_Lib
	$(CXX) $(CFLAGS) mksquashfs.o read_fs.o sort.o -L$(LZMAPATH)/LZMA_Lib -llzma -lpthread -o $@

mksquashfs.o: mksquashfs.c squashfs_fs.h mksquashfs.h global.h sort.h

read_fs.o: read_fs.c squashfs_fs.h read_fs.h global.h

sort.o: sort.c squashfs_fs.h global.h sort.h

unsquashfs-lzma: unsquashfs.o LzmaDecode.o
	$(CXX) $(CFLAGS) unsquashfs.o LzmaDecode.o -lpthread -o $@ 

unsquashfs.o: unsquashfs.c squashfs_fs.h read_fs.h global.h

LzmaDecode.o: 
	$(CC) -c $(LZMAPATH)/LZMA_C/LzmaDecode.c	

clean:
	make -C $(LZMAPATH)/LZMA_Lib clean
	rm -f *.o 
	rm -f mksquashfs-lzma
	rm -f unsquashfs-lzma
