:INPUT,FORWARD,OUTPUT
-p dccp -m dccp --sport 1;=;OK
-p dccp -m dccp --sport 65535;=;OK
-p dccp -m dccp --dport 1;=;OK
-p dccp -m dccp --dport 65535;=;OK
-p dccp -m dccp --sport 1:1023;=;OK
-p dccp -m dccp --sport 1024:65535;=;OK
-p dccp -m dccp --sport 1024:;-p dccp -m dccp --sport 1024:65535;OK
-p dccp -m dccp --sport :;-p dccp -m dccp --sport 0:65535;OK
-p dccp -m dccp --sport :4;-p dccp -m dccp --sport 0:4;OK
-p dccp -m dccp --sport 4:;-p dccp -m dccp --sport 4:65535;OK
-p dccp -m dccp --sport 4:4;-p dccp -m dccp --sport 4;OK
-p dccp -m dccp --sport 4:3;;FAIL
-p dccp -m dccp --dport :;-p dccp -m dccp --dport 0:65535;OK
-p dccp -m dccp --dport :4;-p dccp -m dccp --dport 0:4;OK
-p dccp -m dccp --dport 4:;-p dccp -m dccp --dport 4:65535;OK
-p dccp -m dccp --dport 4:4;-p dccp -m dccp --dport 4;OK
-p dccp -m dccp --dport 4:3;;FAIL
-p dccp -m dccp ! --sport 1;=;OK
-p dccp -m dccp ! --sport 65535;=;OK
-p dccp -m dccp ! --dport 1;=;OK
-p dccp -m dccp ! --dport 65535;=;OK
-p dccp -m dccp --sport 1 --dport 65535;=;OK
-p dccp -m dccp --sport 65535 --dport 1;=;OK
-p dccp -m dccp ! --sport 1 --dport 65535;=;OK
-p dccp -m dccp ! --sport 65535 --dport 1;=;OK
# ERROR: should fail: iptables -A INPUT -p dccp -m dccp --sport 65536
# -p dccp -m dccp --sport 65536;;FAIL
-p dccp -m dccp --sport -1;;FAIL
-p dccp -m dccp --dport -1;;FAIL
-p dccp -m dccp --dccp-types REQUEST,RESPONSE,DATA,ACK,DATAACK,CLOSEREQ,CLOSE,RESET,SYNC,SYNCACK,INVALID;=;OK
-p dccp -m dccp ! --dccp-types REQUEST,RESPONSE,DATA,ACK,DATAACK,CLOSEREQ,CLOSE,RESET,SYNC,SYNCACK,INVALID;=;OK
# DCCP option 0 is valid, see http://tools.ietf.org/html/rfc4340#page-29
# ERROR: cannot load: iptables -A INPUT -p dccp -m dccp --dccp-option 0
#-p dccp -m dccp --dccp-option 0;=;OK
-p dccp -m dccp --dccp-option 255;=;OK
-p dccp -m dccp --dccp-option 256;;FAIL
-p dccp -m dccp --dccp-option -1;;FAIL
# should we accept this below?
-p dccp -m dccp;=;OK
