iwpriv ra0 set SiteSurvey=1
sleep 7
#iwpriv ra0 set Debug=3
iwpriv ra0 set mbo_nr=0-$1
#iwpriv ra0 set Debug=2

#iwpriv ra1 set SiteSurvey=1
#sleep 7
#iwpriv ra1 set Debug=3
#iwpriv ra1 set mbo_nr=1-$1
#iwpriv ra1 set Debug=2

iwpriv rai0 set SiteSurvey=1
sleep 7
#iwpriv rai0 set Debug=3
iwpriv rai0 set mbo_nr=1-$1
#iwpriv rai0 set Debug=2

#iwpriv wlan0 set SiteSurvey=1
#sleep 7
#iwpriv rai0 set Debug=3
#iwpriv wlan0 set mbo_nr=1-$1
#iwpriv rai0 set Debug=2
