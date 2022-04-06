#!/bin/sh

# runs fuzz corpus with standalone fuzzers

result=0

test -d fuzzcorpus && hg --repository fuzzcorpus/ pull || hg clone https://hg.ucc.asn.au/dropbear-fuzzcorpus fuzzcorpus || exit 1
for f in `make list-fuzz-targets`; do
    # use xargs to split the too-long argument list
    # -q quiet because travis has a logfile limit
    echo fuzzcorpus/$f/* | xargs -n 1000 ./$f -q || result=1
done

exit $result
