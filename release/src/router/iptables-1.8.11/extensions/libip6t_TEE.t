:INPUT,FORWARD,OUTPUT
-j TEE --gateway 2001:db8::1;=;OK
-j TEE ! --gateway 2001:db8::1;;FAIL
