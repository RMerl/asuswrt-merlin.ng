:INPUT,FORWARD,OUTPUT
-m recent --set;-m recent --set --name DEFAULT --mask ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff --rsource;OK
-m recent --rcheck --hitcount 8 --name foo --mask ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff --rsource;=;OK
-m recent --rcheck --hitcount 12 --name foo --mask ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff --rsource;=;OK
-m recent --update --rttl;-m recent --update --rttl --name DEFAULT --mask ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff --rsource;OK
-m recent --rcheck --hitcount 65536 --name foo --mask ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff --rsource;;FAIL
# nonsensical, but all should load successfully:
-m recent --rcheck --hitcount 3 --name foo --mask ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff --rsource -m recent --rcheck --hitcount 4 --name foo --mask ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff --rsource;=;OK
-m recent --rcheck --hitcount 4 --name foo --mask ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff --rsource -m recent --rcheck --hitcount 4 --name foo --mask ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff --rsource;=;OK
-m recent --rcheck --hitcount 8 --name foo --mask ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff --rsource -m recent --rcheck --hitcount 12 --name foo --mask ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff --rsource;=;OK
