:PREROUTING,FORWARD,POSTROUTING
*mangle
-m cluster;;FAIL
-m cluster --cluster-total-nodes 3;;FAIL
-m cluster --cluster-total-nodes 2 --cluster-local-node 2;;FAIL
-m cluster --cluster-total-nodes 2 --cluster-local-node 3 --cluster-hash-seed;;FAIL
#
# outputs --cluster-local-nodemask instead of --cluster-local-node
#
-m cluster --cluster-total-nodes 2 --cluster-local-node 2 --cluster-hash-seed 0xfeedcafe;-m cluster --cluster-local-nodemask 0x00000002 --cluster-total-nodes 2 --cluster-hash-seed 0xfeedcafe;OK
