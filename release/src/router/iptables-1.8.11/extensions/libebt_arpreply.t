:PREROUTING
*nat
-j arpreply;=;FAIL
-p ARP -i foo -j arpreply;-p ARP -i foo -j arpreply --arpreply-mac 00:00:00:00:00:00;OK
-p ARP -i foo -j arpreply --arpreply-mac de:ad:00:be:ee:ff --arpreply-target ACCEPT;=;OK
-p ARP -i foo -j arpreply --arpreply-mac de:ad:00:be:ee:ff;=;OK
-p ARP -j arpreply ! --arpreply-mac de:ad:00:be:ee:ff;;FAIL
-p ARP -j arpreply --arpreply-mac de:ad:00:be:ee:ff ! --arpreply-target ACCEPT;;FAIL
