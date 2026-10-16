:INPUT,FORWARD,OUTPUT
-s ::/128;=;OK
! -d ::;! -d ::/128;OK
! -s ::;! -s ::/128;OK
-s ::/64;=;OK
:INPUT
-i + -d c0::fe;-d c0::fe/128;OK
-i + -p tcp;-p tcp;OK
