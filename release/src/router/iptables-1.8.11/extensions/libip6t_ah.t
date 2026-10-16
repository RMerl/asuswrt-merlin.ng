:INPUT,FORWARD,OUTPUT
-m ah --ahspi 0;=;OK
-m ah --ahspi 4294967295;=;OK
-m ah --ahspi 0:4294967295;-m ah;OK
-m ah ! --ahspi 0;=;OK
# ERROR: should fail: iptables -A FORWARD -t mangle -j CLASSIFY --set-class 1:-1
# -m ah --ahres;=;OK
# ERROR: line 7 (cannot find: ip6tables -I INPUT -m ah --ahlen 32
# -m ah --ahlen 32;=;OK
-m ah --ahspi -1;;FAIL
-m ah --ahspi 4294967296;;FAIL
-m ah --ahspi invalid;;FAIL
-m ah --ahspi 0:invalid;;FAIL
-m ah --ahspi;;FAIL
-m ah;=;OK
-m ah --ahspi :;-m ah;OK
-m ah ! --ahspi :;-m ah ! --ahspi 0:4294967295;OK
-m ah --ahspi :3;-m ah --ahspi 0:3;OK
-m ah --ahspi 3:;-m ah --ahspi 3:4294967295;OK
-m ah --ahspi 3:3;-m ah --ahspi 3;OK
-m ah --ahspi 4:3;;FAIL
