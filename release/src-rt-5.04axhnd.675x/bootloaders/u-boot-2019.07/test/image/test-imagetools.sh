#!/bin/bash
# SPDX-License-Identifier: GPL-2.0+
#
# Written by Guilherme Maciel Ferreira <guilherme.maciel.ferreira@gmail.com>
#
# Sanity check for mkimage and dumpimage tools
#
# To run this:
#
# make O=sandbox sandbox_config
# make O=sandbox
# ./test/image/test-imagetools.sh

BASEDIR=sandbox
SRCDIR=${BASEDIR}/boot
IMAGE_NAME="v1.0-test"
IMAGE_MULTI=linux.img
IMAGE_FIT_ITS=linux.its
IMAGE_FIT_ITB=linux.itb
DATAFILE0=vmlinuz
DATAFILE1=initrd.img
DATAFILE2=System.map
DATAFILES="${DATAFILE0} ${DATAFILE1} ${DATAFILE2}"
TEST_OUT=test_output
MKIMAGE=${BASEDIR}/tools/mkimage
DUMPIMAGE=${BASEDIR}/tools/dumpimage
MKIMAGE_LIST=mkimage.list
DUMPIMAGE_LIST=dumpimage.list

# Remove all the files we created
cleanup()
{
	local file

	for file in ${DATAFILES}; do
		rm -f ${file} ${SRCDIR}/${file}
	done
	rm -f ${IMAGE_MULTI}
	rm -f ${DUMPIMAGE_LIST}
	rm -f ${MKIMAGE_LIST}
	rm -f ${TEST_OUT}
	rmdir ${SRCDIR}
}

# Check that two files are the same
assert_equal()
{
	if ! diff -u $1 $2; then
		echo "Failed."
		cleanup
		exit 1
	fi
}

# Create some test files
create_files()
{
	local file

	mkdir -p ${SRCDIR}
	for file in ${DATAFILES}; do
		head -c $RANDOM /dev/urandom >${SRCDIR}/${file}
	done
}

# Run a command, echoing it first
do_cmd()
{
	local cmd="$@"

	echo "# ${cmd}"
	${cmd} 2>&1
}

# Run a command, redirecting output
# Args:
#    redirect_file
#    command...
do_cmd_redir()
{
	local redir="$1"
	shift
	local cmd="$@"

	echo "# ${cmd}"
	${cmd} >${redir}
}

# Write files into an multi-file image
create_multi_image()
{
	local files="${SRCDIR}/${DATAFILE0}:${SRCDIR}/${DATAFILE1}"
	files+=":${SRCDIR}/${DATAFILE2}"

	echo -e "\nBuilding multi-file image..."
	do_cmd ${MKIMAGE} -A x86 -O linux -T multi -n \"${IMAGE_NAME}\" \
		-d ${files} ${IMAGE_MULTI}
	echo "done."
}

# Extract files from an multi-file image
extract_multi_image()
{
	echo -e "\nExtracting multi-file image contents..."
	do_cmd ${DUMPIMAGE} -T multi -p 0 -o ${DATAFILE0} ${IMAGE_MULTI}
	do_cmd ${DUMPIMAGE} -T multi -p 1 -o ${DATAFILE1} ${IMAGE_MULTI}
	do_cmd ${DUMPIMAGE} -T multi -p 2 -o ${DATAFILE2} ${IMAGE_MULTI}
	do_cmd ${DUMPIMAGE} -T multi -p 2 -o ${TEST_OUT} ${IMAGE_MULTI}
	echo "done."
}

# Write files into a FIT image
create_fit_image()
{
	echo " \
	/dts-v1/; \
	/ { \
	    description = \"FIT image\"; \
	    #address-cells = <1>; \
	\
	    images { \
	        kernel@1 { \
	            description = \"kernel\"; \
	            data = /incbin/(\"${DATAFILE0}\"); \
	            type = \"kernel\"; \
	            arch = \"sandbox\"; \
	            os = \"linux\"; \
	            compression = \"gzip\"; \
	            load = <0x40000>; \
	            entry = <0x8>; \
	        }; \
	        ramdisk@1 { \
	            description = \"filesystem\"; \
	            data = /incbin/(\"${DATAFILE1}\"); \
	            type = \"ramdisk\"; \
	            arch = \"sandbox\"; \
	            os = \"linux\"; \
	            compression = \"none\"; \
	            load = <0x80000>; \
	            entry = <0x16>; \
	        }; \
	        fdt@1 { \
	            description = \"device tree\"; \
	            data = /incbin/(\"${DATAFILE2}\"); \
	            type = \"flat_dt\"; \
	            arch = \"sandbox\"; \
	            compression = \"none\"; \
	        }; \
	    }; \
	    configurations { \
	        default = \"conf@1\"; \
	        conf@1 { \
	            kernel = \"kernel@1\"; \
	            fdt = \"fdt@1\"; \
	        }; \
	    }; \
	}; \
	" > ${IMAGE_FIT_ITS}

	echo -e "\nBuilding FIT image..."
	do_cmd ${MKIMAGE} -f ${IMAGE_FIT_ITS} ${IMAGE_FIT_ITB}
	echo "done."
}

# Extract files from a FIT image
extract_fit_image()
{
	echo -e "\nExtracting FIT image contents..."
	do_cmd ${DUMPIMAGE} -T flat_dt -p 0 -o ${DATAFILE0} ${IMAGE_FIT_ITB}
	do_cmd ${DUMPIMAGE} -T flat_dt -p 1 -o ${DATAFILE1} ${IMAGE_FIT_ITB}
	do_cmd ${DUMPIMAGE} -T flat_dt -p 2 -o ${DATAFILE2} ${IMAGE_FIT_ITB}
	do_cmd ${DUMPIMAGE} -T flat_dt -p 2 -o ${TEST_OUT} ${IMAGE_FIT_ITB}
	echo "done."
}

# List the contents of a file
# Args:
#    image filename
list_image()
{
	local image="$1"

	echo -e "\nListing image contents..."
	do_cmd_redir ${MKIMAGE_LIST} ${MKIMAGE} -l ${image}
	do_cmd_redir ${DUMPIMAGE_LIST} ${DUMPIMAGE} -l ${image}
	echo "done."
}

main()
{
	local file

	create_files

	# Compress and extract multi-file images, compare the result
	create_multi_image
	extract_multi_image
	for file in ${DATAFILES}; do
		assert_equal ${file} ${SRCDIR}/${file}
	done
	assert_equal ${TEST_OUT} ${DATAFILE2}

	# List contents of multi-file image and compares output from tools
	list_image ${IMAGE_MULTI}
	assert_equal ${DUMPIMAGE_LIST} ${MKIMAGE_LIST}

	# Compress and extract FIT images, compare the result
	create_fit_image
	extract_fit_image
	for file in ${DATAFILES}; do
		assert_equal ${file} ${SRCDIR}/${file}
	done
	assert_equal ${TEST_OUT} ${DATAFILE2}

	# List contents of FIT image and compares output from tools
	list_image ${IMAGE_FIT_ITB}
	assert_equal ${DUMPIMAGE_LIST} ${MKIMAGE_LIST}

	# Remove files created
	cleanup

	echo "Tests passed."
}

main
