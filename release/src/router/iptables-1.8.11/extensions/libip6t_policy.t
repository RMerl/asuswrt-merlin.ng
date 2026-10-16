:INPUT,FORWARD
-m policy --dir in --pol ipsec --strict --reqid 1 --spi 0x1 --proto esp --mode tunnel --tunnel-dst 2001:db8::/32 --tunnel-src 2001:db8::/32 --next --reqid 2;=;OK
-m policy --dir in --pol ipsec --strict --reqid 1 --spi 0x1 --proto esp --tunnel-dst 2001:db8::/32;;FAIL
-m policy --dir in --pol ipsec --strict --reqid 1 --spi 0x1 --proto ipcomp --mode tunnel --tunnel-dst 2001:db8::/32 --tunnel-src 2001:db8::/32 --next --reqid 2;=;OK
