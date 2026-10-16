:INPUT,FORWARD,OUTPUT
-m dst --dst-len 0;=;OK
-m dst --dst-opts 149:92,12:12,123:12;=;OK
-m dst ! --dst-len 42;=;OK
-m dst --dst-len 42 --dst-opts 149:92,12:12,123:12;=;OK
