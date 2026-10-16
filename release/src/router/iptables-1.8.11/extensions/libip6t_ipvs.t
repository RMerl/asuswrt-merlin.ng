:INPUT,FORWARD,OUTPUT
-m ipvs --vaddr 2001:db8::1;=;OK
-m ipvs ! --vaddr 2001:db8::/64;=;OK
-m ipvs --vproto 6 --vaddr 2001:db8::/64 --vport 22 --vdir ORIGINAL --vmethod GATE;=;OK
