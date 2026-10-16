:INPUT,FORWARD,OUTPUT
-m conntrack --ctorigsrc 2001:db8::1;=;OK
-m conntrack --ctorigdst 2001:db8::1;=;OK
-m conntrack --ctreplsrc 2001:db8::1;=;OK
-m conntrack --ctrepldst 2001:db8::1;=;OK
