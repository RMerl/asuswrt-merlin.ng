#! /bin/bash
#
# Component version generation script, that is called
# from src/include for component list
#
# Create the $2 file from $1 using the tag in $3
#
# Usage: src/tools/release/mkversion.sh \
#        <VERSION_TEMPATE> \
#        <VERSION_FILE> \
#        <TAG>
#
# Example: src/tools/release/mkversion.sh \
#        components/router/shared/version.h.in \
#        components/router/shared/router_version.h \
#        AKASHI_REL_5_110_25
#
#  # $Id: mkversion.sh 641860 2016-06-06 14:48:07Z $
#

NULL="/dev/null"

VERSION_TEMPLATE="$1"
VERSION_FILE="$2"
COMPONENT_TAG="$3"
svncmd="svn --non-interactive"
SRCBASE=${SRCBASE:-..}

# Check for the in file, if not there we're probably in the wrong directory
if [ ! -f "$VERSION_TEMPLATE" ]; then
	echo file "$VERSION_TEMPLATE" not found
	exit 1
fi

# Following SVNURL should be expanded on checkout
SVNURL='$HeadURL: http://bcawlan-svn.sj.broadcom.net/svn/bcawlan/proj/tags/KUDU/KUDU_REL_17_10_121_37/src/tools/release/mkversion.sh $'

# .gclient_info is created by gclient checkout/sync steps
# and contains "DEPS='<deps-url1> <deps-url2> ..." entry
GCLIENT_INFO=${GCLIENT_INFO:-${SRCBASE}/../.gclient_info}

if [ -s "${GCLIENT_INFO}" ]; then
	source ${GCLIENT_INFO}
	if [ -z "$DEPS" ]; then
		echo "ERROR: DEPS entry missing in $GCLIENT_INFO"
		exit 1
	else
		for dep in $DEPS; do
			SVNURL=${SVNURL:-$dep}
			# Set SVNURL to first DEPS with /tags/ (if any)
			if [[ $dep == */tags/* ]]; then
				SVNURL=$dep
				echo "INFO: Found gclient DEPS: $SVNURL"
				break
			fi
		done
	fi
elif [ -f "${GCLIENT_INFO}" ]; then
	echo "ERROR: $GCLIENT_INFO is empty. Please verify its contents"
	exit 1
fi

# If SVNURL isn't expanded, extract it from svn info
if echo "$SVNURL" | egrep -vq 'HeadURL.*mkversion.sh|^[a-z+]+://.*/DEPS'; then
	[ -n "$VERBOSE" ] && \
		echo "DBG: SVN URL wasn't expanded. Getting it from svn info"
	SVNURL=$($svncmd info epivers.sh 2> $NULL | egrep "^URL:")
fi

# Tag should be in the form
#    <NAME>_REL_<MAJ>_<MINOR>
# or
#    <NAME>_REL_<MAJ>_<MINOR>_RC<RCNUM>
# or
#    <NAME>_REL_<MAJ>_<MINOR>_RC<RCNUM>_<INCREMENTAL>

# Given SVNURL path conventions or naming conventions, derive SVNTAG
# TO-DO: SVNTAG derivation logic can move to a central common API
# TO-DO: ${SRCBASE}/tools/build/svnurl2tag.sh
if [ "$COMPONENT_TAG" == "" -o "$COMPONENT_TAG" == "LOCAL_COMPONENT" ]; then
	case "${SVNURL}" in
	*_BRANCH_*)
		SVNTAG=$(echo $SVNURL | tr '/' '\n' | awk '/_BRANCH_/{printf "%s",$1}')
		;;
	*_TWIG_*)
		SVNTAG=$(echo $SVNURL | tr '/' '\n' | awk '/_TWIG_/{printf "%s",$1}')
		;;
	*_REL_*)
		SVNTAG=$(echo $SVNURL | tr '/' '\n' | awk '/_REL_/{printf "%s",$1}')
		;;
	*/branches/*)
		SVNTAG=${SVNURL#*/branches/}
		SVNTAG=${SVNTAG%%/*}
		;;
	*/proj/tags/*|*/deps/tags/*)
		SVNTAG=${SVNURL#*/tags/*/}
		SVNTAG=${SVNTAG%%/*}
		;;
	*/trunk/*)
		SVNTAG=$(date '+TRUNKURL_REL_%Y_%m_%d')
		;;
	*)
		SVNTAG=$(date '+OTHER_REL_%Y_%m_%d')
		;;
	esac

elif [ "$COMPONENT_TAG" == "trunk" ]; then
	SVNTAG=$(date '+TRUNK_REL_%Y_%m_%d')
fi

TAG=${SVNTAG:-${COMPONENT_TAG}}

echo "DBG: Component version string: $TAG"

# Split the tag into an array on underbar or whitespace boundaries.
IFS="_	     " tag=(${TAG})
unset IFS

tagged=1
if [ ${#tag[*]} -eq 0 ]; then
	tag=($(date '+TOT REL %Y %m %d 0 %y'));
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
	vernum=$(printf "0x%02x%02x%02x%02x" ${maj} ${min} ${rcnum} ${incremental})
else
	vernum=$(printf "0x00%02x%02x%02x" ${tag[7]} ${min} ${rcnum})
fi

# PR17029: increment minor number for tagged router builds
#         with an even minor revision
if [ ${tagged} -eq 1 -a $(expr \( \( ${min} + 1 \) % 2 \)) -eq 1 ]; then
	min_router=$(expr ${min} + 1)
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
	-e "s;@MOD_ROUTER_PACKAGE_VERSION@;${maj}.${min_router}.${rcnum}.${incremental};" \
	-e "s;@MOD_VERSION_NUM@;${vernum};" \
	< $VERSION_TEMPLATE > $VERSION_FILE
