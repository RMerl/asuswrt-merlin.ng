:INPUT,FORWARD,OUTPUT
-j AUDIT --type accept;=;OK
-j AUDIT --type drop;=;OK
-j AUDIT --type reject;=;OK
-j AUDIT;;FAIL
-j AUDIT --type wrong;;FAIL
