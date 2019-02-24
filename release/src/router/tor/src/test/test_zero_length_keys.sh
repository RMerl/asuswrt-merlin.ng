#!/bin/sh
# Check that tor regenerates keys when key files are zero-length

exitcode=0

"${SHELL:-sh}" "${abs_top_srcdir:-.}/src/test/zero_length_keys.sh" "${builddir:-.}/src/app/tor" -z || exitcode=1
"${SHELL:-sh}" "${abs_top_srcdir:-.}/src/test/zero_length_keys.sh" "${builddir:-.}/src/app/tor" -d || exitcode=1
"${SHELL:-sh}" "${abs_top_srcdir:-.}/src/test/zero_length_keys.sh" "${builddir:-.}/src/app/tor" -e || exitcode=1

exit ${exitcode}
