if [ -f /sbin/l1dat ]; then
        # variable interface name
        card1_main=`l1dat zone2if dev1 | awk '{print $1}'`
        card2_main=`l1dat zone2if dev2 | awk '{print $1}'`
        card3_main=`l1dat zone2if dev3 | awk '{print $1}'`

        card1_nvram="dev1"
        card2_nvram="dev2"
        card3_nvram="dev3"
else
        # fix interface name
        card1_main="ra0"
        card2_main="rai0"
        card3_main="rax0"

        card1_nvram="2860"
        card2_nvram="rtdev"
        card3_nvram="wifi3"
fi
en=$(nvram_get 2860 MapEnable)
if [ "$en" = "1" ]; then
        iwpriv $card1_main set mapEnable=1;
        iwpriv $card2_main set mapEnable=1;
        iwpriv $card3_main set mapEnable=1;
	iwpriv $card1_main set mapTurnKey=1;
        iwpriv $card2_main set mapTurnKey=1;
        iwpriv $card3_main set mapTurnKey=1;
	if [ -f /usr/bin/EasyMesh_openwrt.sh ]; then
                EasyMesh_openwrt.sh
        elif [ -f /usr/bin/EasyMesh_7629.sh ]; then
                EasyMesh_7629.sh
        elif [ -f /usr/bin/EasyMesh_7622.sh ]; then
                EasyMesh_7622.sh
        else
                EasyMesh.sh
        fi
elif [ "$en" = "0" ]; then
        iwpriv $card1_main set mapEnable=0;
        iwpriv $card2_main set mapEnable=0;
        iwpriv $card3_main set mapEnable=0;
	iwpriv $card1_main set mapTurnKey=0;
        iwpriv $card2_main set mapTurnKey=0;
        iwpriv $card3_main set mapTurnKey=0;
        rm -rf /tmp/wapp_ctrl
        killall -15 wapp
        killall -15 p1905_managerd
        killall -15 mapd
	if [ -f /usr/bin/wapp_openwrt.sh ]; then
		wapp_openwrt.sh
	else
		wapp &
	fi
fi
