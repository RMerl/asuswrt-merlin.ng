:PREROUTING,FORWARD,OUTPUT,POSTROUTING
*mangle
-j ECN;;FAIL
-p tcp -j ECN;;FAIL
-p tcp -j ECN --ecn-tcp-remove;=;OK
