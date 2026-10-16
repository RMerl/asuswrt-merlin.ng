:POSTROUTING
*nat
-j MASQUERADE;=;OK
-j MASQUERADE --random;=;OK
-j MASQUERADE --random-fully;=;OK
-p tcp -j MASQUERADE --to-ports 1024;=;OK
-p udp -j MASQUERADE --to-ports 1024-65535;=;OK
-p udp -j MASQUERADE --to-ports 1024-65536;;FAIL
-p udp -j MASQUERADE --to-ports -1;;FAIL
