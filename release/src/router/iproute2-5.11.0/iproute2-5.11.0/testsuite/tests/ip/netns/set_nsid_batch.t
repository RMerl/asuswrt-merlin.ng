#!/bin/sh

. lib/generic.sh

ts_log "[Testing netns nsid in batch mode]"

NS=testnsid
NSID=99
BATCHFILE=`mktemp`

echo "netns add $NS" >> $BATCHFILE
echo "netns set $NS $NSID" >> $BATCHFILE
echo "netns list-id" >> $BATCHFILE
ts_ip "$0" "Add ns, set nsid and list in batch mode" -b $BATCHFILE
test_on "nsid $NSID \(iproute2 netns name: $NS\)"
rm -f $BATCHFILE

ts_ip "$0" "Delete netns $NS" netns del $NS
