<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=Edge"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title><#Web_Title#> - WireGuard Client</title>
<link rel="stylesheet" href="index_style.css">
<link rel="stylesheet" href="form_style.css">

<script src="/state.js"></script>
<script src="/general.js"></script>
<script src="/help.js"></script>
<script src="/popup.js"></script>
<script src="/validator.js"></script>
<script src="/js/jquery.js"></script>
<script src="/js/httpApi.js"></script>

<script>

<% get_wgc_parameter(); %>

var wgc_enable = '<% nvram_get("wgc_enable"); %>';
var wgc_unit = '<% nvram_get("wgc_unit"); %>';
var directorrules_array = "<% nvram_char_to_ascii("", "vpndirector_rulelist"); %>";

function initial(){
	show_menu();

	var vpn_client_array = {"OpenVPN" : ["OpenVPN", "Advanced_OpenVPNClient_Content.asp"], "PPTP" : ["PPTP/L2TP", "Advanced_VPNClient_Content.asp"], "Wireguard" : ["Wireguard", "Advanced_WireguardClient_Content.asp"]};

	if(!wireguard_support) {
		delete vpn_client_array.Wireguard;
	}
	if(!vpnc_support) {
		delete vpn_client_array.PPTP;
	}
	if(!openvpnd_support) {
		delete vpn_client_array.OpenVPN;
	}

	$('#divSwitchMenu').html(gen_switch_menu(vpn_client_array, "Wireguard"));

	if (wgc_enable == '1') {
		document.getElementById("WgcLogTable").style.display = "";
	}

	// Client list
	free_options(document.form.wgc_unit);
	add_option(document.form.wgc_unit, "1: <% nvram_get("wgc1_desc"); %>", "1", (wgc_unit == 1));
	add_option(document.form.wgc_unit, "2: <% nvram_get("wgc2_desc"); %>", "2", (wgc_unit == 2));
	add_option(document.form.wgc_unit, "3: <% nvram_get("wgc3_desc"); %>", "3", (wgc_unit == 3));
	add_option(document.form.wgc_unit, "4: <% nvram_get("wgc4_desc"); %>", "4", (wgc_unit == 4));
	add_option(document.form.wgc_unit, "5: <% nvram_get("wgc5_desc"); %>", "5", (wgc_unit == 5));

	// State
	var state = "0";
	switch (wgc_unit) {
	case "1":
		state = "<% sysinfo("wgcstatus.1"); %>";
		break;
	case "2":
		state = "<% sysinfo("wgcstatus.2"); %>";
		break;
	case "3":
		state = "<% sysinfo("wgcstatus.3"); %>";
		break;
	case "4":
		state = "<% sysinfo("wgcstatus.4"); %>";
		break;
	case "5":
		state = "<% sysinfo("wgcstatus.5"); %>";
		break;
	}
	document.getElementById("wgcstate").innerHTML = (state == "0" ? "Stopped" : "Connected");

	show_director_rules();
}

function applyRule(){

	if(validForm()){

		showLoading();
		document.form.submit();
	}
}

function validForm(){
	if (document.form.wgc_enable[1].checked)
		return true;

	var ip_RegExp = {
		"IPv4" : "^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$",
		"IPv4_CIDR" : "^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])(\/([0-9]|[1-2][0-9]|3[0-2]))$",
		"IPv6" : "^((([0-9A-Fa-f]{1,4}:){7}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){6}:[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){5}:([0-9A-Fa-f]{1,4}:)?[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){4}:([0-9A-Fa-f]{1,4}:){0,2}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){3}:([0-9A-Fa-f]{1,4}:){0,3}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){2}:([0-9A-Fa-f]{1,4}:){0,4}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){6}((\\b((25[0-5])|(1\\d{2})|(2[0-4]\\d)|(\\d{1,2}))\\b)\\.){3}(\\b((25[0-5])|(1\\d{2})|(2[0-4]\\d)|(\\d{1,2}))\\b))|(([0-9A-Fa-f]{1,4}:){0,5}:((\\b((25[0-5])|(1\\d{2})|(2[0-4]\\d)|(\\d{1,2}))\\b)\\.){3}(\\b((25[0-5])|(1\\d{2})|(2[0-4]\\d)|(\\d{1,2}))\\b))|(::([0-9A-Fa-f]{1,4}:){0,5}((\\b((25[0-5])|(1\\d{2})|(2[0-4]\\d)|(\\d{1,2}))\\b)\\.){3}(\\b((25[0-5])|(1\\d{2})|(2[0-4]\\d)|(\\d{1,2}))\\b))|([0-9A-Fa-f]{1,4}::([0-9A-Fa-f]{1,4}:){0,5}[0-9A-Fa-f]{1,4})|(::([0-9A-Fa-f]{1,4}:){0,6}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){1,7}:))$",
		"IPv6_CIDR" : "^((([0-9A-Fa-f]{1,4}:){7}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){6}:[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){5}:([0-9A-Fa-f]{1,4}:)?[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){4}:([0-9A-Fa-f]{1,4}:){0,2}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){3}:([0-9A-Fa-f]{1,4}:){0,3}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){2}:([0-9A-Fa-f]{1,4}:){0,4}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){6}((\\b((25[0-5])|(1\\d{2})|(2[0-4]\\d)|(\\d{1,2}))\\b)\\.){3}(\\b((25[0-5])|(1\\d{2})|(2[0-4]\\d)|(\\d{1,2}))\\b))|(([0-9A-Fa-f]{1,4}:){0,5}:((\\b((25[0-5])|(1\\d{2})|(2[0-4]\\d)|(\\d{1,2}))\\b)\\.){3}(\\b((25[0-5])|(1\\d{2})|(2[0-4]\\d)|(\\d{1,2}))\\b))|(::([0-9A-Fa-f]{1,4}:){0,5}((\\b((25[0-5])|(1\\d{2})|(2[0-4]\\d)|(\\d{1,2}))\\b)\\.){3}(\\b((25[0-5])|(1\\d{2})|(2[0-4]\\d)|(\\d{1,2}))\\b))|([0-9A-Fa-f]{1,4}::([0-9A-Fa-f]{1,4}:){0,5}[0-9A-Fa-f]{1,4})|(::([0-9A-Fa-f]{1,4}:){0,6}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){1,7}:))(\/([0-9]|[1-9][0-9]|1[01][0-9]|12[0-8]))$"
	};

	var valid_base64 = function(str){
		var format = /^([0-9a-zA-Z+/]{4})*(([0-9a-zA-Z+/]{2}==)|([0-9a-zA-Z+/]{3}=))?$/;
		if(format.test(str))
			return true;
		else
			return false;
	};

	var valid_is_IP_format = function(str, type){
		var cidr_exist = str.indexOf("/");
		if(cidr_exist != -1)
			str = str.substr(0, cidr_exist);
		var format = new RegExp(ip_RegExp[type], "gi");
		return format.test(str);
	};

	var valid_IP_CIDR = function(addr, type, mode){
		//mode, 0:IP, 1:IP/CIDR, 2:IP or IP/CIDR
		var testResultPass = {
			'isError': false,
			'errReason': ''
		};
		var testResultFail = {
			'isError': true,
			'errReason': addr + " <#JS_validip#>"
		};
		var IP = new RegExp(ip_RegExp[type],"gi");
		var IP_CIDR = new RegExp(ip_RegExp[type + "_CIDR"], "gi");
		if(mode == "0"){
			if(IP.test(addr))
				return testResultPass;
			else{
				testResultFail.errReason = testResultFail.errReason + ", IP Address without CIDR."
				return testResultFail;
			}
		}
		else if(mode == "1"){
			if(IP_CIDR.test(addr))
				return testResultPass;
			else{
				testResultFail.errReason = testResultFail.errReason + ", IP Address/CIDR"
				return testResultFail;
			}
		}
		else if(mode == "2"){
			if(IP_CIDR.test(addr) || IP.test(addr))
				return testResultPass;
			else{
				testResultFail.errReason = testResultFail.errReason + ", IP Address without CIDR or IP Address/CIDR."
				return testResultFail;
			}
		}
		else
			return testResultFail;
	};

	var valid_is_IP_format = function(str, type){
		var cidr_exist = str.indexOf("/");
		if(cidr_exist != -1)
			str = str.substr(0, cidr_exist);
		var format = new RegExp(ip_RegExp[type], "gi");
		return format.test(str);
	};

	var valid_domainName = function(str){
		var testResult = {
			'isError': false,
			'errReason': ''
		};
		var format = new RegExp(/^(?:[a-z0-9](?:[a-z0-9-_]{0,61}[a-z0-9])?\.)*[a-z0-9][a-z0-9-_]{0,61}[a-z0-9]$/i);
		if(format.test(str))
			return testResult;
		else{
			testResult.isError = true;
			testResult.errReason = str + " <#JS_valid_FQDN#>";
			return testResult;
		}
	};

// Misc fields
	if (document.form.wgc_desc.value == "") {
		document.form.wgc_desc.focus();
		alert("You must provide a description!");
		return false;
	}

	if (document.form.wgc_priv.value == "") {
		document.form.wgc_priv.focus();
		alert("Missing private key!");
		return false;
	}

	if (!valid_base64(document.form.wgc_priv.value)) {
		alert("Invalid private key!");
		document.form.wgc_priv.focus();
		return false;
	}

	if (document.form.wgc_addr.value == "") {
		document.form.wgc_addr.focus();
		alert("You must provide an address!");
		return false;
	}

// Server
	var wgc_addr_array = $("#wgc_addr").val().split(",");
	var isValid_wgc_addr = "";
	$.each(wgc_addr_array, function(index, address){
		if(valid_is_IP_format(address, "IPv4")){
			isValid_wgc_addr = valid_IP_CIDR(address, "IPv4", "2");
			if(isValid_wgc_addr.isError)
				return false;
		}
		else if(valid_is_IP_format(address, "IPv6")){
			isValid_wgc_addr = valid_IP_CIDR(address, "IPv6", "2");
			if(isValid_wgc_addr.isError)
				return false;
		}
		else{
			isValid_wgc_addr = { 'isError' : true, 'errReason' : "Invalid address!" };
			return false;
		}
	});

	if (isValid_wgc_addr.isError) {
		alert(isValid_wgc_addr.errReason);
		document.form.wgc_addr.focus();
		return false;
	}

// DNS
	var $wgc_dns = $("#wgc_dns");//IPv4, IPv6, host nmae
	$wgc_dns.val($wgc_dns.val().replace(/\s+/g, ''));//remove space
	if($wgc_dns.val().substr($wgc_dns.val().length-1) == ",")
		$wgc_dns.val($wgc_dns.val().slice(0,-1));//remove last ","

	if($wgc_dns.val() != ""){
		var isValid_wgc_dns = "";
		var wgc_dns_array = $wgc_dns.val().split(",");
		$.each(wgc_dns_array, function(index, item){
			if(valid_is_IP_format(item, "IPv4")){
				isValid_wgc_dns = valid_IP_CIDR(item, "IPv4", "0");
				if(isValid_wgc_dns.isError)
					return false;
			}
			else if(valid_is_IP_format(item, "IPv6")){
				isValid_wgc_dns = valid_IP_CIDR(item, "IPv6", "0");
				if(isValid_wgc_dns.isError)
					return false;
			}
			else{
				isValid_wgc_dns = valid_domainName(item);
				if(isValid_wgc_dns.isError)
					return false;
			}
		});

		if(isValid_wgc_dns.isError){
			alert(isValid_wgc_dns.errReason);
			$wgc_dns.focus();
			return false;
		}
	}


// Public key
	var $wgc_ppub = $("#wgc_ppub");
	$wgc_ppub.val($wgc_ppub.val().trim());
	if($wgc_ppub.val() == ""){
		alert("Missing public key!");
		$wgc_ppub.focus();
		return false;
	}
	var isValid_wgc_ppub = valid_base64($wgc_ppub.val());
	if(!isValid_wgc_ppub){
		alert("Invalid public key!");
		$wgc_ppub.focus();
		return false;
	}


// Preshared key
	var $wgc_psk = $("#wgc_psk");
	$wgc_psk.val($wgc_psk.val().trim());
	if($wgc_psk.val() != ""){
		var isValid_wgc_psk = valid_base64($wgc_psk.val());
		if(!isValid_wgc_psk){
			alert("Invalid pre-shared key!");
			$wgc_psk.focus();
			return false;
		}
	}


// AllowedIPs
	var $wgc_aips = $("#wgc_aips");//IPv4_CIDR, IPv6_CIDR
	$wgc_aips.val($wgc_aips.val().replace(/\s+/g, ''));//remove space
	if($wgc_aips.val().substr($wgc_aips.val().length-1) == ",")
		$wgc_aips.val($wgc_aips.val().slice(0,-1));//remove last ","
	if($wgc_aips.val() == ""){
		alert("You must specify allowed IP addresses (use 0.0.0.0/0 for \"any\")");
		$wgc_aips.focus();
		return false;
	}

	var isValid_wgc_aips = "";
	var wgc_aips_array = $wgc_aips.val().split(",");
	$.each(wgc_aips_array, function(index, address){
		if(address == "::/0")
			return true;
		if(valid_is_IP_format(address, "IPv4")){
			isValid_wgc_aips = valid_IP_CIDR(address, "IPv4", "1");
			if(isValid_wgc_aips.isError)
			return false;
		}
		else if(valid_is_IP_format(address, "IPv6")){
			isValid_wgc_aips = valid_IP_CIDR(address, "IPv6", "1");
			if(isValid_wgc_aips.isError)
				return false;
		}
		else{
			isvalid_wgc_aips.isError = true;
			isvalid_wgc_aips.errReason = "Invalid IP address!";
			return false;
		}
	});

	if(isValid_wgc_aips.isError){
		alert(isValid_wgc_aips.errReason);
		$wgc_aips.focus();
		return false;
	}


// Endpoint
	var $wgc_ep_addr = $("#wgc_ep_addr");//IPv4, IPv6, host name
	$wgc_ep_addr.val($wgc_ep_addr.val().trim());
	if($wgc_ep_addr.val() == ""){
		alert("You must specify an endpoint IP address!");
		$wgc_ep_addr.focus();
		return false;
	}
	var isValid_wgc_ep_addr = "";
	if(valid_is_IP_format($wgc_ep_addr.val(), "IPv4")){
		isValid_wgc_ep_addr = valid_IP_CIDR($wgc_ep_addr.val(), "IPv4", "0");
	}
	else if(valid_is_IP_format($wgc_ep_addr.val(), "IPv6")){
		isValid_wgc_ep_addr = valid_IP_CIDR($wgc_ep_addr.val(), "IPv6", "0");
	}
	else{
		isValid_wgc_ep_addr = valid_domainName($wgc_ep_addr.val());
	}
	if(isValid_wgc_ep_addr.isError){
		alert(isValid_wgc_ep_addr.errReason);
		$wgc_ep_addr.focus();
		return false;
	}

	var $wgc_ep_port = $("#wgc_ep_port");
	$wgc_ep_port.val($wgc_ep_port.val().trim());
	if($wgc_ep_port.val() == ""){
		alert("You must specify an endpoint port!");
		$wgc_ep_port.focus();
		return false;
	}

	if(!validator.numberRange(document.getElementById("wgc_ep_port"), 1, 65535))
		return false;

// Keep alive
	var $wgc_alive = $("#wgc_alive");
	$wgc_alive.val($wgc_alive.val().trim());
	if($wgc_alive.val() == ""){
		alert("You must specify a keep alive value!");
		$wgc_alive.focus();
		return false;
	}

	if(!validator.numberRange(document.getElementById("wgc_alive"), 1, 65535))
		return false;

// MTU
	if(document.getElementById("wgc_mtu").value != "" &&
	   !validator.numberRange(document.getElementById("wgc_mtu"), 576, 1500))
		return false;

	return true;
}

function change_wgc_unit(unit){
	document.chg_wgc.wgc_unit.value=unit.toString();
	document.chg_wgc.submit();
}

function Importwg(){
        if (document.getElementById('wgfile').value == "") return false;
        document.getElementById('importWgFile').style.display = "none";
        document.getElementById('loadingicon').style.display = "";

	var postData = {
		"wgc_upload_unit": wgc_unit,
		"file": $('#wgfile').prop('files')[0]
	};
	httpApi.uploadWGCFile(postData);

	var count = 0;
	var timer = 10;
	var interval_check = setInterval(function(inputObj){
		var status_text = wgcFileChecker("init");
		if(status_text != ""){
			clearInterval(interval_check);
			document.getElementById('importWgFile').style.display = "";
			document.getElementById('loadingicon').style.display = "none";

			if(httpApi.nvramGet(["wgc_upload_state"])["wgc_upload_state"] == "0"){
				document.getElementById("importWgFile").innerHTML = status_text;
				setTimeout("location.href='Advanced_WireguardClient_Content.asp';", 3000);
			}
		}
		else if(count >= timer){
			clearInterval(interval_check);
			document.getElementById('loadingicon').style.display = "none";
			document.getElementById("importWgFile").innerHTML = "<#SET_fail_desc#>";
		}
		count++;
	},1000, $(this));
}

function wgcFileChecker(_init){
	var result = "";
	var wgc_upload_state = _init;
	var wgc_upload_state_current = httpApi.nvramGet(["wgc_upload_state"],true)["wgc_upload_state"];
	if(wgc_upload_state_current != "")
		wgc_upload_state = wgc_upload_state_current;

	if(wgc_upload_state != "init"){
		if(wgc_upload_state == "err"){
			result = "<#Setting_upload_hint#>";
		}
		else if(wgc_upload_state == "0"){
			result = "<#Main_alert_proceeding_desc3#>";
		}
	}
	return result;
}

function show_director_rules(){
	var directorrules_row =  decodeURIComponent(directorrules_array).split('<');
	var code = "";
	var line;
	var width = ["10%", "25%", "22%", "22%", "11%"];

	code +='<table width="100%" cellspacing="0" cellpadding="4" align="center" class="list_table" id="directorrules_table">';
	if(directorrules_row.length == 1)
		code +='<tr><td class="hint-color" colspan="6"><#IPConnection_VSList_Norule#></td></tr>';
	else{
		for(var i = 1; i < directorrules_row.length; i++){
			line ='<tr id="row'+i+'">';
			var directorrules_col = directorrules_row[i].split('>');
				for(var j = 0; j < directorrules_col.length; j++){
					if (j == 0) {
						line += '<td width="' + width[j] +'">' + (directorrules_col[0] == "1" ? "<img title='Enabled' src='/images/New_ui/enable.svg'" :
					                "<img title='Disabled' src='/images/New_ui/disable.svg'") +
						        'style="width:25px; height:25px;"></td>';
					} else {
						line +='<td width="' + width[j] +'">'+ directorrules_col[j] +'</td>';
					}
				}
				line += '</tr>';
				if (directorrules_col[4] == "WAN" ||
				    directorrules_col[4] == "WGC" + wgc_unit) {
					code += line;
				}
		}
	}

	code +='</table>';
	document.getElementById("directorrules_Block").innerHTML = code;
}

</script>

</head>

<body onload="initial();" onunLoad="return unload_body();" class="bg">
<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>


<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="Advanced_WireguardClient_Content.asp">
<input type="hidden" name="next_page" value="Advanced_WireguardClient_Content.asp">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="restart_wgc">
<input type="hidden" name="action_wait" value="1">
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

		<!--===================================Beginning of Main Content===========================================-->
<table width="98%" border="0" align="left" cellpadding="0" cellspacing="0">
	<tr>
		<td valign="top" >

			<table width="760px" border="0" cellpadding="4" cellspacing="0" class="FormTitle" id="FormTitle">
			<tbody>
			<tr>
				<td bgcolor="#4D595D" valign="top"  >
					<div>&nbsp;</div>
					<div class="formfonttitle">VPN - WireGuard Client</div>
					<div id="divSwitchMenu" style="margin-top:-40px;float:right;"></div
					<div style="margin:10px 0 10px 5px;" class="splitLine"></div>

					<table id="WgcBasicTable" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable">
						<thead>
							<tr>
								<td colspan="2"><#t2BC#></td>
							</tr>
						</thead>
						<tr id="wgc_unit_field" class="rept ew">
							<th>Select client instance</th>
							<td>
								<select name="wgc_unit" class="input_option" onChange="change_wgc_unit(this.value);">
								</select>
							</td>
						</tr>
						<tr>
							<th>Description</th>
							<td>
								<input type="text" maxlength="25" class="input_25_table" name="wgc_desc" value="<% nvram_get("wgc_desc"); %>">
							</td>
						</tr>
						<tr id="wgc_enable">
							<th>Enable WireGuard</th>
							<td>
								<input type="radio" value="1" name="wgc_enable" class="input" <% nvram_match("wgc_enable", "1", "checked"); %>><#checkbox_Yes#></input>
								<input type="radio" value="0" name="wgc_enable" class="input" <% nvram_match("wgc_enable", "0", "checked"); %>><#checkbox_No#></input>
								<span style="margin-left:20px;" id="wgcstate"></span>
							</td>
						</tr>
						<tr id="wgc_nat">
							<th><#Enable_NAT#></th>
							<td>
								<input type="radio" value="1" name="wgc_nat" class="input" <% nvram_match("wgc_nat", "1", "checked"); %>><#checkbox_Yes#></input>
								<input type="radio" value="0" name="wgc_nat" class="input" <% nvram_match("wgc_nat", "0", "checked"); %>><#checkbox_No#></input>
							</td>
						</tr>
						<tr>
							<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(50,30);">Inbound Firewall</a></th>
							<td>
								<input type="radio" name="wgc_fw" class="input" value="1" <% nvram_match_x("", "wgc_fw", "1", "checked"); %>>Block
								<input type="radio" name="wgc_fw" class="input" value="0" <% nvram_match_x("", "wgc_fw", "0", "checked"); %>>Allow
							</td>
						</tr>
						<tr>
							<th>Import config</th>
							<td>
								<input id="wgfile" type="file" name="file" class="input" style="color:#FFCC00;*color:#000;">
								<input id="" class="button_gen" onclick="Importwg();" type="button" value="<#CTL_upload#>" />
								<img id="loadingicon" style="margin-left:5px;display:none;" src="/images/InternetScan.gif">
								<span id="importWgFile" style="display:none;"><#Main_alert_proceeding_desc3#></span>
							</td>
						</tr>
					</table>

					<table id="WgcInterfaceTable" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable">
						<thead>
							<tr>
								<td colspan="2">Interface</td>
							</tr>
						</thead>
						<tr>
							<th>Private Key</th>
							<td>
								<input type="text" maxlength="63" name="wgc_priv" id="wgc_priv" class="input_32_table" value="<% nvram_get("wgc_priv"); %>" autocorrect="off" autocapitalize="off"></input>
							</td>
						</tr>
						<tr>
							<th>MTU (Optional)</th>
							<td>
								<input type="text" maxlength="4" name="wgc_mtu" id="wgc_mtu" class="input_6_table" onKeyPress="return validator.isNumber(this,event);" value="<% nvram_get("wgc_mtu"); %>" autocorrect="off" autocapitalize="off"></input>
							</td>
						</tr>
						<tr>
							<th>Address</th>
							<td>
								<input type="text" maxlength="39" name="wgc_addr" id="wgc_addr" class="input_32_table" value="<% nvram_get("wgc_addr"); %>" autocorrect="off" autocapitalize="off"></input>
							</td>
						</tr>
						<tr>
							<th>DNS Server (Optional)</th>
							<td>
								<input type="text" maxlength="39" name="wgc_dns" id="wgc_dns" class="input_32_table" value="<% nvram_get("wgc_dns"); %>" autocorrect="off" autocapitalize="off"></input>
							</td>
						</tr>
					</table>

					<table id="WgcPeerTable" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable">
						<thead>
							<tr>
								<td colspan="2">Peer</td>
							</tr>
						</thead>
						<tr>
							<th>Server Public Key</th>
							<td>
								<input type="text" maxlength="63" name="wgc_ppub" id="wgc_ppub" class="input_32_table" value="<% nvram_get("wgc_ppub"); %>" autocorrect="off" autocapitalize="off"></input>
							</td>
						</tr>
						<tr>
							<th>Preshared Key (Optional)</th>
							<td>
								<input type="text" maxlength="63" name="wgc_psk" id="wgc_psk" class="input_32_table" value="<% nvram_get("wgc_psk"); %>" autocorrect="off" autocapitalize="off"></input>
							</td>
						</tr>
						<tr>
							<th>Allowed IPs</th>
							<td>
								<input type="text" maxlength="4095" name="wgc_aips" id="wgc_aips" class="input_32_table" value="<% nvram_get("wgc_aips"); %>" autocorrect="off" autocapitalize="off"></input>
							</td>
						</tr>
						<tr>
							<th>Endpoint Address:Port</th>
							<td>
								<input type="text" maxlength="39" name="wgc_ep_addr" id="wgc_ep_addr" class="input_32_table" value="<% nvram_get("wgc_ep_addr"); %>" autocorrect="off" autocapitalize="off"></input> :
								<input type="text" maxlength="5" name="wgc_ep_port" id="wgc_ep_port" class="input_6_table" onKeyPress="return validator.isNumber(this,event);" value="<% nvram_get("wgc_ep_port"); %>" autocorrect="off" autocapitalize="off"></input>
							</td>
						</tr>
						<tr>
							<th>Persistent Keepalive</th>
							<td>
								<input type="text" maxlength="5" name="wgc_alive" id="wgc_alive" class="input_6_table" onKeyPress="return validator.isNumber(this,event);" value="<% nvram_get("wgc_alive"); %>" autocorrect="off" autocapitalize="off"></input>
							</td>
						</tr>
					</table>

					<div class="apply_gen" id="apply_btn">
						<input class="button_gen" onclick="applyRule();" type="button" value="<#CTL_apply#>"/>
					</div>

					<table id="selectiveTable" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable_table" style="margin-top:8px;">
						<thead>
							<tr>
								<td colspan="5">VPN Director rules related to this client - <a href="Advanced_VPNDirector.asp" style="text-decoration:underline;">click here</a> to edit</td>
							</tr>
						</thead>
						<tr>
							<th width="10%">Enabled</th>
							<th width="25%"><#IPConnection_autofwDesc_itemname#></th>
							<th width="22%">Local IP</th>
							<th width="22%">Remote IP</th>
							<th width="11%">Iface</th>
						</tr>
					</table>
					<div id="directorrules_Block"></div>
					<br>

					<table id="WgcLogTable" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable" style="display:none">
						<thead>
							<tr>
								<td>Client status log:</td>
							</tr>
						</thead>
						<tr>
							<td>
								<div style="margin-top:8px">
									<textarea class="textarea_ssh_table" style="width:99%; font-family:'Courier New', Courier, mono; font-size:13px;" cols="63" rows="25" readonly="readonly" wrap=off><% nvram_dump("wgc.log",""); %></textarea>
								</div>
								<div class="apply_gen">
									<input type="button" onClick="location.reload();" value="<#CTL_refresh#>" class="button_gen">
								</div>
							</td>
						</tr>
					</table>
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
<form method="post" name="chg_wgc" action="apply.cgi" target="hidden_frame">
	<input type="hidden" name="action_mode" value="chg_wgc_unit">
	<input type="hidden" name="action_script" value="">
	<input type="hidden" name="action_wait" value="">
	<input type="hidden" name="current_page" value="Advanced_WireguardClient_Content.asp">
	<input type="hidden" name="wgc_unit" value="">
</form>
<div id="footer"></div>
</body>
</html>
