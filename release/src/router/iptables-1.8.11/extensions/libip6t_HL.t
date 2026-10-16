:PREROUTING,INPUT,FORWARD,OUTPUT,POSTROUTING
*mangle
-j HL --hl-set 42;=;OK
-j HL --hl-inc 1;=;OK
-j HL --hl-dec 1;=;OK
-j HL --hl-set 256;;FAIL
-j HL --hl-inc 0;;FAIL
-j HL --hl-dec 0;;FAIL
-j HL --hl-dec 1 --hl-inc 1;;FAIL
-j HL --hl-set --hl-inc 1;;FAIL
