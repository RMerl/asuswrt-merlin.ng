:INPUT,FORWARD,OUTPUT
-m cpu --cpu 0;=;OK
-m cpu ! --cpu 0;=;OK
-m cpu --cpu 4294967295;=;OK
-m cpu --cpu 4294967296;;FAIL
-m cpu;;FAIL
