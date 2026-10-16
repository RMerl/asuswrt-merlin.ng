:PREROUTING,INPUT,FORWARD,OUTPUT,POSTROUTING
*mangle
-j TTL --ttl-set 42;=;OK
-j TTL --ttl-inc 1;=;OK
-j TTL --ttl-dec 1;=;OK
-j TTL --ttl-set 256;;FAIL
-j TTL --ttl-inc 0;;FAIL
-j TTL --ttl-dec 0;;FAIL
-j TTL --ttl-dec 1 --ttl-inc 1;;FAIL
-j TTL --ttl-set --ttl-inc 1;;FAIL
