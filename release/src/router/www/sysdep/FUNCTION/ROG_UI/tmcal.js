/*
	Tomato GUI
	Copyright (C) 2006-2009 Jonathan Zarate
	http://www.polarcloud.com/tomato/

	For use with Tomato Firmware only.
	No part of this file may be used without permission.
*/

var tabs = [];
var rx_max, rx_avg;
var tx_max, tx_avg;
var xx_max = 0;
var ifname;
var htmReady = 0;
var svgReady = 0;
var updating = 0;
var scaleMode = 0;
var scaleLast = -1;
var drawMode = 0;
var drawLast = -1;
var drawColor = 0;
var drawColorRX = 0;	//Viz add 2010.09
var drawColorTX = 0;	//Viz add 2010.09
var avgMode = 0;
var avgLast = -1;
var colorX = 0;
var colors = [
	['Green &amp; Blue', '#118811', '#6495ed'], ['Blue &amp; Orange', '#003EBA', '#FF9000'],
	['Blue &amp; Red', '#003EDD', '#CC4040'], ['Blue', '#22f', '#225'], ['Gray', '#000', '#999'],
	['Red &amp; Black', '#d00', '#000']];
// Viz add colorRX�BcoloTX 2010.09
// 0Orange 1Blue 2Black 3Red 4Gray  5Green
var colorRX = [ '#FF9000', '#3CF', '#000000',  '#dd0000', '#999999',  '#118811'];
var colorTX = ['#FF9000', '#3CF', '#000000',  '#dd0000', '#999999',  '#118811'];

function getTrafficUnit(){
	var value = 0;
	if(cookie.get('ASUS_TrafficMonitor_unit')){
		value = cookie.get('ASUS_TrafficMonitor_unit');
	}

	return value;
}

function trafficTotalScale(byt){
	var unit = getTrafficUnit();
	var value = '';
	var scale = 'KB';
	if(unit == '1'){	// MB
		value = (byt/1e6).toFixed(2);
		scale = 'MB';
	}
	else if(unit == '2'){	// GB
		value = (byt/1e9).toFixed(2);
		scale = 'GB';
	}
	else if(unit == '3'){	// TB
		value = (byt/1e12).toFixed(2);
		scale = 'TB';
	}
	else{	// unit == 9
		return scaleSize(byt);
	}

	return value + ' <small>'+ scale +'</small>';
}

function xpsb(byt){
/* REMOVE-BEGIN
	kbit/s = 1000 bits/s
	125 = 1000 / 8
	((B * 8) / 1000)
REMOVE-END */
	var unit = getTrafficUnit();
	var value = '';
	var scale = 'KB/s';
	if(unit == '1'){	// MB
		value = (byt/1e6).toFixed(2);
		scale = 'MB/s';
	}
	else if(unit == '2'){	// GB
		value = (byt/1e9).toFixed(2);
		scale = 'GB/s';
	}
	else if(unit == '3'){	// TB
		value = (byt/1e12).toFixed(2);
		scale = 'TB/s';
	}
	else{	// unit == 9
		value = (byt/1000).toFixed(2);
	}

	return value + ' <small>'+ scale +'</small>';
}

function showCTab()
{
	showTab('speed-tab-' + ifname);
}

function showSelectedOption(prefix, prev, now)
{
	var e;

	elem.removeClass(prefix + prev, 'selected');	// safe if prev doesn't exist
	if ((e = E(prefix + now)) != null) {
		elem.addClass(e, 'selected');
		e.blur();
	}
}

function showDraw()
{
	if (drawLast == drawMode) return;
	showSelectedOption('draw', drawLast, drawMode);
	drawLast = drawMode;
}

function switchDraw(n)
{
	if ((!svgReady) || (updating)) return;
	drawMode = n;
	showDraw();
	showCTab();
	cookie.set(cprefix + 'draw', drawMode);
}

// Viz add 2010.09  vvvvvvvvvv
function showColor()
{	
}

function switchColorRX(rev)
{
	if ((!svgReady) || (updating)) return;
	
	drawColorRX = rev;
	showColor();
	showCTab();
}

function switchColorTX(rev)
{
	if ((!svgReady) || (updating)) return;
	
	drawColorTX = rev;
	showColor();
	showCTab();
}
// Viz add 2010.09 ^^^^^^^^^^

function showScale()
{
	if (scaleMode == scaleLast) return;
	showSelectedOption('scale', scaleLast, scaleMode);
	scaleLast = scaleMode;
}

function switchScale(n)
{
	scaleMode = n;
	showScale();
	showTab('speed-tab-' + ifname);
	cookie.set(cprefix + 'scale', scaleMode);
}

function showAvg()
{
	if (avgMode == avgLast) return;
	showSelectedOption('avg', avgLast, avgMode);
	avgLast = avgMode;
}

function switchAvg(n)
{
	if ((!svgReady) || (updating)) return;
	avgMode = n;
	showAvg();
	showCTab();
	cookie.set(cprefix + 'avg', avgMode);
}

function tabSelect(name)
{
	if (!updating) showTab(name);
}

function showTab(name)
{
	var h;
	var max;
	var i;
	var rx, tx;
	var e;
	var wan_num = 0, wireless_num = 0, multi_wan = 0, multi_wireless = 0, wired_num = 0, multi_waggr = 0, multi_wired = 0;

	ifname = name.replace('speed-tab-', '');
	cookie.set(cprefix + 'tab', ifname, 14);
	tabHigh(name);

	for(var i = 0; i < tabs.length; i++){
		if(name == tabs[i][0]){
			document.getElementById("iftitle").innerHTML = tabs[i][1];
			document.getElementById("iftitle").style.display = "block";
		}

		if(tabs[i][0].indexOf("INTERNET") != -1 || tabs[i][0].indexOf("WAGGR") != -1)
			wan_num++;

		if(tabs[i][0].indexOf("WIRELESS") != -1)
			wireless_num++;

		if(tabs[i][0].indexOf("WIRED") != -1 || tabs[i][0].indexOf("LACP") != -1)
			wired_num++;
	}

	if(wan_num > 1)
		multi_wan = 1;

	if(wireless_num > 1)
		multi_wireless = 1;

	if(wired_num > 1)
		multi_wired = 1;

	if(multi_wan){
		if(ifname.indexOf("INTERNET") != -1 || ifname.indexOf("WAGGR") != -1){
			document.getElementById("internet_tabs").style.background = "url(/images/svg_th_hover.png) repeat-x";
			document.getElementById("internet_tabs").style.color = "#FFFFFF";
		}
		else{
			document.getElementById("internet_tabs").style.background = "";
			document.getElementById("internet_tabs").style.color = "#000000";
		}
	}

	if(multi_wireless){
		if(ifname.indexOf("WIRELESS") != -1){
			document.getElementById("wireless_tabs").style.background = "url(/images/svg_th_hover.png) repeat-x";
			document.getElementById("wireless_tabs").style.color = "#FFFFFF";
		}
		else{
			document.getElementById("wireless_tabs").style.background = "";
			document.getElementById("wireless_tabs").style.color = "#000000";
		}
	}

	if(multi_wired){
		if(ifname.indexOf("WIRED") != -1 || ifname.indexOf("LACP") != -1){
			document.getElementById("wired_tabs").style.background = "url(/images/svg_th_hover.png) repeat-x";
			document.getElementById("wired_tabs").style.color = "#FFFFFF";
		}
		else{
			document.getElementById("wired_tabs").style.background = "";
			document.getElementById("wired_tabs").style.color = "#000000";
		}
	}

	h = speed_history[ifname];
	if (!h) return;



	E('rx-current').innerHTML = xpsb(h.rx[h.rx.length - 1] / updateDiv); 
	E('rx-avg').innerHTML = xpsb(h.rx_avg);
	E('rx-max').innerHTML = xpsb(h.rx_max);

	E('tx-current').innerHTML = xpsb(h.tx[h.tx.length - 1] / updateDiv);
	E('tx-avg').innerHTML = xpsb(h.tx_avg);
	E('tx-max').innerHTML = xpsb(h.tx_max);

	E('rx-total').innerHTML = trafficTotalScale(h.rx_total);
	E('tx-total').innerHTML = trafficTotalScale(h.tx_total);

	if (svgReady) {
		max = scaleMode ? MAX(h.rx_max, h.tx_max) : xx_max
		if (max > 12500) max = Math.round((max + 12499) / 12500) * 12500;
			else max += 100;
		if(ifname == "WIRED" || ifname == "WIRELESS0" || ifname == "WIRELESS1" || ifname == "WIRELESS2" || ifname == "WIRELESS3"
		|| ifname == "LACP1" || ifname == "LACP2" || ifname == "LACP3" || ifname == "LACP4" || ifname == "LACP5" || ifname == "LACP6" || ifname == "LACP7" || ifname == "LACP8"){
			updateSVG(h.rx, h.tx, max, drawMode, colorTX[drawColorTX], colorRX[drawColorRX], updateInt, updateMaxL, updateDiv, avgMode, clock);
			document.getElementById("rx-current").className = "blue_line";
			document.getElementById("tx-current").className = "orange_line";
		}else{
			updateSVG(h.tx, h.rx, max, drawMode, colorTX[drawColorTX], colorRX[drawColorRX], updateInt, updateMaxL, updateDiv, avgMode, clock);
			document.getElementById("rx-current").className = "orange_line";
			document.getElementById("tx-current").className = "blue_line";
		}	
	}
}

function loadData()
{
	var old;
	var t, e;
	var name;
	var i;
	var changed;

	xx_max = 0;
	old = tabs;
	tabs = [];
	clock = new Date();

	if (!speed_history) {
		speed_history = [];
	}
	else {
		for (var i in speed_history) {
			var h = speed_history[i];
			if ((typeof(h.rx) == 'undefined') || (typeof(h.tx) == 'undefined')) {
				delete speed_history[i];
				continue;
			}

			if (updateReTotal) {
				h.rx_total = h.rx_max = 0;
				h.tx_total = h.tx_max = 0;
				for (j = (h.rx.length - updateMaxL); j < h.rx.length; ++j) {
					t = h.rx[j];
					if (t > h.rx_max) h.rx_max = t;
							h.rx_total += t;
					t = h.tx[j];
					if (t > h.tx_max) h.tx_max = t;
							h.tx_total += t;
				}
				h.rx_avg = h.rx_total / updateMaxL;
				h.tx_avg = h.tx_total / updateMaxL;
			}

			if (updateDiv > 1) {
				h.rx_max /= updateDiv;
				h.tx_max /= updateDiv;
				h.rx_avg /= updateDiv;
				h.tx_avg /= updateDiv;
			}
			if (h.rx_max > xx_max) xx_max = h.rx_max;
			if (h.tx_max > xx_max) xx_max = h.tx_max;

			if (i == "WIRELESS1"){
				if(wl_info.band5g_2_support)
					t = "<#tm_wireless#> (5GHz-1)";
				else	
					t = "<#tm_wireless#> (5GHz)";
			}
			else if (i == "WIRELESS0")
				t = "<#tm_wireless#> (2.4GHz)";
			else if (i == "WIRELESS2"){
				if(wl_info.band6g_support){
					t = "<#tm_wireless#> (6GHz)";
				}
				else{
					t = "<#tm_wireless#> (5GHz-2)";
				}				
			}
			else if (i == "WIRELESS3")
				t = "<#tm_wireless#> (60GHz)";
			else if (i == "WIRED")
				t = "<#tm_wired#>";
			else if (i == "BRIDGE")
				t = "LAN";
			else if (i == "INTERNET"){
				if(dualWAN_support){
					if(wans_dualwan_array[0] == "usb"){
						if(gobi_support)
							t = "<#Mobile_title#>";
						else
							t = "USB Modem";
					}
					else if(wans_dualwan_array[0] == "wan"){
						if (nvram.bond_wan == '1' && nvram.rc_support.indexOf("wanbonding") != -1)
							t = "Bond";
						else
							t = "WAN";
					}
					else if(wans_dualwan_array[0] == "wan2"){
						if (based_modelid == "GT-AXY16000" || based_modelid == "RT-AX89U")
							t = "10G base-T";
						else
							t = "WAN2";
					}		
					else if(wans_dualwan_array[0] == "lan")
						t = "LAN";
					else if(wans_dualwan_array[0] == "dsl")
						t = "DSL WAN";
					else if(wans_dualwan_array[0] == "sfp+")
						t = "10G SFP+";	
					else
						t = "<#dualwan_primary#>";
				}				
				else
					t = "<#Internet#>";
			}else if (i == "INTERNET1"){
				if(wans_dualwan_array[1] == "usb"){
					if(gobi_support)
						t = "<#Mobile_title#>";
					else
						t = "USB Modem";
				}
				else if(wans_dualwan_array[1] == "wan"){
					if (nvram.bond_wan == '1' && nvram.rc_support.indexOf("wanbonding") != -1)
						t = "Bond";
					else
						t = "WAN";
				}
				else if(wans_dualwan_array[1] == "wan2"){
					if (based_modelid == "GT-AXY16000" || based_modelid == "RT-AX89U")
						t = "10G base-T";
					else
						t = "WAN2";
				}
				else if(wans_dualwan_array[1] == "sfp+")
					t = "10G SFP+";		
				else if(wans_dualwan_array[1] == "lan")
					t = "LAN";
				else
					t = "<#dualwan_secondary#>";
			}
			else if (i.search("WIRELESS") > -1 && i.search(".") > -1)
				t = "NotUsed";
			else if (i.search("WAGGR") > -1){
				var bs_port_id = i.substr(5);
				if (bs_port_id == 0)
					t = "bond-slave (WAN)";
				else if (bs_port_id >= 1 && bs_port_id <= 8)
					t = "bond-slave (LAN"+bs_port_id+")";
				else if (bs_port_id == 30)
					t = "bond-slave (10G base-T)";
				else if (bs_port_id == 31)
					t = "bond-slave (10G SFP+)";
				else
					t = "NotUsed";
			}
			else if (i.search("LACP") > -1){
				var num = i.substr(4);
				t = "bond-slave (LAN"+num+")";
			}			
			else
				t = i;			
 
			if(t != "LAN" && t != "NotUsed"){ // hide Tabs
				/*if(i == "INTERNET")
					tabs.push(['speed-tab-' + i, t]);
				else if(i == "INTERNET1")
					tabs.push(['speed-tab-' + i, t]);
				else if	(i == "WIRED")
					tabs.push(['speed-tab-' + i, t]);
				else if	(i == "WIRELESS0")
					tabs.push(['speed-tab-' + i, t]);
				else if	(i == "WIRELESS1")
					tabs.push(['speed-tab-' + i, t]);
				else if	(i == "BRIDGE")
					tabs.push(['speed-tab-' + i, t]);//tabs[4] = ['speed-tab-' + i, t];
				*/
				tabs.push(['speed-tab-' + i, t]);

			
			}
		}
		
		//Sort tab by Viz 2014.06
		var tabsort = ["speed-tab-INTERNET,<#Internet#>", "speed-tab-INTERNET,<#dualwan_primary#>","speed-tab-INTERNET1,<#dualwan_secondary#>","speed-tab-INTERNET,DSL WAN","speed-tab-INTERNET,WAN","speed-tab-INTERNET,WAN2","speed-tab-INTERNET,10G base-T","speed-tab-INTERNET,10G SFP+","speed-tab-INTERNET,Bond","speed-tab-INTERNET,LAN","speed-tab-INTERNET,USB Modem","speed-tab-INTERNET,<#Mobile_title#>","speed-tab-INTERNET1,WAN","speed-tab-INTERNET1,WAN2","speed-tab-INTERNET1,10G base-T","speed-tab-INTERNET1,10G SFP+","speed-tab-INTERNET1,Bond","speed-tab-INTERNET1,LAN","speed-tab-INTERNET1,<#Mobile_title#>","speed-tab-INTERNET1,USB Modem", "speed-tab-WAGGR0,bond-slave (WAN)", "speed-tab-WAGGR1,bond-slave (LAN1)", "speed-tab-WAGGR2,bond-slave (LAN2)", "speed-tab-WAGGR3,bond-slave (LAN3)", "speed-tab-WAGGR4,bond-slave (LAN4)", "speed-tab-WAGGR5,bond-slave (LAN5)", "speed-tab-WAGGR6,bond-slave (LAN6)", "speed-tab-WAGGR7,bond-slave (LAN7)", "speed-tab-WAGGR8,bond-slave (LAN8)", "speed-tab-WAGGR30,bond-slave (10G base-T)", "speed-tab-WAGGR31,bond-slave (10G SFP+)", "speed-tab-WIRED,<#tm_wired#>", "speed-tab-LACP1,bond-slave (LAN1)", "speed-tab-LACP2,bond-slave (LAN2)", "speed-tab-LACP3,bond-slave (LAN3)", "speed-tab-LACP4,bond-slave (LAN4)", "speed-tab-LACP5,bond-slave (LAN5)", "speed-tab-LACP6,bond-slave (LAN6)", "speed-tab-LACP7,bond-slave (LAN7)", "speed-tab-LACP8,bond-slave (LAN8)", "speed-tab-WIRELESS0,<#tm_wireless#> (2.4GHz)","speed-tab-WIRELESS1,<#tm_wireless#> (5GHz)", "speed-tab-WIRELESS1,<#tm_wireless#> (5GHz-1)", "speed-tab-WIRELESS2,<#tm_wireless#> (5GHz-2)", "speed-tab-WIRELESS3,<#tm_wireless#> (60GHz)", "speed-tab-BRIDGE,LAN"];
		var sortabs = [];
		for(var i=0;i<tabsort.length;i++){
			for(var j=0;j<tabs.length;j++){	
				if(tabsort[i] == tabs[j]){
					sortabs.push(tabs[j]);
				}
			}
		}
		tabs = sortabs;
		
		/*tabs = tabs.sort(
			function(a, b) {
				if (a[1] < b[1]) return -1;
				if (a[1] > b[1]) return 1;
				return 0;
			});*/
	}

	if (tabs.length == old.length) {
		for (i = tabs.length - 1; i >= 0; --i)
			if (tabs[i][0] != old[i][0]) break;
		changed = i > 0;
	}
	else changed = 1;

	if (changed) {
		E('tab-area').innerHTML = _tabCreate.apply(this, tabs);
	}
	if (((name = cookie.get(cprefix + 'tab')) != null) && ((speed_history[name] != undefined))) {
		showTab('speed-tab-' + name);
		return;
	}
	if (tabs.length) showTab(tabs[0][0]);
}

function initData()
{
	if (htmReady) {
		loadData();
		if (svgReady) {
			E('graph').style.visibility = 'visible';
			E('bwm-controls').style.visibility = 'visible';
		}
	}
}

function initCommon(defAvg, defDrawMode, defDrawColorRX, defDrawColorTX) //Viz modify defDrawColor 2010.09
{
	drawMode = fixInt(cookie.get(cprefix + 'draw'), 0, 1, defDrawMode);
	showDraw();

	var c = nvram.rstats_colors.split(',');

	c = (cookie.get(cprefix + 'color') || '').split(',');
	
	drawColorRX = defDrawColorRX;
	drawColorTX = defDrawColorTX;		
	showColor();

	scaleMode = fixInt(cookie.get(cprefix + 'scale'), 0, 1, 0);  //cprefix = 'bw_r';
	showScale();

	avgMode = fixInt(cookie.get(cprefix + 'avg'), 1, 10, defAvg);
	showAvg();

	// if just switched
	if ((nvram.wan_proto == 'disabled') || (nvram.wan_proto == 'wet')) {
		nvram.wan_ifname = '';
	}

	htmReady = 1;
	initData();

}
