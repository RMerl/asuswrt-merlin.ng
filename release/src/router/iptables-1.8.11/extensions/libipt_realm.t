:INPUT,FORWARD,OUTPUT
-m realm --realm 0x1/0x2a;=;OK
-m realm --realm 0x2a;=;OK
-m realm;;FAIL
