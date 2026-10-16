:INPUT,FORWARD,OUTPUT
-j NFLOG --nflog-group 1;=;OK
-j NFLOG --nflog-group 65535;=;OK
-j NFLOG --nflog-group 65536;;FAIL
-j NFLOG --nflog-group 0;-j NFLOG;OK
# `--nflog-range` is broken and only supported by xtables-legacy.
# It has been superseded by `--nflog--group`.
-j NFLOG --nflog-range 1;=;OK;LEGACY;NOMATCH
-j NFLOG --nflog-range 4294967295;=;OK;LEGACY;NOMATCH
-j NFLOG --nflog-range 4294967296;;FAIL
-j NFLOG --nflog-range -1;;FAIL
-j NFLOG --nflog-size 0;=;OK
-j NFLOG --nflog-size 1;=;OK
-j NFLOG --nflog-size 4294967295;=;OK
-j NFLOG --nflog-size 4294967296;;FAIL
-j NFLOG --nflog-size -1;;FAIL
-j NFLOG --nflog-prefix xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;=;OK
-j NFLOG --nflog-prefix xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;-j NFLOG --nflog-prefix xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;OK;LEGACY;=
-j NFLOG --nflog-threshold 1;=;OK
# ERROR: line 13 (should fail: iptables -A INPUT -j NFLOG --nflog-threshold 0
# -j NFLOG --nflog-threshold 0;;FAIL
-j NFLOG --nflog-threshold 65535;=;OK
-j NFLOG --nflog-threshold 65536;;FAIL
-j NFLOG;=;OK
