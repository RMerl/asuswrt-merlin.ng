:INPUT,FORWARD,OUTPUT
# fails: foo does not exist
-j SET --add-set foo src,dst;;FAIL
