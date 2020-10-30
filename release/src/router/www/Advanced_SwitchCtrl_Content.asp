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
<title><#Web_Title#> - <#Switch_itemname#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="other.css">
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>

<script>
var lacp_support = isSupport("lacp");
var lacp_enabled = '<% nvram_get("lacp_enabled"); %>' == 1 ?true: false;
var bonding_policy_support = false;
if( lacp_support
&& (based_modelid == "GT-AC5300" || based_modelid == "RT-AC86U" || based_modelid == "GT-AC2900" || based_modelid == "RT-AC87U" || based_modelid == "RT-AC5300" || based_modelid == "RT-AC88U" || based_modelid == "RT-AC3100")){
	bonding_policy_support = true;
	var bonding_policy_value = '<% nvram_get("bonding_policy"); %>';
}

var jumbo_frame_enable_ori = '<% nvram_get("jumbo_frame_enable"); %>';
var ctf_disable_force_ori = '<% nvram_get("ctf_disable"); %>';
var lacp_enabled_ori = '<% nvram_get("lacp_enabled"); %>';
var wans_lanport = '<% nvram_get("wans_lanport"); %>';
var iptv_port_settings_orig = '<%nvram_get("iptv_port_settings"); %>' == ""? "12": '<%nvram_get("iptv_port_settings"); %>';
var switch_wantag_orig = '<% nvram_get("switch_wantag"); %>';
var switch_stb_x_orig = '<% nvram_get("switch_stb_x"); %>';

function disable_lacp_if_conflicts_with_iptv(){
	if((based_modelid == "RT-AX89U" || based_modelid == "GT-AXY16000")){
		// LAN1 and/or LAN2.
		if(switch_wantag_orig == "none" && (switch_stb_x_orig == 1 || switch_stb_x_orig == 2 || switch_stb_x_orig == 5)){
			var note_str = "This function is disabled because LAN1 or LAN2 is configured as IPTV STB port."; //untranslated
			document.form.lacp_enabled.style.display = "none";
			document.getElementById("lacp_note").innerHTML = note_str;
			document.getElementById("lacp_desc").style.display = "";
			document.form.lacp_enabled.value = "0";
		}
	}
}

function initial(){
	if(qca_support){
		var nataccel = '<% nvram_get("qca_sfe"); %>';
		var nataccel_status = '<% nat_accel_status(); %>';

		if(nataccel == '1' && nataccel_status == '1'){
			document.getElementById("natAccelDesc").innerHTML = "<#NAT_Acceleration_enable#>";
		}
		else{
			document.getElementById("natAccelDesc").innerHTML = "<#NAT_Acceleration_ctf_disable#>";
		}
	}
	else{
		var ctf_disable = '<% nvram_get("ctf_disable"); %>';
		var ctf_fa_mode = '<% nvram_get("ctf_fa_mode"); %>';

		if(ctf_disable == 1){
			document.getElementById("ctfLevelDesc").innerHTML = "<#NAT_Acceleration_ctf_disable#>";
		}
		else{
			if(ctf_fa_mode == '2')
				document.getElementById("ctfLevelDesc").innerHTML = "<#NAT_Acceleration_ctf_fa_mode2#>";
			else
				document.getElementById("ctfLevelDesc").innerHTML = "<#NAT_Acceleration_ctf_fa_mode1#>";
		}
	}

	show_menu();

	if(lacp_support){
		document.getElementById("lacp_tr").style.display = "";
		document.form.lacp_enabled.disabled = false;
		if(lacp_enabled){
			document.getElementById("lacp_desc").style.display = "";
			if(bonding_policy_support){
				document.form.bonding_policy.value = bonding_policy_value;
				check_bonding_policy(document.form.lacp_enabled);
			}
		}
		else
			document.getElementById("lacp_desc").style.display = "none";
	}
	else{
		document.form.lacp_enabled.disabled = true;
		document.form.bonding_policy.disabled = true;
	}

	if(qca_support){
		if(lyra_hide_support){
			document.getElementById("jumbo_tr").style.display = "none";
			document.form.jumbo_frame_enable.disabled = true;
		}

		if(wifison_ready != "1" && sw_mode == "1"){
			document.getElementById("qca_tr").style.display = "";
			document.form.qca_sfe.disabled = false;
		}
	}
	else{
		//MODELDEP
		if(based_modelid == "GT-AC5300"){
			var new_str = "";
			new_str = document.getElementById("lacp_note").innerHTML.replace(/LAN1/g, "LAN5");
			document.getElementById("lacp_note").innerHTML = new_str.replace(/LAN2/g, "LAN6");
		}

		if(hnd_support){
			document.getElementById("ctf_tr").style.display = "none";
			document.form.ctf_disable_force.disabled = true;
		}
		else{
			if(sw_mode == "1" || sw_mode == "5"){
				document.getElementById("ctf_tr").style.display = "";
				document.form.ctf_disable_force.disabled = false;
			}
		}
	}

	if(lacp_support && wans_dualwan_array.indexOf("lan") != -1){
		var wan_lanport_text = "";
		if(based_modelid == "GT-AC5300")
			var bonding_port_settings = [{"val": "4", "text": "LAN5"}, {"val": "3", "text": "LAN6"}];
		else if(based_modelid == "RT-AC86U" || based_modelid == "GT-AC2900")
			var bonding_port_settings = [{"val": "4", "text": "LAN1"}, {"val": "3", "text": "LAN2"}];
		else
			var bonding_port_settings = [{"val": "1", "text": "LAN1"}, {"val": "2", "text": "LAN2"}];

		for(var i = 0; i < bonding_port_settings.length; i++){
			if(wans_lanport == bonding_port_settings[i].val){
				wan_lanport_text = bonding_port_settings[i].text.toUpperCase();
			}
		}

		if(wan_lanport_text!= ""){
			var note_str = "This function is disabled because " + wan_lanport_text + " is configured as WAN. If you want to enable it, please click <a href=\"http://router.asus.com/Advanced_WANPort_Content.asp\" target=\"_blank\" style=\"text-decoration:underline;\">here</a> to change dual wan settings."; //untranslated
			document.form.lacp_enabled.style.display = "none";
			document.getElementById("lacp_note").innerHTML = note_str;
			document.getElementById("lacp_desc").style.display = "";
			document.form.lacp_enabled.disabled = true;
		}
	}

	disable_lacp_if_conflicts_with_iptv();
}

function applyRule(){
	var setting_changed = false;
	if((jumbo_frame_enable_ori != document.form.jumbo_frame_enable.value)
	|| (ctf_disable_force_ori != document.form.ctf_disable_force.value)
	|| (lacp_enabled_ori != document.form.lacp_enabled.value) ){
		setting_changed = true
	}

	if(based_modelid == "GT-AC5300" && (lacp_enabled_ori != document.form.lacp_enabled.value)){
		var msg = "Enable Bonding/ Link aggregation will change settings of IPTV/ VOIP ports on IPTV page to LAN1/ LAN2. Are you sure to do it?";//untranslated
		if(document.form.lacp_enabled.value == "1" && iptv_port_settings_orig == "56"){
			if(confirm(msg)){
				document.form.iptv_port_settings.disabled = false;
				document.form.iptv_port_settings.value = "12";
			}
			else{
				document.form.lacp_enabled.value = "0";
				check_bonding_policy(document.form.lacp_enabled);
				document.form.lacp_enabled.focus();
				return;
			}
		}
	}

	if(!setting_changed){	// only change the bonding policy
		document.form.action_script.value = "restart_net_and_phy";
		document.form.action_wait.value = "35";
	}

	if(lantiq_support){
		document.form.action_script.value = "restart_wan_if;restart_firewall";
		document.form.action_wait.value = "10";
	
		if(!setting_changed){	// only change the bonding policy
			document.form.action_script.value += ";restart_net_and_phy";
			document.form.action_wait.value = "35";
		}
	}

	showLoading();
	document.form.submit();
}

function check_bonding_policy(obj){
	if(obj.value == "1"){
		if(bonding_policy_support){
			document.getElementById("lacp_policy_tr").style.display = "";
			document.form.bonding_policy.disabled = false;
		}
		
		document.getElementById("lacp_desc").style.display = "";
	}
	else{
		if(bonding_policy_support){
			document.getElementById("lacp_policy_tr").style.display = "none";
			document.form.bonding_policy.disabled = true;
		}

		document.getElementById("lacp_desc").style.display = "none";
	}
}
</script>
</head>

<body onload="initial();" onunLoad="return unload_body();" class="bg">
<div id="TopBanner"></div>
<div id="hiddenMask" class="popup_bg">
	<table cellpadding="5" cellspacing="0" id="dr_sweet_advise" class="dr_sweet_advise" align="center">
		<tr>
		<td>
			<div class="drword" id="drword" style="height:110px;"><#Main_alert_proceeding_desc4#> <#Main_alert_proceeding_desc1#>...
				<br/>
				<br/>
	    </div>
		  <div class="drImg"><img src="images/alertImg.png"></div>
			<div style="height:70px;"></div>
		</td>
		</tr>
	</table>
<!--[if lte IE 6.5]><iframe class="hackiframe"></iframe><![endif]-->
</div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="Advanced_SwitchCtrl_Content.asp">
<input type="hidden" name="next_page" value="Advanced_SwitchCtrl_Content.asp">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="reboot">
<input type="hidden" name="action_wait" value="<% nvram_get("reboot_time"); %>">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="iptv_port_settings" value="<% nvram_get("iptv_port_settings"); %>" disabled>
<table class="content" align="center" cellpadding="0" cellspacing="0">
	<tr>
		<td width="17">&nbsp;</td>
		
		<!--=====Beginning of Main Menu=====-->
		<td valign="top" width="202">
		  <div id="mainMenu"></div>
		  <div id="subMenu"></div>
		</td>
		
	    <td valign="top">
			<div id="tabMenu" class="submenuBlock"></div>
				<!--===================================Beginning of Main Content===========================================-->
			<table width="98%" border="0" align="left" cellpadding="0" cellspacing="0">
				<tr>
					<td align="left" valign="top">
		  				<table width="760px" border="0" cellpadding="5" cellspacing="0" class="FormTitle" id="FormTitle">
							<tbody>
								<tr>
				  					<td bgcolor="#4D595D" valign="top">
				  						<div>&nbsp;</div>
				  						<div class="formfonttitle"><#menu5_2#> - <#Switch_itemname#></div>
		      							<div style="margin:10px 0 10px 5px;" class="splitLine"></div>
										<div class="formfontdesc"><#SwitchCtrl_desc#></div>
										<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
											<tr id="jumbo_tr">
												<th><#jumbo_frame#></th>
												<td>
													<select name="jumbo_frame_enable" class="input_option">
														<option class="content_input_fd" value="0" <% nvram_match("jumbo_frame_enable", "0","selected"); %>><#WLANConfig11b_WirelessCtrl_buttonname#></option>
														<option class="content_input_fd" value="1" <% nvram_match("jumbo_frame_enable", "1","selected"); %>><#WLANConfig11b_WirelessCtrl_button1name#></option>
													</select>
												</td>
											</tr>

											<tr id="ctf_tr" style="display: none;">
		      									<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(29,2);"><#NAT_Acceleration#></a></th>
												<td>
													<select name="ctf_disable_force" class="input_option" disabled>
														<option class="content_input_fd" value="1" <% nvram_match("ctf_disable_force", "1","selected"); %>><#WLANConfig11b_WirelessCtrl_buttonname#></option>
														<option class="content_input_fd" value="0" <% nvram_match("ctf_disable_force", "0","selected"); %>><#Auto#></option>
													</select>
													&nbsp
													<span id="ctfLevelDesc"></span>
												</td>
											</tr>

											<tr id="qca_tr" style="display: none;">
												<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(29,2);"><#NAT_Acceleration#></a></th>
												<td>
													<select name="qca_sfe" class="input_option" disabled>
														<option class="content_input_fd" value="0" <% nvram_match("qca_sfe", "0","selected"); %>><#WLANConfig11b_WirelessCtrl_buttonname#></option>
														<option class="content_input_fd" value="1" <% nvram_match("qca_sfe", "1","selected"); %>><#WLANConfig11b_WirelessCtrl_button1name#></option>
													</select>
												&nbsp
													<span id="natAccelDesc"></span>
												</td>
											</tr>

											<tr style="display:none">
												<th><#SwitchCtrl_Enable_GRO#></th>
												<td>
													<input type="radio" name="gro_disable_force" value="0" <% nvram_match("gro_disable_force", "0", "checked"); %>><#checkbox_Yes#>
													<input type="radio" name="gro_disable_force" value="1" <% nvram_match("gro_disable_force", "1", "checked"); %>><#checkbox_No#>
												</td>
											</tr>       

											<tr>
											<th>Spanning-Tree Protocol</th>
												<td>
													<select name="lan_stp" class="input_option">
														<option class="content_input_fd" value="0" <% nvram_match("lan_stp", "0","selected"); %>><#WLANConfig11b_WirelessCtrl_buttonname#></option>
														<option class="content_input_fd" value="1" <% nvram_match("lan_stp", "1","selected"); %>><#WLANConfig11b_WirelessCtrl_button1name#></option>
													</select>
												</td>
											</tr>

											<tr id="lacp_tr" style="display:none;">
		      									<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(29,1);"><#NAT_lacp#></a></th>
												<td>
													<select name="lacp_enabled" class="input_option"  onchange="check_bonding_policy(this);" disabled>
														<option class="content_input_fd" value="0" <% nvram_match("lacp_enabled", "0","selected"); %>><#WLANConfig11b_WirelessCtrl_buttonname#></option>
														<option class="content_input_fd" value="1" <% nvram_match("lacp_enabled", "1","selected"); %>><#WLANConfig11b_WirelessCtrl_button1name#></option>
													</select>
													<div id="lacp_desc" style="display:none"><span id="lacp_note"><#NAT_lacp_note#></span><div>
												</td>
											</tr> 
											<tr id="lacp_policy_tr" style="display:none">
												<th><#SwitchCtrl_BondingPolicy#></th>
												<td>
													<select name="bonding_policy" class="input_option" disabled>
														<option value="0"><#CTL_Default#></option>
														<option value="1"><#SwitchCtrl_BondingPolicy_src#></option>
														<option value="2"><#SwitchCtrl_BondingPolicy_dest#></option>
													</select>
												</td>
											</tr>
										</table>

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
		</td>
	    <td width="10" align="center" valign="top">&nbsp;</td>
	</tr>
</table>
</form>
<div id="footer"></div>
</body>
</html>
