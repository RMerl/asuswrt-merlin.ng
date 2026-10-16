:INPUT,FORWARD,OUTPUT
-m hbh;=;OK
-m hbh --hbh-len 42;=;OK
-m hbh ! --hbh-len 42;=;OK
-m hbh --hbh-len 42 --hbh-opts 1:2,23:42,4:6,8:10,42,23,4:5;=;OK
