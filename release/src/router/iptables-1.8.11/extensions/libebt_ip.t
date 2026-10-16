:INPUT,FORWARD,OUTPUT
-p ip ! --ip-src 192.168.0.0/24 -j ACCEPT;-p IPv4 ! --ip-src 192.168.0.0/24 -j ACCEPT;OK
-p IPv4 --ip-dst 10.0.0.1;=;OK
-p IPv4 ! --ip-dst 10.0.0.1;=;OK
-p IPv4 --ip-tos 0xFF;=;OK
-p IPv4 ! --ip-tos 0xFF;=;OK
-p IPv4 --ip-proto tcp --ip-dport 22;=;OK
-p IPv4 --ip-proto udp --ip-sport 1024:65535;=;OK
-p IPv4 --ip-proto udp --ip-sport :;-p IPv4 --ip-proto udp --ip-sport 0:65535;OK
-p IPv4 --ip-proto udp --ip-sport :4;-p IPv4 --ip-proto udp --ip-sport 0:4;OK
-p IPv4 --ip-proto udp --ip-sport 4:;-p IPv4 --ip-proto udp --ip-sport 4:65535;OK
-p IPv4 --ip-proto udp --ip-sport 3:4;=;OK
-p IPv4 --ip-proto udp --ip-sport 4:4;-p IPv4 --ip-proto udp --ip-sport 4;OK
-p IPv4 --ip-proto udp --ip-sport 4:3;;FAIL
-p IPv4 --ip-proto udp --ip-dport :;-p IPv4 --ip-proto udp --ip-dport 0:65535;OK
-p IPv4 --ip-proto udp --ip-dport :4;-p IPv4 --ip-proto udp --ip-dport 0:4;OK
-p IPv4 --ip-proto udp --ip-dport 4:;-p IPv4 --ip-proto udp --ip-dport 4:65535;OK
-p IPv4 --ip-proto udp --ip-dport 3:4;=;OK
-p IPv4 --ip-proto udp --ip-dport 4:4;-p IPv4 --ip-proto udp --ip-dport 4;OK
-p IPv4 --ip-proto udp --ip-dport 4:3;;FAIL
-p IPv4 --ip-proto 253;=;OK
-p IPv4 ! --ip-proto 253;=;OK
-p IPv4 --ip-proto icmp --ip-icmp-type echo-request;=;OK
-p IPv4 --ip-proto icmp --ip-icmp-type 1/1;=;OK
-p ip --ip-protocol icmp ! --ip-icmp-type 1:10;-p IPv4 --ip-proto icmp ! --ip-icmp-type 1:10/0:255 -j CONTINUE;OK
--ip-proto icmp --ip-icmp-type 1/1;=;FAIL
! -p ip --ip-proto icmp --ip-icmp-type 1/1;=;FAIL
! -p ip --ip-proto tcp --ip-sport 22 --ip-icmp-type echo-reply;;FAIL
! -p ip --ip-proto tcp --ip-sport 22 --ip-igmp-type membership-query;;FAIL
! -p ip --ip-proto tcp --ip-dport 22 --ip-icmp-type echo-reply;;FAIL
! -p ip --ip-proto tcp --ip-dport 22 --ip-igmp-type membership-query;;FAIL
! -p ip --ip-proto icmp --ip-icmp-type echo-reply --ip-igmp-type membership-query;;FAIL
-p IPv4 --ip-proto icmp ! --ip-icmp-type echo-reply;=;OK
