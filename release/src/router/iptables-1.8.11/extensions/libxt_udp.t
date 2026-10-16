:INPUT,OUTPUT,FORWARD
-p udp -m udp --sport 1;=;OK
-p udp -m udp --sport 65535;=;OK
-p udp -m udp --dport 1;=;OK
-p udp -m udp --dport 65535;=;OK
-p udp -m udp --sport 1:1023;=;OK
-p udp -m udp --sport 1024:65535;=;OK
-p udp -m udp --sport 1024:;-p udp -m udp --sport 1024:65535;OK
-p udp -m udp --sport :;-p udp -m udp;OK
-p udp -m udp ! --sport :;-p udp -m udp ! --sport 0:65535;OK
-p udp -m udp --sport :4;-p udp -m udp --sport 0:4;OK
-p udp -m udp --sport 4:;-p udp -m udp --sport 4:65535;OK
-p udp -m udp --sport 4:4;-p udp -m udp --sport 4;OK
-p udp -m udp --sport 4:3;;FAIL
-p udp -m udp --dport :;-p udp -m udp;OK
-p udp -m udp ! --dport :;-p udp -m udp ! --dport 0:65535;OK
-p udp -m udp --dport :4;-p udp -m udp --dport 0:4;OK
-p udp -m udp --dport 4:;-p udp -m udp --dport 4:65535;OK
-p udp -m udp --dport 4:4;-p udp -m udp --dport 4;OK
-p udp -m udp --dport 4:3;;FAIL
-p udp -m udp ! --sport 1;=;OK
-p udp -m udp ! --sport 65535;=;OK
-p udp -m udp ! --dport 1;=;OK
-p udp -m udp ! --dport 65535;=;OK
-p udp -m udp --sport 1 --dport 65535;=;OK
-p udp -m udp --sport 65535 --dport 1;=;OK
-p udp -m udp ! --sport 1 --dport 65535;=;OK
-p udp -m udp ! --sport 65535 --dport 1;=;OK
# ERRROR: should fail: iptables -A INPUT -p udp -m udp --sport 65536
# -p udp -m udp --sport 65536;;FAIL
-p udp -m udp --sport -1;;FAIL
-p udp -m udp --dport -1;;FAIL
-m udp --dport 1;;FAIL
-m udp --dport 1 -p udp;-p udp -m udp --dport 1;OK
-m udp --dport 1 -p 17;-p udp -m udp --dport 1;OK
# should we accept this below?
-p udp -m udp;=;OK
