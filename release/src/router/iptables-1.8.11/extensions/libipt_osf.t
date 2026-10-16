:INPUT,FORWARD
-m osf --genre linux --ttl 0 --log 0;;FAIL
-p tcp -m osf --genre linux --ttl 0 --log 0;=;OK
-p tcp -m osf --genre linux --ttl 3 --log 0;;FAIL
