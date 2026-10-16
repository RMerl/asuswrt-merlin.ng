:INPUT,FORWARD,OUTPUT
-m addrtype;;FAIL
-m addrtype --src-type wrong;;FAIL
-m addrtype --src-type UNSPEC;=;OK
-m addrtype --dst-type UNSPEC;=;OK
-m addrtype --src-type LOCAL --dst-type LOCAL;=;OK
-m addrtype --dst-type UNSPEC;=;OK
-m addrtype --limit-iface-in;;FAIL
-m addrtype --limit-iface-out;;FAIL
-m addrtype --limit-iface-in --limit-iface-out;;FAIL
-m addrtype --src-type LOCAL --limit-iface-in --limit-iface-out;;FAIL
:INPUT
-m addrtype --src-type LOCAL --limit-iface-in;=;OK
-m addrtype --dst-type LOCAL --limit-iface-in;=;OK
:OUTPUT
-m addrtype --src-type LOCAL --limit-iface-out;=;OK
-m addrtype --dst-type LOCAL --limit-iface-out;=;OK
