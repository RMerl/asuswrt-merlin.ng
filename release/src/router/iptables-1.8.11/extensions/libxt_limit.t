:INPUT,FORWARD,OUTPUT
-m limit --limit 1/sec;=;OK
-m limit --limit 1/min;=;OK
-m limit --limit 1000/hour;=;OK
-m limit --limit 1000/day;=;OK
-m limit --limit 1/sec --limit-burst 1;=;OK
