#!/bin/sh
# Compares the parsed EXIF data extracted from test images with the parsed EXIF
# data in the original images. This tests that the tag parsing and writing
# round-trip produces an EXIF structure with the same meaning as the original.
srcdir="${srcdir:-.}"
TMPORIGINAL="$(mktemp)"
TMPEXTRACTED="$(mktemp)"
TMPDATA="$(mktemp)"
trap 'rm -f "${TMPORIGINAL}" "${TMPEXTRACTED}" "${TMPDATA}"' 0

# Remove the file name, which is a harmless difference between the two outputs.
# Also delete the size of the MakerNote. Since the MakerNote is parsed
# internally and rewritten, it can sometimes have slightly different padding
# and therefore slightly different size, which is a semantically meaningless
# difference.
# FIXME: Not all MakerNote differences are harmless. For example,
# olympus_makernote_variant_4.jpg has a huge size difference, probably because
# of a parsing bug in libexif. This should be investigated. Ideally, this would
# ignore small differences in size but trigger on larger differences.
parse_canonicalize () {
    sed \
        -e '/^File /d' \
        -e '/MakerNote (Undefined)$/{N;N;d}'
}

. ${srcdir}/inc-comparetool.sh

# Ensure that names are untranslated
LANG=
LANGUAGE=
LC_ALL=C
export LANG LANGUAGE LC_ALL
for fn in "${srcdir}"/testdata/*.jpg ; do
    ./test-parse$EXEEXT "${fn}" | tr -d '\015' | parse_canonicalize > "${TMPORIGINAL}"
    ./test-extract$EXEEXT -o "${TMPDATA}" "${fn}"
    ./test-parse$EXEEXT "${TMPDATA}" | tr -d '\015' | parse_canonicalize > "${TMPEXTRACTED}"
    if ${comparetool} "${TMPORIGINAL}" "${TMPEXTRACTED}"; then
	: "no differences detected"
    else
        echo Error parsing "$fn"
        exit 1
    fi
done

for fn in "${srcdir}"/testdata/*.jpg ; do
    ./test-parse$EXEEXT           "${fn}" | tr -d '\015' | parse_canonicalize > "${TMPORIGINAL}"
    ./test-parse-from-data$EXEEXT "${fn}" | tr -d '\015' | parse_canonicalize > "${TMPEXTRACTED}"
    if ${comparetool} "${TMPORIGINAL}" "${TMPEXTRACTED}"; then
	echo "no differences detected"
    else
        echo "ERROR: Difference between test-parse and test-parse-from-data for $fn !"
        exit 1
    fi
done
