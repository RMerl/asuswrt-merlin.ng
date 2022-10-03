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
								<input type="text" maxlength="1023" name="wgc_aips" id="wgc_aips" class="input_32_table" value="<% nvram_get("wgc_aips"); %>" autocorrect="off" autocapitalize="off"></input>
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
