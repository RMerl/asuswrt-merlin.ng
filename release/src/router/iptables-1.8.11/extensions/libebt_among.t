:INPUT,FORWARD,OUTPUT
--among-dst de:ad:0:be:ee:ff,c0:ff:ee:0:ba:be;--among-dst c0:ff:ee:0:ba:be,de:ad:0:be:ee:ff;OK
! --among-dst c0:ff:ee:0:ba:be,de:ad:0:be:ee:ff;=;OK
--among-src be:ef:0:c0:ff:ee,c0:ff:ee:0:ba:be,de:ad:0:be:ee:ff;=;OK
--among-src de:ad:0:be:ee:ff=10.0.0.1,c0:ff:ee:0:ba:be=192.168.1.1;--among-src c0:ff:ee:0:ba:be=192.168.1.1,de:ad:0:be:ee:ff=10.0.0.1;OK
! --among-src c0:ff:ee:0:ba:be=192.168.1.1,de:ad:0:be:ee:ff=10.0.0.1;=;OK
--among-src de:ad:0:be:ee:ff --among-dst c0:ff:ee:0:ba:be;=;OK
--among-src de:ad:0:be:ee:ff=10.0.0.1 --among-dst c0:ff:ee:0:ba:be=192.168.1.1;=;OK
! --among-src de:ad:0:be:ee:ff --among-dst c0:ff:ee:0:ba:be;=;OK
--among-src de:ad:0:be:ee:ff=10.0.0.1 ! --among-dst c0:ff:ee:0:ba:be=192.168.1.1;=;OK
! --among-src de:ad:0:be:ee:ff --among-dst c0:ff:ee:0:ba:be=192.168.1.1;=;OK
--among-src de:ad:0:be:ee:ff=10.0.0.1 ! --among-dst c0:ff:ee:0:ba:be=192.168.1.1;=;OK
--among-src;=;FAIL
--among-src 00:11=10.0.0.1;=;FAIL
--among-src de:ad:0:be:ee:ff=10.256.0.1;=;FAIL
--among-src c0:ff:ee:0:ba:be=192.168.1.1,de:ad:0:be:ee:ff;=;OK
