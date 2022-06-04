

CFLAGS := -DWITHOUT_XATTR -I.

mkfs.jffs2: crc32.o compr_rtime.o mkfs.jffs2.o compr_zlib.o compr.o rbtree.o
	$(CC) $(LDFLAGS) -o $@ $^ -lz

clean:
	${RM} *~ *.o

