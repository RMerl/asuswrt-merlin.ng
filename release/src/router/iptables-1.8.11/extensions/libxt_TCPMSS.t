:FORWARD,OUTPUT,POSTROUTING
*mangle
-j TCPMSS;;FAIL
-p tcp -j TCPMSS --set-mss 42;=;FAIL;LEGACY
-p tcp -m tcp --tcp-flags FIN,SYN,RST,ACK SYN -j TCPMSS --set-mss 42;=;OK
-p tcp -m tcp --tcp-flags FIN,SYN,RST,ACK SYN -j TCPMSS --clamp-mss-to-pmtu;=;OK
