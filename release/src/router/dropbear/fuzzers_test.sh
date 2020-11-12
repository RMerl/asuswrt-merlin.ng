#!/bin/sh

# runs fuzz corpus with standalone fuzzers

result=0

test -d fuzzcorpus && hg --repository fuzzcorpus/ pull || hg clone https://hg.ucc.asn.au/dropbear-fuzzcorpus fuzzcorpus || exit 1
for f in `make list-fuzz-targets`; do
    ./$f fuzzcorpus/$f/* || result=1
done

exit $result
