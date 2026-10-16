:POSTROUTING
*nat
-j SNAT --to-source dead::beef;=;OK
-j SNAT --to-source dead::beef-dead::fee7;=;OK
-j SNAT --to-source [dead::beef]:1025-65535;;FAIL
-j SNAT --to-source [dead::beef] --to-source [dead::fee7];;FAIL
-j SNAT --to-source dead::beef --random;=;OK
-j SNAT --to-source dead::beef --random-fully;=;OK
-j SNAT --to-source dead::beef --persistent;=;OK
-j SNAT --to-source dead::beef --random --persistent;=;OK
-j SNAT --to-source dead::beef --random --random-fully;=;OK
-j SNAT --to-source dead::beef --random --random-fully --persistent;=;OK
-p tcp -j SNAT --to-source [dead::beef]:1025-65535;=;OK
-p tcp -j SNAT --to-source [dead::beef-dead::fee7]:1025-65535;=;OK
-p tcp -j SNAT --to-source [dead::beef-dead::fee7]:1025-65536;;FAIL
-p tcp -j SNAT --to-source [dead::beef-dead::fee7]:1025-65535 --to-source [dead::beef-dead::fee8]:1025-65535;;FAIL
-j SNAT;;FAIL
