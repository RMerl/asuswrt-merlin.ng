:PREROUTING,FORWARD,OUTPUT,POSTROUTING
*mangle
-j CONNMARK --restore-mark;-j CONNMARK --restore-mark --nfmask 0xffffffff --ctmask 0xffffffff;OK
-j CONNMARK --save-mark;-j CONNMARK --save-mark --nfmask 0xffffffff --ctmask 0xffffffff;OK
-j CONNMARK --save-mark --nfmask 0xfffffff --ctmask 0xffffffff;=;OK
-j CONNMARK --restore-mark --nfmask 0xfffffff --ctmask 0xffffffff;=;OK
-j CONNMARK;;FAIL
