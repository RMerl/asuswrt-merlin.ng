//For operation mode;
sw_mode = '<% nvram_get_x("IPConnection",  "sw_mode"); %>';
productid = '<% nvram_get_f("general.log","productid"); %>';

var uptimeStr = "<% uptime(); %>";
var timezone = uptimeStr.substring(26,31);
var boottime = parseInt(uptimeStr.substring(32,38));
var newformat_systime = uptimeStr.substring(8,11) + " " + uptimeStr.substring(5,7) + " " + uptimeStr.substring(17,25) + " " + uptimeStr.substring(12,16);  //Ex format: Jun 23 10:33:31 2008
var systime_millsec = Date.parse(newformat_systime); // millsec from system
var JS_timeObj = new Date(); // 1970.1.1

var test_page = 0;
var testEventID = "";
var dr_surf_time_interval = 5;	// second
var show_hint_time_interval = 1;	// second

var wan_route_x = "";
var wan_nat_x = "";
var wan_proto = "";

// Dr. Surf {
// for detect if the status of the machine is changed. {
var manually_stop_wan = "";

// original status {
var old_ifWANConnect = 0;
var old_qos_ready = 1;
var old_wan_link_str = "";
var old_detect_dhcp_pppoe = "";
var old_wan_status_log = "";
var old_detect_wan_conn = "";
var old_wan_ipaddr_t = "";

var old_disk_status = "";
var old_mount_status = "";
var old_printer_sn = "";
var old_wireless_clients = "";
// original status }

// new status {
var new_ifWANConnect = 0;
var new_wan_link_str = "";
var new_detect_dhcp_pppoe = "";
var new_wan_status_log = "";
var new_detect_wan_conn = "";
var new_wan_ipaddr_t = "";

var new_disk_status = "";
var new_mount_status = "";
var new_printer_sn = "";
var new_wireless_clients = "";
// new status }

var id_of_check_changed_status = 0;


function unload_body(){
	disableCheckChangedStatus();
	no_flash_button();
	
	return true;
}

function enableCheckChangedStatus(flag){ // 
	var seconds = this.dr_surf_time_interval*1000;
	
	disableCheckChangedStatus();
	
	if(old_wan_link_str == ""){
		seconds = 1;
		id_of_check_changed_status = setTimeout("get_changed_status('initial');", seconds);
	}
	else
		id_of_check_changed_status = setTimeout("get_changed_status();", seconds);
}

function disableCheckChangedStatus(){
	clearTimeout(id_of_check_changed_status);
	id_of_check_changed_status = 0;
}

function check_if_support_dr_surf(){
	if($("helpname"))
		return 1;
	else
		return 0;
}

function compareWirelessClient(target1, target2){
	if(target1.length != target2.length)
		return (target2.length-target1.length);
	
	for(var i = 0; i < target1.length; ++i)
		for(var j = 0; j < 3; ++j)
			if(target1[i][j] != target2[i][j])
					return 1;
	
	return 0;
}

function check_changed_status(flag){

	if(this.test_page == 1
			|| wan_route_x == "IP_Bridged")
		return;
	
	if(flag == "initial"){
		// for the middle of index.asp.
		if(location.pathname == "/" || location.pathname == "/index.asp"){
			if(old_detect_wan_conn == "1")
					showMapWANStatus(1);
			else if(old_detect_wan_conn == "2")
					showMapWANStatus(2);
			else if(old_wan_ipaddr_t == "0.0.0.0")
				showMapWANStatus(0);
			else
				showMapWANStatus(0);
		}
		
		// Dr. Surf -- stop crying.
/*	if(old_ifWANConnect == 0) // WAN port is not plugged. 
			parent.showDrSurf("1");
		else if(old_qos_ready == 0)
			parent.showDrSurf("40");
		else if(old_wan_link_str == "Disconnected"){

			// PPPoE, PPTP, L2TP
			if(wan_proto != "dhcp" && wan_proto != "static"){
				if(old_wan_status_log.indexOf("Failed to authenticate ourselves to peer") >= 0)
					parent.showDrSurf("2_1");
				else if(old_detect_dhcp_pppoe == "no-respond")
					parent.showDrSurf("2_2");
				else
					parent.showDrSurf("5");
			}
			// dhcp, static
			else{
				parent.showDrSurf("5");
			}
		}
		else if(old_detect_wan_conn != "1")
			parent.showDrSurf("2_2");
		else 
			parent.showDrSurf("0_0"); // connection is ok.
*/		
		enableCheckChangedStatus();
		
		return;
	}
	
	// for the middle of index.asp.
	if(location.pathname == "/" || location.pathname == "/index.asp"){
		if(new_detect_wan_conn == "1")
			showMapWANStatus(1);
		else if(new_detect_wan_conn == "2")
			showMapWANStatus(2);
		else if(new_wan_ipaddr_t == "0.0.0.0")
			showMapWANStatus(0);
		else
			showMapWANStatus(0);
	}
	
	// Dr.Surf.	
	var diff_number = compareWirelessClient(old_wireless_clients, new_wireless_clients);
	
	if(old_ifWANConnect != new_ifWANConnect){ // if WAN port is plugged.
		old_ifWANConnect = new_ifWANConnect;
		
		if(new_ifWANConnect == 1)
			parent.showDrSurf("0_2");	// not plugged -> plugged
		else
			parent.showDrSurf("1");	// plugged -> not plugged
	}	
	else if(old_detect_wan_conn != new_detect_wan_conn){
		if(new_detect_wan_conn == "1")
			parent.showDrSurf("0_0");
		else if(new_detect_wan_conn == "0")
			parent.showDrSurf("2_2");
		else
			parent.showDrSurf("2_3");
		old_detect_wan_conn = new_detect_wan_conn;
	}	
	else if(diff_number != 0){
		old_wireless_clients = new_wireless_clients;
		
		if(diff_number >= 0)
			parent.showDrSurf("11");
		else
			parent.showDrSurf("12");
	} 
	else if(old_disk_status != new_disk_status){
		old_disk_status = new_disk_status;
		
		parent.showDrSurf("20");
	}
	else if(parseInt(old_mount_status) < parseInt(new_mount_status)){
		old_mount_status = new_mount_status;
		
		parent.showDrSurf("21");
	} //lock Add 2009.04.01	
	else if(old_printer_sn != new_printer_sn){
		old_printer_sn = new_printer_sn;
	
		parent.showDrSurf("30");
	} //lock modified 2009.04.01
	else if(old_wan_link_str != new_wan_link_str){
		old_wan_link_str = new_wan_link_str;
		
		if(new_wan_link_str == "Disconnected"){
			old_detect_dhcp_pppoe = new_detect_dhcp_pppoe;
			
			// PPPoE, PPTP, L2TP
			if(wan_proto != "dhcp" && wan_proto != "static"){
				if(old_wan_status_log != new_wan_status_log){ // PPP serial change!
					old_wan_status_log = new_wan_status_log;
					
					if(new_wan_status_log.length > 0){
						if(new_wan_status_log.indexOf("Failed to authenticate ourselves to peer") >= 0)
							parent.showDrSurf("2_1");
						else
							parent.showDrSurf("2_2");
					}
					else if(new_detect_dhcp_pppoe == "no-respond")
						parent.showDrSurf("2_2");
					else
						parent.showDrSurf("5");
				}
				else if(new_detect_dhcp_pppoe == "no-respond")
					parent.showDrSurf("2_2");
				else
					parent.showDrSurf("3");
			}
			// dhcp, static
			else{
				if(new_detect_dhcp_pppoe == "no-respond")
					parent.showDrSurf("2_2");
				else if(new_detect_dhcp_pppoe == "error")
					parent.showDrSurf("3");
				else
					parent.showDrSurf("5");
			}
		}
		else if(new_detect_wan_conn != "1")
			parent.showDrSurf("2_2");
		else
			parent.showDrSurf("0_1"); 
	}

	enableCheckChangedStatus();
}

function get_changed_status(flag){
/*	if(flag == "initial")
		status_flag = "initial";
	else
		status_flag = "";
*/
	updateStatus_AJAX();
}

function initial_change_status(manually_stop_wan,
															 ifWANConnect,
														   wan_link_str,
														   detect_dhcp_pppoe,
														   wan_status_log,
														   disk_status,
														   mount_status,
														   printer_sn,
														   wireless_clients,
														   qos_ready,
															 detect_wan_conn,
															 wan_ipaddr_t
														   ){
	this.manually_stop_wan = manually_stop_wan;
	this.old_ifWANConnect = ifWANConnect;
	this.old_wan_link_str = wan_link_str;
	this.old_detect_dhcp_pppoe = detect_dhcp_pppoe;
	this.old_wan_status_log = wan_status_log;
	this.old_disk_status = disk_status;
	this.old_mount_status = mount_status;
	this.old_printer_sn = printer_sn;
	this.old_wireless_clients = wireless_clients;
	this.old_qos_ready = qos_ready;
	this.old_detect_wan_conn = detect_wan_conn;
	this.old_wan_ipaddr_t = wan_ipaddr_t;
}

function set_changed_status(manually_stop_wan,
														ifWANConnect,
														wan_link_str,
														detect_dhcp_pppoe,
														wan_status_log,
														disk_status,
														mount_status,
														printer_sn,
														wireless_clients,
														detect_wan_conn,
														wan_ipaddr_t		
														){

	this.manually_stop_wan = manually_stop_wan;
	this.new_ifWANConnect = ifWANConnect;
	this.new_wan_link_str = wan_link_str;
	this.new_detect_dhcp_pppoe = detect_dhcp_pppoe;
	this.new_new_wan_status_log = wan_status_log;
	this.new_disk_status = disk_status;
	this.new_mount_status = mount_status;
	this.new_printer_sn = printer_sn;
	this.new_wireless_clients = wireless_clients;
	this.new_detect_wan_conn = detect_wan_conn;
	this.new_wan_ipaddr_t = wan_ipaddr_t;
}
// for detect if the status of the machine is changed. }

function set_Dr_work(flag){
	/*if(flag != "help"){
		$("Dr_body").onclick = function(){
				showDrSurf();
			};
		
		$("Dr_body").onmouseover = function(){
				showDrSurf();
			};
		
		$("Dr_body").onmouseout = function(){
				showDrSurf();
			};
	}
	else{
		$("Dr_body").onclick = function(){
				showDrSurf(null, "help");
			};
		
		$("Dr_body").onmouseover = function(){
				showDrSurf(null, "help");
			};
		
		$("Dr_body").onmouseout = function(){
				showDrSurf(null, "help");
			};
	}*/
}

var slowHide_ID_start = 0;
var slowHide_ID_mid = 0;

function clearHintTimeout(){
	if(slowHide_ID_start != 0){
		clearTimeout(slowHide_ID_start);
		slowHide_ID_start = 0;
	}
	
	if(slowHide_ID_mid != 0){
		clearTimeout(slowHide_ID_mid);
		slowHide_ID_mid = 0;
	}
}

function showHelpofDrSurf(hint_array_id, hint_show_id){
	var seconds = this.show_hint_time_interval*1000;
	
	if(!$("eventDescription")){
		setTimeout('showHelpofDrSurf('+hint_array_id+', '+hint_show_id+');', 100);
		return;
	}
	
	disableCheckChangedStatus();
	clearHintTimeout();
	
	if(typeof(hint_show_id) == "number" && hint_show_id > 0){
		if(hint_array_id == "23"){
			var ssid_len = helpcontent[hint_array_id][hint_show_id].length;
			if(ssid_len > 14)
				clicked_help_string = "<span>"+helptitle[hint_array_id][hint_show_id][0]+"</span><br />"+"<p><div>"+decodeURIComponent(helpcontent[hint_array_id][hint_show_id]).substring(0,14)+"<br />"+decodeURIComponent(helpcontent[hint_array_id][hint_show_id]).substring(14,ssid_len)+"</div></p>";
			else
				clicked_help_string = "<span>"+helptitle[hint_array_id][hint_show_id][0]+"</span><br />"+"<p><div align='center'>"+decodeURIComponent(helpcontent[hint_array_id][hint_show_id])+"</div></p>";
		}
		else
			clicked_help_string = "<span>"+helptitle[hint_array_id][hint_show_id][0]+"</span><br />"+helpcontent[hint_array_id][hint_show_id];
	}
	$("eventDescription").innerHTML = clicked_help_string;
	
	set_Dr_work("help");
	$("eventLink").onclick = function(){};
	showtext($("linkDescription"), "");
	
	$("drsword").style.filter = "alpha(opacity=100)";
	$("drsword").style.opacity = 1;	
	$("drsword").style.visibility = "visible";
	
	$("wordarrow").style.filter	= "alpha(opacity=100)";
	$("wordarrow").style.opacity = 1;	
	$("wordarrow").style.visibility = "visible";
	
	slowHide_ID_start = setTimeout("slowHide(100);", seconds);
}

var current_eventID = null;
var now_alert = new Array(3);

var alert_event0_0 = new Array("<#DrSurf_word_connection_ok#>", "", "");
var alert_event0_1 = new Array("<#DrSurf_word_connection_recover#>", "<#DrSurf_refresh_page#>", refreshpage);
var alert_event0_2 = new Array("<#DrSurf_word_connection_WANport_recover#>", "<#DrSurf_refresh_page#>", refreshpage);
var alert_event1 = new Array("<#web_redirect_reason1#>", "<#DrSurf_referto_diagnosis#>", drdiagnose);
var alert_event2_1 = new Array("<#web_redirect_reason2_1#>", "<#DrSurf_referto_diagnosis#>", drdiagnose);
var alert_event2_2 = new Array("<#web_redirect_reason2_2#>", "<#DrSurf_referto_diagnosis#>", drdiagnose);
var alert_event2_3 = new Array("<#QKSet_detect_desc2#>", "", drdiagnose);
var alert_event3 = new Array("<#web_redirect_reason3_1#>", "<#DrSurf_referto_diagnosis#>", drdiagnose);
var alert_event4 = new Array("<#web_redirect_reason4#>", "<#DrSurf_referto_diagnosis#>", drdiagnose);  //wan_gateway & lan_ipaddr;
var alert_event5 = new Array("1. <#web_redirect_reason5_1#><br>2. <#web_redirect_reason5_2#>", "<#DrSurf_referto_diagnosis#>", drdiagnose);

var alert_event10 = new Array("<#DrSurf_Alert10#>", "<#DrSurf_referto_diagnosis#>", drdiagnose);
var alert_event11 = new Array("<#DrSurf_Alert11#>", "<#DrSurf_referto_diagnosis#>", drdiagnose);
var alert_event12 = new Array("<#DrSurf_Alert12#>", "<#DrSurf_referto_diagnosis#>", drdiagnose);
var alert_event20 = new Array("<#DrSurf_Alert20#>", "<#DrSurf_referto_diagnosis#>", drdiagnose);
var alert_event21 = new Array("<#DrSurf_Alert21#>", "<#DrSurf_referto_diagnosis#>", drdiagnose);
var alert_event30 = new Array("<#DrSurf_Alert30#>", "<#DrSurf_referto_diagnosis#>", drdiagnose);
var alert_event40 = new Array("<#DrSurf_Alert40#>", "<#DrSurf_referto_diagnosis#>", drdiagnose);

function showDrSurf(eventID, flag){
	var seconds = this.show_hint_time_interval*1000;
	var temp_eventID;
	
	// for test
	if(this.testEventID != "")
		eventID = this.testEventID;
	
	if(eventID){
		this.current_eventID = eventID;
		temp_eventID = eventID;
	}
	else
		temp_eventID = this.current_eventID;
	
	if(!temp_eventID || temp_eventID.length <= 0){
		id_of_check_changed_status = setTimeout("enableCheckChangedStatus();", 1000);
		return;
	}
	
	disableCheckChangedStatus();
	clearHintTimeout();
	
	if(flag != "help"){
		now_alert[0] = eval("alert_event"+temp_eventID+"[0]");
		if(temp_eventID != "5")
			showtext($("eventDescription"), now_alert[0]);
		else if(this.manually_stop_wan == "1")
			showtext($("eventDescription"), "<#web_redirect_reason5_1#>");
		else
			showtext($("eventDescription"), "<#web_redirect_reason5_2#>");
		
		now_alert[1] = eval("alert_event"+temp_eventID+"[1]");
		if(now_alert[1] != ""){
			now_alert[2] = eval("alert_event"+temp_eventID+"[2]");
			
			$("eventLink").onclick = function(){
					now_alert[2](temp_eventID);
				};
			
			showtext($("linkDescription"), now_alert[1]);
		}
	}
	
	$("drsword").style.filter = "alpha(opacity=100)";
	$("drsword").style.opacity = 1;	
	$("drsword").style.visibility = "visible";
	
	$("wordarrow").style.filter	= "alpha(opacity=100)";
	$("wordarrow").style.opacity = 1;	
	$("wordarrow").style.visibility = "visible";
	
	slowHide_ID_start = setTimeout("slowHide(100);", seconds);
}

function slowHide(filter){
	clearHintTimeout();
	
	$("drsword").style.filter = "alpha(opacity="+filter+")";
	$("drsword").style.opacity = filter*0.01;
	$("wordarrow").style.filter	= "alpha(opacity="+filter+")";
	$("wordarrow").style.opacity = filter*0.01;
	
	filter -= 5;
	if(filter <= 0){
		hideHint();
		
		enableCheckChangedStatus();
	}
	else
		slowHide_ID_mid = setTimeout("slowHide("+filter+");", 100);
}

function hideHint(){
	if(this.current_eventID){
		now_alert[0] = eval("alert_event"+this.current_eventID+"[0]");
		showtext($("eventDescription"), now_alert[0]);
		
		now_alert[1] = eval("alert_event"+this.current_eventID+"[1]");
		if(now_alert[1] != ""){
			now_alert[2] = eval("alert_event"+this.current_eventID+"[2]");
			
			$("eventLink").onclick = function(){
					now_alert[2](current_eventID);
				};
			
			showtext($("linkDescription"), now_alert[1]);
		}
	}
	
	$("drsword").style.visibility = "hidden";
	//$("wordarrow").style.visibility = "hidden";
}

function drdiagnose(eventID){
	if(!check_if_support_dr_surf()){
		alert("Don't yet support Dr. Surf!");
		return;
	}
	
	if($('statusIcon'))
		$('statusIcon').src = "/images/iframe-iconDr.gif";
	
	if(typeof(openHint) == "function")
		openHint(0, 0);
	
	showtext($('helpname'), "<#DrSurf_Diagnose_title#>");
	
	if($("hint_body"))
		$("hint_body").style.display = "none";
	
	$("statusframe").style.display = "block";
	$('statusframe').src = "/device-map/diagnose"+eventID+".asp";
}
// Dr. Surf }

var banner_code, menu_code="", menu1_code="", menu2_code="", tab_code="", footer_code;


function show_banner(L3){// L3 = The third Level of Menu

	var banner_code = "";
	
	/*	
	// for chang language
	banner_code +='<form method="post" name="titleForm" id="titleForm" action="/start_apply.htm" target="hidden_frame">\n';
	banner_code +='<input type="hidden" name="current_page" value="">\n';
	banner_code +='<input type="hidden" name="sid_list" value="LANGUAGE;">\n';
	banner_code +='<input type="hidden" name="action_mode" value=" Apply ">\n';
	banner_code +='<input type="hidden" name="preferred_lang" value="">\n';
	banner_code +='<input type="hidden" name="flag" value="">\n';
	banner_code +='</form>\n';
	
	banner_code +='<div class="banner1" align="center"></div>\n';
	banner_code +='<table width="983" border="0" align="center" cellpadding="0" cellspacing="0">\n';
	banner_code +='<tr>\n';
	banner_code +='<td class="top-logo"><a href="/"><div id="modelName"><#Web_Title#></div></a></td>\n';
	
	banner_code +='<td class="top-message" width="250">\n';
	//banner_code +='<span class="top-messagebold"><#Time#>: </span><span class="time" id="systemtime"></span><br/>\n';
	banner_code +='<span class="top-messagebold"><#menu5_1#>: </span><input type="button" class="button2_NW" style="width:55px;" value="2.4GHz" id="elliptic_ssid_2g" onclick="go_setting(2);"><input type="button" style="width:55px;" class="button5_NW" value="5GHz" id="elliptic_ssid" onclick="go_setting(5);"><br/>\n';
	//banner_code +='<span class="top-messagebold">SSID: </span><input class="top_ssid" type="text" value="" id="elliptic_ssid" readonly=readonly><br/>\n';
	banner_code +='<span class="top-messagebold"><#General_x_FirmwareVersion_itemname#> </span><a href="/Advanced_FirmwareUpgrade_Content.asp"><span id="firmver" class="time"></span></a><br/>\n';
	banner_code +='<span class="top-messagebold" title="<#OP_desc1#>"><#menu5_6_1_title#>: </span><a href="/Advanced_OperationMode_Content.asp"><span id="sw_mode_span" class="time"></span></a>\n';	
	banner_code +='</td>\n';
	
	banner_code +='<td class="top-message"width="150">\n';
	banner_code +='<span class="top-messagebold"><#PASS_LANG#></span><br>\n';
	banner_code +='<select name="select_lang" id="select_lang" class="top-input" onchange="change_language();">\n';
	banner_code +='<% shown_language_option(); %>';
	banner_code +='</select>\n';
	banner_code +='<input type="button" id="change_lang_btn" class="button" value="<#CTL_ok#>" onclick="submit_language();" style="float:right; margin:5px 10px 0 0;" disabled=disabled>\n';
	
	banner_code +='</td>\n';
	banner_code +='<td class="top-message" width="120">\n';
	banner_code +='<div id="logout_btn" class="buttonquit"><a style="text-decoration:none;" href="javascript:;" onclick="logout();"><#t1Logout#></a></div>\n';
	banner_code +='<div id="reboto_btn" class="buttonquit"><a style="text-decoration:none;" href="javascript:;" onclick="reboot();"><#BTN_REBOOT#></a></div>\n';
	banner_code +='</td>\n';
	
// Dr. Surf {
	banner_code += '<td id="Dr_body" class="top-message" width="40">\n';
	
	banner_code += '<div id="dr" class="dr"></div>\n';
	banner_code += '<div id="drsword" class="drsword">\n';
	banner_code += '<span id="eventDescription"></span>\n';
	banner_code += '<br>\n';
	banner_code += '<a id="eventLink" href="javascript:void(0);"><span id="linkDescription"></span></a>\n';
	banner_code += '</div>\n';
	banner_code += '<div id="wordarrow" class="wordarrow"><img src="/images/wordarrow.png"></div>\n';
	
	banner_code += '&nbsp;</td>\n';
// Dr. Surf }
	
	banner_code +='<td width="11"><img src="images/top-03.gif" width="11" height="78" /></td>\n';
	banner_code +='</td></tr></table>\n';
	
	if(L3 == 0) 		// IF Without Level 3 menu, banner style will use top.gif.
		banner_code +='<div id="banner3" align="center"><img src="images/top.gif" width="983" height="19" /></div>\n';
	else
		banner_code +='<div id="banner3" align="center"><img src="images/top-advance.gif" width="983" height="19" /></div>\n';
*/
	
	// for chang language
	/*banner_code +='<link rel="stylesheet" type="text/css" href="/routercss.css">\n';
	banner_code +='<form method="post" name="titleForm" id="titleForm" action="/start_apply.htm" target="hidden_frame">\n';
	banner_code +='<input type="hidden" name="current_page" value="">\n';
	banner_code +='<input type="hidden" name="sid_list" value="LANGUAGE;">\n';
	banner_code +='<input type="hidden" name="action_mode" value=" Apply ">\n';
	banner_code +='<input type="hidden" name="preferred_lang" value="">\n';
	banner_code +='<input type="hidden" name="flag" value="">\n';
	banner_code +='</form>\n';*/
	
	banner_code +='<div align="right">\n';
	banner_code +='<div style="position:absolute;"><img src="images/Hand_ui/asustitle.png" width="121" height="30" align="left"></div>\n';
	banner_code +='<input type="button" value="Logout" onclick="logout();" />';          //chang margin-left 100->300
	/* banner_code +='<a href="javascript:reboot();"><div style="margin-top:13px;margin-left:20px;" class="titlebtn" align="center"><span><#BTN_REBOOT#></span></div></a>\n';    delete by alan   */
	banner_code +='<span style="color:white;display:none;">'+multiLanguage_all_array[multi_INT][2]+'</span>\n';
	banner_code +='<select name="select_lang" id="select_lang" style="display:none;" class="input_option_lang" onchange="submit_language(this.value);">\n';    
	//banner_code +='<% shown_language_option(); %>';
	banner_code +='<option value="EN" id="select_lang_0">English</option>\n<option value="TW" id="select_lang_1">繁體中文</option>\n<option value="CN" id="select_lang_2">简体中文</option>\n<option value="RU" id="select_lang_3">Russian</option>\n<option value="FR" id="select_lang_4">French</option>\n<option value="DE" id="select_lang_5">German</option>\n<option value="BR" id="select_lang_6">Brazil portuguese</option>\n<option value="CZ" id="select_lang_7">Czech</option>\n<option value="DA" id="select_lang_8">Danish</option>\n<option value="FI" id="select_lang_9">Finnish</option>\n<option value="MS" id="select_lang_10">Malay</option>\n<option value="NO" id="select_lang_11">Norwegian</option>\n<option value="PL" id="select_lang_12">Polish</option>\n<option value="SV" id="select_lang_13">Swedish</option>\n<option value="TH" id="select_lang_14">Thai</option>\n<option value="TR" id="select_lang_15">Turkish</option>\n';
	banner_code +='</select>\n';
	//banner_code +='<a id="helpAddress" href=""><img src="/images/icon/help.png" title="help" align="right" /></a>'
	banner_code +='</div>\n';

	banner_code +='<table width="100%" border="0" align="center" cellpadding="0" cellspacing="0">\n';
	
	// Dr. Surf {
	banner_code +='<tr>\n';
	//banner_code +='<td><div><img src="/images/New_ui/export/line_export.png" style="width:100%;">\n';
	//banner_code +='</div></td>\n';
	//banner_code +='<td class="top-logo" ><a href="/"><div id="modelName" style="display:none"><#Web_Title#></div></a></td>\n';
	//banner_code +='</td>\n';
	banner_code += '<td id="Dr_body" class="top-message" width="40" style="display:none">\n';
	banner_code += '<div id="dr" class="dr"></div>\n';
	banner_code += '<div id="drsword" class="drsword">\n';
	banner_code += '<span id="eventDescription"></span>\n';
	banner_code += '<br>\n';
	banner_code += '<a id="eventLink" href="javascript:void(0);"><span id="linkDescription"></span></a>\n';
	banner_code += '</div>\n';
	//banner_code += '<div id="wordarrow" class="wordarrow"><img src="/images/wordarrow.png"></div>\n';
	banner_code += '&nbsp;</td>\n';
	banner_code +='</tr>\n';
	// Dr. Surf }
	
  //banner_code +='<td valign="center" class="titledown" width="40">SSID:</td>\n';
  // Viz edited start
  //banner_code +='<td valign="center" class="titledownbt" width="56"><input class="button" type="button" value="2.4GHz" id="elliptic_ssid_2g" onclick="go_setting(2);"></td>\n';
  //banner_code +='<td><a href="javascript:go_setting(2);"><div class="titledownbtntext"><span id="elliptic_ssid_2g"></span></div></a></td>\n';
  //banner_code +='<td valign="center" class="titledownbt" width="122"><input class="button" type="button" value="5GHz" id="elliptic_ssid" onclick="go_setting(5);"></td>\n';
 	//banner_code +='<td valign="center" class="titledown" width="auto"><span id="elliptic_ssid"></span></td>\n';
	//SSID:<a href="/Advanced_Wireless_Content.asp" style="color:white">    </a>     //chang by alan   put behind <td>
  //Viz edited end
 // banner_code +='<td valign="center" class="titledown" width="auto"><span id="sw_mode_span" class="time"></span></td>\n';
  //<#menu5_6_1_title#>:<a href="/Advanced_OperationMode_Content.asp" style="color:white"> </a>  //chang by alan   put behind <td>
 // banner_code +='<td valign="center" class="titledown" width="auto"><span id="firmver" class="time"></span></td>\n';
  //<#General_x_FirmwareVersion_itemname#><a href="/Advanced_FirmwareUpgrade_Content.asp" style="color:white"> </a>   //chang by alan   put behind <td>
  //banner_code +='<td valign="center" class="titledownbt" width="73"><a href="/Advanced_FirmwareUpgrade_Content.asp" style="color:white"><span id="firmver" class="time"></span></a></td>\n';
  //banner_code +='<td valign="center" class="titledownbt" width="213"></td>\n';
  //banner_code +='<td width="30"><img src="images/New_ui/contectstatus.png" width="30" height="30" align="left"></td>\n';
  /* banner_code +='<td width="30"><div id="conncet_status"></div></td>\n';
  banner_code +='<td width="30"><div id="usb_status"></div></td>\n';
  banner_code +='<td width="30"><div id="printer_status"></div></td>\n'; delete by alan*/
  banner_code +='</table>\n';

	$("TopBanner").innerHTML = banner_code;
	
	show_loading_obj();
	
	if(location.pathname == "/" || location.pathname == "/index.asp"){
		if(wan_route_x != "IP_Bridged")
			id_of_check_changed_status = setTimeout('hideLoading();', 3000);
	}
	else
		id_of_check_changed_status = setTimeout('hideLoading();', 1);
	
	//show_time();
	//show_top_status();
	set_Dr_work();
}

var tabtitle = new Array(7);
tabtitle[0] = new Array("", "<#menu5_1_1#>", "<#menu5_1_2#>", "<#menu5_1_3#>", "<#menu5_1_4#>", "<#menu5_1_5#>", "<#menu5_1_6#>");
tabtitle[1] = new Array("", "<#menu5_2_1#>", "<#menu5_2_2#>", "<#menu5_2_3#>");
tabtitle[2] = new Array("", "<#menu5_3_1#>", "<#menu5_3_2#>", "<#menu5_3_3#>", "<#menu5_3_4#>", "<#menu5_3_5#>", "<#menu5_3_6#>");
tabtitle[3] = new Array("", "<#menu5_4_1#>", "<#menu5_4_2#>", "<#menu5_4_3#>", "<#menu5_4_4#>");
tabtitle[4] = new Array("", "<#menu5_5_1#>", "<#menu5_5_2#>", "<#menu5_5_3#>", "<#menu5_5_4#>");
tabtitle[5] = new Array("", "<#menu5_6_1#>", "<#menu5_6_2#>", "<#menu5_6_3#>", "<#menu5_6_4#>");
tabtitle[6] = new Array("", "<#menu5_7_2#>", "<#menu5_7_3#>", "<#menu5_7_4#>", "<#menu5_7_5#>", "<#menu5_7_6#>");
tabtitle[7] = new Array("", "EZQoS", "Traffic Monitor");

//Level 3 Tab title
var tablink = new Array(7);
tablink[0] = new Array("", "Setting_General.asp", "Advanced_Wireless_Content.asp", "Advanced_WWPS_Content.asp", "Advanced_WMode_Content.asp", "Advanced_ACL_Content.asp", "Advanced_WSecurity_Content.asp");
tablink[1] = new Array("", "Setting_BT.asp", "Advanced_LAN_Content.asp", "Advanced_DHCP_Content.asp", "Advanced_GWStaticRoute_Content.asp");
tablink[2] = new Array("", "Setting_NZB.asp", "Advanced_QOSUserSpec_Content.asp", "Advanced_PortTrigger_Content.asp", "Advanced_VirtualServer_Content.asp", "Advanced_Exposed_Content.asp");
tablink[3] = new Array("", "Setting_HTTP.asp", "Advanced_AiDisk_samba.asp", "Advanced_AiDisk_ftp.asp", "Advanced_HSDPA_others.asp");
tablink[4] = new Array("", "Setting_FTP.asp", "Advanced_URLFilter_Content.asp", "Advanced_MACFilter_Content.asp", "Advanced_Firewall_Content.asp");
tablink[5] = new Array("", "Advanced_OperationMode_Content.asp", "Advanced_System_Content.asp", "Advanced_FirmwareUpgrade_Content.asp", "Advanced_SettingBackup_Content.asp");
tablink[6] = new Array("", "Main_LogStatus_Content.asp", "Main_DHCPStatus_Content.asp", "Main_WStatus_Content.asp", "Main_IPTStatus_Content.asp", "Main_RouteStatus_Content.asp");
tablink[7] = new Array("", "QoS_EZQoS.asp", "Main_TrafficMonitor_realtime.asp", "Main_TrafficMonitor_last24.asp", "Main_TrafficMonitor_daily.asp");

//Level 2 Menu
/* menuL2_title = new Array("", "<#menu5_1#>", "<#menu5_2#>", "<#menu5_3#>", "<#menu5_4#>", "<#menu5_5#>", "<#menu5_6#>", "<#menu5_7#>");
menuL2_link  = new Array("", tablink[0][1], tablink[1][1], tablink[2][1], tablink[3][1], tablink[4][1], tablink[5][1], tablink[6][1]);  delete by alan */
//menuL2_title = new Array("", "General", "Bit Torrent", "NZB", "HTTP", "FTP");//add by alan
menuL2_title = new Array("", multiLanguage_all_array[multi_INT][3], "Bit Torrent", "NZB");
//menuL2_link  = new Array("", tablink[0][1], tablink[1][1], tablink[2][1], tablink[3][1], tablink[4][1]);//add by alan
menuL2_link  = new Array("", tablink[0][1], tablink[1][1], tablink[2][1]);
//Level 1 Menu in Gateway, Router mode
//menuL1_title = new Array("", "<#menu1#>", "<#menu3#>", "<#menu2#>", "<#menu4#>", "<#menu5#>");  delete by alan
menuL1_title = new Array("", multiLanguage_all_array[multi_INT][0], "");    // add by alan
//menuL1_link = new Array("", "index.asp", "aidisk.asp", "upnp.asp", "QoS_EZQoS.asp", "as.asp");  delete by alan
menuL1_link = new Array("", "index.asp", "as.asp");     //add by alan

// Modulized by jerry5_chang start
// support list
var usb_support = new Array(1,1,0); // 0:SAMBA; 1:FTP; 2:3G
var traffic_monitor_support = 1;
var dual_band_support = 1;
var multissid_support = 1;
var qis_support = 1;
var nat_support = 1;
var bridge_support = 1;

function remove_url(){ // define every unneeded item
	if(traffic_monitor_support == 0){ // ifdef traffic_monitor_support	
		remove_menu_item(7, "QoS_EZQoS.asp");
		remove_menu_item(7, "Main_TrafficMonitor_realtime.asp");
	}
	
	if(nat_support == 0){ // ifdef nat_support
		remove_menu_item(2, "Advanced_PortTrigger_Content.asp");
		remove_menu_item(2, "Advanced_VirtualServer_Content.asp");
		remove_menu_item(2, "Advanced_Exposed_Content.asp");
	}
	
	if(bridge_support == 0){ // ifdef bridge_support
	}

	if(usb_support == (0,0,0)){	// ifdef usb_support
		remove_menu_item(3, "Advanced_AiDisk_samba.asp");
		remove_menu_item(3, "Advanced_AiDisk_ftp.asp");
		remove_menu_item(3, "Advanced_AiDisk_other.asp");
		remove_menu_item(3, "Advanced_HSDPA_others.asp");
		menuL2_link[4] = "";
		menuL2_title[4] = "";
	}
	else{
		if(usb_support[0] == 0){
			remove_menu_item(3, "Advanced_AiDisk_samba.asp");
		}
		if(usb_support[1] == 0){
			remove_menu_item(3, "Advanced_AiDisk_ftp.asp");
		}
		if(usb_support[2] == 0){
			remove_menu_item(3, "Advanced_HSDPA_others.asp");
		}
	}
}

function remove_menu_item(L2, remove_url){
	var dx;
	
	for(var i = 0; i < tablink[L2].length; i++){
		dx = tablink[L2].getIndexByValue(remove_url);
		tabtitle[L2].splice(dx, 1);
		tablink[L2].splice(dx, 1);
		break;
	}
}

Array.prototype.getIndexByValue= function(value){
	var index = -1;
	for (var i = 0; i < this.length; i++){
		if (this[i] == value){
			index = i;
			break;
		}
	}
	return index;
}

function show_menu(){
	// show menu without allocate variables
	var L1 = 0, L2 = 0, L3 = 0;
	var url = location.pathname.substring(location.pathname.lastIndexOf('/') + 1);
	
	remove_url();
	
	for(var i = 1; i < menuL1_link.length; i++){
		if(url == menuL1_link[i]){
			L1 = i;
			break;
		}
		else
			L1 = 5;
	}
	if(L1 == 5){
		for(var j = 0; j < tablink.length; j++){
			for(var k = 1; k < tablink[j].length; k++){
				if(url == tablink[j][k]){
					L2 = j+1;
					L3 = k;
					break;
				}
			}
		}
	}
	
	// special case
	if(traffic_monitor_support == 1 && (L1 == 4 || L2 == 8)){ // ifdef traffic_monitor_support
		if(url.indexOf("Main_TrafficMonitor_") == 0){
			L1 = 4;
			L2 = -2; 
			L3 = 2;
		}
		else{
			L1 = 4;
			L2 = -2; 
			L3 = 1;
		}
	}
	
	show_banner(L3);
	show_footer();
	browser_compatibility();	
	// end
	
	menu1_code += '<div class="m0_r" id="option0" style="margin-top:-160px;">';
	menu1_code += '<span>'+multiLanguage_all_array[multi_INT][0]+'</span>\n';
	menu1_code += '</div>\n'; 
	
	for(i = 1; i <= menuL1_title.length-2; i++){
		if(menuL1_title[i] == "")
			continue;
		else if(L1 == i && L2 <= 0){
		  menu1_code += '<div class="m'+i+'_r" id="option'+i+'">'+'<img border="0" src="images/New_ui/icon_index_'+i+'.png" style="margin-left:7px"/><div style="margin-top:-30px; margin-left:65px">'+menuL1_title[i]+'</div>\n';
		  menu1_code += '</div>\n';
		}
		else{
		  menu1_code += '<div class="menu" id="option'+i+'"><a href="'+menuL1_link[i]+'">'+'<img border="0" src="images/New_ui/icon_index_'+i+'.png"/><div style="margin-top:-30px; margin-left:55px">'+menuL1_title[i]+'</div>\n';
		  menu1_code += '</a></div>\n';
		}
	}

	menu1_code += '<div class="m0_r" id="option0">';
	menu1_code += '<span>'+multiLanguage_all_array[multi_INT][1]+'</span>\n';
	menu1_code += '</div>\n';
	
	//$("mainMenu").innerHTML = menu1_code;
	
	if(L2 != -1){
		for(var i = 1; i <= menuL2_title.length-1; ++i){
			if(i == menuL2_title.length-1){
				if(menuL2_title[i] == "")
					continue;
				else if(L2 == i){
					menu2_code += '<div class="m4_r" id="option4">'+'<img border="0" src="images/New_ui/icon_index_setting.png"/><div style="margin-top:-30px; margin-left:55px">'+menuL2_title[i]+'</div>\n';
					menu2_code += '</div>\n';
					
					}
				else
					menu2_code += '<div class="menu" id="option4"><a border="0" href="'+menuL2_link[i]+'">'+'<img border="0" src="images/New_ui/icon_index_setting.png"/><div style="margin-top:-30px; margin-left:55px">'+menuL2_title[i]+'</div>\n';
					menu2_code += '</a></div>\n';
			}
			else{
				if(menuL2_title[i] == "")
					continue;
				else if(L2 == i){
					menu2_code += '<div class="m1_r" id="option1">'+'<img border="0" src="images/New_ui/icon_index_setting.png"/><div style="margin-top:-30px; margin-left:55px">'+menuL2_title[i]+'</div>\n';
					menu2_code += '</div>\n';
					}
				else
					menu2_code += '<div class="menu" id="option1"><a href="'+menuL2_link[i]+'">'+'<img border="0" src="images/New_ui/icon_index_setting.png"/><div style="margin-top:-30px; margin-left:55px">'+menuL2_title[i]+'</div>';
					menu2_code += '</a></div>\n';
			}
		}
	}
	
	//menu2_code += '<div><img src="images/m-button-07end.gif" width="187" height="47" /></div>\n';
	//$("subMenu").innerHTML = menu2_code;
	
	if(L3){
		if(L2 != -2){
			tab_code = '<table border="0" cellspacing="0" cellpadding="0"><tr>\n';
			for(var i = 1; i < tabtitle[L2-1].length; ++i){
				if(tabtitle[L2-1][i] == "")
					continue;
				else if(L3 == i)
					tab_code += '<td><div id="t1" class="tabclick"><span id="span1">'+ tabtitle[L2-1][i] +'</span></div></td>';
					//tab_code += '<td class=\"b1\">'+ tabtitle[L2-1][i] +'</td>\n';
				else
					tab_code += '<td><a href="' +tablink[L2-1][i]+ '"><div id="t0" class="tab"><span id="span0">'+ tabtitle[L2-1][i] +'</span></div></a></td>';
					//tab_code += '<td class=\"b2\"><a href="' +tablink[L2-1][i]+ '">'+ tabtitle[L2-1][i] +'</a></td>\n';
			}
			tab_code += '</tr></table>\n';		
			//$("tabMenu").innerHTML = tab_code;
		}
		else{
			tab_code = '<table border="0" cellspacing="0" cellpadding="0"><tr>\n';
			for(var i = 1; i < tabtitle[7].length; ++i){
				if(tabtitle[7][i] == "")
					continue;
				else if(L3 == i)
					tab_code += '<td><div id="t1" class="tabclick"><span id="span1">'+ tabtitle[7][i] +'</span></div></td>';
					/*tab_code += '<td class=\"b1\">'+ tabtitle[7][i] +'</td>\n';*/
				else
					tab_code += '<td><a href="' +tablink[7][i]+ '"><div id="t0" class="tab"><span id="span0">'+ tabtitle[7][i] +'</span></div></a></td>';
					/*tab_code += '<td class=\"b2\"><a href="' +tablink[7][i]+ '">'+ tabtitle[7][i] +'</a></td>\n';*/
			}
			tab_code += '</tr></table>\n';		
			//$("tabMenu").innerHTML = tab_code;
		}
	}
	else
		{
		//$("tabMenu").innerHTML = "";
		}
}
// Modulized by jerry5_chang end

function show_footer(){
	//var footer_code;
	//footer_code = '<div align="center" class="bottom-image"></div>\n';
	footer_code ='<div align="center"><a style="color:#F90;">手机版</a> | <a id="handToPC" style="color:#CCC;">完整版</a></div>\n';
	footer_code +='<div align="right" class="copyright">'+multiLanguage_all_array[multi_INT][4]+'</div>\n';
	
	$("footer").innerHTML = footer_code;
	
	if($("helpname"))
		showtext($("helpname"), "<#CTL_help#>");
	if($("hint_body"))
		showtext($("hint_body"), "<#Help_init_word1#> <a class=\"hintstyle\" style=\"background-color:#7aa3bd\"><#Help_init_word2#></a> <#Help_init_word3#>");
	flash_button();
		
	/*$("elliptic_ssid").onmouseover = function(){
		//parent.showHelpofDrSurf(23, 1);
	};
	
	$("elliptic_ssid_2g").onmouseover = function(){
		//parent.showHelpofDrSurf(23, 2);
	};*/
}



function browser_compatibility(){
	var isFirefox = navigator.userAgent.search("Firefox") > -1;
	var isOpera = navigator.userAgent.search("Opera") > -1;
	var isIE8 = navigator.userAgent.search("MSIE 8") > -1; 
	
	if((isFirefox || isOpera) && document.getElementById("FormTitle")){
		document.getElementById("FormTitle").className = "FormTitle_firefox"; 
	}
	/*else if(isIE8 && document.getElementByClassName("content")){
		document.getElementByClassName("content").style.marginTop = "-5px"; 
	}*/
}	

/*  delete by alan 
function show_top_status(){
	
	var ssid_status_5g = "<% nvram_get_x("WLANConfig11b", "wl_ssid"); %>";
	var ssid_status_2g = "<% nvram_get_x("WLANConfig11b", "rt_ssid"); %>";	
	
	if(ssid_status_5g.length > 18 || ssid_status_2g.length > 18){
		if(ssid_status_5g.length > 18)
			ssid_status_5g = ssid_status_5g.substring(0,15) + "...";
		if(ssid_status_2g.length > 18)
			ssid_status_2g = ssid_status_2g.substring(0,15) + "...";

		//$("elliptic_ssid").title = ssid_status_5g + "  /  " + ssid_status_2g;	
	}
	
	//$("elliptic_ssid").innerHTML = ssid_status_5g + "  /  " + ssid_status_2g;	
	
	showtext($("firmver"), document.form.firmver.value);
	
	/*if(sw_mode == "1")  // Show operation mode in banner, Lock add at 2009/02/19
		$("sw_mode_span").innerHTML = "IP Sharing";
	else if(sw_mode == "2")
		$("sw_mode_span").innerHTML = "Router";
	else
		$("sw_mode_span").innerHTML = "AP";	*/
//}*/


function go_setting(band){
//	if(band == "2")
//		location.href = "Advanced_Wireless_Content.asp";
//	else
		location.href = "Advanced_Wireless_Content.asp";
}

function show_time(){	
	JS_timeObj.setTime(systime_millsec); // Add millsec to it.	
	JS_timeObj3 = JS_timeObj.toString();	
	JS_timeObj3 = checkTime(JS_timeObj.getHours()) + ":" +
				  			checkTime(JS_timeObj.getMinutes()) + ":" +
				  			checkTime(JS_timeObj.getSeconds());
	$('systemtime').innerHTML ="<a href='/Advanced_System_Content.asp'>" + JS_timeObj3 + "</a>";
	systime_millsec += 1000;		
	
	stime_ID = setTimeout("show_time();", 1000);
}

function checkTime(i)
{
if (i<10) 
  {i="0" + i}
  return i
}

function show_loading_obj(){
	var obj = $("Loading");
	var code = "";
	
	code +='<table cellpadding="5" cellspacing="0" id="loadingBlock" class="loadingBlock" align="center">\n';
	code +='<tr>\n';
	code +='<td width="20%" height="80" align="center"><img src="images/loading.gif"></td>\n';
	code +='<td><span id="proceeding_main_txt"></span><span id="proceeding_txt" style="color:#FFFFCC;"></span></td>\n';
	code +='</tr>\n';
	code +='</table>\n';
	code +='<!--[if lte IE 6.5]><iframe class="hackiframe"></iframe><![endif]-->\n';
	
	obj.innerHTML = code;
}

var nav;

if(navigator.appName == 'Netscape')
	nav = true;
else{
	nav = false;
	document.onkeydown = MicrosoftEventHandler_KeyDown;
}

function MicrosoftEventHandler_KeyDown(){
	return true;
}

function submit_language(LanguageValue){
	var url = "ms_apply.cgi";
	var action_mode = "DM_LANG";
	var lang;
	url += "?action_mode="+action_mode +"&DM_language="+LanguageValue+"&t="+Math.random();
	if(LanguageValue == "EN")
	lang = 0;
	else if(LanguageValue == "TW")
	lang = 1;
	else if(LanguageValue == "CN")
	lang = 2;
	else if(LanguageValue == "RU")
	lang = 3;
	else if(LanguageValue == "FR")
	lang = 4;
	else if(LanguageValue == "DE")
	lang = 5;
	else if(LanguageValue == "BR")
	lang = 6;
	else if(LanguageValue == "CZ")
	lang = 7;
	else if(LanguageValue == "DA")
	lang = 8;
	else if(LanguageValue == "FI")
	lang = 9;
	else if(LanguageValue == "MS")
	lang = 10;
	else if(LanguageValue == "NO")
	lang = 11;
	else if(LanguageValue == "PL")
	lang = 12;
	else if(LanguageValue == "SV")
	lang = 13;
	else if(LanguageValue == "TH")
	lang = 14;
	else if(LanguageValue == "TR")
	lang = 15;
	if (lang != multi_INT){
		$j.get(url,function(data){});
		showLoading(3);
		setTimeout('window.location.reload(true);',3500);
	}
	/*if(LanguageValue == "EN"){
		multi_INT = 0;
		document.getElementById("languageCss").href = "multiLanguageCss/english.css";
		document.getElementById("languageCss_1").href = "multiLanguageCss/english_1.css";
		}
	else if(LanguageValue == "TC"){
		multi_INT = 1;
		document.getElementById("languageCss").href = "multiLanguageCss/taiwan.css";
		document.getElementById("languageCss_1").href = "multiLanguageCss/taiwan_1.css";
		}
	else if(LanguageValue == "SC"){
		multi_INT = 2;
		document.getElementById("languageCss").href = "multiLanguageCss/chinese.css";
		document.getElementById("languageCss_1").href = "multiLanguageCss/chinese_1.css";
		}
	else if(LanguageValue == "RU"){
		multi_INT = 3;
		document.getElementById("languageCss").href = "multiLanguageCss/russian.css";
		document.getElementById("languageCss_1").href = "multiLanguageCss/russian_1.css";
		}
	else if(LanguageValue == "FR"){
		multi_INT = 4;
		document.getElementById("languageCss").href = "multiLanguageCss/french.css";
		document.getElementById("languageCss_1").href = "multiLanguageCss/french_1.css";
		}
	else if(LanguageValue == "GE"){
		multi_INT = 5;
		document.getElementById("languageCss").href = "multiLanguageCss/german.css";
		document.getElementById("languageCss_1").href = "multiLanguageCss/german_1.css";
		}
	else if(LanguageValue == "BR"){
		multi_INT = 6;
		document.getElementById("languageCss").href = "multiLanguageCss/brazil.css";
		document.getElementById("languageCss_1").href = "multiLanguageCss/brazil_1.css";
		}
	else if(LanguageValue == "CZ"){
		multi_INT = 7;
		document.getElementById("languageCss").href = "multiLanguageCss/czech.css";
		document.getElementById("languageCss_1").href = "multiLanguageCss/czech_1.css";
		}
	else if(LanguageValue == "DA"){
		multi_INT = 8;
		document.getElementById("languageCss").href = "multiLanguageCss/danish.css";
		document.getElementById("languageCss_1").href = "multiLanguageCss/danish_1.css";
		}
	else if(LanguageValue == "FI"){
		multi_INT = 9;
		document.getElementById("languageCss").href = "multiLanguageCss/finnish.css";
		document.getElementById("languageCss_1").href = "multiLanguageCss/finnish_1.css";
		}
	else if(LanguageValue == "MA"){
		multi_INT = 10;
		document.getElementById("languageCss").href = "multiLanguageCss/malay.css";
		document.getElementById("languageCss_1").href = "multiLanguageCss/malay_1.css";
		}
	else if(LanguageValue == "NO"){
		multi_INT = 11;
		document.getElementById("languageCss").href = "multiLanguageCss/norwegian.css";
		document.getElementById("languageCss_1").href = "multiLanguageCss/norwegian_1.css";
		}
	else if(LanguageValue == "PO"){
		multi_INT = 12;
		document.getElementById("languageCss").href = "multiLanguageCss/polish.css";
		document.getElementById("languageCss_1").href = "multiLanguageCss/polish_1.css";
		}
	else if(LanguageValue == "SW"){
		multi_INT = 13;
		document.getElementById("languageCss").href = "multiLanguageCss/swedish.css";
		document.getElementById("languageCss_1").href = "multiLanguageCss/swedish_1.css";
		}
	else if(LanguageValue == "TH"){
		multi_INT = 14;
		document.getElementById("languageCss").href = "multiLanguageCss/thai.css";
		document.getElementById("languageCss_1").href = "multiLanguageCss/thai_1.css";
		}
	else if(LanguageValue == "TU"){
		multi_INT = 15;
		document.getElementById("languageCss").href = "multiLanguageCss/turkish.css";
		document.getElementById("languageCss_1").href = "multiLanguageCss/turkish_1.css";
		}*/
	}
/*function submit_language(){
	if($("select_lang").value != $("preferred_lang").value){
		showLoading();
		
		with(document.titleForm){
			action = "/start_apply.htm";
			
			if(location.pathname == "/")
				current_page.value = "/index.asp";
			else
				current_page.value = location.pathname;
			
			preferred_lang.value = $("select_lang").value;
			flag.value = "set_language";
			
			submit();
		}
	}
	else
		alert("No change LANGUAGE!");
}*/

/*function change_language(){
	if($("select_lang").value != $("preferred_lang").value)
		$("change_lang_btn").disabled = false;
	else
		$("change_lang_btn").disabled = true;
} delete by alan*/

function logout(){
	if(confirm('Are you sure to logout?')){
		setTimeout('location = "Logout.asp";', 1);
	}
}

function reboot(){
	if(confirm("<#Main_content_Login_Item7#>")){
 		 if(window.frames["statusframe"] && window.frames["statusframe"].stopFlag == 0){
  		 window.frames["statusframe"].stopFlag = 1;
  		 //alert(window.frames["statusframe"].stopFlag);
 		 }
		showLoading(60);
		setTimeout("location.href = '/index.asp';", 60000);
		$("hidden_frame").src = "Reboot.asp";
	}
}

function kb_to_gb(kilobytes){
	if(typeof(kilobytes) == "string" && kilobytes.length == 0)
		return 0;
	
	return (kilobytes*1024)/(1024*1024*1024);
}

function simpleNum(num){
	if(typeof(num) == "string" && num.length == 0)
		return 0;
	
	return parseInt(kb_to_gb(num)*1000)/1000;
}

function simpleNum2(num){
	if(typeof(num) == "string" && num.length == 0)
		return 0;
	
	return parseInt(num*1000)/1000;
}

function simpleNum3(num){
	if(typeof(num) == "string" && num.length == 0)
		return 0;
	
	return parseInt(num)/1024;
}

function $(){
	var elements = new Array();
	
	for(var i = 0; i < arguments.length; ++i){
		var element = arguments[i];
	if(typeof element == 'string')
		element = document.getElementById(element);
		
		if(arguments.length == 1)
			return element;
		
		elements.push(element);
	}
	
	return elements;
}

function getElementsByName_iefix(tag, name){
	var tagObjs = document.getElementsByTagName(tag);
	var objsName;
	var targetObjs = new Array();
	var targetObjs_length;
	
	if(!(typeof(name) == "string" && name.length > 0))
		return [];
	
	for(var i = 0, targetObjs_length = 0; i < tagObjs.length; ++i){
		objsName = tagObjs[i].getAttribute("name");
		
		if(objsName && objsName.indexOf(name) == 0){
			targetObjs[targetObjs_length] = tagObjs[i];
			++targetObjs_length;
		}
	}
	
	return targetObjs;
}

function getElementsByClassName_iefix(tag, name){
	var tagObjs = document.getElementsByTagName(tag);
	var objsName;
	var targetObjs = new Array();
	var targetObjs_length;
	
	if(!(typeof(name) == "string" && name.length > 0))
		return [];
	
	for(var i = 0, targetObjs_length = 0; i < tagObjs.length; ++i){
		if(navigator.appName == 'Netscape')
			objsName = tagObjs[i].getAttribute("class");
		else
			objsName = tagObjs[i].getAttribute("className");
		
		if(objsName == name){
			targetObjs[targetObjs_length] = tagObjs[i];
			++targetObjs_length;
		}
	}
	
	return targetObjs;
}

function showtext(obj, str){
	if(obj)
		obj.innerHTML = str;//*/
}

function showhtmlspace(ori_str){
	var str = "", head, tail_num;
	
	head = ori_str;
	while((tail_num = head.indexOf(" ")) >= 0){
		str += head.substring(0, tail_num);
		str += "&nbsp;";
		
		head = head.substr(tail_num+1, head.length-(tail_num+1));
	}
	str += head;
	
	return str;
}

function showhtmland(ori_str){
	var str = "", head, tail_num;
	
	head = ori_str;
	while((tail_num = head.indexOf("&")) >= 0){
		str += head.substring(0, tail_num);
		str += "&amp;";
		
		head = head.substr(tail_num+1, head.length-(tail_num+1));
	}
	str += head;
	
	return str;
}

// A dummy function which just returns its argument. This was needed for localization purpose
function translate(str){
	return str;
}

function trim(val){
	val = val+'';
	for (var startIndex=0;startIndex<val.length && val.substring(startIndex,startIndex+1) == ' ';startIndex++);
	for (var endIndex=val.length-1; endIndex>startIndex && val.substring(endIndex,endIndex+1) == ' ';endIndex--);
	return val.substring(startIndex,endIndex+1);
}

function IEKey(){
	return event.keyCode;
}

function NSKey(){
	return 0;
}

function is_string(o){
	if(!nav)
		keyPressed = IEKey();
	else
		keyPressed = NSKey();
	
	if(keyPressed == 0)
		return true;
	else if(keyPressed >= 0 && keyPressed <= 126)
		return true;
	
	alert('<#JS_validchar#>');
	return false;
}

function validate_string(string_obj, flag){
	if(string_obj.value.charAt(0) == '"'){
		if(flag != "noalert")
			alert('<#JS_validstr1#> ["]');
		
		string_obj.value = "";
		string_obj.focus();
		
		return false;
	}
	else{
		invalid_char = "";
		
		for(var i = 0; i < string_obj.value.length; ++i){
			if(string_obj.value.charAt(i) < ' ' || string_obj.value.charAt(i) > '~'){
				invalid_char = invalid_char+string_obj.value.charAt(i);
			}
		}
		
		if(invalid_char != ""){
			if(flag != "noalert")
				alert("<#JS_validstr2#> '"+invalid_char+"' !");
			string_obj.value = "";
			string_obj.focus();
			
			return false;
		}
	}
	
	return true;
}

function validate_hex(obj){
	var obj_value = obj.value
	var re = new RegExp("[^a-fA-F0-9]+","gi");
	
	if(re.test(obj_value))
		return false;
	else
		return true;
}

function validate_psk(psk_obj){
	var psk_length = psk_obj.value.length;
	
	if(psk_length < 8){
		alert("<#JS_passzero#>");
		psk_obj.value = "00000000";
		psk_obj.focus();
		psk_obj.select();
		
		return false;
	}
	
	if(psk_length > 64){
		alert("<#JS_PSK64Hex#>");
		psk_obj.value = psk_obj.value.substring(0, 64);
		psk_obj.focus();
		psk_obj.select();
		
		return false;
	}
	
	if(psk_length >= 8 && psk_length <= 63 && !validate_string(psk_obj)){
		alert("<#JS_PSK64Hex#>");
		psk_obj.value = "00000000";
		psk_obj.focus();
		psk_obj.select();
		
		return false;
	}
	
	if(psk_length == 64 && !validate_hex(psk_obj)){
		alert("<#JS_PSK64Hex#>");
		psk_obj.value = "00000000";
		psk_obj.focus();
		psk_obj.select();
		
		return false;
	}
	
	return true;
}

function validate_wlkey(key_obj){

	var wep_type = document.form.wl_wep_x.value;
	var iscurrect = true;
	var str = "<#JS_wepkey#>";

	if(wep_type == "0")
		iscurrect = true;	// do nothing
	else if(wep_type == "1"){
		if(key_obj.value.length == 5 && validate_string(key_obj)){
			document.form.wl_key_type.value = 1; /*Lock Add 11.25 for ralink platform*/
			iscurrect = true;
		}
		else if(key_obj.value.length == 10 && validate_hex(key_obj)){
			document.form.wl_key_type.value = 0; /*Lock Add 11.25 for ralink platform*/
			iscurrect = true;
		}
		else{
			str += "(<#WLANConfig11b_WEPKey_itemtype1#>)";
			
			iscurrect = false;
		}
	}
	else if(wep_type == "2"){
		if(key_obj.value.length == 13 && validate_string(key_obj)){
			document.form.wl_key_type.value = 1; /*Lock Add 11.25 for ralink platform*/
			iscurrect = true;
		}
		else if(key_obj.value.length == 26 && validate_hex(key_obj)){
			document.form.wl_key_type.value = 0; /*Lock Add 11.25 for ralink platform*/
			iscurrect = true;
		}
		else{
			str += "(<#WLANConfig11b_WEPKey_itemtype2#>)";
			
			iscurrect = false;
		}
	}
	else{
		alert("System error!");
		iscurrect = false;
	}
	
	if(iscurrect == false){
		alert(str);
		
		key_obj.focus();
		key_obj.select();
	}
	
	return iscurrect;
}

function checkDuplicateName(newname, targetArray){
	var existing_string = targetArray.join(',');
	existing_string = ","+existing_string+",";
	var newstr = ","+trim(newname)+",";
	
	var re = new RegExp(newstr, "gi");
	var matchArray = existing_string.match(re);
	
	if(matchArray != null)
		return true;
	else
		return false;
}

function alert_error_msg(error_msg){
	alert(error_msg);
	refreshpage();
}

function refreshpage(seconds){
	if(typeof(seconds) == "number")
		setTimeout("refreshpage()", seconds*1000);
	else
		location.href = location.href;
}

function hideLinkTag(){
	if(document.all){
		var tagObjs = document.all.tags("a");
		
		for(var i = 0; i < tagObjs.length; ++i)
			tagObjs(i).outerHTML = tagObjs(i).outerHTML.replace(">"," hidefocus=true>");
	}
}

function buttonOver(o){	//Lockchou 1206 modified
	o.style.color = "#FFFFFF";
	o.style.background = "url(/images/bgaibutton.gif) #ACCCE1";
	o.style.cursor = "hand";
}

function buttonOut(o){	//Lockchou 1206 modified
	o.style.color = "#000000";
	o.style.background = "url(/images/bgaibutton0.gif) #ACCCE1";
}

function flash_button(){
	if(navigator.appName.indexOf("Microsoft") < 0)
		return;
	
	var btnObj = getElementsByClassName_iefix("input", "button");
	
	for(var i = 0; i < btnObj.length; ++i){
		btnObj[i].onmouseover = function(){
				buttonOver(this);
			};
		
		btnObj[i].onmouseout = function(){
				buttonOut(this);
			};
	}
}

function no_flash_button(){
	if(navigator.appName.indexOf("Microsoft") < 0)
		return;
	
	var btnObj = getElementsByClassName_iefix("input", "button");
	
	for(var i = 0; i < btnObj.length; ++i){
		btnObj[i].onmouseover = "";
		
		btnObj[i].onmouseout = "";
	}
}

function gotoprev(formObj){
	var prev_page = formObj.prev_page.value;
	
	if(prev_page == "/")
		prev_page = "/index.asp";
	
	if(prev_page.indexOf('QIS') < 0){
		formObj.action = prev_page;
		formObj.target = "_parent";
		formObj.submit();
	}
	else{
		formObj.action = prev_page;
		formObj.target = "";
		formObj.submit();
	}
}

function add_option(selectObj, str, value, selected){
	var tail = selectObj.options.length;
	
	if(typeof(str) != "undefined")
		selectObj.options[tail] = new Option(str);
	else
		selectObj.options[tail] = new Option();
	
	if(typeof(value) != "undefined")
		selectObj.options[tail].value = value;
	else
		selectObj.options[tail].value = "";
	
	if(selected == 1)
		selectObj.options[tail].selected = selected;
}

/*function free_options(selectObj){
	if(selectObj == null)
		return;
	
	for(var i = 0; i < selectObj.options.length; ++i){
		selectObj.options[0].value = null;
		selectObj.options[0] = null;
	}
}*/

function free_options(selectObj){
	if(selectObj == null)
		return;
	
	for(var i = selectObj.options.length-1; i >= 0; --i){
  		selectObj.options[i].value = null;
		selectObj.options[i] = null;
	}
}

function blocking(obj_id, show){
	var state = show?'block':'none';
	
	if(document.getElementById)
		$(obj_id).style.display = state;
	else if(document.layers)
		document.layers[obj_id].display = state;
	else if(document.all)
		document.all[obj_id].style.display = state;
}

function inputCtrl(obj, flag){
	if(flag == 0){
		obj.disabled = true;
		if(obj.type != "select-one")
			obj.style.backgroundColor = "#CCCCCC";
		if(obj.type == "radio" || obj.type == "checkbox")
			obj.style.backgroundColor = "#C0DAE4";
	}
	else{
		obj.disabled = false;
		if(obj.type != "select-one")	
			obj.style.backgroundColor = "#FFF";
		if(obj.type == "radio" || obj.type == "checkbox")
			obj.style.backgroundColor = "#C0DAE4";
	}
}

function setText(a, b){
	x = document.getElementById(a);
	if(x)
		x.innerHTML = b.options[b.selectedIndex].innerHTML;
}

//Update current status via AJAX
var http_request_status = false;

function makeRequest_status(url) {
	http_request_status = new XMLHttpRequest();
	if (http_request_status && http_request_status.overrideMimeType)
		http_request_status.overrideMimeType('text/xml');
	else
		return false;

	http_request_status.onreadystatechange = alertContents_status;
	http_request_status.open('GET', url, true);
	http_request_status.send(null);
}

var xmlDoc_ie;

function makeRequest_status_ie(file)
{
	xmlDoc_ie = new ActiveXObject("Microsoft.XMLDOM");
	xmlDoc_ie.async = false;
	if (xmlDoc_ie.readyState==4)
	{
		xmlDoc_ie.load(file);
		//refresh_info_status(xmlDoc_ie);
	}
}

function alertContents_status()
{
	if (http_request_status != null && http_request_status.readyState != null && http_request_status.readyState == 4)
	{
		if (http_request_status.status != null && http_request_status.status == 200)
		{
			var xmldoc_mz = http_request_status.responseXML;
			//refresh_info_status(xmldoc_mz);
		}
	}
}

function updateStatus_AJAX()
{
	//alert("AJAX XMLHttp Request sent!");
	//var ie = window.ActiveXObject;

	//if (ie)
	//	makeRequest_status_ie('/ajax_status.asp');
	//else
	//	makeRequest_status('/ajax_status.asp');
}

/*function refresh_info_status(xmldoc)
{
	var devicemapXML = xmldoc.getElementsByTagName("devicemap");
	var wanStatus = devicemapXML[0].getElementsByTagName("wan");
	
	var link_status = wanStatus[0].firstChild.nodeValue;
	var usb_path1 = wanStatus[1].firstChild.nodeValue;
	var usb_path2 = wanStatus[2].firstChild.nodeValue;
	var print_status = "off";
	
	if(link_status != "link_internet=1"){
		$("conncet_status").className = "conncetstatusoff";
			if(location.pathname == "/" || location.pathname == "/index.asp")
		   $("NM_connect_status").innerHTML = "<#Disconnected#>";		 	
	}
	else{
		$("conncet_status").className = "conncetstatuson";
			if(location.pathname == "/" || location.pathname == "/index.asp")
				$("NM_connect_status").innerHTML = "<#Connected#>";
	}
	
	if(usb_path1 == "usb_path1=" && usb_path2 == "usb_path2=")
		$("usb_status").className = "usbstatusoff";
	else 
		$("usb_status").className = "usbstatuson";

	if(print_status == "on")
		$("printer_status").className = "printstatuson";
	else 
		$("printer_status").className = "printstatusoff";
	
	//if (location.pathname == "/Advanced_FirmwareUpgrade_Content.asp") return;
	if (window.frames["statusframe"] && window.frames["statusframe"].stopFlag == 1) return;
	
	setTimeout("updateStatus_AJAX();", 3000);
}*/
