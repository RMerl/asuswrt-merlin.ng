:INPUT,FORWARD,OUTPUT
-m connbytes --connbytes 0:1000 --connbytes-mode packets --connbytes-dir original;=;OK
-m connbytes --connbytes 0:1000 --connbytes-mode packets --connbytes-dir reply;=;OK
-m connbytes --connbytes 0:1000 --connbytes-mode packets --connbytes-dir both;=;OK
-m connbytes --connbytes 0:1000 --connbytes-mode bytes --connbytes-dir original;=;OK
-m connbytes --connbytes 0:1000 --connbytes-mode bytes --connbytes-dir reply;=;OK
-m connbytes --connbytes 0:1000 --connbytes-mode bytes --connbytes-dir both;=;OK
-m connbytes --connbytes 0:1000 --connbytes-mode avgpkt --connbytes-dir original;=;OK
-m connbytes --connbytes 0:1000 --connbytes-mode avgpkt --connbytes-dir reply;=;OK
-m connbytes --connbytes 0:1000 --connbytes-mode avgpkt --connbytes-dir both;=;OK
-m connbytes --connbytes -1:0 --connbytes-mode packets --connbytes-dir original;;FAIL
-m connbytes --connbytes 0:-1 --connbytes-mode packets --connbytes-dir original;;FAIL
-m connbytes --connbytes : --connbytes-mode packets --connbytes-dir original;-m connbytes --connbytes 0 --connbytes-mode packets --connbytes-dir original;OK
-m connbytes --connbytes :1000 --connbytes-mode packets --connbytes-dir original;-m connbytes --connbytes 0:1000 --connbytes-mode packets --connbytes-dir original;OK
-m connbytes --connbytes 1000 --connbytes-mode packets --connbytes-dir original;=;OK
-m connbytes --connbytes 1000: --connbytes-mode packets --connbytes-dir original;-m connbytes --connbytes 1000 --connbytes-mode packets --connbytes-dir original;OK
-m connbytes --connbytes 1000:1000 --connbytes-mode packets --connbytes-dir original;=;OK
-m connbytes --connbytes 1000:0 --connbytes-mode packets --connbytes-dir original;;FAIL
# ERROR: cannot find: iptables -I INPUT -m connbytes --connbytes 0:18446744073709551615 --connbytes-mode avgpkt --connbytes-dir both
# -m connbytes --connbytes 0:18446744073709551615 --connbytes-mode avgpkt --connbytes-dir both;=;OK
-m connbytes --connbytes 0:18446744073709551616 --connbytes-mode avgpkt --connbytes-dir both;;FAIL
-m connbytes --connbytes 0:1000 --connbytes-mode wrong --connbytes-dir both;;FAIL
-m connbytes --connbytes 0:1000 --connbytes-dir original;;FAIL
-m connbytes --connbytes 0:1000 --connbytes-mode packets;;FAIL
-m connbytes --connbytes-dir original;;FAIL
-m connbytes --connbytes 0:1000;;FAIL
-m connbytes;;FAIL
