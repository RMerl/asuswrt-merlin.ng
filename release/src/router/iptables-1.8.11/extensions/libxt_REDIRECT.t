:PREROUTING,OUTPUT
*nat
-p tcp -j REDIRECT --to-ports 42;=;OK
-p tcp -j REDIRECT --to-ports 0;=;OK
-p tcp -j REDIRECT --to-ports 65535;=;OK
-p tcp -j REDIRECT --to-ports 65536;;FAIL
-p udp -j REDIRECT --to-ports 0-0;-p udp -j REDIRECT --to-ports 0;OK
-p udp -j REDIRECT --to-ports 512-512;-p udp -j REDIRECT --to-ports 512;OK
-p udp -j REDIRECT --to-ports 42-1234;=;OK
-p tcp -j REDIRECT --to-ports 42-1234 --random;=;OK
-p tcp -j REDIRECT --to-ports 42-1234/567;;FAIL
-p tcp -j REDIRECT --to-ports ssh;-p tcp -j REDIRECT --to-ports 22;OK
-p tcp -j REDIRECT --to-ports ftp-data;-p tcp -j REDIRECT --to-ports 20;OK
-p tcp -j REDIRECT --to-ports ftp-ssh;;FAIL
-p tcp -j REDIRECT --to-ports 10-ssh;;FAIL
-j REDIRECT --to-ports 42;;FAIL
-j REDIRECT --random;=;OK
