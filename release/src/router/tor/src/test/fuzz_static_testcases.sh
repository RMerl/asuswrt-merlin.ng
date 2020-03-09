#!/bin/sh

# Copyright (c) 2016-2019, The Tor Project, Inc.
# See LICENSE for licensing information

set -e

if [ -z "${TOR_FUZZ_CORPORA}" ] || [ ! -d "${TOR_FUZZ_CORPORA}" ] ; then
    echo "You need to set TOR_FUZZ_CORPORA to point to a checkout of "
    echo "the 'fuzzing-corpora' repository."
    exit 77
fi



for fuzzer in "${builddir:-.}"/src/test/fuzz/fuzz-* ; do
    f=$(basename "$fuzzer")
    case="${f#fuzz-}"
    if [ -d "${TOR_FUZZ_CORPORA}/${case}" ]; then
        echo "Running tests for ${case}"
        for entry in "${TOR_FUZZ_CORPORA}/${case}/"*; do
	    "${fuzzer}" "--err" < "$entry"
        done
    else
	echo "No tests found for ${case}"
    fi
done
