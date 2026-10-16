:INPUT,FORWARD,OUTPUT
-m ttl --ttl-eq 0;=;OK
-m ttl --ttl-eq 255;=;OK
-m ttl ! --ttl-eq 0;=;OK
-m ttl ! --ttl-eq 255;=;OK
-m ttl --ttl-gt 0;=;OK
# not possible have anything greater than 255, TTL is 8-bit long
# ERROR: should fail: iptables -A INPUT -m ttl --ttl-gt 255
## -m ttl --ttl-gt 255;;FAIL
# not possible have anything below 0
# ERROR: should fail: iptables -A INPUT -m ttl --ttl-lt 0
## -m ttl --ttl-lt 0;;FAIL
-m ttl --ttl-eq 256;;FAIL
-m ttl --ttl-eq -1;;FAIL
-m ttl;;FAIL
