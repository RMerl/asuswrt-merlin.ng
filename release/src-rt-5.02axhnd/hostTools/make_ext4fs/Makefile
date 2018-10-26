
EXE = make_ext4fs
LIBS = -lsparse -lz -lselinux
LIB_PATH = ../../
EXT4_UTIL_SRC := make_ext4fs_main.c make_ext4fs.c ext4fixup.c ext4_utils.c allocate.c contents.c extent.c indirect.c uuid.c sha1.c wipe.c crc16.c
LIBSELINUX_SRC := callbacks.c check_context.c freecon.c init.c label.c label_file.c label_android_property.c
LIBZ_SRC := adler32.c compress.c crc32.c deflate.c gzclose.c gzlib.c gzread.c gzwrite.c infback.c inflate.c inftrees.c inffast.c trees.c uncompr.c zutil.c
LIBSPARSE_SRC := backed_block.c output_file.c sparse.c sparse_crc32.c sparse_err.c sparse_read.c

$(EXE) : $(OBJS) libsparse.a libz.a libselinux.a
	cd extras/ext4_utils;\
	$(CC) -o $@ $(EXT4_UTIL_SRC) -I../../libselinux/include -I../../core/libsparse/include -I../../core/include/ -DHOST -DANDROID -L$(LIB_PATH) $(LIBS);\
	cp $@ ../../

libselinux.a :
	cd libselinux/src ; \
	$(CC) -c $(LIBSELINUX_SRC) -I../include -I../../core/include -DHOST; \
	$(AR) rcs ../../libselinux.a *.o

libz.a :
	cd zlib/src; \
	$(CC) -c $(LIBZ_SRC) -O3 -USE_MMAP -I..; \
	$(AR) rcs ../../libz.a *.o

libsparse.a :
	cd core/libsparse;\
	$(CC) -c $(LIBSPARSE_SRC) -Iinclude ;\
	$(AR) rcs ../../libsparse.a *.o

install :
	mkdir -p $(HOSTTOOLS_DIR)/local_install
	cp $(EXE) $(HOSTTOOLS_DIR)/local_install/

clean :
	rm -f $(HOSTTOOLS_DIR)/local_install/$(EXE)
	find . -name "*.[o|a]" | xargs rm -f
	find . -name "$(EXE)" | xargs rm -f
