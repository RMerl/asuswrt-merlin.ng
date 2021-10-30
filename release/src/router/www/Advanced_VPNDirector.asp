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
<title><#Web_Title#> - VPN Director</title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="device-map/device-map.css">
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script type="text/javascript" language="JavaScript" src="/help.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script language="JavaScript" type="text/javascript" src="/client_function.js"></script>
<script language="JavaScript" type="text/javascript" src="/validator.js"></script>
<script type="text/javascript" language="JavaScript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/js/httpApi.js"></script>
<script type="text/javascript" src="form.js"></script>
<style type="text/css">
.contentM_qis{
	position:absolute;
	-webkit-border-radius: 5px;
	-moz-border-radius: 5px;
	border-radius: 5px;
	z-index: 200;
	display: none;
	margin-left: 35%;
	top: 280px !important;
	width: 600px;
	box-shadow: 1px 5px 10px #000;
	font-size: 12px;
	color: #FFFFFF;
	padding: 20px;
}
.addRuleFrame {
	margin-top: 5px;
	height: 35px;
	padding-left: 5px;
}
.addRuleText {
	float: left;
	height: 35px;
	line-height: 35px;
	font-weight: bolder;
	font-family: Arial, Verdana, Arial, Helvetica, sans-serif;
	color: #FFFFFF;
	font-size: 14px;
}

</style>
<script>
<% login_state_hook(); %>

var vpndirector_rulelist_array = [];

var ruleMaxNum = 199;
var ovpn_info = {
        "desc": "",
        "routing": "",
	"enforce" : "",
        "state": ""
        }

function initial(){
	show_menu();

	var parseNvramToArray = function(oriNvram) {
		var parseArray = [];
		var oriNvramRow = decodeURIComponent(oriNvram).split('<');

		for (var i = 0; i < oriNvramRow.length; i++) {
			if (oriNvramRow[i] != "") {
				var oriNvramCol = oriNvramRow[i].split('>');
				var eachRuleArray = new Array();
				var enable = oriNvramCol[0];
				eachRuleArray["enable"] = enable;
				var description = oriNvramCol[1];
				eachRuleArray["description"] = description;
				var localIP = (oriNvramCol[2] == undefined) ? "" : oriNvramCol[2];
				eachRuleArray["localIP"] = localIP;
				var remoteIP = (oriNvramCol[3] == undefined) ? "" : oriNvramCol[3];
				eachRuleArray["remoteIP"] = remoteIP;
				var interface = oriNvramCol[4];
				eachRuleArray["interface"] = interface;
				parseArray.push(eachRuleArray);
			}
		}
		return parseArray;
	};
	vpndirector_rulelist_array = parseNvramToArray('<% nvram_char_to_ascii("", "vpndirector_rulelist"); %>');

	setTimeout("showDropdownClientList('setClientIP', 'ip', 'all', 'ClientList_Block', 'pull_arrow', 'online');", 1000);

	show_vpndirector_rulelist();
	build_interface_list();
	show_vpn_summary(0);
}


function build_interface_list() {
	free_options(document.form.vpn_client_unit);

	$('#iface_x').append("<option value='WAN'>WAN</option>")
	             .append("<option value='OVPN1'>OpenVPN 1: " + get_ovpn_infos(1, 0).desc + "</option>")
	             .append("<option value='OVPN2'>OpenVPN 2: " + get_ovpn_infos(2, 0).desc + "</option>")
	             .append("<option value='OVPN3'>OpenVPN 3: " + get_ovpn_infos(3, 0).desc + "</option>")
	             .append("<option value='OVPN4'>OpenVPN 4: " + get_ovpn_infos(4, 0).desc + "</option>")
	             .append("<option value='OVPN5'>OpenVPN 5: " + get_ovpn_infos(5, 0).desc + "</option>")
	             .val('WAN');
}


function get_ovpn_infos(unit, refresh) {
	if (refresh) {
		var vpnstate = httpApi.nvramGet(["vpn_client1_state", "vpn_client2_state", "vpn_client3_state", "vpn_client4_state", "vpn_client5_state"], true);
	} else {
		var vpnstate = Array();
		vpnstate["vpn_client1_state"] = "<% nvram_get("vpn_client1_state"); %>";
		vpnstate["vpn_client2_state"] = "<% nvram_get("vpn_client2_state"); %>";
		vpnstate["vpn_client3_state"] = "<% nvram_get("vpn_client3_state"); %>";
		vpnstate["vpn_client4_state"] = "<% nvram_get("vpn_client4_state"); %>";
		vpnstate["vpn_client5_state"] = "<% nvram_get("vpn_client5_state"); %>";
	}

	switch (unit) {
		case 1:
			ovpn_info.desc = "<% nvram_get("vpn_client1_desc"); %>";
			ovpn_info.routing = "<% nvram_get("vpn_client1_rgw"); %>";
			ovpn_info.enforce = "<% nvram_get("vpn_client1_enforce"); %>";
			ovpn_info.state = vpnstate.vpn_client1_state;
			break;
                case 2:
			ovpn_info.desc = "<% nvram_get("vpn_client2_desc"); %>";
			ovpn_info.routing = "<% nvram_get("vpn_client2_rgw"); %>";
			ovpn_info.enforce = "<% nvram_get("vpn_client2_enforce"); %>";
			ovpn_info.state = vpnstate.vpn_client2_state;
			break;
                case 3:
			ovpn_info.desc = "<% nvram_get("vpn_client3_desc"); %>";
			ovpn_info.routing = "<% nvram_get("vpn_client3_rgw"); %>";
			ovpn_info.enforce = "<% nvram_get("vpn_client3_enforce"); %>";
			ovpn_info.state = vpnstate.vpn_client3_state;
			break;
                case 4:
			ovpn_info.desc = "<% nvram_get("vpn_client4_desc"); %>";
			ovpn_info.routing = "<% nvram_get("vpn_client4_rgw"); %>";
			ovpn_info.enforce = "<% nvram_get("vpn_client4_enforce"); %>";
			ovpn_info.state = vpnstate.vpn_client4_state;
			break;
                case 5:
			ovpn_info.desc = "<% nvram_get("vpn_client5_desc"); %>";
			ovpn_info.routing = "<% nvram_get("vpn_client5_rgw"); %>";
			ovpn_info.enforce = "<% nvram_get("vpn_client5_enforce"); %>";
			ovpn_info.state = vpnstate.vpn_client5_state;
			break;
		default:
			ovpn_info.desc = "";
			ovpn_info.routing = "";
			ovpn_info.enforce = "";
			ovpn_info.state = "";
	}

	return ovpn_info;
}


function setClientIP(num){
	document.getElementById('localIP_x').value = num;
	hideClients_Block();
}


function pullLANIPList(obj){
	var element = document.getElementById('ClientList_Block');
	var isMenuopen = element.offsetWidth > 0 || element.offsetHeight > 0;
	if (isMenuopen == 0) {
		obj.src = "/images/arrow-top.gif"
		element.style.display = 'block';
		document.getElementById("localIP_x").focus();
	}
	else
		hideClients_Block();
}


function hideClients_Block(){
	document.getElementById("pull_arrow").src = "/images/arrow-down.gif";
	document.getElementById('ClientList_Block').style.display = 'none';
}


function del_Row(_this){
	if (!confirm("Are you sure you want to delete this rule?"))
		return false;

	var row_idx = $(_this).closest("*[row_tr_idx]").attr( "row_tr_idx" );
	var interface = vpndirector_rulelist_array[row_idx].interface;
	var state = vpndirector_rulelist_array[row_idx].state;

	vpndirector_rulelist_array.splice(row_idx, 1);

	show_vpndirector_rulelist();
}


function show_vpn_summary(refresh) {
	var code = '<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable_table">';
	code += '<thead><tr><td colspan="4">OpenVPN clients status</td></tr></thead>';

	for (i = 1; i<= 5; i++) {
		get_ovpn_infos(i, refresh);
		switch (ovpn_info.routing) {
			case "0":
				var routing = "No Internet traffic";
				break;
			case "1":
				var routing = "Redirect all";
				if (ovpn_info.enforce == 1)
					routing += ' + killswitch';
				break;
			case "2":
				var routing = '<span class="hint-color">VPN Director</span>';
				if (ovpn_info.enforce == 1)
					routing += ' + <span class="hint-color" ">killswitch</span>';
				break;
			default:
				var routing = "unknown";
				break;
		}

		code += '<tr><th style="text-align:left; padding-left:10px;">OVPN' + i + ': ' + ovpn_info.desc + '</th>';
		code += '<td style="text-align:left; padding-left:10px;">' + ( ovpn_info.state == 2 ? '<span class="hint-color">Connected</span>' : 'Disconnected') + '</td>';
		code +=	'<td style="text-align:left; padding-left:10px;">' + routing + '</td>';
		if (ovpn_info.state == 2)
			code += '<td style="text-align:left; padding-left:10px;"><span onclick="stop_client(\''+i+'\', this);" style="text-decoration:underline; cursor:pointer;">Stop Client</span></td></tr>';
		else if ((ovpn_info.state == 0) || (ovpn_info.state == -1))
                        code += '<td style="text-align:left; padding-left:10px;"><span onclick="start_client(\''+i+'\', this);" style="text-decoration:underline; cursor:pointer;">Start Client</span></td></tr>';
		else
			code += '<td>...&nbsp;</td></tr>';
	}
	code += "</table>";

	document.getElementById("vpn_status_Block").innerHTML = code;
}

function stop_client(unit, _this) {
	_this.outerHTML = '<img id="SearchingIcon" src="/images/InternetScan.gif">';
	var obj = {
		"action_mode": "apply",
	}

	obj["rc_service"] = "stop_vpnclient" + unit;
	httpApi.nvramSet(obj);

	setTimeout("show_vpn_summary(1)", 5000);
}

function start_client(unit, _this) {
	_this.outerHTML = '<img id="SearchingIcon" src="/images/InternetScan.gif">';
	var obj = {
		"action_mode": "apply",
	}
	obj["rc_service"] = "start_vpnclient" + unit;
	httpApi.nvramSet(obj);

	setTimeout("show_vpn_summary(1)", 5000);
	// Some clients take longer to start, so refresh a second time
	setTimeout("show_vpn_summary(1)", 10000);
}


function show_vpndirector_rulelist() {
	var code = "";
	code += '<table width="100%" cellspacing="0" cellpadding="1" align="center" class="list_table" style="word-break:break-word;">';
	if (vpndirector_rulelist_array.length == 0)
		code += '<tr><td class="hint-color" colspan="6">No rules</td></tr>';
	else {
		vpndirector_rulelist_array.sort(sort_by_interface);

		for (var i = 0; i < vpndirector_rulelist_array.length; i++) {
			var rule = vpndirector_rulelist_array[i];
			code += '<tr row_tr_idx="' + i + '">';
			code += '<td width="9%">' + (rule.enable == "1" ? '<img title="Enabled" src="/images/New_ui/enable.svg"' :
			        '<img title="Disabled" src="/images/New_ui/disable.svg"') +
			        'onMouseOver="EnableMouseOver(this, \'' + rule.enable +'\');" onMouseOut="EnableMouseOut(this, \'' + rule.enable + '\');" onclick="toggleState(this);" style="width:25px; height:25px; cursor:pointer;"></td>';
			code += '<td width="25%">' + rule.description + '</td>';
			code += '<td width="22%">' + rule.localIP + '</td>';
			code += '<td width="22%">' + rule.remoteIP + '</td>';
			code += '<td width="10%">' + rule.interface + '</td>';
			code += '<td width="12%"><input class="edit_btn" onclick="editRule(\'edit\', this);" value=""/>';
			code += '<input class="remove_btn" onclick="del_Row(this);" value=""/></td>';
			code += '</tr>';
		}
	}
	code += '</table>';
	document.getElementById("vpndirector_rulelist_Block").innerHTML = code;	     
}


function sort_by_interface(a, b) {
	if (a.interface == b.interface)
		return 0;

	if (a.interface == "WAN")
		return -1;
	if (b.interface == "WAN")
		return 1;

	if (a.interface > b.interface)
		return 1;
	if (a.interface < b.interface)
		return -1;

	return 0;
}


function parseArrayToNvram(_dataArray) {
	var tmp_value = "";

	for (var i = 0; i < _dataArray.length; i++) {
		tmp_value += "<";
		var enable = ((_dataArray[i]["enable"] != undefined) ? _dataArray[i]["enable"] : "");
		tmp_value += enable + ">";
		var description = ((_dataArray[i]["description"] != undefined) ? _dataArray[i]["description"] : "");
		tmp_value += description + ">";
		var localIP = ((_dataArray[i]["localIP"] != undefined) ? _dataArray[i]["localIP"] : "");
		tmp_value += localIP + ">";
		var remoteIP = ((_dataArray[i]["remoteIP"] != undefined) ? _dataArray[i]["remoteIP"] : "");
		tmp_value += remoteIP + ">";
		var interface = ((_dataArray[i]["interface"] != undefined) ? _dataArray[i]["interface"] : "WAN");
		tmp_value += interface;
	}
	return tmp_value;
}


function toggleState(_this) {
	var row_idx = $(_this).closest("*[row_tr_idx]").attr( "row_tr_idx" );

	if (vpndirector_rulelist_array[row_idx].enable == "1")
		vpndirector_rulelist_array[row_idx].enable = "0";
	else
		vpndirector_rulelist_array[row_idx].enable = "1";

	show_vpndirector_rulelist();
}


function EnableMouseOver(obj, enable) {
	if (enable == "1")
		obj.src = "images/New_ui/enable_hover.svg";
	else
		obj.src = "images/New_ui/disable_hover.svg";
}


function EnableMouseOut(obj, enable) {
	if (enable == "1")
		obj.src = "images/New_ui/enable.svg";
	else
		obj.src = "images/New_ui/disable.svg";
}


function editRule(_mode, _this) {
	if (_mode == "new" && vpndirector_rulelist_array.length > ruleMaxNum) {
		alert("<#JS_itemlimit1#> " + ruleMaxNum + " <#JS_itemlimit2#>");
		return false;
	}

	$("#iface_x").prop("selectedIndex", 0);
	$("#enable_x").prop("checked", true);
	$("#desc_x").val("");
	$("#localIP_x").val("");
	$("#remoteIP_x").val("");

	$("#saveRule").unbind("click");
	$("#rule_setting").fadeIn(300);
	adjust_panel_block_top("rule_setting", 100);
	cal_panel_block("rule_setting", 0.25);

	if (_mode == "edit") {
		var row_idx = $(_this).closest("*[row_tr_idx]").attr( "row_tr_idx" );

		$("#iface_x").val(vpndirector_rulelist_array[row_idx].interface);
		$("#enable_x").prop("checked", (vpndirector_rulelist_array[row_idx].enable == "1"));
		$("#desc_x").val(vpndirector_rulelist_array[row_idx].description);
		$("#localIP_x").val(vpndirector_rulelist_array[row_idx].localIP);
		$("#remoteIP_x").val(vpndirector_rulelist_array[row_idx].remoteIP);
		$("#saveRule").click(
			function() {
				saveRule("edit", row_idx);
			}
		);
	}
	else if (_mode == "new") {
		$("#saveRule").click(
			function() {
				saveRule("new");
			}
		);
	}
}


function saveRule(_mode, _rowIdx) {
	if (!Block_chars(document.getElementById("desc_x"), ["<" ,">" ,"'" ,"%"]))
		return false;

	if ( $("#localIP_x").val() == "" && $("#remoteIP_x").val() == "") {
		alert("You need to specify either a local or remote address!");
		return false;
	}

	if (!validator.ipv4cidr(document.getElementById("remoteIP_x")) ||
	    !validator.ipv4cidr(document.getElementById("localIP_x"))) {
		return false;
	}

	var ruleArray = new Array();
	ruleArray["enable"] = ($("#enable_x").prop("checked") ? 1 : 0);
	ruleArray["description"] = ($("#desc_x").val() != undefined ? $("#desc_x").val() : "");
	ruleArray["localIP"] = ($("#localIP_x").val() != undefined ? $("#localIP_x").val() : "");
	ruleArray["remoteIP"] = ($("#remoteIP_x").val() != undefined ? $("#remoteIP_x").val() : "");
	ruleArray["interface"] = $("#iface_x").val();

	if (_mode == "new")
		vpndirector_rulelist_array.push(ruleArray);
	else if (_mode == "edit") {
		vpndirector_rulelist_array[_rowIdx] = ruleArray;
	}

	show_vpndirector_rulelist();

	$("#rule_setting").fadeOut(300);
}


function cancelRule() {
	$("#rule_setting").fadeOut(300);
}


function applyRule() {
	document.form.vpndirector_rulelist.value = parseArrayToNvram(vpndirector_rulelist_array);
	if (document.form.vpndirector_rulelist.value.length > 7999) {
		alert("Resulting ruleset is too long (over 7999 characters total), please remove some rules, or use shorter descriptions.");
		return false;
	}
	showLoading();
	document.form.submit();
}

</script>
</head>

<body onload="initial();" onunLoad="return unload_body();" class="bg">
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>
<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" action="/start_apply.htm" target="hidden_frame" >
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="Advanced_VPNDirector.asp">
<input type="hidden" name="next_page" value="Advanced_VPNDirector.asp">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_wait" value="5">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="restart_vpnrouting0">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="vpndirector_rulelist" value="">
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
			<table width="98%" border="0" align="left" cellpadding="0" cellspacing="0">
				<tr>
					<td valign="top" >
						<table width="760px" border="0" cellpadding="4" cellspacing="0" class="FormTitle" id="FormTitle">
							<tbody>
							<tr>
								<td bgcolor="#4D595D" valign="top">
								<div>&nbsp;</div>
								<div class="formfonttitle">VPN Director</div>
								<div style="margin:10px 0 10px 5px;" class="splitLine"></div>
								<div id="page_title" class="formfontdesc" style="margin-bottom:0px;">VPN Director allows you to direct LAN traffic through specific OpenVPN tunnels.
									<ul>
										<li>OpenVPN clients set to redirect all traffic have the highest priority</li>
										<li>WAN rules will have priority over OpenVPN rules</li>
										<li>OpenVPN 1 rules have higher priority than OpenVPN 5 rules</li>
										<li>Rules can be individually enabled or disabled by clicking on the first column</li>
									</ul>
								</div>
								<div id="vpn_status_Block"></div>
								<br>
								<div class="addRuleFrame">
									<div class="addRuleText">Add new rule ( <#List_limit#> 199 )</div>
									<div class="add_btn" style="float:left;" onclick="editRule('new')"></div>
								</div>

								<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable_table" style="word-break:break-word;">
									<tr>
										<th width="9%">Enable</th>
										<th width="25%">Description</th>
										<th width="22%">Local IP</th>
										<th width="22%">Remote IP</th>
										<th width="10%">Iface</th>
										<th width="12%">Edit</th>
									</tr>
								</table>
	
								<div id="vpndirector_rulelist_Block"></div>

								<div class="apply_gen">
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
<div id="rule_setting"  class="contentM_qis pop_div_bg">
	<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable">
		<thead>
			<tr>
				<td colspan="2"><#vpn_openvpn_CustomConf#></td>
			</tr>
		</thead>
		<tr>
			<th>Interface</th>
			<td>
				<select id="iface_x" class="input_option">
				</select>
			</td>
		</tr>
		<tr>
			<th>Enable</th>
			<td>
				<input id="enable_x" type="checkbox" id="enable_x" checked="1"/>
			</td>
		</tr>
			<th>Description</th>
			<td>
				<input type="text" maxlength="32" class="input_25_table" id="desc_x" onKeyPress="return validator.isString(this, event);" autocomplete="off" autocorrect="off" autocapitalize="off"/>
				<span><#feedback_optional#></span>
			</td>
		</tr>
		<tr>
			<th>Local IP</th>
			<td>
				<input type="text" maxlength="18" class="input_18_table" id="localIP_x" align="left" onKeyPress="return validator.isIPAddrPlusNetmask(this, event)" style="float:left;" onClick="hideClients_Block();" autocomplete="off" autocorrect="off" autocapitalize="off">
				<img id="pull_arrow" class="pull_arrow" height="16px;" src="images/arrow-down.gif" align="right" onclick="pullLANIPList(this);" title="<#select_IP#>">
				<div id="ClientList_Block" class="clientlist_dropdown" style="margin-left:2px;margin-top:27px;width:238px;"></div>
				<span style="margin-left:3px; line-height:25px;"><#feedback_optional#></span>
			</td>
		</tr>
		<tr>
			<th>Remote IP</th>
			<td>
				<input type="text" maxlength="18" class="input_18_table" id="remoteIP_x" onKeyPress="return validator.isIPAddrPlusNetmask(this, event)" autocomplete="off" autocorrect="off" autocapitalize="off"/>
				<span><#feedback_optional#></span>
			</td>
		</tr>
	</table>
	<div class="hint-color" style="margin:10px 0px;">
		* IP addresses can be entered in CIDR format (for example, 192.168.1.0/24).
		<br>
	</div>
	<div style="margin-top:15px;text-align:center;">
		<input class="button_gen" type="button" onclick="cancelRule();" value="<#CTL_Cancel#>">
		<input id="saveRule" class="button_gen" type="button" value="<#CTL_ok#>">
	</div>
</div>
<div id="footer"></div>
</form>
</body>
</html>
