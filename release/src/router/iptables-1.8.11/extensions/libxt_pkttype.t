:INPUT,FORWARD,OUTPUT
-m pkttype --pkt-type unicast;=;OK
-m pkttype --pkt-type broadcast;=;OK
-m pkttype --pkt-type multicast;=;OK
-m pkttype --pkt-type wrong;;FAIL
-m pkttype;;FAIL
