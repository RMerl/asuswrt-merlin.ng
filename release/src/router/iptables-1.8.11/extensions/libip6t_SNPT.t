:INPUT,POSTROUTING
*mangle
-j SNPT --src-pfx dead::/64 --dst-pfx 1c3::/64;=;OK
-j SNPT --src-pfx dead::beef --dst-pfx 1c3::/64;;FAIL
-j SNPT --src-pfx dead::/64;;FAIL
-j SNPT --dst-pfx dead::/64;;FAIL
-j SNPT;;FAIL
