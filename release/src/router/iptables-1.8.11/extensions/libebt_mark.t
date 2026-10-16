:INPUT,FORWARD,OUTPUT
-j mark --mark-set 1;-j mark --mark-set 0x1 --mark-target ACCEPT;OK
-j mark --mark-or 0xa --mark-target CONTINUE;=;OK
-j mark --mark-and 0x1 --mark-target RETURN;=;OK
-j mark --mark-xor 0x1 --mark-target CONTINUE;=;OK
