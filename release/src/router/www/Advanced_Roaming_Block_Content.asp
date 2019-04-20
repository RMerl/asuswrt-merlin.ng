<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=Edge"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">

<title><#Web_Title#> - <#WiFi_Roaming_Block_List#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="device-map/device-map.css">
<script language="JavaScript" type="text/javascript" src="state.js"></script>
<script language="JavaScript" type="text/javascript" src="general.js"></script>
<script language="JavaScript" type="text/javascript" src="popup.js"></script>
<script language="JavaScript" type="text/javascript" src="client_function.js"></script>
<script language="JavaScript" type="text/javascript" src="merlin.js"></script>
<script language="JavaScript" type="text/javascript" src="validator.js"></script>
<script language="JavaScript" type="text/javascript" src="js/jquery.js"></script>
<script language="JavaScript" type="text/javascript" src="js/httpApi.js"></script>
<style>
#pull_arrow{
 	float:center;
 	cursor:pointer;
 	border:2px outset #EFEFEF;
 	background-color:#CCC;
 	padding:3px 2px 4px 0px;
	*margin-left:-3px;
	*margin-top:1px;
}
</style>
<script>
var wl_rast_static_client_array = new Array();
function initial(){
	show_menu();
	var wl_rast_static_client = httpApi.nvramGet(["wl0_rast_static_client"]).wl0_rast_static_client;
	var wl_rast_static_client_row = wl_rast_static_client.split("&#60");
	for(var i = 1; i < wl_rast_static_client_row.length; i += 1) {
		wl_rast_static_client_array[wl_rast_static_client_row[i]] = true;
	}
	genClientList();
	show_wl_maclist_x();
	enable_roaming_block();
	setTimeout("showDropdownClientList('setClientmac', 'mac', 'wl', 'WL_MAC_List_Block', 'pull_arrow', 'all');", 1000);
}

function show_wl_maclist_x(){
	var code = "";
	var clientListEventData = [];
	code += '<table width="100%" cellspacing="0" cellpadding="4" align="center" class="list_table"  id="wl_maclist_x_table">';
	if(Object.keys(wl_rast_static_client_array).length == 0)
		code += '<tr><td style="color:#FFCC00;"><#IPConnection_VSList_Norule#></td></tr>';
	else{
		//user icon
		var userIconBase64 = "NoIcon";
		var clientName, deviceType, deviceVender;
		Object.keys(wl_rast_static_client_array).forEach(function(key) {
			var clientMac = key.toUpperCase();
			var clientIconID = "clientIcon_" + clientMac.replace(/\:/g, "");
			if(clientList[clientMac]) {
				clientName = (clientList[clientMac].nickName == "") ? clientList[clientMac].name : clientList[clientMac].nickName;
				deviceType = clientList[clientMac].type;
				deviceVender = clientList[clientMac].vendor;
			}
			else {
				clientName = "New device";
				deviceType = 0;
				deviceVender = "";
			}
			code +='<tr id="row_'+clientMac+'">';
			code +='<td width="80%" align="center">';
			code += '<table style="width:100%;"><tr><td style="width:35%;height:56px;border:0px;float:right;">';
			if(clientList[clientMac] == undefined) {
				code += '<div id="' + clientIconID + '" class="clientIcon type0"></div>';
			}
			else {
				if(usericon_support) {
					userIconBase64 = getUploadIcon(clientMac.replace(/\:/g, ""));
				}
				if(userIconBase64 != "NoIcon") {
					code += '<div id="' + clientIconID + '" style="text-align:center;"><img class="imgUserIcon_card" src="' + userIconBase64 + '"></div>';
				}
				else if(deviceType != "0" || deviceVender == "") {
					code += '<div id="' + clientIconID + '" class="clientIcon type' + deviceType + '"></div>';
				}
				else if(deviceVender != "" ) {
					var venderIconClassName = getVenderIconClassName(deviceVender.toLowerCase());
					if(venderIconClassName != "" && !downsize_4m_support) {
						code += '<div id="' + clientIconID + '" class="venderIcon ' + venderIconClassName + '"></div>';
					}
					else {
						code += '<div id="' + clientIconID + '" class="clientIcon type' + deviceType + '"></div>';
					}
				}
			}
			code += '</td><td style="width:60%;border:0px;">';
			code += '<div>' + clientName + '</div>';
			code += '<div>' + clientMac + '</div>';
			code += '</td></tr></table>';
			code += '</td>';
			code += '<td width="20%"><input type="button" class=\"remove_btn\" onclick=\"deleteRow(this, \'' + clientMac + '\');\" value=\"\"/></td></tr>';
			if(validator.mac_addr(clientMac))
				clientListEventData.push({"mac" : clientMac, "name" : "", "ip" : "", "callBack" : "RoamingBlock"});
		});
	}
	code += '</table>';
	document.getElementById("wl_rast_static_client_Block").innerHTML = code;
	for(var i = 0; i < clientListEventData.length; i += 1) {
		var clientIconID = "clientIcon_" + clientListEventData[i].mac.replace(/\:/g, "");
		var clientIconObj = $("#wl_rast_static_client_Block").children("#wl_maclist_x_table").find("#" + clientIconID + "")[0];
		var paramData = JSON.parse(JSON.stringify(clientListEventData[i]));
		paramData["obj"] = clientIconObj;
		$("#wl_rast_static_client_Block").children("#wl_maclist_x_table").find("#" + clientIconID + "").click(paramData, popClientListEditTable);
	}
}

function deleteRow(r, delMac){
	var i = r.parentNode.parentNode.rowIndex;
	delete wl_rast_static_client_array[delMac];
	document.getElementById('wl_maclist_x_table').deleteRow(i);

	if(Object.keys(wl_rast_static_client_array).length == 0)
		show_wl_maclist_x();
}

function addRow(obj, upper){
	var mac = obj.value.toUpperCase();

	if(Object.keys(wl_rast_static_client_array).length >= upper) {
		alert("<#JS_itemlimit1#> " + upper + " <#JS_itemlimit2#>");
		return false;
	}
	
	if(mac == "") {
		alert("<#JS_fieldblank#>");
		obj.focus();
		obj.select();
		return false;
	}
	else if(!check_macaddr(obj, check_hwaddr_flag(obj, 'inner'))) {
		obj.focus();
		obj.select();
		return false;
	}
	if(wl_rast_static_client_array[mac] != null) {
		alert("<#JS_duplicate#>");
		obj.focus();
		obj.select();
		return false;
	}

	wl_rast_static_client_array[mac] = true;
	obj.value = "";
	show_wl_maclist_x();
}

function applyRule(){
	var isWLclient = function () {  //detect login client is by wireless or wired
		<% login_state_hook(); %>
		var wireless = [<% wl_auth_list(); %>];	// [[MAC, associated, authorized], ...]
			
		if(wireless.length > 0) {
			for(var i = 0; i < wireless.length; i += 1) {
				if(wireless[i][0].toUpperCase() == login_mac_str().toUpperCase()) {
					return true;  //wireless
				}
			}
		}

		return false; //wired
	};
	var tmp_value = "";
	Object.keys(wl_rast_static_client_array).forEach(function(key) {
		tmp_value += "<" + key;
	});
	httpApi.nvramSet({
		"rast_static_cli_enable" : getRadioValue(document.form.enable_roaming),
		"wl0_rast_static_client" : tmp_value,
		"wl1_rast_static_client" : tmp_value,
		"wl2_rast_static_client" : tmp_value,
		"action_mode" : "apply",
		"rc_service" : "restart_wireless"
	}, function(){
		if(isWLclient()){
			var makeRequest = {
				_notSuccessCount: 0,
				_notSupportXML: false,

				start: function(url, callBackSuccess, callBackError){
					var xmlHttp;
					if(window.XMLHttpRequest)
						xmlHttp = new XMLHttpRequest();
					else if(window.ActiveXObject)
						xmlHttp = new ActiveXObject("Microsoft.XMLHTTP");
					else{
						makeRequest._notSupportXML = true;
						alert("Your browser does not support XMLHTTP.");
						return false;
					}

					xmlHttp.onreadystatechange = function(){
						if(xmlHttp.readyState == 4){
							if(xmlHttp.status == 200){
								callBackSuccess(xmlHttp);
							}
							else{
								makeRequest._notSuccessCount++;
								callBackError();
							}	
				 		}
					}

					xmlHttp.open('GET', url, true);
					xmlHttp.send(null);
				}
			};
			var getXMLAndRedirect = function(){
				if(makeRequest._notSupportXML)
					location.href = "Advanced_Roaming_Block_Content.asp";

				makeRequest.start('/httpd_check.xml', function(xhr){
					var httpds = xhr.responseXML.getElementsByTagName("httpd");
					if (httpds !== null && httpds[0] !== null)
						location.href = "Advanced_Roaming_Block_Content.asp";
				}, getXMLAndRedirect);
			};
			showWlHint();
			setTimeout(getXMLAndRedirect, 5000);
		}
		else {
			showLoading(10);
			setTimeout(function(){
				refreshpage();
			}, 10000);
		}
	});
}

function check_macaddr(obj,flag){ //control hint of input mac address
	if(flag == 1){
		var childsel=document.createElement("div");
		childsel.setAttribute("id","check_mac");
		childsel.style.color="#FFCC00";
		obj.parentNode.appendChild(childsel);
		document.getElementById("check_mac").innerHTML="<#LANHostConfig_ManualDHCPMacaddr_itemdesc#>";
		document.getElementById("check_mac").style.display = "";
		return false;
	}else if(flag ==2){
		var childsel=document.createElement("div");
		childsel.setAttribute("id","check_mac");
		childsel.style.color="#FFCC00";
		obj.parentNode.appendChild(childsel);
		document.getElementById("check_mac").innerHTML="<#IPConnection_x_illegal_mac#>";
		document.getElementById("check_mac").style.display = "";
		return false;
	}else{
		document.getElementById("check_mac") ? document.getElementById("check_mac").style.display="none" : true;
		return true;
	}	
}

function pullWLMACList(obj){
	var element = document.getElementById('WL_MAC_List_Block');
	var isMenuopen = element.offsetWidth > 0 || element.offsetHeight > 0;
	if(isMenuopen == 0){
		obj.src = "/images/arrow-top.gif"
		element.style.display = "block";
		document.form.wlX_rast_static_client.focus();
	}
	else
		hideClients_Block();
}

function hideClients_Block(){
	document.getElementById("pull_arrow").src = "/images/arrow-down.gif";
	document.getElementById("WL_MAC_List_Block").style.display = "none";
}

function setClientmac(macaddr){
	document.form.wlX_rast_static_client.value = macaddr;
	hideClients_Block();
}

function enable_roaming_block(){
	var enable_roaming_status = getRadioValue(document.form.enable_roaming);
	if(enable_roaming_status == "1") {
		document.getElementById('MainTable2').style.display = "";
		document.getElementById('wl_rast_static_client_Block').style.display = "";
	}	
	else {
		document.getElementById('MainTable2').style.display = "none";
		document.getElementById('wl_rast_static_client_Block').style.display = "none";
	}	
}
function showWlHint(){
	if(!isSwMode('rt') && !isSwMode('ap')) return false;

	var genWlObj = (function(){
		var wlObj = [];
		var wlUnit = function(_band, _ssid, _key){
			this.band = _band;
			this.ssid = _ssid;
			this.key = _key;
		}

		var ssid_nvram = [decodeURIComponent('<% nvram_char_to_ascii("", "wl0_ssid"); %>'), decodeURIComponent('<% nvram_char_to_ascii("", "wl1_ssid"); %>'), decodeURIComponent('<% nvram_char_to_ascii("", "wl2_ssid"); %>')];
		var auth_nvram = [decodeURIComponent('<% nvram_char_to_ascii("", "wl0_auth_mode_x"); %>'), decodeURIComponent('<% nvram_char_to_ascii("", "wl1_auth_mode_x"); %>'), decodeURIComponent('<% nvram_char_to_ascii("", "wl2_auth_mode_x"); %>')];
		var key_nvram = [decodeURIComponent('<% nvram_char_to_ascii("", "wl0_wpa_psk"); %>'), decodeURIComponent('<% nvram_char_to_ascii("", "wl1_wpa_psk"); %>'), decodeURIComponent('<% nvram_char_to_ascii("", "wl2_wpa_psk"); %>')];
		var smart_connect_nvram = '<% nvram_get("smart_connect_x"); %>';

		var ssid_param = [decodeURIComponent('<% get_ascii_parameter("wl0_ssid"); %>'), decodeURIComponent('<% get_ascii_parameter("wl1_ssid"); %>'), decodeURIComponent('<% get_ascii_parameter("wl2_ssid"); %>')];
		var auth_param = [decodeURIComponent('<% get_ascii_parameter("wl0_auth_mode_x"); %>'), decodeURIComponent('<% get_ascii_parameter("wl1_auth_mode_x"); %>'), decodeURIComponent('<% get_ascii_parameter("wl2_auth_mode_x"); %>')];
		var key_param = [decodeURIComponent('<% get_ascii_parameter("wl0_wpa_psk"); %>'), decodeURIComponent('<% get_ascii_parameter("wl1_wpa_psk"); %>'), decodeURIComponent('<% get_ascii_parameter("wl2_wpa_psk"); %>')];

		var applyParam = {
			unit: decodeURIComponent('<% get_ascii_parameter("wl_unit"); %>'),
			ssid: decodeURIComponent('<% get_ascii_parameter("wl_ssid"); %>'),
			auth: decodeURIComponent('<% get_ascii_parameter("wl_auth_mode_x"); %>'),
			key: decodeURIComponent('<% get_ascii_parameter("wl_wpa_psk"); %>'),
			smartConnect: decodeURIComponent('<% get_ascii_parameter("smart_connect_x"); %>')
		}

		// original profile
		for(var i=0; i<wl_nband_title.length; i++){
			wlObj.push(new wlUnit(wl_nband_title[i], ssid_nvram[i], (auth_nvram[i] == "open") ? "" : key_nvram[i]));
		}

		if(applyParam.ssid != ""){
			// handle wl
			wlObj[applyParam.unit].ssid = applyParam.ssid;
			wlObj[applyParam.unit].key = (applyParam.auth == "open") ? "" : applyParam.key;
		}
		else{
			// handle wlX
			for(var i=0; i<wlObj.length; i++){
				if(ssid_param[i] != ""){
					wlObj[i].ssid = ssid_param[i];
					wlObj[i].key = (auth_param[i] == "open") ? "" : key_param[i];
				}
			}
		}

		// handle smart connect
		if(applyParam.smartConnect == ""){
			if(smart_connect_nvram == 1){
				// Tri band steering
				wlObj.length = 1;
				wlObj[0].band = "Tri-band Smart Connect";
			}
			else if(smart_connect_nvram == 2){
				// 5GHz band steering
				wlObj.length = 2;
				wlObj[1].band = "5 GHz Smart Connect";
			}
		}
		else if(applyParam.smartConnect == 1){
			// Tri band steering
			wlObj.length = 1;
			wlObj[0].band = "Tri-band Smart Connect";
		}
		else if(applyParam.smartConnect == 2){
			// 5GHz band steering
			wlObj.length = 2;
			wlObj[1].band = "5 GHz Smart Connect";
		}

		return wlObj;
	})();

	(function(wlObj){
		if(wlObj.length == 0 || typeof wlObj == "undefined") return false;

		var wlHintCss = "";
		wlHintCss += "<style type='text/css'>"
		// Desktop style sheet
		wlHintCss += "#wlHint{";
		wlHintCss += "font-family: Arial;";
		wlHintCss += "background:url(/images/New_ui/login_bg.png) #283437 no-repeat;";
		wlHintCss += "background-size: 1280px 1076px;";
		wlHintCss += "z-index:9999;";
		wlHintCss += "position:absolute;";
		wlHintCss += "left:0;";
		wlHintCss += "top:0;";
		wlHintCss += "width:100%;";
		wlHintCss += "height:100%;";
		wlHintCss += "background-position: center 0%;";
		wlHintCss += "margin: 0px;";
		wlHintCss += "}.prod_madelName{";
		wlHintCss += "font-size: 26pt;";
		wlHintCss += "color:#fff;";
		wlHintCss += "margin-top: 10px;";
		wlHintCss += "}.nologin{";
		wlHintCss += "word-break: break-all;";
		wlHintCss += "margin:10px 0px 0px 78px;";
		wlHintCss += "background-color:rgba(255,255,255,0.2);";
		wlHintCss += "padding:20px;";
		wlHintCss += "line-height:36px;";
		wlHintCss += "border-radius: 5px;";
		wlHintCss += "width: 480px;";
		wlHintCss += "border: 0;";
		wlHintCss += "color:#fff;";
		wlHintCss += "font-size:16pt;";
		wlHintCss += "}.div_table{";
		wlHintCss += "display:table;";
		wlHintCss += "}.div_tr{";
		wlHintCss += "display:table-row;";
		wlHintCss += "}.div_td{";
		wlHintCss += "display:table-cell;";
		wlHintCss += "}.title_gap{";
		wlHintCss += "margin:20px 0px 0px 78px;";
		wlHintCss += "width: 480px;";
		wlHintCss += "font-size: 16pt;";
		wlHintCss += "color:#fff;";
		wlHintCss += "}.main_field_gap{";
		wlHintCss += "margin:100px auto 0;";
		wlHintCss += "}b{color:#00BBFF;";
		wlHintCss += "}.title_name{";
		wlHintCss += "font-size: 40pt;";
		wlHintCss += "color:#93d2d9;";
		wlHintCss += "}.img_gap{";
		wlHintCss += "padding-right:30px;";
		wlHintCss += "vertical-align:middle;";
		wlHintCss += "}.login_img{";
		wlHintCss += "width:43px;";
		wlHintCss += "height:43px;";
		wlHintCss += "background-image: url('images/New_ui/icon_titleName.png');";
		wlHintCss += "background-repeat: no-repeat;}";
		// Mobile style sheet
		wlHintCss += "@media screen and (max-width: 1000px){";
		if(top.location.pathname.search("QIS") != -1){
			wlHintCss += "#wlHint{background:url('/images/qis/pattern3-3_10_A15.png'),url('/images/qis/pattern3_05_4.png'),url('/images/qis/mainimage_img4.png') #1D1E1F no-repeat;";
			wlHintCss += "background-size:auto;}b{color:#279FD9;}";
		}
		wlHintCss += ".prod_madelName{";
		wlHintCss += "font-size: 13pt;";
		wlHintCss += "}.nologin{";
		wlHintCss += "margin-left:10px;";
		wlHintCss += "padding:10px;";
		wlHintCss += "line-height:18pt;";
		wlHintCss += "width: 100%;";
		wlHintCss += "font-size:14px;";
		wlHintCss += "}.main_field_gap{";
		wlHintCss += "width:82%;";
		wlHintCss += "margin:10px 0 0 15px;";
		wlHintCss += "}.title_name{";
		wlHintCss += "font-size:20pt;";
		wlHintCss += "margin-left:15px;";
		wlHintCss += "}.login_img{";
		wlHintCss += "background-size: 75%;";
		wlHintCss += "}.img_gap{";
		wlHintCss += "padding-right:0;";
		wlHintCss += "vertical-align:middle;";
		wlHintCss += "}.title_gap{";
		wlHintCss += "margin:15px 0px 0px 15px;";
		wlHintCss += "width: 100%;";
		wlHintCss += "font-size: 12pt;";
		wlHintCss += "}}";
		wlHintCss += "</style>";

		var wlHintHtml = '';
		wlHintHtml += '<meta content="telephone=no" name="format-detection"><meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=yes">';
		wlHintHtml += '<div id="wlHint">';
		wlHintHtml += '<div class="div_table main_field_gap">';
		wlHintHtml += '<div class="div_tr">';
		wlHintHtml += '<div class="prod_madelName"><div class="title_name"><div class="div_td img_gap"><div class="login_img"></div></div><div class="div_td"><#Web_Title2#></div></div></div>';
		wlHintHtml += '<div id="login_filed">';
		wlHintHtml += "<div class='p1 title_gap'><#DrSurf_sweet_advise1#></div>";
		for(var i=0; i<wlObj.length; i++){
			wlHintHtml += '<div class="p1 title_gap">'+ wlObj[i].band +'</div>';
			wlHintHtml += '<div class="nologin">';
			wlHintHtml += '<#QIS_finish_wireless_item1#>: <b>';
			wlHintHtml += wlObj[i].ssid + '</b><br>';
			wlHintHtml += '<#Network_key#>: <b>';
			wlHintHtml += (wlObj[i].key == "") ? "Open System" : wlObj[i].key;
			wlHintHtml += '</b></div>';
		}
		wlHintHtml += '</div></div></div></div>';

		top.document.write(wlHintCss + wlHintHtml);
	})(genWlObj);
}
</script>
</head>

<body onload="initial();">
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>
<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="current_page" value="Advanced_Roaming_Block_Content.asp">
<input type="hidden" name="next_page" value="Advanced_Roaming_Block_Content.asp">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<table class="content" align="center" cellpadding="0" cellspacing="0">
	<tr>
		<td width="17">&nbsp;</td>
		<td valign="top" width="202">
			<div  id="mainMenu"></div>
			<div  id="subMenu"></div>
		</td>
		<td valign="top">
			<div id="tabMenu" class="submenuBlock"></div>
			<!--===================================Beginning of Main Content===========================================-->
			<table width="98%" border="0" align="left" cellpadding="0" cellspacing="0">
				<tr>
					<td valign="top" >		
						<table width="760px" border="0" cellpadding="4" cellspacing="0" class="FormTitle" id="FormTitle">
						<tbody>
							<tr>
								<td bgcolor="#4D595D" valign="top">
									<div>&nbsp;</div>
									<div class="formfonttitle"><#menu5_1#> - <#WiFi_Roaming_Block_List#></div>
									<div style="margin:10px 0 10px 5px;" class="splitLine"></div>
									<div class="formfontdesc"><#Roaming_block_list_desc#></div>
									<table id="MainTable1" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
										<thead>
										<tr>
											<td colspan="2"><#t2BC#></td>
										</tr>
										</thead>
										<tr>
											<th width="30%"><#Roaming_block_list_enable#></th>
											<td>
												<label><input type="radio" name="enable_roaming" value="1" onclick="enable_roaming_block();" <% nvram_match("rast_static_cli_enable", "1","checked"); %>><#checkbox_Yes#></label>
												<label><input type="radio" name="enable_roaming" value="0" onclick="enable_roaming_block();" <% nvram_match("rast_static_cli_enable", "0","checked"); %>><#checkbox_No#></label>
											</td>
										</tr>
									</table>
									<table id="MainTable2" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable_table">
										<thead>
											<tr>
												<td colspan="2"><#WiFi_Roaming_Block_List#>&nbsp;(<#List_limit#>&nbsp;64)</td>
											</tr>
										</thead>
										<tr>
											<th width="80%"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,10);"><#Client_Name#> (<#PPPConnection_x_MacAddressForISP_itemname#>)</a></th>
											<th width="20%"><#list_add_delete#></th>
										</tr>
										<tr>
											<td width="80%">
												<input type="text" maxlength="17" class="input_macaddr_table" name="wlX_rast_static_client" onKeyPress="return validator.isHWAddr(this,event)" onClick="hideClients_Block();" autocorrect="off" autocapitalize="off" placeholder="ex: <% nvram_get("lan_hwaddr"); %>" style="width:255px;">
												<img id="pull_arrow" height="14px;" src="/images/arrow-down.gif" style="position:absolute;" onclick="pullWLMACList(this);" title="<#select_wireless_MAC#>">
												<div id="WL_MAC_List_Block" class="clientlist_dropdown" style="margin-left:167px;"></div>
											</td>
											<td width="20%">
												<input type="button" class="add_btn" onClick="addRow(document.form.wlX_rast_static_client, 64);" value="">
											</td>
										</tr>
									</table>
									<div id="wl_rast_static_client_Block"></div>
									<div id="submitBtn" class="apply_gen">
										<input class="button_gen" onclick="applyRule()" type="button" value="<#CTL_apply#>"/>
									</div>
								</td>
							</tr>
						</tbody>
						</table>
					</td>
				</tr>
			</table>
			<!--===================================Ending of Main Content===========================================-->
		</td>
		<td width="10" align="center" valign="top">&nbsp;</td>
	</tr>
</table>
</form>
<div id="footer"></div>
</body>
</html>
