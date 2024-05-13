<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=Edge"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<title><#Web_Title#>DNS Director</title>
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<link rel="stylesheet" type="text/css" href="ParentalControl.css">
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="usp_style.css">
<link rel="stylesheet" type="text/css" href="/device-map/device-map.css">
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/js/httpApi.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/client_function.js"></script>
<script type="text/javascript" src="/validator.js"></script>
<script type="text/javascript" src="/switcherplugin/jquery.iphone-switch.js"></script>
<script>

<% login_state_hook(); %>

var dnsfilter_rule_list = '<% nvram_get("dnsfilter_rulelist"); %>'.replace(/&#60/g, "<");
if (isSupport("hnd")) {
	dnsfilter_rule_list += '<% nvram_get("dnsfilter_rulelist1"); %>'.replace(/&#60/g, "<") +
		'<% nvram_get("dnsfilter_rulelist2"); %>'.replace(/&#60/g, "<") +
		'<% nvram_get("dnsfilter_rulelist3"); %>'.replace(/&#60/g, "<") +
		'<% nvram_get("dnsfilter_rulelist4"); %>'.replace(/&#60/g, "<") +
		'<% nvram_get("dnsfilter_rulelist5"); %>'.replace(/&#60/g, "<");
}

var dnsfilter_rule_list_row = dnsfilter_rule_list.split('<');

var modes_array = [[ "",   "System" ],
		  [ "0",  "No Redirection" ],
		  [ "11", "Router" ],
		  [ "8",  "User Defined 1" ],
		  [ "9",  "User Defined 2" ],
		  [ "10", "User Defined 3" ],
		  [ "",   "Unfiltered" ],
		  [ "17", "Cloudflare Safe" ],
		  [ "",   "Security filters" ],
		  [ "19", "AdGuard Ad block" ],
		  [ "14", "CleanBrowsing Security" ],
		  [ "12", "Comodo Secure DNS" ],
		  [ "1",  "OpenDNS Home" ],
		  [ "13", "Quad9" ],
		  [ "5",  "Yandex Safe" ],
		  [ "",   "Family-friendly filters" ],
		  [ "20", "AdGuard Family" ],
		  [ "15", "CleanBrowsing Adult" ],
		  [ "16", "CleanBrowsing Family" ],
		  [ "18", "Cloudflare Family" ],
		  [ "7",  "OpenDNS Family" ],
		  [ "6",  "Yandex Family" ]];


if (isSupport("mtlancfg")) {
	sdnRuleTable = [
		"idx",
		"sdn_name",
		"sdn_enable",
		"vlan_idx",
		"subnet_idx",
		"apg_idx",
		"vpnc_idx",
		"vpns_idx",
		"dns_filter_idx",
		"urlf_idx",
		"nwf_idx",
		"cp_idx",
		"gre_idx",
		"firewall_idx",
		"kill_switch",
		"access_host_service",
		"wan_idx",
		"pppoe-relay",
		"wan6_idx",
		"createby",
		"mtwan_idx",
		"mswan_idx"
	];

	var sdn_rl = decodeURIComponent(httpApi.nvramCharToAscii(["sdn_rl"]).sdn_rl)
	var sdn_rl_json = convertRulelistToJson(sdnRuleTable, sdn_rl);
}


function initial(){
	show_menu();
	show_footer();

	show_dnsfilter_list();
	if (isSupport("mtlancfg")) {
		document.getElementById("sdnTable_Table").style.display = "";
		show_sdn_list();
	}
	showDropdownClientList('setclientmac', 'mac', 'all', 'ClientList_Block_PC', 'pull_arrow', 'all');

	showhide_settings(document.form.dnsfilter_enable_x.value);

	gen_modeselect("dnsfilter_mode", "<% nvram_get("dnsfilter_mode"); %>", "");
	gen_modeselect("client_modesel", "-1", "");
}

/*------------ Mouse event of fake LAN IP select menu {-----------------*/
function setclientmac(macaddr){
	document.form.rule_mac.value = macaddr;
	hideClients_Block();
}


function pullLANIPList(obj){
	var element = document.getElementById('ClientList_Block_PC');
	var isMenuopen = element.offsetWidth > 0 || element.offsetHeight > 0;
	if(isMenuopen == 0){
		obj.src = "/images/unfold_less.svg"
		element.style.display = 'block';
		document.form.rule_mac.focus();
	}
	else
		hideClients_Block();
}

function hideClients_Block(){
	document.getElementById("pull_arrow").src = "/images/unfold_more.svg";
	document.getElementById('ClientList_Block_PC').style.display='none';
}


function gen_modeselect(name, value, onchange){
	var code = "";
	var i;
	var optGroup = "", opt;
	var obj = document.getElementById(name);

	if ((name == "client_modesel") || (name == "dnsfilter_mode")) {
		for(i = 0; i < modes_array.length; i++){
			if (modes_array[i][0] == "") {
				if (optGroup != "") obj.appendChild(optGroup);
				optGroup = document.createElement('optgroup');
				optGroup.label = modes_array[i][1];
			} else {
				if (optGroup == "")
					optGroup = document.createElement('optgroup');  // No group was initialized, so do one
				opt = document.createElement('option');
				opt.innerHTML = modes_array[i][1];
				opt.value = modes_array[i][0];
				opt.selected = (value == modes_array[i][0]);
				optGroup.appendChild(opt);
			}
	        }
		if (optGroup != "") obj.appendChild(optGroup);
	} else {
		code = '<select class="input_option" id="'+name+'" name="'+name+'" value="'+value+'" onchange="'+onchange+'">';
		for (i = 0; i < modes_array.length; i++){
			if (modes_array[i][0] == "") {
				if (optGroup != "") code += '</optgroup>';
				code += '<optgroup label="' + modes_array[i][1] + '">';
			} else {
				var itemval = modes_array[i][0];
				code +='<option value="' + itemval + '"' + (value == itemval ? "selected" : "") +'>' + modes_array[i][1] + '</option>';
			}
		}
		if (optGroup != "") code += '</optgroup>';
		code += '</select>';
	}

	return code;
}


function show_dnsfilter_list(){
	var code = "";
	var clientListEventData = [];

	code = '<table width="100%" border="1" cellspacing="0" cellpadding="4" align="center" class="list_table" id="clientTable">';
	if(dnsfilter_rule_list_row.length < 2)
		code += '<tr><td class="hint-color" colspan="3"><#IPConnection_VSList_Norule#></td></tr>';
	else{
		//user icon
		var userIconBase64 = "NoIcon";
		var clientName, deviceType, deviceVender;
		for(var i=1; i<dnsfilter_rule_list_row.length; i++){
			var ruleArray = dnsfilter_rule_list_row[i].split('&#62');
			var clientMac = ruleArray[1].toUpperCase();
			var rule_mode = ruleArray[2];

                        var clientIconID = "clientIcon_" + clientMac.replace(/\:/g, "");

			if(clientList[clientMac]) {
				clientName = (clientList[clientMac].nickName == "") ? clientList[clientMac].name : clientList[clientMac].nickName;
				deviceType = clientList[clientMac].type;
				deviceVender = clientList[clientMac].vendor;
				dnsfilter_rule_list_row[clientMac] = clientName;
			}
			else {
				clientName = clientMac;
				deviceType = 0;
				deviceVender = "";
			}
			code +='<tr id="row'+i+'">';
			code +='<td width="50%" title="'+clientName+'">';

			code += '<table style="width:100%;"><tr><td style="width:40%;height:56px;border:0px;float:right;">';
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
			code += '</td><td id="client_info_'+i+'" style="width:65%;text-align:left;border:0;">';
			code += '<div>' + clientName + '</div>';
			code += '<div>' + clientMac + '</div>';
			code += '</td></tr></table>';
			code += '</td>';

			code +='<td width="35%">'+gen_modeselect("rule_mode"+i, rule_mode, "changeRow_main(this);")+'</td>';
                        code +='<td width="15%"><input class="remove_btn" onclick="deleteRow_main(this);" value=""/></td></tr>';
			if(validator.mac_addr(clientMac))
				clientListEventData.push({"mac" : clientMac, "name" : "", "ip" : "", "callBack" : "DNSFilter"});
		}
	}

	code += '</table>';
	document.getElementById("mainTable_Block").innerHTML = code;
	for(var i = 0; i < clientListEventData.length; i += 1) {
		var clientIconID = "clientIcon_" + clientListEventData[i].mac.replace(/\:/g, "");
		var clientIconObj = $("#mainTable_Block").children("#clientTable").find("#" + clientIconID + "")[0];
		var paramData = JSON.parse(JSON.stringify(clientListEventData[i]));
		paramData["obj"] = clientIconObj;
		$("#mainTable_Block").children("#clientTable").find("#" + clientIconID + "").click(paramData, popClientListEditTable);
	}
}

function applyRule(){
	if (isSupport("hnd")) {
		if (( document.form.dnsfilter_custom61.value != "" && !validator.isLegal_ipv6(document.form.dnsfilter_custom61)) ||
		    ( document.form.dnsfilter_custom62.value != "" && !validator.isLegal_ipv6(document.form.dnsfilter_custom62)) ||
		    ( document.form.dnsfilter_custom63.value != "" && !validator.isLegal_ipv6(document.form.dnsfilter_custom63)))
			return false;
	}

	if (isSupport("hnd"))
		split_clientlist(dnsfilter_rule_list.replace(/&#62/g, ">"));
	else
		document.form.dnsfilter_rulelist.value = dnsfilter_rule_list.replace(/&#62/g, ">") ;

	if (isSupport("mtlancfg")) {
		save_sdn_rules();
	}

	showLoading();
	document.form.submit();
}

function split_clientlist(clientlist){
	var counter = 0;

	document.form.dnsfilter_rulelist.value = clientlist.substring(counter, (counter+=255))
	document.form.dnsfilter_rulelist1.value = clientlist.substring(counter, (counter+=255));
	document.form.dnsfilter_rulelist2.value = clientlist.substring(counter, (counter+=255));
	document.form.dnsfilter_rulelist3.value = clientlist.substring(counter, (counter+=255));
	document.form.dnsfilter_rulelist4.value = clientlist.substring(counter, (counter+=255));
	document.form.dnsfilter_rulelist5.value = clientlist.substring(counter, (counter+=255));
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
	}else if(flag == 2){
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

function addRow_main(upper){
	var rule_num = document.getElementById('clientTable').rows.length;
	var item_num = document.getElementById('clientTable').rows[0].cells.length;

	if(rule_num >= upper){
		alert("<#JS_itemlimit1#> " + upper + " <#JS_itemlimit2#>");
		return false;
	}

	if(document.form.rule_mac.value == ""){
		alert("<#JS_fieldblank#>");
		document.form.rule_mac.focus();
		return false;
	}

	if(!check_macaddr(document.form.rule_mac, check_hwaddr_flag(document.form.rule_mac,'inner'))){
		document.form.rule_mac.focus();
		document.form.rule_mac.select();
		return false;
	}

	dnsfilter_rule_list += "<";
	dnsfilter_rule_list += "&#62";	// Formerly name field
	dnsfilter_rule_list += document.form.rule_mac.value + "&#62";
	dnsfilter_rule_list += document.form.rule_mode.value;

	dnsfilter_rule_list_row = dnsfilter_rule_list.split('<');

	document.form.rule_mac.value = "";
	document.form.rule_mode.value = 0;
	show_dnsfilter_list();
}

function deleteRow_main(r){
	var delIndex = r.parentNode.parentNode.rowIndex;
	dnsfilter_rule_list = "";

	for(var i = 1; i < dnsfilter_rule_list_row.length; i++)
	{
		var ruleArray = dnsfilter_rule_list_row[i].split('&#62');
		if( (delIndex) != i-1)
		{
			dnsfilter_rule_list += "<";
			dnsfilter_rule_list += "&#62";       // Formerly name field
			dnsfilter_rule_list += ruleArray[1] + "&#62";
			dnsfilter_rule_list += ruleArray[2];
		}
	}

	dnsfilter_rule_list_row = dnsfilter_rule_list.split('<');
	show_dnsfilter_list();
}

function changeRow_main(r){
	var index = r.parentNode.parentNode.rowIndex;
	dnsfilter_rule_list = "";

	for(var i = 1; i < dnsfilter_rule_list_row.length; i++)
	{
		var ruleArray = dnsfilter_rule_list_row[i].split('&#62');

		dnsfilter_rule_list += "<";
		dnsfilter_rule_list += "&#62";       // Formerly name field
		dnsfilter_rule_list += ruleArray[1] + "&#62";

		if( (index) == i-1)
		{
			dnsfilter_rule_list += r.value;
		} else {
			dnsfilter_rule_list += ruleArray[2];
		}
	}

	dnsfilter_rule_list_row = dnsfilter_rule_list.split('<');
	show_dnsfilter_list();
}

function showhide_settings(state) {
	showhide("dnsfilter_mode_tr", state);
	showhide("dnsfilter_custom1", state);
	showhide("dnsfilter_custom2", state);
	showhide("dnsfilter_custom3", state);
	if (isSupport("hnd")) {
		showhide("dnsfilter_custom61", state);
		showhide("dnsfilter_custom62", state);
		showhide("dnsfilter_custom63", state);
	}
	showhide("mainTable_Table", state);
	showhide("mainTable_Block", state);
}


function convertRulelistToJson(attrArray, rulelist) {
	var rulelist_json = [];

	var each_rule = rulelist.split("<");
	var convertAtoJ = function(rule_array) {
		var rule_json = {}
		$.each(rule_array, function(index, value) {
			if (index > attrArray.length - 1)
				attr = "ext" + index;
			else
				attr = attrArray[index];
			rule_json[attr] = rule_array[index];
		});
		return rule_json;
	}

	$.each(each_rule, function(index, value) {
		if (value != "") {
			var one_rule_array = value.split(">");
			var one_rule_json = convertAtoJ(one_rule_array);
			if (!one_rule_json.error) rulelist_json.push(one_rule_json);
		}
	});

	return rulelist_json;
}


function show_sdn_list() {
	var code;
	var i = 0;

	code = '<table width="100%" border="1" cellspacing="0" cellpadding="4" align="center" class="list_table" id="clientTable">';

	$.each(sdn_rl_json, function(index, entry){
		var sdn_name = decodeURIComponent(httpApi.nvramCharToAscii(["apg" + entry.apg_idx + "_ssid"])["apg" + entry.apg_idx + "_ssid"])
		if (entry.idx != "0") {	// Skip DEFAULT sdn
			i++;
			code +='<tr id="row'+i+'">';
			code +='<td width="50%" title="'+sdn_name+'">'+sdn_name+'</td>';
			code +='<td width="50%">'+gen_modeselect("sdn_dns_filter_idx"+entry.idx, entry.dns_filter_idx, "")+'</td>';
			code += '</tr>';
		}
	});
	code += '</table>';

	document.getElementById("sdnTable_Block").innerHTML = code;
}


function save_sdn_rules() {
	var nv = "";
	var new_entry;

	$.each(sdn_rl_json, function(index, entry){
		if (entry.idx != 0) {
			entry.dns_filter_idx = document.getElementById("sdn_dns_filter_idx"+entry.idx).value;
		}
	});

	$.each(sdn_rl_json, function(idx, profile){
		nv += "<";
		new_entry = 1;
		for (var attr in profile) {
			if (new_entry)
				new_entry = 0;
			else
				nv += ">";
			nv += profile[attr];
		}
	});
	document.form.sdn_rl.value = nv;
}

</script>
</head>

<body onload="initial();" onunload="unload_body();" class="bg">
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="DNSDirector.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_wait" value="5">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="restart_dnsfilter">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="dnsfilter_enable_x" value="<% nvram_get("dnsfilter_enable_x"); %>">
<input type="hidden" name="dnsfilter_rulelist" value="">
<input type="hidden" name="dnsfilter_rulelist1" value="">
<input type="hidden" name="dnsfilter_rulelist2" value="">
<input type="hidden" name="dnsfilter_rulelist3" value="">
<input type="hidden" name="dnsfilter_rulelist4" value="">
<input type="hidden" name="dnsfilter_rulelist5" value="">
<input type="hidden" name="sdn_rl" value="<% nvram_get("sdn_rl"); %>">

<table class="content" align="center" cellpadding="0" cellspacing="0" >
	<tr>
		<td width="17">&nbsp;</td>
		<td valign="top" width="202">
		<div  id="mainMenu"></div>
		<div  id="subMenu"></div>
		</td>

    <td valign="top">
	<div id="tabMenu" class="submenuBlock"></div>
		<!--===================================Beginning of Main Content===========================================-->
<table width="98%" border="0" align="left" cellpadding="0" cellspacing="0" >
	<tr>
		<td valign="top" >

<table width="730px" border="0" cellpadding="4" cellspacing="0" class="FormTitle" id="FormTitle">
	<tbody>
	<tr>
		<td bgcolor="#4D595D" valign="top">
		<div>&nbsp;</div>
		<div class="formfonttitle">DNS Director</div>
		<div style="margin:10px 0 10px 5px;" class="splitLine"></div>

		<div class="formfontdesc">
			<p>DNS Director allows you to force LAN devices to use a
			   specific DNS server, which can be useful if you want
			   to force them to use a filtering service that would
			   block malicious or adult sites.  You can set a global
			   network-wide server, or client-specific servers.
			   Beside the available presets you can also define up to three
			   different custom servers to use.</p>
			<br>
			<p>A few special System options are available in the presets.  "No Redirection" will bypass a global redirection,
			   and "Router" will force clients to use the DNS provided by the router's DHCP server (or, the router itself if it's not defined).
		</div>

			<!--=====Beginning of Main Content=====-->
			<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
				<thead><tr><td colspan="3">Settings</td></tr></thead>
				<tr>
					<th>Enable DNS Director</th>
					<td>
						<div align="center" class="left" style="width:94px; float:left; cursor:pointer;" id="radio_dnsfilter_enable"></div>
						<div class="iphone_switch_container" style="height:32px; width:74px; position: relative; overflow: hidden">
							<script type="text/javascript">
								$('#radio_dnsfilter_enable').iphoneSwitch(document.form.dnsfilter_enable_x.value,
									function(){
										document.form.dnsfilter_enable_x.value = 1;
										showhide_settings(1);
									},
									function(){
										document.form.dnsfilter_enable_x.value = 0;
										showhide_settings(0);
									},
										{
											switch_on_container_path: '/switcherplugin/iphone_switch_container_off.png'
									});
							</script>
						</div>
					</td>
				</tr>
				<tr id="dnsfilter_mode_tr">
					<th>Global Redirection</th>
					<td>
						<select name="dnsfilter_mode" id="dnsfilter_mode" class="input_option">
						</select>
					</td>
				</tr>
				<tr id="dnsfilter_custom1">
					<th width="200">User defined DNS 1</th>
					<td>
						<span>IPv4: <input type="text" maxlength="15" class="input_15_table" name="dnsfilter_custom1" value="<% nvram_get("dnsfilter_custom1"); %>" onKeyPress="return validator.isIPAddr(this,event)"></span>
						<span id="dnsfilter_custom61" style="padding-left:10px;display:none;">IPv6: <input type="text" maxlength="39" class="input_25_table" name="dnsfilter_custom61" value="<% nvram_get("dnsfilter_custom61"); %>"></span>
					</td>
				</tr>
				<tr id="dnsfilter_custom2">
					<th width="200">User defined DNS 2</th>
					<td>
						<span>IPv4: <input type="text" maxlength="15" class="input_15_table" name="dnsfilter_custom2" value="<% nvram_get("dnsfilter_custom2"); %>" onKeyPress="return validator.isIPAddr(this,event)"></span>
						<span id="dnsfilter_custom62" style="padding-left:10px;display:none;">IPv6: <input type="text" maxlength="39" class="input_25_table" name="dnsfilter_custom62" value="<% nvram_get("dnsfilter_custom62"); %>"></span>
					</td>
				</tr>
				<tr id="dnsfilter_custom3">
					<th width="200">User defined DNS 3</th>
					<td>
						<span>IPv4: <input type="text" maxlength="15" class="input_15_table" name="dnsfilter_custom3" value="<% nvram_get("dnsfilter_custom3"); %>" onKeyPress="return validator.isIPAddr(this,event)"></span>
						<span id="dnsfilter_custom63" style="padding-left:10px;display:none;">IPv6: <input type="text" maxlength="39" class="input_25_table" name="dnsfilter_custom63" value="<% nvram_get("dnsfilter_custom63"); %>"></span>
					</td>
				</tr>
			</table>

			<table width="100%" border="1" cellspacing="0" cellpadding="4" align="center" class="FormTable_table" style="margin-top:8px;" id="mainTable_Table">
				<thead><tr><td colspan="3"><#ConnectedClient#>&nbsp;(<#List_limit#>&nbsp;64)</td></tr></thead>
				<tr>
					<th>Client MAC address</th>
					<th>Redirection</th>
					<th><#list_add_delete#></th>
				</tr>
				<tr>
					<td width="50%">
						<input type="text" maxlength="17" style="margin-left:10px;width:255px;" autocorrect="off" autocapitalize="off" class="input_macaddr_table" name="rule_mac" onClick="hideClients_Block();" onKeyPress="return validator.isHWAddr(this,event)" placeholder="ex: <% nvram_get("lan_hwaddr"); %>">
						<img id="pull_arrow" height="14px;" src="/images/unfold_more.svg" style="position:absolute;" onclick="pullLANIPList(this);" title="<#select_client#>">
						<div id="ClientList_Block_PC" style="margin:0 0 0 52px" class="clientlist_dropdown"></div>
					</td>
					<td width="35%">
						<select class="input_option" name="rule_mode" id="client_modesel"></select>
					</td>

					<td width="15%"><input class="add_btn" type="button" onClick="addRow_main(64)" value=""></td>
				</tr>
			</table>

			<!-- Client list -->
			<div id="mainTable_Block"></div>

			<table width="100%" border="1" cellspacing="0" cellpadding="4" align="center" class="FormTable_table" style="margin-top:8px; display:none;" id="sdnTable_Table">
			<thead><tr><td colspan="2">Guest Network Pro profiles</td></tr></thead>
				<tr>
					<th>Network</th>
					<th>Redirection</th>
				</tr>
			</table>

			<!-- SDN list -->
			<div id="sdnTable_Block"></div>

			<div class="apply_gen">
				<input name="button" type="button" class="button_gen" onclick="applyRule()" value="<#CTL_apply#>"/>
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

<div id="footer"></div>
</form>
<script>

</script>
</body>
</html>
