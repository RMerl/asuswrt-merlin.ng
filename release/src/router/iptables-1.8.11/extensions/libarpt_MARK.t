:INPUT,OUTPUT
-j MARK -d 0.0.0.0/8 --set-mark 1;=;OK
-s ! 0.0.0.0 -j MARK --and-mark 0x17;-j MARK ! -s 0.0.0.0 --and-mark 17;OK
-j MARK -s 0.0.0.0 --or-mark 17;=;OK
