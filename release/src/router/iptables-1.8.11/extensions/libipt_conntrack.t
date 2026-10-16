:INPUT,FORWARD,OUTPUT
-m conntrack --ctorigsrc 1.1.1.1;=;OK
-m conntrack --ctorigdst 1.1.1.1;=;OK
-m conntrack --ctreplsrc 1.1.1.1;=;OK
-m conntrack --ctrepldst 1.1.1.1;=;OK
