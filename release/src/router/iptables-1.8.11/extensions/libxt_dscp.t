:INPUT,FORWARD,OUTPUT
-m dscp --dscp 0x00;=;OK
-m dscp --dscp 0x3f;=;OK
-m dscp --dscp -1;;FAIL
-m dscp --dscp 0x40;;FAIL
-m dscp --dscp 0x3f --dscp-class CS0;;FAIL
-m dscp --dscp-class CS0;-m dscp --dscp 0x00;OK
-m dscp --dscp-class BE;-m dscp --dscp 0x00;OK
-m dscp --dscp-class EF;-m dscp --dscp 0x2e;OK
-m dscp;;FAIL
