:INPUT,FORWARD,OUTPUT
-j TEE --gateway 1.1.1.1;=;OK
-j TEE ! --gateway 1.1.1.1;;FAIL
