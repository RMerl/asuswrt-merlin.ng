:FORWARD
-i alongifacename0;=;OK
-i thisinterfaceistoolong0;;FAIL
-i eth+ -o alongifacename+;=;OK
! -i eth0;=;OK
! -o eth+;=;OK
-i + -j ACCEPT;-j ACCEPT;OK
! -i +;=;OK
-c "";;FAIL
-c ,3;;FAIL
-c 3,;;FAIL
-c ,;;FAIL
-c 2,3 -j ACCEPT;-j ACCEPT;OK
