:PREROUTING,INPUT,OUTPUT,POSTROUTING
*nat
-j NETMAP --to dead::/64;=;OK
-j NETMAP --to dead::beef;-j NETMAP --to dead::beef/128;OK
