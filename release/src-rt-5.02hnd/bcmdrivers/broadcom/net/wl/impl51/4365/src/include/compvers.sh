#!/bin/bash
#
# Given a list of components, generate <component>_version.h
# from version.h.in in <component>'s directory
#
# Author: Prakash Dhavali
#
# Copyright 2005, Broadcom, Inc.
#
# $Id: compvers.sh 535184 2015-02-17 08:56:23Z $
#

# Optional argument
ACTION=$1
[ -n "$VERBOSE" ] && export VERBOSE

svncmd="svn --non-interactive"
SRCBASE=${SRCBASE:-..}

# List of components
# TODO: In the long term component versioning model, following list
# TODO: or table of components will come from a central file
COMPONENTS=(	\
	upnp	\
	phy	\
	router	\
	wps	\
)

SRCBASE_ROUTER=${SRCBASE}/../components/router
# Component dirs. Need one entry for each of above COMPONENTS
if [ -z "$SRCBASE_ROUTER" ]; then
    COMPONENT_DIR_upnp=${SRCBASE}/router/libupnp/include
    COMPONENT_DIR_router=${SRCBASE}/router/shared
else
    COMPONENT_DIR_upnp=${SRCBASE_ROUTER}/libupnp/include
    COMPONENT_DIR_router=${SRCBASE_ROUTER}/shared
fi

COMPONENT_DIR_phy=${SRCBASE}/wl/phy
COMPONENT_DIR_wps=${SRCBASE}/wps/common/include

# For a given component, query automerger for a different
# path than COMPONENT_DIR_<component>.
# Force router component to be pointing to local branch or tag.
COMPONENT_QUERY_router=src_force_local_component



# ===== DO NOT CHANGE ANYTHING BELOW THIS LINE =====

NULL=/dev/null
MKCOMPVER=${SRCBASE}/tools/release/mkversion.sh
MERGERLOG=${SRCBASE}/../merger_sources.log

# TODO: Post svn transition, network paths will be taken away
GETCOMPVER=getcompver.py
GETCOMPVER_NET=/projects/hnd_software/gallery/src/tools/build/$GETCOMPVER
GETCOMPVER_NET_WIN=Z:${GETCOMPVER_NET}

#
# If there is a local copy GETCOMPVER use it ahead of network copy
#
if [ -s "$GETCOMPVER" ]; then
	GETCOMPVER_PATH="$GETCOMPVER"
elif [ -s "${SRCBASE}/../src/tools/build/$GETCOMPVER" ]; then
	GETCOMPVER_PATH="${SRCBASE}/../src/tools/build/$GETCOMPVER"
elif [ -s "$GETCOMPVER_NET" ]; then
	GETCOMPVER_PATH="$GETCOMPVER_NET"
elif [ -s "$GETCOMPVER_NET_WIN" ]; then
	GETCOMPVER_PATH="$GETCOMPVER_NET_WIN"
fi

#
# If $GETCOMPVER isn't found, fetch it from SVN
# (this is very rare)
#
if [ ! -s "$GETCOMPVER_PATH" ]; then
	$svncmd export -q \
		^/proj/trunk/src/tools/build/${GETCOMPVER} \
		${GETCOMPVER} 2> $NULL
	GETCOMPVER_PATH=$GETCOMPVER
fi

#
# Now walk through each specified component to generate its
# component_version.h file from version.h.in template
#
for component in ${COMPONENTS[*]}
do
	# Get relative path of component from current dir
	tmp="COMPONENT_DIR_$component"
	eval rel_path=\$$tmp

	# Get query path for component
	tmp="COMPONENT_QUERY_$component"
	eval query_path=\$$tmp

	if [ ! -d "$rel_path" ]; then
		continue
	fi

	if [ "$query_path" != "" ]; then
		abs_path=$(echo $query_path | sed -e "s%\.\.%src%g")
	else
		abs_path=$(echo $rel_path | sed -e "s%\.\.%src%g")
	fi

	[ -n "$VERBOSE" ] && \
		echo "DBG: python $GETCOMPVER_PATH $MERGERLOG $abs_path"

	tag=$(python $GETCOMPVER_PATH $MERGERLOG $abs_path 2> $NULL | sed -e 's/[[:space:]]*//g')

	template=$rel_path/version.h.in
	verfile=$rel_path/${component}_version.h

	if [ "$ACTION" == "clean" ]; then
		rm -fv $verfile
		continue
	fi

	# MKCOMPVER always has defaults if tag isn't set correctly
	if [ ! -f "$verfile" -o "$FORCE" != "" ]; then
		echo ""
		echo ">>> Generate $abs_path/${component}_version.h${tag:+ from $tag}"

		[ -n "$VERBOSE" ] && \
			echo "DBG: bash $MKCOMPVER $template $verfile $tag"

		bash $MKCOMPVER $template $verfile $tag
	fi
done
