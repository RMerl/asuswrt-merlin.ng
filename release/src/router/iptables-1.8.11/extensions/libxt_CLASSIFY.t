:FORWARD,OUTPUT,POSTROUTING
*mangle
-j CLASSIFY --set-class 0000:ffff;=;OK
# maximum handle accepted by tc is 0xffff
# ERROR : should fail: iptables -A FORWARD -t mangle -j CLASSIFY --set-class  0000:ffffffff
# -j CLASSIFY --set-class 0000:ffffffff;;FAIL
# ERROR: should fail: iptables -A FORWARD -t mangle -j CLASSIFY --set-class 1:-1
# -j CLASSIFY --set-class 1:-1;;FAIL
-j CLASSIFY;;FAIL
