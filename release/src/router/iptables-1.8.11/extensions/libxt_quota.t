:INPUT,FORWARD,OUTPUT
-m quota --quota 0;=;OK
-m quota ! --quota 0;=;OK
-m quota --quota 18446744073709551615;=;OK
-m quota ! --quota 18446744073709551615;=;OK
-m quota --quota 18446744073709551616;;FAIL
-m quota;;FAIL
