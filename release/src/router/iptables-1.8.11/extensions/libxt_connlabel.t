:INPUT,FORWARD,OUTPUT
-m connlabel --label "40";=;OK
-m connlabel ! --label "40";=;OK
-m connlabel --label "41" --set;=;OK
-m connlabel ! --label "41" --set;=;OK
-m connlabel --label "2048";;FAIL
-m connlabel --label "foobar_not_there";;FAIL
