:INPUT,FORWARD,OUTPUT
-m ecn --ecn-tcp-cwr;;FAIL
-p tcp -m ecn --ecn-tcp-cwr;=;OK
-p tcp -m ecn --ecn-tcp-ece --ecn-tcp-cwr --ecn-ip-ect 2;=;OK
-p tcp -m ecn ! --ecn-tcp-ece ! --ecn-tcp-cwr ! --ecn-ip-ect 2;=;OK
