:POSTROUTING
*nat
-j SNAT --to-source 1.1.1.1;=;OK
-j SNAT --to-source 1.1.1.1-1.1.1.10;=;OK
-j SNAT --to-source 1.1.1.1:1025-65535;;FAIL
-j SNAT --to-source 1.1.1.1 --to-source 2.2.2.2;;FAIL
-j SNAT --to-source 1.1.1.1 --random;=;OK
-j SNAT --to-source 1.1.1.1 --random-fully;=;OK
-j SNAT --to-source 1.1.1.1 --persistent;=;OK
-j SNAT --to-source 1.1.1.1 --random --persistent;=;OK
-j SNAT --to-source 1.1.1.1 --random --random-fully;=;OK
-j SNAT --to-source 1.1.1.1 --random --random-fully --persistent;=;OK
-p tcp -j SNAT --to-source 1.1.1.1:1025-65535;=;OK
-p tcp -j SNAT --to-source 1.1.1.1-1.1.1.10:1025-65535;=;OK
-p tcp -j SNAT --to-source 1.1.1.1-1.1.1.10:1025-65536;;FAIL
-p tcp -j SNAT --to-source 1.1.1.1-1.1.1.10:1025-65535 --to-source 2.2.2.2-2.2.2.20:1025-65535;;FAIL
-j SNAT;;FAIL
