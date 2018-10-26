#!/bin/bash
export BUILD_TOP=${1}
export HOSTTOOLS_DIR=$BUILD_TOP/hostTools
export WDIR=${2}
export GEN3_CRED_DIR=${3}
CFEROM=${4}
IMAGE_IN=${5}
CHIP=${6}
export PERL5LIB=`make -f $HOSTTOOLS_DIR/SecureBootUtils/Makefile vars`
$HOSTTOOLS_DIR/SecureBootUtils/build_tk.pl --cferom $CFEROM --chip $CHIP --boot_part_size_kb=1024 \
	--image  $IMAGE_IN --cred_dir $GEN3_CRED_DIR  --wdir $WDIR
