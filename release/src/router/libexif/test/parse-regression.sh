#!/bin/sh
# Parses test EXIF files and compares the results to that expected
srcdir="${srcdir:-.}"
TMPLOG="$(mktemp)"
trap 'rm -f "${TMPLOG}"' 0

. ${srcdir}/inc-comparetool.sh

# Ensure that names are untranslated
LANG=
LANGUAGE=
LC_ALL=C
export LANG LANGUAGE LC_ALL
for fn in "${srcdir}"/testdata/*.jpg ; do
    # The *.parsed text files have LF line endings, so the tr removes
    # the CR from CRLF line endings, while keeping LF line endings the
    # same.
    ./test-parse$EXEEXT "${fn}" | tr -d '\015' > "${TMPLOG}"
    if ${comparetool} "${fn}.parsed" "${TMPLOG}"; then
	: "no differences detected"
    else
        echo "Error parsing $fn"
        exit 1
    fi
done
