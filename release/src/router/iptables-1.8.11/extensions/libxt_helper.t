:INPUT,FORWARD,OUTPUT
-m helper --helper ftp;=;OK
# should be OK?
# ERROR: should fail: iptables -A INPUT -m helper --helper wrong
# -m helper --helper wrong;;FAIL
-m helper;;FAIL
