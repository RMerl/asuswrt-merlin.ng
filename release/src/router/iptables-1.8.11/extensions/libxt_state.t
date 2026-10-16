:INPUT,FORWARD,OUTPUT
-m state --state INVALID;=;OK
-m state --state NEW,RELATED;=;OK
-m state --state UNTRACKED;=;OK
-m state wrong;;FAIL
-m state;;FAIL
