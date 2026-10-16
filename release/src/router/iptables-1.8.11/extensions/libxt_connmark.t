:PREROUTING,FORWARD,OUTPUT,POSTROUTING
*mangle
-m connmark --mark 0xffffffff;=;OK
-m connmark --mark 0xffffffff/0xffffffff;-m connmark --mark 0xffffffff;OK
-m connmark --mark 0xffffffff/0x0;=;OK
-m connmark --mark 0/0xffffffff;-m connmark --mark 0x0;OK
-m connmark --mark -1;;FAIL
-m connmark --mark 0xfffffffff;;FAIL
-m connmark;;FAIL
