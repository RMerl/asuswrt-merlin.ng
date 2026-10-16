:OUTPUT
-o lo --destination-mac 11:22:33:44:55:66;-o lo --dst-mac 11:22:33:44:55:66;OK
--dst-mac Broadcast ;--dst-mac ff:ff:ff:ff:ff:ff;OK
! -o eth+ -d 1.2.3.4/24 -j CLASSIFY --set-class 0:0;-j CLASSIFY ! -o eth+ -d 1.2.3.0/24 --set-class 0:0;OK
