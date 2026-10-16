:POSTROUTING
*nat
-o someport -j snat --to-source a:b:c:d:e:f;-o someport -j snat --to-src 0a:0b:0c:0d:0e:0f --snat-target ACCEPT;OK
-o someport+ -j snat --to-src de:ad:00:be:ee:ff --snat-target CONTINUE;=;OK
-j snat;;FAIL
-j snat --to-src de:ad:00:be:ee:ff;-j snat --to-src de:ad:00:be:ee:ff --snat-target ACCEPT;OK
