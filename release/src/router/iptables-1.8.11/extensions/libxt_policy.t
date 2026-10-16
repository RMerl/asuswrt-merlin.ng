:INPUT,FORWARD
-m policy --dir in --pol ipsec;=;OK
-m policy --dir in --pol ipsec --proto ipcomp;=;OK
-m policy --dir in --pol ipsec --strict;;FAIL
-m policy --dir in --pol ipsec --strict --reqid 1 --spi 0x1 --proto ipcomp;=;OK
