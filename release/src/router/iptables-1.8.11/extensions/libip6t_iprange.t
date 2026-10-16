:INPUT,FORWARD,OUTPUT
-m iprange --src-range 2001:db8::1-2001:db8::10;=;OK
-m iprange ! --src-range 2001:db8::1-2001:db8::10;=;OK
-m iprange --dst-range 2001:db8::1-2001:db8::10;=;OK
-m iprange ! --dst-range 2001:db8::1-2001:db8::10;=;OK
# it shows -A INPUT -m iprange --src-range 2001:db8::1-2001:db8::1, should we support this?
# ERROR: should fail: ip6tables -A INPUT -m iprange --src-range 2001:db8::1
# -m iprange --src-range 2001:db8::1;;FAIL
# ERROR: should fail: ip6tables -A INPUT -m iprange --dst-range 2001:db8::1
#-m iprange --dst-range 2001:db8::1;;FAIL
