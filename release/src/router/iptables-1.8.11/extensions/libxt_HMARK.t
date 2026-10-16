:INPUT,FORWARD,OUTPUT
-j HMARK;;FAIL
-j HMARK --hmark-src-prefix 32 --hmark-rnd 0x00000004 --hmark-mod 42;=;OK
-j HMARK --hmark-src-prefix 32 --hmark-dst-prefix 32 --hmark-sport-mask 0xffff --hmark-dport-mask 0xffff --hmark-proto-mask 0xffff --hmark-rnd 0x00000004 --hmark-mod 42 --hmark-offset 1 --hmark-tuple ct;=;OK
-j HMARK --hmark-src-prefix 32 --hmark-dst-prefix 32 --hmark-spi-mask 0x00000004 --hmark-proto-mask 0xffff --hmark-rnd 0x00000004 --hmark-mod 42 --hmark-offset 1 --hmark-tuple ct;=;OK
-j HMARK --hmark-src-prefix 1 --hmark-dst-prefix 2 --hmark-sport-mask 0x0003 --hmark-dport-mask 0x0004 --hmark-proto-mask 0x05 --hmark-rnd 0x00000004 --hmark-mod 42 --hmark-offset 1 --hmark-tuple ct;=;OK
# cannot mix in spi mask:
-j HMARK --hmark-src-prefix 32 --hmark-dst-prefix 32 --hmark-sport-mask 0xffff --hmark-dport-mask 0xffff --hmark-proto-mask 0xffff --hmark-rnd 0x00000004 --hmark-mod 42 --hmark-offset 1 --hmark-tuple ct --hmark-spi-mask 4;;FAIL
