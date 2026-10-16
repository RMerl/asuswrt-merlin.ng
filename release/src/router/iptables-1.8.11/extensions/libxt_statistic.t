:INPUT,FORWARD,OUTPUT
-m statistic;;FAIL
-m statistic --mode random ! --probability 0.50000000000;=;OK
-m statistic --mode random ! --probability 1.1;;FAIL
-m statistic --probability 1;;FAIL
-m statistic --mode nth ! --every 5 --packet 2;=;OK
-m statistic --mode nth ! --every 5;;FAIL
-m statistic --mode nth ! --every 5 --packet 5;;FAIL
