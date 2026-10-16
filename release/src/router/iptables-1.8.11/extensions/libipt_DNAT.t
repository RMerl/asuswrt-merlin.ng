:PREROUTING
*nat
-j DNAT --to-destination 1.1.1.1;=;OK
-j DNAT --to-destination 1.1.1.1-1.1.1.10;=;OK
-j DNAT --to-destination 1.1.1.1:1025-65535;;FAIL
-j DNAT --to-destination 1.1.1.1 --to-destination 2.2.2.2;;FAIL
-p tcp -j DNAT --to-destination 1.1.1.1:1025-65535;=;OK
-p tcp -j DNAT --to-destination 1.1.1.1-1.1.1.10:1025-65535;=;OK
-p tcp -j DNAT --to-destination 1.1.1.1-1.1.1.10:1025-65536;;FAIL
-p tcp -j DNAT --to-destination 1.1.1.1-1.1.1.10:1025-65535 --to-destination 2.2.2.2-2.2.2.20:1025-65535;;FAIL
-p tcp -j DNAT --to-destination 1.1.1.1:1000-2000/1000;=;OK
-p tcp -j DNAT --to-destination 1.1.1.1:1000-2000/3000;=;OK
-p tcp -j DNAT --to-destination 1.1.1.1:1000-2000/65535;=;OK
-p tcp -j DNAT --to-destination 1.1.1.1:1000-2000/0;;FAIL
-p tcp -j DNAT --to-destination 1.1.1.1:1000-2000/65536;;FAIL
-p tcp -j DNAT --to-destination 1.1.1.1:ssh;-p tcp -j DNAT --to-destination 1.1.1.1:22;OK
-p tcp -j DNAT --to-destination 1.1.1.1:ftp-data;-p tcp -j DNAT --to-destination 1.1.1.1:20;OK
-p tcp -j DNAT --to-destination 1.1.1.1:echo-ssh;;FAIL
-p tcp -j DNAT --to-destination 1.1.1.1:10-20/ftp;-p tcp -j DNAT --to-destination 1.1.1.1:10-20/21;OK
-j DNAT;;FAIL
