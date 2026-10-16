:PREROUTING,INPUT,FORWARD,OUTPUT,POSTROUTING
*mangle
-j TOS --set-tos 0x1f;-j TOS --set-tos 0x1f/0xff;OK
-j TOS --set-tos 0x1f/0x1f;=;OK
# maximum TOS is 0x1f (5 bits)
# ERROR: should fail: iptables -A PREROUTING -t mangle -j TOS --set-tos 0xff
# -j TOS --set-tos 0xff;;FAIL
-j TOS --set-tos Minimize-Delay;-j TOS --set-tos 0x10/0x3f;OK
-j TOS --set-tos Maximize-Throughput;-j TOS --set-tos 0x08/0x3f;OK
-j TOS --set-tos Maximize-Reliability;-j TOS --set-tos 0x04/0x3f;OK
-j TOS --set-tos Minimize-Cost;-j TOS --set-tos 0x02/0x3f;OK
-j TOS --set-tos Normal-Service;-j TOS --set-tos 0x00/0x3f;OK
-j TOS --and-tos 0x12;-j TOS --set-tos 0x00/0xed;OK
-j TOS --or-tos 0x12;-j TOS --set-tos 0x12/0x12;OK
-j TOS --xor-tos 0x12;-j TOS --set-tos 0x12/0x00;OK
-j TOS;;FAIL
