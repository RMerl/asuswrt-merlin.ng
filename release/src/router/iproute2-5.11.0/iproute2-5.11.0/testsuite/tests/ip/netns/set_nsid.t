#!/bin/sh

. lib/generic.sh

ts_log "[Testing netns nsid]"

NS=testnsid
NSID=99

ts_ip "$0" "Add new netns $NS" netns add $NS
ts_ip "$0" "Set $NS nsid to $NSID" netns set $NS $NSID

ts_ip "$0" "List netns" netns list
test_on "$NS \(id: $NSID\)"

ts_ip "$0" "List netns without explicit list or show" netns
test_on "$NS \(id: $NSID\)"

ts_ip "$0" "List nsid" netns list-id
test_on "$NSID \(iproute2 netns name: $NS\)"

ts_ip "$0" "Delete netns $NS" netns del $NS
