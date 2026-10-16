:INPUT,FORWARD,OUTPUT
-m ipvs --vaddr 1.2.3.4;=;OK
-m ipvs ! --vaddr 1.2.3.4/255.255.255.0;-m ipvs ! --vaddr 1.2.3.4/24;OK
-m ipvs --vproto 6 --vaddr 1.2.3.4/16 --vport 22 --vdir ORIGINAL --vmethod GATE;=;OK
