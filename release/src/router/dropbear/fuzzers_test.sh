#!/bin/sh

# runs fuzz corpus with standalone fuzzers

result=0

hg clone https://secure.ucc.asn.au/hg/dropbear-fuzzcorpus fuzzcorpus || exit 1
for f in `make list-fuzz-targets`; do
    ./$f fuzzcorpus/$f/* || result=1
done

exit $result
