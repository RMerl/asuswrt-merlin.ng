:INPUT,FORWARD
-m mac --mac-source 42:01:02:03:04:05;=;OK
-m mac --mac-source 42:01:02:03:04;=;FAIL
-m mac --mac-source 42:01:02:03:04:05:06;=;FAIL
-m mac;;FAIL
