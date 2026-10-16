:INPUT,FORWARD,OUTPUT
-m set --match-set foo;;FAIL
# fails: foo does not exist
-m set --match-set foo src,dst;;FAIL
