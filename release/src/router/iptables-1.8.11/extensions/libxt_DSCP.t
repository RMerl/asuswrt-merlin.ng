:PREROUTING,INPUT,FORWARD,OUTPUT,POSTROUTING
*mangle
-j DSCP --set-dscp 0x00;=;OK
-j DSCP --set-dscp 0x3f;=;OK
-j DSCP --set-dscp -1;;FAIL
-j DSCP --set-dscp 0x40;;FAIL
-j DSCP --set-dscp 0x3f --set-dscp-class CS0;;FAIL
-j DSCP --set-dscp-class CS0;-j DSCP --set-dscp 0x00;OK
-j DSCP --set-dscp-class BE;-j DSCP --set-dscp 0x00;OK
-j DSCP --set-dscp-class EF;-j DSCP --set-dscp 0x2e;OK
-j DSCP;;FAIL
