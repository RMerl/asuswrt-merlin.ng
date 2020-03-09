#!/bin/sh

[ "$1" = "-q" ] && quiet=true || quiet=false

set -e
SRC="http://pci-ids.ucw.cz/v2.2/pci.ids"
DEST=pci.ids
PCI_COMPRESSED_IDS=
GREP=grep

# if pci.ids is read-only (because the filesystem is read-only),
# then just skip this whole process.
if ! touch ${DEST} >/dev/null 2>&1 ; then
	${quiet} || echo "${DEST} is read-only, exiting." 1>&2
	exit 1
fi

if [ "$PCI_COMPRESSED_IDS" = 1 ] ; then
	DECOMP="cat"
	SRC="$SRC.gz"
	GREP=zgrep
elif which bzip2 >/dev/null 2>&1 ; then
	DECOMP="bzip2 -d"
	SRC="$SRC.bz2"
elif which gzip >/dev/null 2>&1 ; then
	DECOMP="gzip -d"
	SRC="$SRC.gz"
else
	DECOMP="cat"
fi

if which curl >/dev/null 2>&1 ; then
	DL="curl -o $DEST.new $SRC"
    ${quiet} && DL="$DL -s -S"
elif which wget >/dev/null 2>&1 ; then
	DL="wget --no-timestamping -O $DEST.new $SRC"
	${quiet} && DL="$DL -q"
elif which lynx >/dev/null 2>&1 ; then
	DL="eval lynx -source $SRC >$DEST.new"
else
	echo >&2 "update-pciids: cannot find curl, wget or lynx"
	exit 1
fi

if ! $DL ; then
	echo >&2 "update-pciids: download failed"
	rm -f $DEST.new
	exit 1
fi

if ! $DECOMP <$DEST.new >$DEST.neww ; then
	echo >&2 "update-pciids: decompression failed, probably truncated file"
	exit 1
fi

if ! $GREP >/dev/null "^C " $DEST.neww ; then
	echo >&2 "update-pciids: missing class info, probably truncated file"
	exit 1
fi

if [ -f $DEST ] ; then
	mv $DEST $DEST.old
	# --reference is supported only by chmod from GNU file, so let's ignore any errors
	chmod -f --reference=$DEST.old $DEST.neww 2>/dev/null || true
fi
mv $DEST.neww $DEST
rm $DEST.new

# Older versions did not compress the ids file, so let's make sure we
# clean that up.
if [ ${DEST%.gz} != ${DEST} ] ; then
	rm -f ${DEST%.gz} ${DEST%.gz}.old
fi

${quiet} || echo "Done."
