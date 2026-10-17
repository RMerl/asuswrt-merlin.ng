#!/bin/bash

set -e

SCRIPT=$( realpath $0 )
RDPDIR=$( dirname "$SCRIPT" )
DEVELDIR=$( realpath "$RDPDIR/.." )
COMDIR=$( realpath "$DEVELDIR/.." )
RDP_VER=REL_6.2
export REPOVER=2.45  # --> pending Ticket#SR2113842, avoid update warnings

if [[ -n "$1" ]] ; then
    RDP_TOOLS_TARGET_DIR=$( realpath $1 )
elif [[ -z "${RDP_TOOLS_ROOT_DIR}" ]] ; then
    RDP_TOOLS_TARGET_DIR=$COMDIR/tools_$RDP_VER
    echo "WARNING: tools destination directory not set, syncing to $RDP_TOOLS_TARGET_DIR"
else
    RDP_TOOLS_TARGET_DIR=$RDP_TOOLS_ROOT_DIR/$RDP_VER
fi

mkdir -p ${RDP_TOOLS_TARGET_DIR}
cd ${RDP_TOOLS_TARGET_DIR}
if [ ! -d .repo ]; then
    echo "Extracting tools to ${RDP_TOOLS_TARGET_DIR}/tools"
    repo init \
        --repo-rev=v2.45 \
        -u ssh://bca-gerrit.ams.broadcom.net:29418/rdp_tools/manifests \
        -b $RDP_VER \
        -m cpe.xml
fi
repo sync
cd -

if [[ "$DEVELDIR/tools" != "$RDP_TOOLS_TARGET_DIR/tools" ]]; then
echo Linking $DEVELDIR/tools to $RDP_TOOLS_TARGET_DIR/tools
ln -fs $RDP_TOOLS_TARGET_DIR/tools $DEVELDIR/
fi

