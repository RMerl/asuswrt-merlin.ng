:INPUT,FORWARD,OUTPUT
! --802_3-sap 0x0a -j CONTINUE;=;FAIL
--802_3-type 0x000a -j RETURN;=;FAIL
-p Length --802_3-sap 0x0a -j CONTINUE;=;OK
-p Length ! --802_3-sap 0x0a -j CONTINUE;=;OK
-p Length --802_3-type 0x000a -j RETURN;=;OK
-p Length ! --802_3-type 0x000a -j RETURN;=;OK
