<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=Edge"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title><#Web_Title#> - WireGuard Server</title>
<link rel="stylesheet" href="index_style.css">
<link rel="stylesheet" href="form_style.css">

<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/validator.js"></script>
<script type="text/javascript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/form.js"></script>
<style>

#wgs_QRCode_block{
	position: absolute;
	width: 230px;
	height: 260px;
	background-color: #444f53;
	padding: 3px 3px;
	margin-top: -40px;
}

</style>

<script>

<% get_wgs_parameter(); %>
<% get_wgsc_parameter(); %>

window.onresize = function() {
	cal_panel_block("wgs_QRCode_block", 0.18);
}
var wgs_enable = '<% nvram_get("wgs_enable"); %>';
var wgsc_enable = '<% nvram_get("wgsc_enable"); %>';
var wgsc_status = <% get_wgsc_status(); %>;

function initial(){
	show_menu();

	if (wgs_enable == '1') {
		document.getElementById("WgsLogTable").style.display = "";
		if (wgsc_enable == '1') {
			document.getElementById("wg_export_setting").style.display = "";
			document.getElementById("WgscConfTable").style.display = "";
		}
	}
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

function change_wgs_unit(unit){
	document.chg_wgs.wgs_unit.value=unit.toString();
	document.chg_wgs.submit();
}

function change_wgsc_unit(unit){
	document.chg_wgsc.wgsc_unit.value=unit.toString();
	document.chg_wgsc.submit();
}

function exportConfig() {
	location.href = 'wgs_client.conf';
}

function showQRCode() {
	$('#wgs_QRCode_block').show();
	cal_panel_block("wgs_QRCode_block", 0.18);
}

function hideQRCode(){
	$('#wgs_QRCode_block').hide();
}

</script>

</head>

<body onload="initial();" onunLoad="return unload_body();" class="bg">

<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="Advanced_WireguardServer_Content.asp">
<input type="hidden" name="next_page" value="Advanced_WireguardServer_Content.asp">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="restart_wgs;restart_dnsmasq">
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
					<div class="formfonttitle">VPN - WireGuard Server</div>
					<div style="margin:10px 0 10px 5px;" class="splitLine"></div>
					<div id="titl_desc" class="formfontdesc">WireGuard Server</div>

					<table id="WgcBasicTable" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable">
						<thead>
							<tr>
								<td colspan="2"><#t2BC#></td>
							</tr>
						</thead>
						<tr id="wgs_unit_field" class="rept ew">
							<th>WireGuard Index</th>
							<td>
								<select name="wgs_unit" class="input_option" onChange="change_wgs_unit(this.value);">
									<option class="content_input_fd" value="1" <% nvram_match("wgs_unit", "1","selected"); %>>1</option>
								</select>
							</td>
						</tr>
						<tr id="wgs_enable">
							<th>Enable WireGuard</th>
							<td>
								<input type="radio" value="1" name="wgs_enable" class="input" <% nvram_match("wgs_enable", "1", "checked"); %>><#checkbox_Yes#></input>
								<input type="radio" value="0" name="wgs_enable" class="input" <% nvram_match("wgs_enable", "0", "checked"); %>><#checkbox_No#></input>
							</td>
						</tr>
						<tr id="wgs_dns">
							<th>Allow DNS</th>
							<td>
								<input type="radio" value="1" name="wgs_dns" class="input" <% nvram_match("wgs_dns", "1", "checked"); %>><#checkbox_Yes#></input>
								<input type="radio" value="0" name="wgs_dns" class="input" <% nvram_match("wgs_dns", "0", "checked"); %>><#checkbox_No#></input>
							</td>
						</tr>
						<tr id="wgs_nat6">
							<th>IPv6 NAT</th>
							<td>
								<input type="radio" value="1" name="wgs_nat6" class="input" <% nvram_match("wgs_nat6", "1", "checked"); %>><#checkbox_Yes#></input>
								<input type="radio" value="0" name="wgs_nat6" class="input" <% nvram_match("wgs_nat6", "0", "checked"); %>><#checkbox_No#></input>
							</td>
						</tr>
						<tr id="wgs_psk">
							<th>Use Preshared Key</th>
							<td>
								<input type="radio" value="1" name="wgs_psk" class="input" <% nvram_match("wgs_psk", "1", "checked"); %>><#checkbox_Yes#></input>
								<input type="radio" value="0" name="wgs_psk" class="input" <% nvram_match("wgs_psk", "0", "checked"); %>><#checkbox_No#></input>
							</td>
						</tr>
						<tr>
							<th>Persistent Keepalive</th>
							<td>
								<input type="text" maxlength="5" name="wgs_alive" id="wgs_alive" class="input_6_table" onKeyPress="return validator.isNumber(this,event);" value="<% nvram_get("wgs_alive"); %>" autocorrect="off" autocapitalize="off"></input>
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
								<div id="wgs_priv" name="wgs_priv" style="color:#FFFFFF;margin-left:8px;"><% nvram_get("wgs_priv"); %></div>
							</td>
						</tr>
						<tr>
							<th>Public Key</th>
							<td>
								<div id="wgs_pub" name="wgs_pub" style="color:#FFFFFF;margin-left:8px;"><% nvram_get("wgs_pub"); %></div>
							</td>
						</tr>
						<tr>
							<th>Address</th>
							<td>
								<input type="text" maxlength="63" name="wgs_addr" id="wgs_addr" class="input_32_table" value="<% nvram_get("wgs_addr"); %>" autocorrect="off" autocapitalize="off"></input>
							</td>
						</tr>
						<tr>
							<th>Listen Port</th>
							<td>
								<input type="text" maxlength="5" name="wgs_port" id="wgs_port" class="input_6_table" onKeyPress="return validator.isNumber(this,event);" value="<% nvram_get("wgs_port"); %>" autocorrect="off" autocapitalize="off"></input>
							</td>
						</tr>
					</table>

					<table id="WgcPeerTable" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable">
						<thead>
							<tr>
								<td colspan="2">Peer</td>
							</tr>
						</thead>
						<tr id="wgsc_unit_field" class="rept ew">
							<th>WireGuard Client Index</th>
							<td>
								<select name="wgsc_unit" class="input_option" onChange="change_wgsc_unit(this.value);">
									<option class="content_input_fd" value="1" <% nvram_match("wgsc_unit", "1","selected"); %>>1</option>
									<option class="content_input_fd" value="2" <% nvram_match("wgsc_unit", "2","selected"); %>>2</option>
									<option class="content_input_fd" value="3" <% nvram_match("wgsc_unit", "3","selected"); %>>3</option>
									<option class="content_input_fd" value="4" <% nvram_match("wgsc_unit", "4","selected"); %>>4</option>
									<option class="content_input_fd" value="5" <% nvram_match("wgsc_unit", "5","selected"); %>>5</option>
									<option class="content_input_fd" value="6" <% nvram_match("wgsc_unit", "6","selected"); %>>6</option>
									<option class="content_input_fd" value="7" <% nvram_match("wgsc_unit", "7","selected"); %>>7</option>
									<option class="content_input_fd" value="8" <% nvram_match("wgsc_unit", "8","selected"); %>>8</option>
									<option class="content_input_fd" value="9" <% nvram_match("wgsc_unit", "9","selected"); %>>9</option>
									<option class="content_input_fd" value="10" <% nvram_match("wgsc_unit", "10","selected"); %>>10</option>
								</select>
							</td>
						</tr>
						<tr id="wgsc_enable">
							<th>Enable Client</th>
							<td>
								<input type="radio" value="1" name="wgsc_enable" class="input" <% nvram_match("wgsc_enable", "1", "checked"); %>><#checkbox_Yes#></input>
								<input type="radio" value="0" name="wgsc_enable" class="input" <% nvram_match("wgsc_enable", "0", "checked"); %>><#checkbox_No#></input>
							</td>
						</tr>
						<tr id="wgsc_name" class="rept ew">
							<th>WireGuard Client Name</th>
							<td>
								<input type="text" maxlength="64" name="wgsc_name" id="wgsc_name" class="input_32_table" value="<% nvram_get("wgsc_name"); %>" autocorrect="off" autocapitalize="off"></input>
							</td>
						</tr>
						<tr>
							<th>Preshared Key (Optional)</th>
							<td>
								<div id="wgsc_psk" name="wgsc_psk" style="color:#FFFFFF;margin-left:8px;"><% nvram_get("wgsc_psk"); %></div>
							</td>
						</tr>
						<tr>
							<th>Address</th>
							<td>
								<input type="text" maxlength="63" name="wgsc_addr" id="wgsc_addr" class="input_32_table" value="<% nvram_get("wgsc_addr"); %>" autocorrect="off" autocapitalize="off"></input>
							</td>
						</tr>
						<tr>
							<th>Allowed IPs (Server)</th>
							<td>
								<input type="text" maxlength="1023" name="wgsc_aips" id="wgsc_aips" class="input_32_table" value="<% nvram_get("wgsc_aips"); %>" autocorrect="off" autocapitalize="off"></input>
							</td>
						</tr>
						<tr>
							<th>Allowed IPs (Client)</th>
							<td>
								<input type="text" maxlength="1023" name="wgsc_caips" id="wgsc_aips" class="input_32_table" value="<% nvram_get("wgsc_caips"); %>" autocorrect="off" autocapitalize="off"></input>
							</td>
						</tr>
						<tr id="wg_export_setting" style="display:none">
							<th>Export configuration file</th>
							<td>
								<input class="button_gen" type="button" value="<#btn_Export#>" onClick="exportConfig();"/>
								<input class="button_gen" type="button" value="QR Code" onClick="showQRCode();"/>
							</td>
						</tr>
					</table>

					<div id="wgs_QRCode_block" style="display:none">
						<div style="display:flex; align-items: center;">
							<div style="width:28px;height:28px;background-image:url('images/New_ui/disable.svg');cursor:pointer" onclick="hideQRCode();"></div>
						</div>
						<img src='wgs_client.png'></img>
					</div>

					<div class="apply_gen" id="apply_btn">
						<input class="button_gen" onclick="applyRule();" type="button" value="<#CTL_apply#>"/>
					</div>

					<table id="WgscConfTable" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable" style="display:none">
						<thead>
							<tr>
								<td colspan="2">Content of Client Configuration File</td>
							</tr>
						</thead>
						<tr>
							<td>
								<div style="margin-top:8px">
									<textarea class="textarea_ssh_table" style="width:99%; font-family:'Courier New', Courier, mono; font-size:13px;" cols="63" rows="10" readonly="readonly" wrap=off><% nvram_dump("wgs_client.conf",""); %></textarea>
								</div>
							</td>
						</tr>
					</table>

					<table id="WgsLogTable" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable" style="display:none">
						<thead>
							<tr>
								<td colspan="2">Server Status</td>
							</tr>
						</thead>
						<tr>
							<td>
								<div style="margin-top:8px">
									<textarea class="textarea_ssh_table" style="width:99%; font-family:'Courier New', Courier, mono; font-size:13px;" cols="63" rows="15" readonly="readonly" wrap=off><% nvram_dump("wgs.log",""); %></textarea>
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

<form method="post" name="chg_wgs" action="apply.cgi" target="hidden_frame">
	<input type="hidden" name="action_mode" value="chg_wgs_unit">
	<input type="hidden" name="action_script" value="">
	<input type="hidden" name="action_wait" value="">
	<input type="hidden" name="current_page" value="Advanced_WireguardServer_Content.asp">
	<input type="hidden" name="wgsc_unit" value="">
</form>
<form method="post" name="chg_wgsc" action="apply.cgi" target="hidden_frame">
	<input type="hidden" name="action_mode" value="chg_wgsc_unit">
	<input type="hidden" name="action_script" value="">
	<input type="hidden" name="action_wait" value="">
	<input type="hidden" name="current_page" value="Advanced_WireguardServer_Content.asp">
	<input type="hidden" name="wgsc_unit" value="">
</form>
<div id="footer"></div>
</body>
</html>
