:INPUT,FORWARD,OUTPUT
-p tcp -m multiport --sports 53,1024:65535;=;OK
-p tcp -m multiport --dports 53,1024:65535;=;OK
-p udp -m multiport --sports 53,1024:65535;=;OK
-p udp -m multiport --dports 53,1024:65535;=;OK
-p udp -m multiport --ports 53,1024:65535;=;OK
-p udp -m multiport --ports 53,1024:65535;=;OK
-p sctp -m multiport --sports 53,1024:65535;=;OK
-p sctp -m multiport --dports 53,1024:65535;=;OK
-p dccp -m multiport --sports 53,1024:65535;=;OK
-p dccp -m multiport --dports 53,1024:65535;=;OK
-p udplite -m multiport --sports 53,1024:65535;=;OK
-p udplite -m multiport --dports 53,1024:65535;=;OK
-p tcp -m multiport --sports 1024:65536;;FAIL
-p udp -m multiport --sports 1024:65536;;FAIL
-p tcp -m multiport --ports 1024:65536;;FAIL
-p udp -m multiport --ports 1024:65536;;FAIL
-p tcp -m multiport --ports 1,2,3,4,6,7,8,9,10,11,12,13,14,15;=;OK
# fix manpage, it says "up to 15 ports supported"
# ERROR: should fail: iptables -A INPUT -p tcp -m multiport --ports 1,2,3,4,6,7,8,9,10,11,12,13,14,15,16
# -p tcp -m multiport --ports 1,2,3,4,6,7,8,9,10,11,12,13,14,15,16;;FAIL
-p tcp --multiport;;FAIL
-m multiport;;FAIL
