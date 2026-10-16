:INPUT,FORWARD,OUTPUT
-j LOG;-j LOG;OK
-j LOG --log-prefix "test: ";=;OK
-j LOG --log-prefix "test: " --log-level 1;=;OK
# iptables displays the log-level output using the number; not the string
-j LOG --log-prefix "test: " --log-level alert;-j LOG --log-prefix "test: " --log-level 1;OK
-j LOG --log-prefix "test: " --log-tcp-sequence;=;OK
-j LOG --log-prefix "test: " --log-tcp-options;=;OK
-j LOG --log-prefix "test: " --log-ip-options;=;OK
-j LOG --log-prefix "test: " --log-uid;=;OK
-j LOG --log-prefix "test: " --log-level bad;;FAIL
-j LOG --log-prefix;;FAIL
