:INPUT,FORWARD,OUTPUT
-m hashlimit --hashlimit-above 1/sec --hashlimit-burst 5 --hashlimit-name mini1;=;OK
-m hashlimit --hashlimit-above 1000000/sec --hashlimit-burst 5 --hashlimit-name mini1;=;OK
-m hashlimit --hashlimit-above 1/min --hashlimit-burst 5 --hashlimit-name mini1;=;OK
-m hashlimit --hashlimit-above 1/hour --hashlimit-burst 5 --hashlimit-name mini1;=;OK
-m hashlimit --hashlimit-above 1/day --hashlimit-burst 1 --hashlimit-name mini1;=;OK
-m hashlimit --hashlimit-upto 1/sec --hashlimit-burst 5 --hashlimit-name mini1;=;OK
-m hashlimit --hashlimit-upto 1000000/sec --hashlimit-burst 5 --hashlimit-name mini1;=;OK
-m hashlimit --hashlimit-upto 1/min --hashlimit-burst 5 --hashlimit-name mini1;=;OK
-m hashlimit --hashlimit-upto 1/hour --hashlimit-burst 5 --hashlimit-name mini1;=;OK
-m hashlimit --hashlimit-upto 1/day --hashlimit-burst 1 --hashlimit-name mini1;=;OK
-m hashlimit --hashlimit-upto 1/sec --hashlimit-burst 1 --hashlimit-name mini1 --hashlimit-htable-expire 2000;=;OK
-m hashlimit --hashlimit-upto 1/sec --hashlimit-burst 1 --hashlimit-mode srcip --hashlimit-name mini1 --hashlimit-htable-expire 2000;=;OK
-m hashlimit --hashlimit-upto 1/sec --hashlimit-burst 1 --hashlimit-mode dstip --hashlimit-name mini1 --hashlimit-htable-expire 2000;=;OK
-m hashlimit --hashlimit-upto 1/sec --hashlimit-burst 1 --hashlimit-mode dstip --hashlimit-name mini1 --hashlimit-htable-max 2000 --hashlimit-htable-expire 2000;=;OK
-m hashlimit --hashlimit-upto 1/sec --hashlimit-burst 1 --hashlimit-mode dstip --hashlimit-name mini1 --hashlimit-htable-max 2000 --hashlimit-htable-gcinterval 60000 --hashlimit-htable-expire 2000;=;OK
-m hashlimit --hashlimit-upto 1/sec --hashlimit-name mini1;-m hashlimit --hashlimit-upto 1/sec --hashlimit-burst 5 --hashlimit-name mini1;OK
-m hashlimit --hashlimit-upto 4kb/s --hashlimit-burst 400kb --hashlimit-name mini5;=;OK
-m hashlimit --hashlimit-upto 10mb/s --hashlimit-name mini6;=;OK
-m hashlimit --hashlimit-upto 123456b/s --hashlimit-burst 1mb --hashlimit-name mini7;=;OK
# should work, it says "iptables v1.4.15: burst cannot be smaller than 96b"
# ERROR: cannot load: iptables -A INPUT -m hashlimit --hashlimit-upto 96b/s --hashlimit-burst 5 --hashlimit-name mini1
# -m hashlimit --hashlimit-upto 96b/s --hashlimit-burst 5 --hashlimit-name mini1;=;OK
-m hashlimit --hashlimit-name mini1;;FAIL
-m hashlimit --hashlimit-upto 1/sec;;FAIL
-m hashlimit;;FAIL
-m hashlimit --hashlimit-upto 40/sec --hashlimit-burst 20 --hashlimit-mode srcip --hashlimit-name syn-flood;=;OK
-m hashlimit --hashlimit-upto 40/sec --hashlimit-burst 20 --hashlimit-mode srcip --hashlimit-name rate1 --hashlimit-rate-match;=;OK
-m hashlimit --hashlimit-upto 40mb/s --hashlimit-mode srcip --hashlimit-name rate2 --hashlimit-rate-match;=;OK
-m hashlimit --hashlimit-upto 40/sec --hashlimit-burst 20 --hashlimit-mode srcip --hashlimit-name rate3 --hashlimit-rate-match --hashlimit-rate-interval 10;=;OK
-m hashlimit --hashlimit-upto 40mb/s --hashlimit-mode srcip --hashlimit-name rate4 --hashlimit-rate-match --hashlimit-rate-interval 10;=;OK
