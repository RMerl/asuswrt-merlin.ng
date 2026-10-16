:PREROUTING
*nat
-i someport -j dnat --to-dst de:ad:0:be:ee:ff;-i someport -j dnat --to-dst de:ad:00:be:ee:ff --dnat-target ACCEPT;OK
-j dnat --to-dst de:ad:00:be:ee:ff --dnat-target ACCEPT;=;OK
-j dnat --to-dst de:ad:00:be:ee:ff --dnat-target CONTINUE;=;OK
