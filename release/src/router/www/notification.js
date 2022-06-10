var noti_auth_mode_2g = "";
var noti_auth_mode_5g = "";
var noti_auth_mode_5g2 = "";

if(isSwMode('rt') || isSwMode('ap') || '<% nvram_get("wlc_band"); %>' == ''){
	noti_auth_mode_2g = '<% nvram_get("wl0_auth_mode_x"); %>';
	noti_auth_mode_5g = '<% nvram_get("wl1_auth_mode_x"); %>';
	noti_auth_mode_5g2 = '<% nvram_get("wl2_auth_mode_x"); %>';
}
else if(isSwMode('mb')){
	noti_auth_mode_2g = '';
	noti_auth_mode_5g = '';
	noti_auth_mode_5g2 = "";
}
else{
	noti_auth_mode_2g = ('<% nvram_get("wlc_band"); %>' == 0) ? '<% nvram_get("wl0.1_auth_mode_x"); %>' : '<% nvram_get("wl0_auth_mode_x"); %>';
	noti_auth_mode_5g = ('<% nvram_get("wlc_band"); %>' == 1) ? '<% nvram_get("wl1.1_auth_mode_x"); %>' : '<% nvram_get("wl1_auth_mode_x"); %>';

	if(concurrep_support){
		noti_auth_mode_2g = (wlc_express == 1) ? '' : '<% nvram_get("wl0.1_auth_mode_x"); %>';
		noti_auth_mode_5g = (wlc_express == 2) ? '' : '<% nvram_get("wl1.1_auth_mode_x"); %>';
	}
}

var webs_state_info = '<% nvram_get("webs_state_info_am"); %>';
var webs_state_info_beta = '<% nvram_get("webs_state_info_beta"); %>';
var webs_state_flag = '<% nvram_get("webs_state_flag"); %>';

var st_ftp_mode = '<% nvram_get("st_ftp_mode"); %>';
var st_ftp_force_mode = '<% nvram_get("st_ftp_force_mode"); %>';
var st_samba_mode = '<% nvram_get("st_samba_mode"); %>';
var st_samba_force_mode = '<% nvram_get("st_samba_force_mode"); %>';
var enable_samba = '<% nvram_get("enable_samba"); %>';
var enable_ftp = '<% nvram_get("enable_ftp"); %>';
var autodet_state = '<% nvram_get("autodet_state"); %>';
var autodet_auxstate = '<% nvram_get("autodet_auxstate"); %>';
var wan_proto = '<% nvram_get("wan0_proto"); %>';
// MODELDEP : DSL-AC68U | DSL-AX82U Only for now
if(dsl_support){
	var dla_modified = (vdsl_support == false) ? "0" :'<% nvram_get("dsltmp_dla_modified"); %>';	
	var dsl_loss_sync = "";
	if(dla_modified == "1")
		dsl_loss_sync = "1";
	else
		dsl_loss_sync = (vdsl_support == false) ? "0" :'<% nvram_get("dsltmp_syncloss"); %>';	
	var experience_fb = (dsl_support == false) ? "2" : '<% nvram_get("fb_experience"); %>';
}
else{
	var dla_modified = "0";
	var dsl_loss_sync = "0";
	var experience_fb = "2";
}
if(dsl_support){
	var noti_notif_Flag = '<% nvram_get("webs_notif_flag"); %>';
	var notif_hint_index = '<% nvram_get("webs_notif_index"); %>';
	var notif_hint_info = '<% nvram_get("webs_notif_info"); %>';    
	var notif_hint_infomation = notif_hint_info;
	if(notif_hint_infomation.charAt(0) == "+")      //remove start with '++'
		notif_hint_infomation = notif_hint_infomation.substring(2, notif_hint_infomation.length);
	var notif_msg = "";             
	var notif_hint_array = notif_hint_infomation.split("++");
	if(notif_hint_array[0] != ""){ 
		notif_msg = "<ol style=\"margin-left:-20px;*margin-left:20px;\">";
		for(var i=0; i<notif_hint_array.length; i++){
			if(i==0){
				notif_msg += "<li>"+notif_hint_array[i];
			}
			else{
				notif_msg += "<div style=\"width:240px;margin: 2px 0 2px -20px;*margin-left:-20px;\" class=\"splitLine\"></div><li>" + notif_hint_array[i];
			}
		}
		notif_msg += "</ol>";
    	}	
}

var aimesh_system_new_fw_flag = false;
if(amesh_support && ameshRouter_support) {
	var get_cfg_clientlist = [<% get_cfg_clientlist(); %>][0];
	for (var idx in get_cfg_clientlist) {
		if(get_cfg_clientlist.hasOwnProperty(idx)) {
			if(get_cfg_clientlist[idx].online == "1") {
				if(get_cfg_clientlist[idx].newfwver != "") {
					aimesh_system_new_fw_flag = true;
					break;
				}
			}
		}
	}
}

var get_s46_hgw_case = '<% nvram_get("s46_hgw_case"); %>';	//topology 2,3,6
var s46_ports_check_flag = (get_s46_hgw_case=='3' || get_s46_hgw_case=='6')? true:false;	//true for topology 3||6
var check_ipv6_s46_ports_hook = (Softwire46_support && wan_proto=="v6plus")? '<%chk_s46_port_range();%>':'0';
// '{"pf":"1","open_nat":"0","pt":"1","https":"0","ssh":"0","openvpn":"0","ftp":"1","ipsec":"1"}';
var check_ipv6_s46_ports = "0";
if(check_ipv6_s46_ports_hook != "" && check_ipv6_s46_ports_hook != "0"){
	check_ipv6_s46_ports = JSON.parse(check_ipv6_s46_ports_hook);
}

var get_ipv6_s46_ports = (Softwire46_support && wan_proto=="v6plus")? '<%nvram_get("ipv6_s46_ports");%>':'0';
var array_ipv6_s46_ports = new Array("");
if(get_ipv6_s46_ports!="0" && get_ipv6_s46_ports!=""){
	array_ipv6_s46_ports = get_ipv6_s46_ports.split(" ");
}

var ipsec_server_enable = '<% nvram_get("ipsec_server_enable"); %>'; //higher priority
var ipsec_ig_enable = '<% nvram_get("ipsec_ig_enable"); %>';

function pop_s46_ports(p, flag){
	var isMobile = function() {
		
		if(	navigator.userAgent.match(/iPhone/i)	|| 
			navigator.userAgent.match(/iPod/i)		||
			navigator.userAgent.match(/iPad/i)		||
			(navigator.userAgent.match(/Android/i) && (navigator.userAgent.match(/Mobile/i) || navigator.userAgent.match(/Tablet/i))) ||
			(navigator.userAgent.match(/Opera/i) && (navigator.userAgent.match(/Mobi/i) || navigator.userAgent.match(/Mini/i))) ||	// Opera mobile or Opera Mini
			navigator.userAgent.match(/IEMobile/i)	||	// IE Mobile
			navigator.userAgent.match(/BlackBerry/i)	//BlackBerry
		 ) {
			return true;
		}
		else {
			return false;
		}
	};

	var left_tuned=p.left;
	if(isMobile()){
		
		if(flag=="game"){
			var top_tuned=p.top+1165;
		}
		else if(flag=="pf"){
			var top_tuned=p.top-170;
		}
		else if(flag=="table"){
			var top_tuned=p.top-200;
		}
		else{
			var top_tuned=p.top-200;
		}
	}
	else{
		
		if(flag=="game"){
			var top_tuned=p.top+575;
		}
		else if(flag=="pf"){
			var top_tuned=p.top;
		}
		else if(flag=="table"){
			var top_tuned=p.top;
		}
		else{
			var top_tuned=p.top-24;
		}
	}
	var margin_set=top_tuned +"px 0px 0px "+left_tuned+"px";
	if(document.getElementById("s46_ports_content") != null) {
		$("#s46_ports_content").remove();
	}
	var divObj = document.createElement("div");
	divObj.setAttribute("id","s46_ports_content");
	divObj.className = "s46_ports";
	divObj.style.zIndex = "300";
	divObj.style.margin = margin_set;
	divObj.innerHTML = "<div style='float:right;'><img src='/images/button-close.gif' style='width:30px;cursor:pointer' onclick='close_s46_ports();'></div>Since you are currently using v6plus connection, please make sure your external port settings are within the following port range:<br><br>"+get_ipv6_s46_ports+"<br>";
	document.body.prepend(divObj);
	if(flag=="pf")
		adjust_panel_block_top("s46_ports_content", -70);
	else if(flag=="table")
		adjust_panel_block_top("s46_ports_content", 30);
	
	cal_panel_s46_ports("s46_ports_content", 0.045);
	$("#s46_ports_content").fadeIn();
}
function close_s46_ports(){
	$("#s46_ports_content").fadeOut();
}

function pop_s46_ports_conflict(){

	var conflict_links = gen_conflict_links();
	var confilct_content = "<div style='float:right;'><img src='/images/button-close.gif' style='width:30px;cursor:pointer;margin:-28px -28px 0 0;' onclick='close_s46_ports_conflict();'></div>The following port related settings may not work properly since port number mismatch in current v6plus usable port range. Please set up a usable port listed in <a target='_self' style='text-decoration:underline;' href='Main_IPV6Status_Content.asp'>IPv6 Log</a>.<br><br>"+conflict_links;
	
	var left_tuned=0;
	var top_tuned=130;
	var margin_set=top_tuned +"px 0px 0px "+left_tuned+"px";

	if(document.getElementById("s46_ports_confilct") != null) {
		$("#s46_ports_confilct").remove();
	}
	var divObj = document.createElement("div");
	divObj.setAttribute("id","s46_ports_confilct");
	divObj.className = "noti_s46_ports";
	divObj.style.zIndex = "300";
	divObj.style.margin = margin_set;
	divObj.innerHTML = confilct_content;
	document.body.prepend(divObj);
	
	cal_panel_s46_ports("s46_ports_confilct", 0.045);
	$("#s46_ports_confilct").fadeIn();
}
function close_s46_ports_conflict(){
	$("#s46_ports_confilct").fadeOut();
}
function cal_panel_s46_ports(obj, multiple) {
	var isMobile = function() {		
		if(	navigator.userAgent.match(/iPhone/i)	|| 
			navigator.userAgent.match(/iPod/i)		||
			navigator.userAgent.match(/iPad/i)		||
			(navigator.userAgent.match(/Android/i) && (navigator.userAgent.match(/Mobile/i) || navigator.userAgent.match(/Tablet/i))) ||
			(navigator.userAgent.match(/Opera/i) && (navigator.userAgent.match(/Mobi/i) || navigator.userAgent.match(/Mini/i))) ||	// Opera mobile or Opera Mini
			navigator.userAgent.match(/IEMobile/i)	||	// IE Mobile
			navigator.userAgent.match(/BlackBerry/i)	//BlackBerry
		 ) {
			return true;
		}
		else {
			return false;
		}
	};
	var blockmarginLeft;
	if (window.innerWidth) {
		winWidth = window.innerWidth;
	}
	else if ((document.body) && (document.body.clientWidth)) {
		winWidth = document.body.clientWidth;
	}

	if (document.documentElement  && document.documentElement.clientHeight && document.documentElement.clientWidth) {
		winWidth = document.documentElement.clientWidth;
	}

	if(winWidth > 1050) {
		winPadding = (winWidth - 1050) / 2;
		winWidth = 1105;
		blockmarginLeft = (winWidth * multiple) + winPadding;
	}
	else if(winWidth <= 1050) {
		if(isMobile()) {
			if(document.body.scrollLeft < 50) {
				blockmarginLeft= (winWidth) * multiple + document.body.scrollLeft;
			}
			else if(document.body.scrollLeft >320) {
				blockmarginLeft = 320;
			}
			else {
				blockmarginLeft = document.body.scrollLeft;
			}	
		}
		else {
			blockmarginLeft = (winWidth) * multiple + document.body.scrollLeft;	
		}
	}

	document.getElementById(obj).style.marginLeft = blockmarginLeft + "px";
}

function exist_v6plus_conflict(){	//0 for no issue
	var count=0;
	$.each(check_ipv6_s46_ports, function (key, data) {
	if(data=='1'){count += 1;}
	})
	return count;
}

function gen_conflict_links(){
	var items="";
	$.each(check_ipv6_s46_ports, function (key, data) {
    	if(data=='1'){
    		switch(key) {
                case "pf" :
                        items += "<li><a href='/Advanced_VirtualServer_Content.asp' style='text-decoration:underline;cursor:pointer;'><#menu5_3_4#></a></li>";
                        break;
                case "open_nat" :
                        items += "<li><a href='/GameProfile.asp' target='_blank' style='text-decoration:underline;cursor:pointer;'>OPEN NAT</a></li>";
                        break;
                case "pt" :
                        items += "<li><a href='/Advanced_PortTrigger_Content.asp' target='_blank' style='text-decoration:underline;cursor:pointer;'><#menu5_3_3#></a></li>";
                        break;
                case "https" :
                        items += "<li><a href='/Advanced_System_Content.asp' target='_blank' style='text-decoration:underline;cursor:pointer;'><#FirewallConfig_x_WanWebPort_itemname#></a></li>";
                        break;
                case "ssh" :
                        items += "<li><a href='/Advanced_System_Content.asp' target='_blank' style='text-decoration:underline;cursor:pointer;'><#Port_SSH#></a></li>";
                        break;
                case "openvpn" :
                        items += "<li><a href='/Advanced_VPN_OpenVPN.asp' target='_blank' style='text-decoration:underline;cursor:pointer;'>OpenVPN</a></li>";
                        break;
                case "ftp" :
                        items += "<li><a href='/Advanced_NATPassThrough_Content.asp' target='_blank' style='text-decoration:underline;cursor:pointer;'><#NAT_passthrough_itemname#> - <#FTP_ALG_port#></a></li>";
                        break;
		case "ipsec" :
			if(ipsec_server_enable=='1'){
				items += "<li><a href='/Advanced_VPN_IPSec.asp' target='_blank' style='text-decoration:underline;cursor:pointer;'>IP Sec</a></li>";
			}
			if(ipsec_ig_enable=='1'){
				items += "<li><a href='/Advanced_Instant_Guard.asp' target='_blank' style='text-decoration:underline;cursor:pointer;'><#Instant_Guard_title#></a></li>";
			}
			break;
                default :
                        break;
        	}

    	}
	})

	return items;
}
var notification = {
	stat: "off",
	flash: "off",
	flashTimer: 0,
	hoverText: "",
	clickText: "",
	array: [],
	desc: [],
	action_desc: [],
	upgrade: 0,
	wifi_2g: 0,
	wifi_5g: 0,
	wifi_5g2: 0,
	ftp: 0,
	samba: 0,
	loss_sync: 0,
	experience_FB: 0,
	notif_hint: 0,
	mobile_traffic: 0,
	send_debug_log: 0,
	low_nvram: 0,
	low_jffs: 0,
	clicking: 0,
	sim_record: 0,
	redirectftp:function(){location.href = 'Advanced_AiDisk_ftp.asp';},
	redirectsamba:function(){location.href = 'Advanced_AiDisk_samba.asp';},
	redirectFeedback:function(){location.href = 'Advanced_Feedback.asp';},
	redirectFeedbackInfo:function(){location.href = 'Feedback_Info.asp';},
	redirectRefresh:function(){
		var header_info = [<% get_header_info(); %>];
		location.href = header_info[0].current_page;
	},
	redirectHint:function(){location.reload();},
	clickCallBack: [],
	pppoe_tw: 0,
	ie_legacy: 0,
	s46_ports: 0,
	notiClick: function(){
		// stop flashing after the event is checked.
		cookie.set("notification_history", [notification.upgrade, notification.wifi_2g ,notification.wifi_5g ,notification.wifi_5g2 ,notification.ftp ,notification.samba ,notification.loss_sync ,notification.experience_FB ,notification.notif_hint, notification.mobile_traffic, 
											notification.send_debug_log, notification.sim_record, notification.pppoe_tw, notification.pppoe_tw_static, notification.ie_legacy, notification.low_nvram, notification.low_jffs, notification.s46_ports].join(), 1000);
		clearInterval(notification.flashTimer);
		document.getElementById("notification_status").className = "notification_on";
		if(notification.clicking == 0){
			var txt = '<div id="notiDiv"><table width="100%">'

			for(i=0; i<notification.array.length; i++){
				if(notification.array[i] != null && notification.array[i] != "off"){
					if(notification.array[2] != null && notification.array[2] != "off"){//filter 5G when 2G have notification
						if(i == 3 || i == 19)
							continue;
					}
					else if(i == 19 && notification.array[3] != null && notification.array[3] != "off"){
							continue;
					}

						txt += '<tr><td><table id="notiDiv_table3" width="100%" border="0" cellpadding="0" cellspacing="0" bgcolor="#232629">';
		  			txt += '<tr><td><table id="notiDiv_table5" border="0" cellpadding="5" cellspacing="0" bgcolor="#232629" width="100%">';
		  			txt += '<tr><td valign="TOP" width="100%"><div style="white-space:pre-wrap;font-size:13px;color:white;cursor:text">' + notification.desc[i] + '</div>';
		  			txt += '</td></tr>';

					if( i == 2 ){
		  				txt += '<tr><td width="100%"><div style="text-decoration:underline;text-align:right;color:#FFCC00;font-size:14px;cursor: pointer" onclick="' + notification.clickCallBack[i] + '">' + notification.action_desc[i] + '</div></td></tr>';
		  				if(band5g_support && notification.array[3] != null && notification.array[3] != "off"){
		  						txt += '<tr><td width="100%"><div style="text-decoration:underline;text-align:right;color:#FFCC00;font-size:14px;cursor: pointer" onclick="' + notification.clickCallBack[i+1] + '">' + notification.action_desc[i+1] + '</div></td></tr>';
		  				}else
			  				notification.array[3] = "off";

						if(band5g2_support && notification.array[19] != null && notification.array[19] != "off"){
								txt += '<tr><td width="100%"><div style="text-decoration:underline;text-align:right;color:#FFCC00;font-size:14px;cursor: pointer" onclick="' + notification.clickCallBack[19] + '">' + notification.action_desc[19] + '</div></td></tr>';
						}else
							notification.array[19] = "off";
					}
					else if( i == 3 ){
						txt += '<tr><td width="100%"><div style="text-decoration:underline;text-align:right;color:#FFCC00;font-size:14px;cursor: pointer" onclick="' + notification.clickCallBack[i] + '">' + notification.action_desc[i] + '</div></td></tr>';
						if(band5g2_support && notification.array[19] != null && notification.array[19] != "off"){
								txt += '<tr><td width="100%"><div style="text-decoration:underline;text-align:right;color:#FFCC00;font-size:14px;cursor: pointer" onclick="' + notification.clickCallBack[19] + '">' + notification.action_desc[19] + '</div></td></tr>';
						}else
							notification.array[19] = "off";
					}
					else if( i == 7){
						if(notification.array[18] != null){
							txt += '<tr><td width="100%"><div style="text-align:right;text-decoration:underline;color:#FFCC00;font-size:14px;"><span style="cursor: pointer" onclick="' + notification.clickCallBack[18] + '">' + notification.action_desc[18] + '</span>';
						}
						txt += '<span style="margin-left:10px;cursor: pointer" onclick="' + notification.clickCallBack[i] + '">' + notification.action_desc[i] + '</span></div></td></tr>';
						notification.array[18] = "off";
					}
					else if( i == 9){
						txt += '<tr><td width="100%"><div style="text-decoration:underline;text-align:right;color:#FFCC00;font-size:14px;cursor: pointer" onclick="' + notification.clickCallBack[i] + '">' + notification.action_desc[i] + '</div></td></tr>';
						if(notification.array[10] != null && notification.array[10] != "off"){
							txt += '<tr><td width="100%"><div style="text-decoration:underline;text-align:right;color:#FFCC00;font-size:14px;cursor: pointer" onclick="' + notification.clickCallBack[i+1] + '">' + notification.action_desc[i+1] + '</div></td></tr>';
						}
						notification.array[10] = "off";
					}
		  			else{
	  					txt += '<tr><td><table width="100%"><div style="text-decoration:underline;text-align:right;color:#FFCC00;font-size:14px;cursor: pointer" onclick="' + notification.clickCallBack[i] + '">' + notification.action_desc[i] + '</div></table></td></tr>';
		  			}

		  			txt += '</table></td></tr></table></td></tr>'
	  			}
			}
			txt += '</table></div>';

			document.getElementById("notification_desc").innerHTML = txt;
			notification.clicking = 1;
		}else{
			document.getElementById("notification_desc").innerHTML = "";
			notification.clicking = 0;
		}
	},
	
	updateNTDB_Status: function(){
		var data_usage = tx_bytes + rx_bytes;
		if(gobi_support && (usb_index != -1) && (notification.sim_state != "") && (modem_bytes_data_limit > 0) && (data_usage >= modem_bytes_data_limit)){
			notification.array[12] = 'noti_mobile_traffic';
			notification.mobile_traffic = 1;
			notification.desc[12] = "<#Mobile_limit_warning#>";
			notification.action_desc[12] = "<#ASUSGATE_act_change#>";
			notification.clickCallBack[12] = "setTrafficLimit();";
		}
		else{
			notification.array[12] = 'off';
			notification.mobile_traffic = 0;
		}

		if(gobi_support && (usb_index != -1) && (sim_state != "") && (modem_sim_order == -1)){
			notification.array[13] = 'noti_sim_record';
			notification.sim_record = 1;
			notification.desc[13] = "<#Mobile_record_limit_warning#>";
			notification.action_desc[13] = "Delete now";
			notification.clickCallBack[13] = "upated_sim_record();";
		}
		else{
			notification.array[13] = 'off';
			notification.sim_record = 0;
		}

		if(notification.stat != "on" && (notification.mobile_traffic || notification.sim_record)){
			notification.stat = "on";
			notification.flash = "on";
			notification.run_notice();
		}
		else if(notification.stat == "on" && !notification.mobile_traffic && !notification.sim_record && !notification.upgrade && !notification.wifi_2g &&
				!notification.wifi_5g && !notification.ftp && !notification.samba && !notification.loss_sync && !notification.experience_FB && !notification.notif_hint && !notification.mobile_traffic && 
				!notification.send_debug_log && !notification.pppoe_tw && !notification.pppoe_tw_static && !notification.ie_legacy && !notification.low_nvram && !notification.low_jffs && !notification.s46_ports){
			cookie.unset("notification_history");
			clearInterval(notification.flashTimer);
			document.getElementById("notification_status").className = "notification_off";
		}
	},

	run: function(){
	/*-- notification start --*/
		cookie.unset("notification_history");
		if(notice_pw_is_default == 1){	//case1
			notification.array[0] = 'noti_acpw';
			notification.acpw = 1;
			notification.desc[0] = '<#ASUSGATE_note1#>';
			notification.action_desc[0] = '<#ASUSGATE_act_change#>';
			notification.clickCallBack[0] = "location.href = 'Advanced_System_Content.asp?af=http_passwd2';";	
		}else
			notification.acpw = 0;

		if(amesh_support && ameshRouter_support) {
			if(aimesh_system_new_fw_flag || webs_state_flag == 1 || webs_state_flag == 2) {
				notification.array[1] = 'noti_upgrade';
				notification.upgrade = 1;
				notification.desc[1] = '<#ASUSGATE_note2#>';
				notification.action_desc[1] = '<#ASUSGATE_act_update#>';
				notification.clickCallBack[1] = "location.href = 'Advanced_FirmwareUpgrade_Content.asp?confirm_show=0';"
			}
			else
				notification.upgrade = 0;
		}
		else {
			if(webs_state_flag == 1 || webs_state_flag == 2){
				notification.array[1] = 'noti_upgrade';
				notification.upgrade = 1;
				notification.desc[1] = 'A new firmware version ('+webs_state_info.replace('_','.').replace('_0','')+') is now available.';
				if(!live_update_support || !HTTPS_support){
					notification.action_desc[1] = '<a id="link_to_downlodpage" target="_blank" href="'+get_helplink()+'" style="color:#FFCC00;"><#ASUSGATE_act_update#></a>';
					notification.clickCallBack[1] = "";
				}
				else{
					notification.action_desc[1] = '<#ASUSGATE_act_update#>';
					notification.clickCallBack[1] = "location.href = 'Advanced_FirmwareUpgrade_Content.asp?confirm_show=0'";
				}
			}else
				notification.upgrade = 0;
		}
		
		if(band2g_support && sw_mode != 4 && noti_auth_mode_2g == 'open'){ //case3-1
				notification.array[2] = 'noti_wifi_2g';
				notification.wifi_2g = 1;
				notification.desc[2] = '<#ASUSGATE_note3#>';
				notification.action_desc[2] = '<#ASUSGATE_act_change#> (2.4GHz)';
				notification.clickCallBack[2] = "change_wl_unit_status(0);";
		}else
			notification.wifi_2g = 0;
			
		if(band5g_support && sw_mode != 4 && noti_auth_mode_5g == 'open'){	//case3-2
				notification.array[3] = 'noti_wifi_5g';
				notification.wifi_5g = 1;
				notification.desc[3] = '<#ASUSGATE_note3#>';
				if(band5g2_support)
					notification.action_desc[3] = '<#ASUSGATE_act_change#> (5GHz-1)';
				else
					notification.action_desc[3] = '<#ASUSGATE_act_change#> (5GHz)';
				notification.clickCallBack[3] = "change_wl_unit_status(1);";
		}else
			notification.wifi_5g = 0;

		if(band5g2_support && sw_mode != 4 && noti_auth_mode_5g2 == 'open'){	//case3-3
				notification.array[19] = 'noti_wifi_5g2';
				notification.wifi_5g2 = 1;
				notification.desc[19] = '<#ASUSGATE_note3#>';
				notification.action_desc[19] = '<#ASUSGATE_act_change#> (5GHz-2)';
				notification.clickCallBack[19] = "change_wl_unit_status(2);";
		}else
			notification.wifi_5g2 = 0;
		
		if(usb_support && !noftp_support && enable_ftp == 1 && st_ftp_mode == 1 && st_ftp_force_mode == '' ){ //case4_1
				notification.array[4] = 'noti_ftp';
				notification.ftp = 1;
				notification.desc[4] = '<#ASUSGATE_note4_1#>';
				notification.action_desc[4] = '<#web_redirect_suggestion_etc#>';
				notification.clickCallBack[4] = "showLoading();setTimeout('document.noti_ftp.submit();', 1);setTimeout('notification.redirectftp()', 2000);";
		}else if(usb_support && !noftp_support && enable_ftp == 1 && st_ftp_mode != 2){	//case4
				notification.array[4] = 'noti_ftp';
				notification.ftp = 1;
				notification.desc[4] = '<#ASUSGATE_note4#>';
				notification.action_desc[4] = '<#ASUSGATE_act_change#>';
				notification.clickCallBack[4] = "showLoading();setTimeout('document.noti_ftp.submit();', 1);setTimeout('notification.redirectftp()', 2000);";
		}else
			notification.ftp = 0;

		if(usb_support && enable_samba == 1 && st_samba_mode == 1 && st_samba_force_mode == ''){ //case5_1
				notification.array[5] = 'noti_samba';
				notification.samba = 1;
				notification.desc[5] = '<#ASUSGATE_note5_1#>';
				notification.action_desc[5] = '<#web_redirect_suggestion_etc#>';	
				notification.clickCallBack[5] = "showLoading();setTimeout('document.noti_samba.submit();', 1);setTimeout('notification.redirectsamba()', 2000);";
		}else if(usb_support && enable_samba == 1 && st_samba_mode != 4){	//case5
				notification.array[5] = 'noti_samba';
				notification.samba = 1;
				notification.desc[5] = '<#ASUSGATE_note5#>';
				notification.action_desc[5] = '<#ASUSGATE_act_change#>';
				notification.clickCallBack[5] = "showLoading();setTimeout('document.noti_samba.submit();', 1);setTimeout('notification.redirectsamba()', 2000);";
		}else
			notification.samba = 0;

		//Higher priority: DLA intervened case dsltmp_dla_modified 0: default / 1:need to feedback / 2:Feedback submitted 
		//Lower priority: dsl_loss_sync  0: default / 1:need to feedback / 2:Feedback submitted
		// Only DSL-AC68U | DSL-AX82U for now
		if(frs_feedback_support && dsl_loss_sync == 1){         //case9(case10 act) + case6
			
			notification.loss_sync = 1;
			if(dla_modified == 1){
					notification.array[9] = 'noti_dla_modified';
					notification.desc[9] = Untranslated.ASUSGATE_note9;
					notification.action_desc[9] = Untranslated.ASUSGATE_DSL_setting;
					notification.clickCallBack[9] = "location.href = '/Advanced_ADSL_Content.asp?af=dslx_dla_enable';";
					notification.array[10] = 'noti_dla_modified_fb';
					notification.action_desc[10] = Untranslated.ASUSGATE_act_feedback;
					notification.clickCallBack[10] = "location.href = '/Advanced_Feedback.asp';";
			}
			else{
					notification.array[6] = 'noti_loss_sync';
					notification.desc[6] = Untranslated.ASUSGATE_note6;
					notification.action_desc[6] = Untranslated.ASUSGATE_act_feedback;
					notification.clickCallBack[6] = "location.href = '/Advanced_Feedback.asp';";
			}						
		}else
			notification.loss_sync = 0;
			
		//experiencing DSL issue experience_fb=0: notif, 1:no display again.
		if(frs_feedback_support && experience_fb == 0){		//case7
				notification.array[7] = 'noti_experience_FB';
				notification.array[18] = 'noti_experience_FB_cancel';
				notification.experience_FB = 1;
				notification.desc[7] = Untranslated.ASUSGATE_note7;
				notification.action_desc[7] = Untranslated.ASUSGATE_act_feedback;
				notification.clickCallBack[7] = "setTimeout('document.noti_experience_Feedback.submit();', 1);setTimeout('notification.redirectFeedback()', 1000);";
				notification.action_desc[18] = '<#CTL_Cancel#>';
				notification.clickCallBack[18] = "setTimeout('document.noti_experience_Feedback.submit();', 1);setTimeout('notification.redirectRefresh()', 1000);";
		}else
				notification.experience_FB = 0;

		//Notification hint-- null&0: default, 1:display info
		if(noti_notif_Flag == 1 && notif_msg != ""){               //case8
			notification.array[8] = 'noti_notif_hint';
			notification.notif_hint = 1;
			notification.desc[8] = notif_msg;
			notification.action_desc[8] = "<#CTL_ok#>";
			notification.clickCallBack[8] = "setTimeout('document.noti_notif_hint.submit();', 1);setTimeout('notification.redirectHint()', 100);"
		}else
			notification.notif_hint = 0;

		//DLA send debug log  -- 4: send by manual, else: nothing to show
		if(wan_diag_state == "4"){               //case11
			notification.array[11] = 'noti_send_debug_log';
			notification.send_debug_log = 1;
			notification.desc[11] = "-	The debug log of diagnostic DSL captured.";
			notification.action_desc[11] = "Send debug log now";
			notification.clickCallBack[11] = "setTimeout('notification.redirectFeedbackInfo()', 1000);";
		
		}else
			notification.send_debug_log = 0;

		var browser = getBrowser_info();
		if(browser.ie){
			if(browser.ie.indexOf('8') != "-1" || browser.ie.indexOf('9') != "-1" || browser.ie.indexOf('10') != "-1"){
				notification.ie_legacy = 1;
				notification.array[16] = 'noti_ie_legacy';
				notification.desc[16] = '<#IE_notice1#><#IE_notice2#><#IE_notice3#><#IE_notice4#>';
				notification.action_desc[16] = "";
				notification.clickCallBack[16] = "";
			}
		}

		if(Softwire46_support && wan_proto=="v6plus"){
			var exist_conflict = exist_v6plus_conflict();
			if(check_ipv6_s46_ports != "0" && exist_conflict>0){
				notification.s46_ports = 1;
				notification.array[20] = 'noti_s46_ports';
				notification.desc[20] = 'Port settings mismatch v6plus usable port range.';  /* Untranslated */
				notification.action_desc[20] = "Detail";
				notification.clickCallBack[20] = "setTimeout('pop_s46_ports_conflict()', 100);"
			}
		}

		/*if(is_TW_sku && wan_proto == "pppoe" && is_CHT_pppoe && !is_CHT_pppoe_static){
			notification.pppoe_tw_static = 1;
			notification.array[17] = 'noti_pppoe_tw_static';
			notification.desc[17] = '中華電信撥號連線。是否更改成固定IP撥號連線?<br>須與ISP確認是否有申請此服務';
			notification.action_desc[17] = '改成固定IP撥號連線(PPPoE)';
			notification.clickCallBack[17] = "change_cht_pppoe_static();";					
		}
		else if(is_TW_sku && autodet_state == 2 && autodet_auxstate == 6 && !is_CHT_pppoe_static){*/
		if(is_TW_sku && autodet_state == 2 && autodet_auxstate == 6 && wan_proto != "pppoe"){	
			notification.pppoe_tw = 1;
			notification.array[15] = 'noti_pppoe_tw';
			notification.desc[15] = '<#CHT_ppp_notice_1#>';
			notification.action_desc[15] = '<#CHT_ppp_notice_2#>';
			notification.clickCallBack[15] = "location.href = 'Advanced_WAN_Content.asp?af=wan_proto'";			
		}
		// Low NVRAM
		if((<% sysinfo("nvram.total"); %> - <% sysinfo("nvram.used"); %>) < 3000){
			notification.array[17] = 'noti_low_nvram';
			notification.low_nvram = 1;
			notification.desc[17] = "Your router is running low on free NVRAM, which might affect its stability.<br>Review long parameter lists (like DHCP reservations), or consider doing a factory default reset and reconfiguring.";
			notification.action_desc[17] = "Review System Information now";
			notification.clickCallBack[17] = "location.href = 'Tools_Sysinfo.asp';"
		}else
			notification.low_nvram = 0;

		// Low JFFS
		var jffs_free = "<% sysinfo("jffs.free"); %>";
		if(jffs_free < 3){
			notification.array[18] = 'noti_low_jffs';
			notification.low_jffs = 1;
			if(jffs_free == -1)
				notification.desc[18] = "Your router was unable to mount the JFFS partition, which will prevent it from working correctly.  Try rebooting it.";
			else
				notification.desc[18] = "Your router is running low on free JFFS storage, which might affect its stability.<br>Review the content of the /jffs directory on your router.";
			notification.action_desc[18] = "Review System Information now";
			notification.clickCallBack[18] = "location.href = 'Tools_Sysinfo.asp';"
		}else
			notification.low_jffs = 0;

		if( notification.acpw || notification.upgrade || notification.wifi_2g || notification.wifi_5g || notification.wifi_5g2 || notification.ftp || notification.samba || notification.loss_sync || notification.experience_FB || notification.notif_hint || notification.send_debug_log || notification.mobile_traffic || notification.sim_record || notification.pppoe_tw || notification.pppoe_tw_static || notification.ie_legacy || notification.low_nvram || notification.low_jffs || notification.s46_ports){
			notification.stat = "on";
			notification.flash = "on";
			notification.run_notice();
		}
		/*--notification end--*/		
	},

	run_notice: function(){
		var tarObj = document.getElementById("notification_status");
		var tarObj1 = document.getElementById("notification_status1");

		if(tarObj === null)	
			return false;		

		if(this.stat == "on"){
			tarObj1.onclick = this.notiClick;
			tarObj.className = "notification_on";
			tarObj1.className = "notification_on1";
		}

		if(this.flash == "on" && cookie.get("notification_history") != [notification.upgrade, notification.wifi_2g ,notification.wifi_5g ,notification.ftp ,notification.samba ,notification.loss_sync ,notification.experience_FB ,notification.notif_hint, notification.mobile_traffic, notification.send_debug_log, notification.sim_record, notification.pppoe_tw, notification.pppoe_tw_static, notification.ie_legacy, notification.low_nvram, notification.low_jffs, notification.s46_ports].join()){
			notification.flashTimer = setInterval(function(){
				tarObj.className = (tarObj.className == "notification_on") ? "notification_off" : "notification_on";
			}, 1000);
		}
	},

	reset: function(){
		this.stat = "off";
		this.flash = "off";
		this.flashTimer = 100;
		this.hoverText = "";
		this.clickText = "";
		this.upgrade = 0;
		this.wifi_2g = 0;
		this.wifi_5g = 0;
		this.wifi_5g2 = 0;
		this.ftp = 0;
		this.samba = 0;
		this.loss_sync = 0;
		this.experience_FB = 0;
		this.notif_hint = 0;
		this.mobile_traffic = 0;
		this.send_debug_log = 0;
		this.low_nvram = 0;
		this.low_jffs = 0;
		this.sim_record = 0;
		this.action_desc = [];
		this.desc = [];
		this.array = [];
		this.clickCallBack = [];
		this.run();
	}
}
