:INPUT,FORWARD,OUTPUT
-m ipv6header --header hop-by-hop;=;OK
-m ipv6header --header hop-by-hop --soft;=;OK
-m ipv6header --header ipv6-nonxt;=;OK
