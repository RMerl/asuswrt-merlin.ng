:INPUT,FORWARD,OUTPUT
--pkttype-type ! host;! --pkttype-type host -j CONTINUE;OK
--pkttype-type host;=;OK
! --pkttype-type host;=;OK
--pkttype-type broadcast;=;OK
! --pkttype-type broadcast;=;OK
--pkttype-type multicast;=;OK
! --pkttype-type multicast;=;OK
--pkttype-type otherhost;=;OK
! --pkttype-type otherhost;=;OK
--pkttype-type outgoing;=;OK
! --pkttype-type outgoing;=;OK
--pkttype-type loopback;=;OK
! --pkttype-type loopback;=;OK
