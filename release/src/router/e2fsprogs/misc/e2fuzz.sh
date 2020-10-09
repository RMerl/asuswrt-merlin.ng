#!/bin/bash

# Test harness to fuzz a filesystem over and over...
# Copyright (C) 2014 Oracle.

DIR=/tmp
PASSES=10000
SZ=32m
SCRIPT_DIR="$(dirname "$0")"
FEATURES="has_journal,extent,huge_file,flex_bg,uninit_bg,dir_nlink,extra_isize,64bit,metadata_csum,bigalloc,sparse_super2,inline_data"
BLK_SZ=4096
INODE_SZ=256
EXTENDED_OPTS="discard"
EXTENDED_FSCK_OPTIONS=""
RUN_FSCK=1
OVERRIDE_PATH=1
HAS_FUSE2FS=0
USE_FUSE2FS=0
MAX_FSCK=10
SRCDIR=/etc
test -x "${SCRIPT_DIR}/fuse2fs" && HAS_FUSE2FS=1

print_help() {
	echo "Usage: $0 OPTIONS"
	echo "-b:	FS block size is this. (${BLK_SZ})"
	echo "-B:	Corrupt this many bytes per run."
	echo "-d:	Create test files in this directory. (${DIR})"
	echo "-E:	Extended mke2fs options."
	echo "-f:	Do not run e2fsck after each pass."
	echo "-F:	Extended e2fsck options."
	echo "-I:	Create inodes of this size. (${INODE_SZ})"
	echo "-n:	Run this many passes. (${PASSES})"
	echo "-O:	Create FS with these features."
	echo "-p:	Use system's mke2fs/e2fsck/tune2fs tools."
	echo "-s:	Create FS images of this size. (${SZ})"
	echo "-S:	Copy files from this dir. (${SRCDIR})"
	echo "-x:	Run e2fsck at most this many times. (${MAX_FSCK})"
	test "${HAS_FUSE2FS}" -gt 0 && echo "-u:	Use fuse2fs instead of the kernel."
	exit 0
}

GETOPT="d:n:s:O:I:b:B:E:F:fpx:S:"
test "${HAS_FUSE2FS}" && GETOPT="${GETOPT}u"

while getopts "${GETOPT}" opt; do
	case "${opt}" in
	"B")
		E2FUZZ_ARGS="${E2FUZZ_ARGS} -b ${OPTARG}"
		;;
	"d")
		DIR="${OPTARG}"
		;;
	"n")
		PASSES="${OPTARG}"
		;;
	"s")
		SZ="${OPTARG}"
		;;
	"O")
		FEATURES="${FEATURES},${OPTARG}"
		;;
	"I")
		INODE_SZ="${OPTARG}"
		;;
	"b")
		BLK_SZ="${OPTARG}"
		;;
	"E")
		EXTENDED_OPTS="${OPTARG}"
		;;
	"F")
		EXTENDED_FSCK_OPTS="-E ${OPTARG}"
		;;
	"f")
		RUN_FSCK=0
		;;
	"p")
		OVERRIDE_PATH=0
		;;
	"u")
		USE_FUSE2FS=1
		;;
	"x")
		MAX_FSCK="${OPTARG}"
		;;
	"S")
		SRCDIR="${OPTARG}"
		;;
	*)
		print_help
		;;
	esac
done

if [ "${OVERRIDE_PATH}" -gt 0 ]; then
	PATH="${SCRIPT_DIR}:${SCRIPT_DIR}/../e2fsck/:${PATH}"
	export PATH
fi

TESTDIR="${DIR}/tests/"
TESTMNT="${DIR}/mnt/"
BASE_IMG="${DIR}/e2fuzz.img"

cat > /tmp/mke2fs.conf << ENDL
[defaults]
        base_features = ${FEATURES}
        default_mntopts = acl,user_xattr,block_validity
        enable_periodic_fsck = 0
        blocksize = ${BLK_SZ}
        inode_size = ${INODE_SZ}
        inode_ratio = 4096
	cluster_size = $((BLK_SZ * 2))
	options = ${EXTENDED_OPTS}
ENDL
MKE2FS_CONFIG=/tmp/mke2fs.conf
export MKE2FS_CONFIG

# Set up FS image
echo "+ create fs image"
umount "${TESTDIR}"
umount "${TESTMNT}"
rm -rf "${TESTDIR}"
rm -rf "${TESTMNT}"
mkdir -p "${TESTDIR}"
mkdir -p "${TESTMNT}"
rm -rf "${BASE_IMG}"
truncate -s "${SZ}" "${BASE_IMG}"
mke2fs -F -v "${BASE_IMG}"
if [ $? -ne 0 ]; then
	exit $?
fi

# Populate FS image
echo "+ populate fs image"
modprobe loop
mount "${BASE_IMG}" "${TESTMNT}" -o loop
if [ $? -ne 0 ]; then
	exit $?
fi
SRC_SZ="$(du -ks "${SRCDIR}" | awk '{print $1}')"
FS_SZ="$(( $(stat -f "${TESTMNT}" -c '%a * %S') / 1024 ))"
NR="$(( (FS_SZ * 4 / 10) / SRC_SZ ))"
if [ "${NR}" -lt 1 ]; then
	NR=1
fi
echo "+ make ${NR} copies"
seq 1 "${NR}" | while read nr; do
	cp -pRdu "${SRCDIR}" "${TESTMNT}/test.${nr}" 2> /dev/null
done
umount "${TESTMNT}"
e2fsck -fn "${BASE_IMG}"
if [ $? -ne 0 ]; then
	echo "fsck failed??"
	exit 1
fi

# Run tests
echo "+ run test"
ret=0
seq 1 "${PASSES}" | while read pass; do
	echo "+ pass ${pass}"
	PASS_IMG="${TESTDIR}/e2fuzz-${pass}.img"
	FSCK_IMG="${TESTDIR}/e2fuzz-${pass}.fsck"
	FUZZ_LOG="${TESTDIR}/e2fuzz-${pass}.fuzz.log"
	OPS_LOG="${TESTDIR}/e2fuzz-${pass}.ops.log"

	echo "++ corrupt image"
	cp "${BASE_IMG}" "${PASS_IMG}"
	if [ $? -ne 0 ]; then
		exit $?
	fi
	tune2fs -L "e2fuzz-${pass}" "${PASS_IMG}"
	e2fuzz -v "${PASS_IMG}" ${E2FUZZ_ARGS} > "${FUZZ_LOG}"
	if [ $? -ne 0 ]; then
		exit $?
	fi

	echo "++ mount image"
	if [ "${USE_FUSE2FS}" -gt 0 ]; then
		"${SCRIPT_DIR}/fuse2fs" "${PASS_IMG}" "${TESTMNT}"
		res=$?
	else
		mount "${PASS_IMG}" "${TESTMNT}" -o loop
		res=$?
	fi

	if [ "${res}" -eq 0 ]; then
		echo "+++ ls -laR"
		ls -laR "${TESTMNT}/test.1/" > /dev/null 2> "${OPS_LOG}"

		echo "+++ cat files"
		find "${TESTMNT}/test.1/" -type f -size -1048576k -print0 | xargs -0 cat > /dev/null 2>> "${OPS_LOG}"

		echo "+++ expand"
		find "${TESTMNT}/" -type f 2> /dev/null | head -n 50000 | while read f; do
			attr -l "$f" > /dev/null 2>> "${OPS_LOG}"
			if [ -f "$f" -a -w "$f" ]; then
				dd if=/dev/zero bs="${BLK_SZ}" count=1 >> "$f" 2>> "${OPS_LOG}"
			fi
			mv "$f" "$f.longer" > /dev/null 2>> "${OPS_LOG}"
		done
		sync

		echo "+++ create files"
		cp -pRdu "${SRCDIR}" "${TESTMNT}/test.moo" 2>> "${OPS_LOG}"
		sync

		echo "+++ remove files"
		rm -rf "${TESTMNT}/test.moo" 2>> "${OPS_LOG}"

		umount "${TESTMNT}"
		res=$?
		if [ "${res}" -ne 0 ]; then
			ret=1
			break
		fi
		sync
		test "${USE_FUSE2FS}" -gt 0 && sleep 2
	fi
	if [ "${RUN_FSCK}" -gt 0 ]; then
		cp "${PASS_IMG}" "${FSCK_IMG}"
		pass_img_sz="$(stat -c '%s' "${PASS_IMG}")"

		seq 1 "${MAX_FSCK}" | while read fsck_pass; do
			echo "++ fsck pass ${fsck_pass}: $(which e2fsck) -fy ${FSCK_IMG} ${EXTENDED_FSCK_OPTS}"
			FSCK_LOG="${TESTDIR}/e2fuzz-${pass}-${fsck_pass}.log"
			e2fsck -fy "${FSCK_IMG}" ${EXTENDED_FSCK_OPTS} > "${FSCK_LOG}" 2>&1
			res=$?
			echo "++ fsck returns ${res}"
			if [ "${res}" -eq 0 ]; then
				exit 0
			elif [ "${fsck_pass}" -eq "${MAX_FSCK}" ]; then
				echo "++ fsck did not fix in ${MAX_FSCK} passes."
				exit 1
			fi
			if [ "${res}" -gt 0 -a \
			     "$(grep 'Memory allocation failed' "${FSCK_LOG}" | wc -l)" -gt 0 ]; then
				echo "++ Ran out of memory, get more RAM"
				exit 0
			fi
			if [ "${res}" -gt 0 -a \
			     "$(grep 'Could not allocate block' "${FSCK_LOG}" | wc -l)" -gt 0 -a \
			     "$(dumpe2fs -h "${FSCK_IMG}" | grep '^Free blocks:' | awk '{print $3}')0" -eq 0 ]; then
				echo "++ Ran out of space, get a bigger image"
				exit 0
			fi
			if [ "${fsck_pass}" -gt 1 ]; then
				diff -u "${TESTDIR}/e2fuzz-${pass}-$((fsck_pass - 1)).log" "${FSCK_LOG}"
				if [ $? -eq 0 ]; then
					echo "++ fsck makes no progress"
					exit 2
				fi
			fi

			fsck_img_sz="$(stat -c '%s' "${FSCK_IMG}")"
			if [ "${fsck_img_sz}" -ne "${pass_img_sz}" ]; then
				echo "++ fsck image size changed"
				exit 3
			fi
		done
		fsck_loop_ret=$?
		if [ "${fsck_loop_ret}" -gt 0 ]; then
			break;
		fi
	fi

	echo "+++ check fs for round 2"
	FSCK_LOG="${TESTDIR}/e2fuzz-${pass}-round2.log"
	e2fsck -fn "${FSCK_IMG}" ${EXTENDED_FSCK_OPTS} >> "${FSCK_LOG}" 2>&1
	res=$?
	if [ "${res}" -ne 0 ]; then
		echo "++++ fsck failed."
		exit 1
	fi

	echo "++ mount image (2)"
	mount "${FSCK_IMG}" "${TESTMNT}" -o loop
	res=$?

	if [ "${res}" -eq 0 ]; then
		echo "+++ ls -laR (2)"
		ls -laR "${TESTMNT}/test.1/" > /dev/null 2> "${OPS_LOG}"

		echo "+++ cat files (2)"
		find "${TESTMNT}/test.1/" -type f -size -1048576k -print0 | xargs -0 cat > /dev/null 2>> "${OPS_LOG}"

		echo "+++ expand (2)"
		find "${TESTMNT}/" -type f 2> /dev/null | head -n 50000 | while read f; do
			attr -l "$f" > /dev/null 2>> "${OPS_LOG}"
			if [ -f "$f" -a -w "$f" ]; then
				dd if=/dev/zero bs="${BLK_SZ}" count=1 >> "$f" 2>> "${OPS_LOG}"
			fi
			mv "$f" "$f.longer" > /dev/null 2>> "${OPS_LOG}"
		done
		sync

		echo "+++ create files (2)"
		cp -pRdu "${SRCDIR}" "${TESTMNT}/test.moo" 2>> "${OPS_LOG}"
		sync

		echo "+++ remove files (2)"
		rm -rf "${TESTMNT}/test.moo" 2>> "${OPS_LOG}"

		umount "${TESTMNT}"
		res=$?
		if [ "${res}" -ne 0 ]; then
			ret=1
			break
		fi
		sync
		test "${USE_FUSE2FS}" -gt 0 && sleep 2

		echo "+++ check fs (2)"
		e2fsck -fn "${FSCK_IMG}" >> "${FSCK_LOG}" 2>&1
		res=$?
		if [ "${res}" -ne 0 ]; then
			echo "++ fsck failed."
			exit 1
		fi
	else
		echo "++ mount(2) failed with ${res}"
		exit 1
	fi
	rm -rf "${FSCK_IMG}" "${PASS_IMG}" "${FUZZ_LOG}" "${TESTDIR}"/e2fuzz*.log
done

exit $ret
