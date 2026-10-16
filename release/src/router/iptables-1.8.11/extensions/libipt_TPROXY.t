:PREROUTING
*mangle
-j TPROXY --on-port 12345 --on-ip 10.0.0.1 --tproxy-mark 0x23/0xff;;FAIL
-p udp -j TPROXY --on-port 12345 --on-ip 10.0.0.1 --tproxy-mark 0x23/0xff;=;OK
-p tcp -m tcp --dport 2342 -j TPROXY --on-port 12345 --on-ip 10.0.0.1 --tproxy-mark 0x23/0xff;=;OK
