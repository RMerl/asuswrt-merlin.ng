﻿<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=Edge"/>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title><#Web_Title#> - <#menu5_7_4#></title>
<link rel="stylesheet" type="text/css" href="index_style.css">
<link rel="stylesheet" type="text/css" href="form_style.css">
<script type="text/javascript" src="/js/jquery.js"></script>
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script language="JavaScript" type="text/javascript" src="/help.js"></script>
<script language="JavaScript" type="text/javascript" src="/js/httpApi.js"></script>
<script language="JavaScript" type="text/javascript" src="/client_function.js"></script>
<style>
p{
	font-weight: bolder;
}

.contentM_details{
        position:absolute;
        -webkit-border-radius: 5px;
        -moz-border-radius: 5px;
        border-radius: 5px;
        z-index:500;
        display:none;
        margin-left: 18%;
        top: 250px;
        width:945px;
}
</style>

<script>
// disable auto log out
AUTOLOGOUT_MAX_MINUTE = 0;
overlib_str_tmp = "";
overlib.isOut = true;

var refreshRate = 3;
var timedEvent = 0;

var dataarray0 = [], wificlients0 = [];
var dataarray1 = [], wificlients1 = [];
var dataarray2 = [], wificlients2 = [];
var dataarray3 = [], wificlients3 = [];

var dfs_statusarray0 = [], dfs_statusarray1 = [], dfs_statusarray2 = [];

<% get_wl_status(); %>;

var classObj= {
        ToHexCode:function(str){
                return encodeURIComponent(str).replace(/%/g,"\\x").toLowerCase();
        },
        UnHexCode:function(str){
		return str.replace(/(?:\\x[\da-fA-F]{2})+/g, m =>
decodeURIComponent(m.replace(/\\x/g, '%'))).replace(/\\n/g,
'<br>');
        }
}

var content = "";
function Get_wireless_log(){
	var dead = 0;
	$.ajax({
		url: '/wl_log.asp',
		dataType: 'text',
		timeout: 1500,
		error: function(xhr){
			if(dead > 30){
				$("#wl_log").html("Fail to grab wireless log.");
			}
			else{
				dead++;
				setTimeout("Get_wireless_log();", 1000);
			}
		},

		success: function(resp){
			content = decodeURIComponent(resp);
			content = classObj.UnHexCode(content);
			content = htmlEnDeCode.htmlEncode(content);
			if(content.length > 10){
				$("#wl_log").html(content);
			}
			else{
				$("#wl_log").html("Fail to grab wireless log.");
			}
		}
	});
}

function initial(){
	show_menu();
	refreshRate = getRefresh();
	get_wlclient_list();

	if (bcm_mumimo_support) {
                document.getElementById("flags_mumimo_div").style.display = "";
		document.getElementById("flags_div").style.display = "none";
	}
}


function redraw(){
	var client_array = [];
	var apinfo_array = [];
	var dfs_array = [];
	var bandname_array = [];
	var code = "";

	if (based_modelid === 'GT-AXE16000') {
		apinfo_array = [dataarray3, dataarray0, dataarray1, dataarray2];
		client_array = [wificlients3, wificlients0, wificlients1, wificlients2];
		dfs_array = [[],dfs_statusarray0, dfs_statusarray1, []];
		bandname_array = ["2.4 GHz", "5 GHz-1", "5 GHz-2", "6 GHz"];
	} else if (based_modelid === 'GT-BE98_PRO') {
		apinfo_array = [dataarray3, dataarray0, dataarray1, dataarray2];
		client_array = [wificlients3, wificlients0, wificlients1, wificlients2];
		dfs_array = [[], dfs_statusarray0, [], []];
		bandname_array = ["2.4 GHz", "5 GHz", "6 GHz-1", "6 GHz-2"];
	} else {
		apinfo_array = [dataarray0];
		client_array = [wificlients0];
		dfs_array = [[]];
		bandname_array = ["2.4 GHz"];

		if (band5g_support) {
			apinfo_array.push(dataarray1);
			client_array.push(wificlients1);
			dfs_array.push(dfs_statusarray1);
			if (wl_info.band5g_2_support)
				bandname_array.push("5 GHz-1");
			else
				bandname_array.push("5 GHz");
		}
		if (band6g_support) {
			apinfo_array.push(dataarray2);
			client_array.push(wificlients2);
			dfs_array.push([]);
			bandname_array.push("6 GHz");
		} else if (wl_info.band5g_2_support) {
			apinfo_array.push(dataarray2);
			client_array.push(wificlients2);
			dfs_array.push(dfs_statusarray2);
			bandname_array.push("5 GHz-2");
		}
	}

	for (i = 0; i < client_array.length; i++) {
		if (client_array[i].length == 0) {
			code += '<div><span class="hint-color" style="font-size: 125%;">Wireless '+bandname_array[i]+' is disabled.</span></div><br><br>';
		} else {
			code += '<div>';
			code += generate_header(apinfo_array[i], bandname_array[i], dfs_array[i]);
			code += generate_clients(client_array[i]);
			code += '</div><br><br>';
		}
	}
	document.getElementById("datablock").innerHTML = code;
	Get_wireless_log();
}


function generate_clients(clientsarray) {
	var code, i, ii, client, overlib_str;
	var mac, ipaddr, hostname;
	var nmapentry;
	var guestheader = "";

	code = '<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">';
	code += '<thead><tr>';
	code += '<td width="25%">Device</td>';
	code += '<td width="29%">IP Address</td>';
	code += '<td width="16%">Rx/Tx & RSSI</td><td width="10%">Connected</td>';
	if (clientsarray.length > 1) {
		if (clientsarray[0][8] != "")
			code += '<td width="10%">Streams</td>';
		else
			code += '<td width="10%">PHY</td>';
	}
	code += '<td width="10%">Flags</td>';
	code += '</tr></thead>';

	if (clientsarray.length > 1) {
		for (i = 0; i < clientsarray.length-1; ++i) {
			client = clientsarray[i];

			// Display a new Guest header?
			if (client[12] != "" && client[12] != guestheader) {
				guestheader = client[12];
				if (sw_mode == "2") {
					code += '<tr><th colspan="6" style="color:white;height:20px;"><span class="hint-color" style="font-weight:bolder;">Local Clients: </span> ' + guestheader;
				} else {
					code += '<tr><th colspan="6" style="color:white;height:20px;"><span class="hint-color" style="font-weight:bolder;">Guest Network: </span> ' + guestheader;
					if (client[13] != "") {
						code += '<span style="float:right;"><span class="hint-color" style="font-weight:bolder;">VLAN: </span> ' + client[13] + '</span>';
					}
				}
				code += '</th></tr>';
			}
			code += '<tr>';

			// Mac
			mac = client[0];
			overlib_str = "<p><#MAC_Address#>:</p>" + mac;
			code += '<td><span style="margin-top:-15px; color: white;" class="link" onclick="oui_query_full_vendor(\'' + mac +'\');overlib_str_tmp=\''+ overlib_str +'\';return overlib(\''+ overlib_str +'\');" onmouseout="nd();" style="cursor:pointer; text-decoration:underline;">'+ mac +'</span>';

			if (typeof clientList[mac] === "undefined")
				nmapentry = false;
			else
				nmapentry = true;

			hostname = client[2];	// Name
			if (nmapentry && (hostname == "*" || hostname == "<unknown>")) {
				if (clientList[mac].nickName != "")
					hostname = clientList[mac].nickName;
				else if (clientList[mac].name != "")
					hostname = clientList[mac].name;
			}

			if (hostname.length > 24) {		// Name
				code +='<br><span style="margin-top:-15px; color: cyan;" title="' + hostname + '">'+ hostname.substring(0,20) +'...</span></td>';
			} else {
				code +='<br><span style="margin-top:-15px; color: cyan;">'+ htmlEnDeCode.htmlEncode(hostname) +'</span></td>';
			}

			ipaddr = client[1];
			if (nmapentry && ipaddr == "<unknown>") {
				if (clientList[mac].ip != "")
					ipaddr = clientList[mac].ip;
			}
			code += '<td style="vertical-align: top;">' + htmlEnDeCode.htmlEncode(ipaddr);	// IPv4

			if (client[3].length) {
				overlib_str = client[3].replace(/,/g, "<br>");
				code += '<br><span style="margin-top:-15px; color:cyan; text-decoration:underline; cursor:pointer;" onclick="return overlib(\''+ overlib_str +'\');" onmouseout="nd();">IPv6 addresses</span></td>';
			} else
				code += '<br></td>'; // IPv6

			code += '<td style="text-align: right;">' + client[5] + ' / ' + client[6] +' Mbps';	// Rate
			code += '<br><span style="margin-top:-15px; color: cyan;">' + client[4] + ' dBm</td>';	// RSSI
			code += '<td style="text-align: right;vertical-align:top;">' + client[7] + '</td>';	// Time

			if (client[8] != "") {
				code += '<td style="vertical-align:top;">' + client[8] + ' ('+ client[9] +')';	// NSS + PHY
			} else if (client[9] != "") {
				code += '<td style="vertical-align:top;">' + client[9];	// PHY
			} else {
				code += '<td>';
			}
			if (client[10] != "") {
				code += '<br><span style="margin-top:-15px; color: cyan;">' + client[10] + '</td>';  // BW
			} else {
				code += '</td>';
			}
			code += '<td style="vertical-align:top;">' + client[11] + '</td>';	// Flags
			code += '</tr>';
		}
	} else {
		code += '<tr><td colspan="7">No clients</td></tr>';
	}

	code += '</tr></table>';
	return code;
}


function generate_header(dataarray, title, dfs_statusarray) {
	var code;
	var channel, i;
	var time, formatted_time;

	code = '<table width="100%" style="border: none;">';
	code += '<thead><tr><span class="hint-color" style="font-size: 125%;">Wireless ' + title +'</span></tr></thead>';
	code += '<tr><td colspan="2"><span class="hint-color">SSID: </span>' + dataarray[0] + '</td>';
	code += '<td><span class="hint-color">BSSID: </span>' + dataarray[5] +'</td>';
	code += '<td><span class="hint-color">Mode: </span>' + dataarray[7] + '</td>';
	code += '</tr>';

	code += '<tr>';
	code += '<td><span class="hint-color">Channel: </span>'+ dataarray[4] + '</td>';
	if (dataarray[1] != 0)
		code += '<td><span class="hint-color">RSSI: </span>' + dataarray[1] + ' dBm</td>';
	else
		code += '<td>&nbsp;</td>';
	if (dataarray[2] != 0)
		code += '<td><span class="hint-color">SNR: </span>' + dataarray[2] +' dB</td>';
	if (dataarray[3] != 0)
		code += '<td><span class="hint-color">Noise: </span>' + dataarray[3] + ' dBm</td>';
        if (dataarray[6] != -1)
                code += '<td><span class="hint-color">Utilization: </span>'+ dataarray[6] + '%</td>';
	code += '</tr>';

	if (dfs_statusarray.length > 1) {
		code += '<tr><td colspan="2"><span class="hint-color">DFS State: </span>' + dfs_statusarray[0] + '</td>';
		time = parseInt(dfs_statusarray[1]);
		formatted_time = Math.floor(time / 3600) + "h " + Math.floor(time / 60) % 60 + "m " + time % 60 + "s";
		code += '<td><span class="hint-color">Time elapsed: </span>' + formatted_time + '</td>';
		code += '<td><span class="hint-color">Channel cleared for radar: </span>' + dfs_statusarray[2] + '</td></tr>';
	}

	code += '</table>';
	return code;
}


function get_wlclient_list() {

	if (timedEvent) {
		clearTimeout(timedEvent);
		timedEvent = 0;
	}

	$.ajax({
		url: '/ajax_wificlients.asp',
		dataType: 'script', 
		error: function(xhr){
				get_wlclient_list();
				},
		success: function(response){
			redraw();
			if (refreshRate > 0)
				timedEvent = setTimeout("get_wlclient_list();", refreshRate * 1000);
		}
	});

}


function getRefresh() {
	val  = parseInt(cookie.get('awrtm_wlrefresh'));

	if ((val != 0) && (val != 1) && (val != 3) && (val != 5) && (val != 10))
		val = 3;

	document.getElementById('refreshrate').value = val;

	return val;
}


function setRefresh(obj) {
	refreshRate = obj.value;
	cookie.set('awrtm_wlrefresh', refreshRate, 365);
	get_wlclient_list();
}


function open_details_window(){
        $("#details_window").fadeIn(300);
}

function hide_details_window(){
        $("#details_window").fadeOut(300);
}
</script>
</head>
<body onload="initial();" class="bg">
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>
<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" action="apply.cgi" target="hidden_frame">
<input type="hidden" name="current_page" value="Main_WStatus_Content.asp">
<input type="hidden" name="next_page" value="Main_WStatus_Content.asp">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="action_wait" value="">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<table class="content" align="center" cellpadding="0" cellspacing="0">
	<tr>
		<td width="17">&nbsp;</td>
		<td valign="top" width="202">
			<div id="mainMenu"></div>
			<div id="subMenu"></div>
		</td>
		<td valign="top">
			<div id="tabMenu" class="submenuBlock"></div>   
		<!-- ===================================Beginning of Main Content===========================================-->
			<table width="98%" border="0" align="left" cellpadding="0" cellspacing="0">
				<tr>
					<td valign="top" >           
						<table width="760px" border="0" cellpadding="5" cellspacing="0" bordercolor="#6b8fa3" class="FormTitle" id="FormTitle">
							<tr bgcolor="#4D595D">
								<td valign="top">
									<div>&nbsp;</div>
									<div class="formfonttitle"><#System_Log#> - <#menu5_7_4#></div>
									<div style="margin:10px 0 10px 5px;" class="splitLine"></div>
									<div class="formfontdesc">List of connected Wireless clients</div>

									<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
										<tr>
											<th>Refresh frequency</th>
											<td>
												<select name="refreshrate" class="input_option" onchange="setRefresh(this);" id="refreshrate">
													<option value="0">No refresh</option>
													<option value="1">1 second</option>
													<option value="3" selected>3 seconds</option>
													<option value="5">5 seconds</option>
													<option value="10">10 seconds</option>
												</select>
											</td>
										</tr>
										<tr>
											<th>Display low level details</th>
											<td>
												<input class="button_gen" type="button" onclick="open_details_window();" value="Open">
											</td>
										</tr>
									</table>
									<br>
									<div id="datablock"></div>
									<div id="flags_mumimo_div" style="display:none;">Flags: <span class="hint-color">P</span>=Powersave Mode, <span class="hint-color">S</span>=Short GI, <span class="hint-color">T</span>=STBC, <span class="hint-color">M</span>=MU Beamforming, <span class="hint-color">A</span>=Associated, <span class="hint-color">U</span>=Authenticated</div>
									<div id="flags_div">Flags: <span class="hint-color">P</span>=Powersave Mode, <span class="hint-color">S</span>=Short GI, <span class="hint-color">T</span>=STBC, <span class="hint-color">A</span>=Associated, <span class="hint-color">U</span>=Authenticated</div>
									<br>
									<div class="apply_gen">
										<input type="button" onClick="location.reload();" value="<#CTL_refresh#>" class="button_gen" >
									</div>
								</td>
							</tr>
						</table>
					</td>
				</tr>
			</table>
		</td>
	</tr>
</table>
<div id="footer"></div>
</form>

<div id="details_window"  class="contentM_details  pop_div_bg">
	<div style="margin: 15px;">
		<textarea id="wl_log" cols="63" rows="30" class="textarea_ssh_table" style="width:99%;font-family:'Courier New', Courier, mono; font-size:13px;" readonly="readonly" wrap="off"></textarea>
	</div>
	<div style="margin-top:5px;margin-bottom:5px;width:100%;text-align:center;">
		<input class="button_gen" type="button" onclick="hide_details_window();" value="Close">
	</div>
</div>
</body>
</html>

