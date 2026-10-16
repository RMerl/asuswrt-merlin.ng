:INPUT,FORWARD,OUTPUT
-m iprange --src-range 1.1.1.1-1.1.1.10;=;OK
-m iprange ! --src-range 1.1.1.1-1.1.1.10;=;OK
-m iprange --dst-range 1.1.1.1-1.1.1.10;=;OK
-m iprange ! --dst-range 1.1.1.1-1.1.1.10;=;OK
# it shows -A INPUT -m iprange --src-range 1.1.1.1-1.1.1.1, should we support this?
# ERROR: should fail: iptables -A INPUT -m iprange --src-range 1.1.1.1
# -m iprange --src-range 1.1.1.1;;FAIL
# ERROR: should fail: iptables -A INPUT -m iprange --dst-range 1.1.1.1
#-m iprange --dst-range 1.1.1.1;;FAIL
