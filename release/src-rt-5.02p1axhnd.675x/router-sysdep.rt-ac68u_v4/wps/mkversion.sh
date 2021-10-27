#! /bin/bash
#
# Create the $2 file from $1 using the tag in $3
#
# Copyright 2019 Broadcom
#
# This program is the proprietary software of Broadcom and/or
# its licensors, and may only be used, duplicated, modified or distributed
# pursuant to the terms and conditions of a separate, written license
# agreement executed between you and Broadcom (an "Authorized License").
# Except as set forth in an Authorized License, Broadcom grants no license
# (express or implied), right to use, or waiver of any kind with respect to
# the Software, and Broadcom expressly reserves all rights in and to the
# Software and all intellectual property rights therein.  IF YOU HAVE NO
# AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
# WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
# THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1. This program, including its structure, sequence and organization,
# constitutes the valuable trade secrets of Broadcom, and you shall use
# all reasonable efforts to protect the confidentiality thereof, and to
# use this information only in connection with your use of Broadcom
# integrated circuit products.
#
# 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
# "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
# REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
# OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
# DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
# NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
# ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
# CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
# OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
# BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
# SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
# IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
# IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
# ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
# OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
# NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
#
# <<Broadcom-WL-IPTag/Proprietary:>>
#
# $Id: mkversion.sh 534106 2015-02-12 08:59:59Z $

# Check for the in file, if not there we're probably in the wrong directory
if [ ! -f $1 ]; then
	echo file "$1" not found
	exit 1
fi

	# Tag should be in the form
	#    <NAME>_REL_<MAJ>_<MINOR>
	# or
	#    <NAME>_REL_<MAJ>_<MINOR>_RC<RCNUM>
	# or
	#    <NAME>_REL_<MAJ>_<MINOR>_RC<RCNUM>_<INCREMENTAL>
	TAG=$3

	# Remove leading cvs "Name: " and trailing " $"
	TAG=${TAG/#*: /}
	TAG=${TAG/% $/}

	# Split the tag into an array on underbar or whitespace boundaries.
	IFS="_	     " tag=(${TAG})
	unset IFS

        tagged=1
	if [ ${#tag[*]} -eq 0 ]; then
	   tag=(`date '+TOT REL %Y %m %d 0 %y'`);
	   tagged=0
	fi

	# Allow environment variable to override values.
	# Missing values default to 0
	#
	maj=${MOD_MAJOR_VERSION:-${tag[2]:-0}}
	min=${MOD_MINOR_VERSION:-${tag[3]:-0}}
	rcnum=${MOD_RC_NUMBER:-${tag[4]:-0}}
	incremental=${MOD_INCREMENTAL_NUMBER:-${tag[5]:-0}}
	build=${MOD_BUILD_NUMBER:-0}

	# Strip 'RC' from front of rcnum if present
	rcnum=${rcnum/#RC/}

	# strip leading zero off the number (otherwise they look like octal)
	maj=${maj/#0/}
	min=${min/#0/}
	min_router=${min}
	rcnum=${rcnum/#0/}
	incremental=${incremental/#0/}
	build=${build/#0/}

	# some numbers may now be null.  replace with with zero.
	maj=${maj:-0}
	min=${min:-0}
	rcnum=${rcnum:-0}
	incremental=${incremental:-0}
	build=${build:-0}

	if [ ${tagged} -eq 1 ]; then
	    vernum=`printf "0x%02x%02x%02x%02x" ${maj} ${min} ${rcnum} ${incremental}`
	else
	    vernum=`printf "0x00%02x%02x%02x" ${tag[7]} ${min} ${rcnum}`
	fi

        # PR17029: increment minor number for tagged router builds
        #         with an even minor revision
	if [ ${tagged} -eq 1 -a `expr \( \( ${min} + 1 \) % 2 \)` -eq 1 ]; then
	   min_router=`expr ${min} + 1`
	fi

	# OK, go do it

	echo "maj=${maj}, min=${min}, rc=${rcnum}, inc=${incremental}, build=${build}"
	echo "Router maj=${maj}, min=${min_router}, rc=${rcnum}, inc=${incremental}, build=${build}"

	sed \
		-e "s;@MOD_MAJOR_VERSION@;${maj};" \
		-e "s;@MOD_MINOR_VERSION@;${min};" \
		-e "s;@MOD_RC_NUMBER@;${rcnum};" \
		-e "s;@MOD_INCREMENTAL_NUMBER@;${incremental};" \
		-e "s;@MOD_BUILD_NUMBER@;${build};" \
		-e "s;@MOD_VERSION@;${maj}, ${min}, ${rcnum}, ${incremental};" \
		-e "s;@MOD_VERSION_STR@;${maj}.${min}.${rcnum}.${incremental};" \
                -e "s;@MOD_ROUTER_VERSION_STR@;${maj}.${min_router}.${rcnum}.${incremental};" \
                -e "s;@MOD_VERSION_NUM@;${vernum};" \
		< $1 > $2
