#!/bin/sh

ipset x test >/dev/null 2>&1
ipset n test hash:ip
for x in `seq 0 255`; do
    for y in `seq 0 255`; do
        echo "a test 10.10.$x.$y"
    done
done | ipset r
ipset -t list > .foo
diff .foo big_sort.terse
ipset -s save > .foo
diff .foo big_sort.saved
ipset x test
