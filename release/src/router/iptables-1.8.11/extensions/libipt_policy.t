:INPUT,FORWARD
-m policy --dir in --pol ipsec --strict --reqid 1 --spi 0x1 --proto esp --mode tunnel --tunnel-dst 10.0.0.0/8 --tunnel-src 10.0.0.0/8 --next --reqid 2;=;OK
-m policy --dir in --pol ipsec --strict --reqid 1 --spi 0x1 --proto esp --tunnel-dst 10.0.0.0/8;;FAIL
-m policy --dir in --pol ipsec --strict --reqid 1 --spi 0x1 --proto ipcomp --mode tunnel --tunnel-dst 10.0.0.0/8 --tunnel-src 10.0.0.0/8 --next --reqid 2;=;OK
