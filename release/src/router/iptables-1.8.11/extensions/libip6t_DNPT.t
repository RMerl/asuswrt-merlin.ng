:PREROUTING
*mangle
-j DNPT --src-pfx dead::/64 --dst-pfx 1c3::/64;=;OK
-j DNPT --src-pfx dead::beef --dst-pfx 1c3::/64;;FAIL
-j DNPT --src-pfx dead::/64;;FAIL
-j DNPT --dst-pfx dead::/64;;FAIL
-j DNPT;;FAIL
