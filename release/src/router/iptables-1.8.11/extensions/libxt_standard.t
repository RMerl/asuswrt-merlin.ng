:INPUT,FORWARD,OUTPUT
-p tcp -j ACCEPT;=;OK
! -p udp -j ACCEPT;=;OK
-j DROP;=;OK
-j ACCEPT;=;OK
-j RETURN;=;OK
! -p 0 -j ACCEPT;=;FAIL
:FORWARD
-i + -p tcp;-p tcp;OK
