:PREROUTING
*raw
-m eui64;=;OK
:INPUT,FORWARD
*filter
-m eui64;=;OK
:OUTPUT
-m eui64;;FAIL
