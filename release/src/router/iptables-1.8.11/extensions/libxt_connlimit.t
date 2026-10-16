:INPUT,FORWARD,OUTPUT
-m connlimit --connlimit-upto -1;;FAIL
-m connlimit --connlimit-above -1;;FAIL
-m connlimit --connlimit-upto 1 --conlimit-above 1;;FAIL
-m connlimit --connlimit-above 10 --connlimit-saddr --connlimit-daddr;;FAIL
-m connlimit;;FAIL
