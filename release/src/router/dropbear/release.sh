#!/bin/sh

set -e

if [ "$1" = '--testrel' ]; then
    # --testrel won't check changelog version correctness and will build in a temporary dir
    TESTREL=1
else
    TESTREL=0
fi

VERSION=$(echo '#include "default_options.h"\n#include "sysoptions.h"\necho DROPBEAR_VERSION' | cpp -DHAVE_CRYPT - | sh)

if [ $TESTREL -eq 1 ]; then
    echo Making test tarball for "$VERSION" ...
    echo Not checking version mismatches.
    WORKDIR=$(mktemp -d)
    TARSUFFIX="-testrel"
else
    echo Releasing version "$VERSION" ...
    if ! head -n1 CHANGES | grep -q $VERSION ; then
        echo "CHANGES needs updating"
        exit 1
    fi

    if ! head -n1 debian/changelog | grep -q $VERSION ; then
        echo "debian/changelog needs updating"
        exit 1
    fi
    WORKDIR=$PWD/..
    TARSUFFIX=""
fi

RELDIR=$WORKDIR/dropbear-$VERSION
ARCHIVE=${RELDIR}${TARSUFFIX}.tar.bz2

head -n1 CHANGES

if tar --version | grep -q 'GNU tar'; then
	TAR=tar
else
	TAR=gtar
fi

if test -e $RELDIR; then
	echo "$RELDIR exists"
	exit 1
fi

if test -e $ARCHIVE; then
	echo "$ARCHIVE exists"
	exit 1
fi

if [ -d .hg ]; then
    hg archive "$RELDIR"  || exit 2
    # .hg_archival.txt seems to differ between hg versions, isn't good for reproducibility
    rm "$RELDIR/.hg_archival.txt"
elif [ -d .git ]; then
    git -c tar.umask=0022 archive --format tar -o /dev/stdout --prefix=dropbear-$VERSION/ HEAD | tar xf - -C $WORKDIR || exit 2
else
    echo "This isn't a hg or git checkout"
    exit 1
fi

chmod -R a+rX $RELDIR

RELDATE=$(head -n1 CHANGES | cut -d - -f 2)
# timezone keeps it consistent, choose a plausible release time
RELTIME="22:30:00 +0800"

# from https://reproducible-builds.org/docs/archives/
TAROPTS="--sort=name --owner=0 --group=0 --numeric-owner"
(cd "$RELDIR/.." && $TAR cjf $ARCHIVE $TAROPTS --mtime="$RELDATE $RELTIME" `basename "$RELDIR"`) || exit 2

ls -l $ARCHIVE
openssl sha256 $ARCHIVE
echo Done to
echo "$ARCHIVE"

if [ $TESTREL -eq 0 ]; then
    echo Sign it with
    echo gpg2 --detach-sign -a -u F29C6773 "$ARCHIVE"
fi
