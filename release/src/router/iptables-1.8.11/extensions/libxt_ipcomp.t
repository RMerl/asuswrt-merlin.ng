:INPUT,OUTPUT
-p ipcomp -m ipcomp --ipcompspi 18 -j DROP;=;OK
-p ipcomp -m ipcomp ! --ipcompspi 18 -j ACCEPT;=;OK
-p ipcomp -m ipcomp --ipcompspi :;-p ipcomp -m ipcomp;OK
-p ipcomp -m ipcomp ! --ipcompspi :;-p ipcomp -m ipcomp ! --ipcompspi 0:4294967295;OK
-p ipcomp -m ipcomp --ipcompspi :4;-p ipcomp -m ipcomp --ipcompspi 0:4;OK
-p ipcomp -m ipcomp --ipcompspi 4:;-p ipcomp -m ipcomp --ipcompspi 4:4294967295;OK
-p ipcomp -m ipcomp --ipcompspi 3:4;=;OK
-p ipcomp -m ipcomp --ipcompspi 4:4;-p ipcomp -m ipcomp --ipcompspi 4;OK
-p ipcomp -m ipcomp --ipcompspi 4:3;;FAIL
