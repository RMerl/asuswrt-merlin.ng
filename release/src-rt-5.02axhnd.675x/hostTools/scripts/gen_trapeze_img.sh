#!/bin/sh

####### Initial Paths and PROFILES. EDIT THESE TO SUIT YOUR BUILD ENVIRONMENT ######
REL_502_BUILD_DIR="<Absolute path to 502 build base directory>"
REL_502_PROFILE="<502 Trapeze build profile>"
REL_504_BUILD_DIR="<Absolute path to 504 build base directory>"
REL_504_PROFILE="<504 standard build profile>"
if [ ! -d "$REL_502_BUILD_DIR/targets/$REL_502_PROFILE" ]; then
	echo "ERROR: Cannot access 502 build profile at $REL_502_BUILD_DIR/targets/$REL_502_PROFILE"
	exit 1
fi
if [ ! -d "$REL_504_BUILD_DIR/targets/$REL_504_PROFILE" ]; then
	echo "ERROR: Cannot access 504 build profile at $REL_504_BUILD_DIR/targets/$REL_504_PROFILE"
	exit 1
fi

### (1) Build 504/502 clean builds ###
if [ "$1" != "noclean" ]; then
	# Build 504 Img
	cd $REL_504_BUILD_DIR
	make PROFILE=$REL_504_PROFILE clean
fi
script -c "make PROFILE=$REL_504_PROFILE" REL_504_build_log.txt

if [ "$1" != "noclean" ]; then
	# Build 502 Img
	cd $REL_502_BUILD_DIR
	make PROFILE=$REL_502_PROFILE clean
fi
script -c "make PROFILE=$REL_502_PROFILE" REL_502_build_log.txt

# Setup /re-image/ directory
RE_IMG_DIR=$REL_504_BUILD_DIR/targets/$REL_504_PROFILE/fs.install/re-image
rm -rf $RE_IMG_DIR
mkdir -p $RE_IMG_DIR

### (2) Copy 504 loader and metadata binaries to re-imaging dir ###
cp $REL_504_BUILD_DIR/bootloaders/obj/binaries/bootstrap_image_emmc_boot_part.bin  $RE_IMG_DIR/REL_504_loader.bin
echo -ne "COMMITTED=1\\0VALID=1,0\\0SEQ=0,0\\0" | $REL_504_BUILD_DIR/bootloaders/obj/uboot/tools/mkenvimage --metadata -s 256 -o $RE_IMG_DIR/metadata_img1_commit.bin
echo -ne "COMMITTED=2\\0VALID=2,0\\0SEQ=0,0\\0" | $REL_504_BUILD_DIR/bootloaders/obj/uboot/tools/mkenvimage --metadata -s 256 -o $RE_IMG_DIR/metadata_img2_commit.bin

### (3) Copy 502 busybox, mmc, sgdisk, nvram harvest tool, re-imaging scripts to re-imaging dir ###
cp $REL_502_BUILD_DIR/userspace/public/apps/scripts/emmc_trapeze/* $RE_IMG_DIR/
cp $REL_502_BUILD_DIR/targets/$REL_502_PROFILE/fs/bin/busybox $RE_IMG_DIR/
cp $REL_502_BUILD_DIR/targets/$REL_502_PROFILE/fs/bin/sgdisk $RE_IMG_DIR/
cp $REL_502_BUILD_DIR/targets/$REL_502_PROFILE/fs/bin/mmc $RE_IMG_DIR/
cp $REL_502_BUILD_DIR/targets/$REL_502_PROFILE/fs/bin/nvram_to_env $RE_IMG_DIR/
ln -s busybox $RE_IMG_DIR/init
ln -s busybox $RE_IMG_DIR/reboot
ln -s busybox $RE_IMG_DIR/ls
ln -s busybox $RE_IMG_DIR/echo
ln -s busybox $RE_IMG_DIR/expr
ln -s busybox $RE_IMG_DIR/dd
ln -s busybox $RE_IMG_DIR/chmod
ln -s busybox $RE_IMG_DIR/sync
ln -s busybox $RE_IMG_DIR/cp
ln -s busybox $RE_IMG_DIR/ls
ln -s busybox $RE_IMG_DIR/cat
ln -s busybox $RE_IMG_DIR/rm
ln -s busybox $RE_IMG_DIR/sh
ln -s busybox $RE_IMG_DIR/mount
ln -s busybox $RE_IMG_DIR/[
ln -s busybox $RE_IMG_DIR/sleep
ln -s busybox $RE_IMG_DIR/blockdev
ln -s busybox $RE_IMG_DIR/grep
ln -s busybox $RE_IMG_DIR/sha256sum

### (4) Generate 504 trapeze enabled rootfs binary ###
cd $REL_504_BUILD_DIR
script -c "make PROFILE=$REL_504_PROFILE" REL_504_trapeze_log.txt

# Append 504 FIT to 504 trapeze rootfs to form final trapeze rootfs binary
rootfs_bin=$REL_504_BUILD_DIR/bootloaders/obj/binaries/rootfs.enc
fit_bin=$REL_504_BUILD_DIR/bootloaders/obj/binaries/brcm_full_linux.itb
rootfs_bin_trapeze=$REL_504_BUILD_DIR/bootloaders/obj/binaries/rootfs.enc_trapeze
fit_magic="FiTMaGic504"
pad_bound=16384
fit_rec=fit_record.tmp
fit_bin_padded=fit_padded.tmp
\rm -rf $fit_rec
\rm -rf $rootfs_bin_trapeze
\rm -rf $fit_bin_padded

# Write rootfs to final binary. Pad out to $pad_bound
dd if=$rootfs_bin of=${rootfs_bin_trapeze} ibs=$pad_bound conv=sync

# Get offset of FIT record
fit_rec_blk_offs=`stat --format=%s ${rootfs_bin_trapeze}`
fit_rec_blk_offs=`expr $fit_rec_blk_offs / $pad_bound`
echo "FIT_REC_OFFS: $fit_rec_blk_offs"

# Get offset of actual FIT, one block after FIT record
fit_blk_offs=`expr $fit_rec_blk_offs + 1`
echo "FIT_BLK_OFFS: $fit_blk_offs"

# Get size of block padded FIT
dd if=$fit_bin ibs=$pad_bound of=$fit_bin_padded conv=sync
fit_size=`stat --format=%s ${fit_bin}`
fit_size_blks=`stat --format=%s $fit_bin_padded`
fit_size_blks=`expr $fit_size_blks / $pad_bound`
echo "FIT_SIZE: $fit_size"
echo "FIT_SIZE_BLKS: $fit_size_blks"

# Calculate sha256
fit_sha256=`sha256sum $fit_bin_padded`
fit_sha256=${fit_sha256%% *}

# Create FIT record
echo -ne "FIT_MAGIC=$fit_magic\\nFIT_SIZE_BLKS=$fit_size_blks\\nFIT_BLK_OFFS=$fit_blk_offs\\nFIT_SHA256=$fit_sha256\\n" > $fit_rec

# Add FIT record to final binary, padded to $pad_bound
dd if=$fit_rec ibs=$pad_bound conv=sync >> ${rootfs_bin_trapeze}

# Add FIT to final trapeze rootfs binary
dd if=$fit_bin_padded conv=sync >> ${rootfs_bin_trapeze}

### (5) Generate 502 Trapeze image ###
cd $REL_502_BUILD_DIR
script -c "make PROFILE=$REL_502_PROFILE TRAPEZE_ROOTFS=$rootfs_bin_trapeze" REL_502_trapeze_log.txt
cd build; make -f make.image TRAPEZE_ROOTFS=$rootfs_bin_trapeze emmc_trapeze; cd ..;

