:PREROUTING,INPUT,FORWARD,OUTPUT,POSTROUTING
*mangle
-j TCPOPTSTRIP;;FAIL
-p tcp -j TCPOPTSTRIP;=;OK
-p tcp -j TCPOPTSTRIP --strip-options 2,3,4,5,6,7;=;OK
-p tcp -j TCPOPTSTRIP --strip-options 0;;FAIL
-p tcp -j TCPOPTSTRIP --strip-options 1;;FAIL
-p tcp -j TCPOPTSTRIP --strip-options 1,2;;FAIL
