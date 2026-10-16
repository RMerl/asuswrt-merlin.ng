:INPUT,FORWARD,OUTPUT
-m time --timestart 01:02:03 --timestop 04:05:06 --monthdays 1,2,3,4,5 --weekdays Mon,Fri,Sun --datestart 2001-02-03T04:05:06 --datestop 2012-09-08T09:06:05 --kerneltz;=;OK
-m time --timestart 01:02:03 --timestop 04:05:06 --monthdays 1,2,3,4,5 --weekdays Mon,Fri,Sun --datestart 2001-02-03T04:05:06 --datestop 2012-09-08T09:06:05;=;OK
-m time --timestart 02:00:00 --timestop 03:00:00 --datestart 1970-01-01T02:00:00 --datestop 1970-01-01T03:00:00;=;OK
