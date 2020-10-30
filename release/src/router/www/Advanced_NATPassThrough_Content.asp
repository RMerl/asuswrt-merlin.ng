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
<title><#Web_Title#> - NAT Pass-Through</title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/help.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script language="JavaScript" type="text/javascript" src="/validator.js"></script>
<script language="JavaScript" type="text/javascript" src="js/jquery.js"></script>
<script language="JavaScript" type="text/javascript" src="js/httpApi.js"></script>
<script>function initial(){
	show_menu();
	update_pppoerelay_option();
	update_sip_alg_mode_option();
}

function update_pppoerelay_option(){
	var wans_dualwan_array = '<% nvram_get("wans_dualwan"); %>'.split(" ");
	var wans_mode = '<%nvram_get("wans_mode");%>';
	if(document.form.fw_pt_pppoerelay.value == "0" || !dualWAN_support || wans_mode != "lb" || wans_dualwan_array.indexOf("none") != -1){
		document.getElementById("pppoerelay_unit_th").style.display = "none";
		document.getElementById("pppoerelay_unit_td").style.display = "none";
	}else{
		document.getElementById("pppoerelay_unit_th").style.display = "";
		document.getElementById("pppoerelay_unit_td").style.display = "";
	}
}

function update_sip_alg_mode_option(){
	document.getElementById("fw_pt_sip_mode_th").style.display = "none";
	document.getElementById("fw_pt_sip_mode_td").style.display = "none";
	if(based_modelid != "BRT-AC828")
		return;

	if(document.form.fw_pt_sip.value == "1"){
		document.getElementById("fw_pt_sip_mode_th").style.display = "";
		document.getElementById("fw_pt_sip_mode_td").style.display = "";
	}
}

function applyRule(){
	if(usb_support){
		if(!validator.numberRange(document.form.vts_ftpport, 1, 65535))
			return false;
	}
	if(document.form.fw_pt_sip.value == "1" && httpApi.nvramGet(["fw_pt_sip"], true).fw_pt_sip == "0") {
		document.form.action_script.value = "restart_net_and_phy";
		document.form.action_wait.value = "30";
	}
	showLoading();
	document.form.submit();	
}
</script>
</head>

<body onload="initial();" class="bg">
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>
<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
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
			<input type="hidden" name="current_page" value="Advanced_NATPassThrough_Content.asp">
			<input type="hidden" name="next_page" value="Advanced_NATPassThrough_Content.asp">
			<input type="hidden" name="modified" value="0">
			<input type="hidden" name="action_mode" value="apply">
			<input type="hidden" name="action_wait" value="5">
			<input type="hidden" name="action_script" value="restart_firewall;restart_pppoe_relay">
			<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
			<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
			<input type="hidden" name="wl_ssid" value="<% nvram_get("wl_ssid"); %>">

			<table width="98%" border="0" align="left" cellpadding="0" cellspacing="0">
				<tr>
					<td valign="top" >
						<table width="760px" border="0" cellpadding="4" cellspacing="0" class="FormTitle" id="FormTitle">
							<tbody>
								<tr>
								  <td bgcolor="#4D595D" valign="top"  >
								  <div>&nbsp;</div>
								  <div class="formfonttitle"><#menu5_3#> - <#NAT_passthrough_itemname#></div>
								  <div style="margin:10px 0 10px 5px;" class="splitLine"></div>
								  <div class="formfontdesc"><#NAT_passthrough_desc#></div>
									<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
										
										<tr>
											<th><#NAT_PPTP_Passthrough#></th>
											<td>
												<select name="fw_pt_pptp" class="input_option">
													<option class="content_input_fd" value="0" <% nvram_match("fw_pt_pptp", "0","selected"); %>><#btn_disable#></option>
													<option class="content_input_fd" value="1"<% nvram_match("fw_pt_pptp", "1","selected"); %>><#WLANConfig11b_WirelessCtrl_button1name#></option>
												</select>			
											</td>
									  </tr>
							
										<tr>
											<th><#NAT_L2TP_Passthrough#></th>
											<td>
												<select name="fw_pt_l2tp" class="input_option">
													<option class="content_input_fd" value="0" <% nvram_match("fw_pt_l2tp", "0","selected"); %>><#btn_disable#></option>
													<option class="content_input_fd" value="1"<% nvram_match("fw_pt_l2tp", "1","selected"); %>><#WLANConfig11b_WirelessCtrl_button1name#></option>
												</select>			
											</td>
									  </tr>
							
										<tr>
											<th><#NAT_IPSec_Passthrough#></th>
											<td>
												<select name="fw_pt_ipsec" class="input_option">
													<option class="content_input_fd" value="0" <% nvram_match("fw_pt_ipsec", "0","selected"); %>><#btn_disable#></option>
													<option class="content_input_fd" value="1"<% nvram_match("fw_pt_ipsec", "1","selected"); %>><#WLANConfig11b_WirelessCtrl_button1name#></option>
												</select>			
											</td>
									  </tr>

						<tr>
  	         					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(50,14);"><#NAT_RTSP_Passthrough#></a></th>
    	       					<td>
												<select name="fw_pt_rtsp" class="input_option">
													<option class="content_input_fd" value="0" <% nvram_match("fw_pt_rtsp", "0","selected"); %>><#btn_disable#></option>
													<option class="content_input_fd" value="2"<% nvram_match("fw_pt_rtsp", "2","selected"); %>><#WLANConfig11b_WirelessCtrl_button1name#></option>
													<option class="content_input_fd" value="1"<% nvram_match("fw_pt_rtsp", "1","selected"); %>>Enabled + NAT helper</option>
												</select>			
        	    				</td>
           					</tr>

						<tr>
							<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(50,14);"><#NAT_H323_Passthrough#></a></th>
						<td>
												<select name="fw_pt_h323" class="input_option">
													<option class="content_input_fd" value="0" <% nvram_match("fw_pt_h323", "0","selected"); %>><#btn_disable#></option>
													<option class="content_input_fd" value="2"<% nvram_match("fw_pt_h323", "2","selected"); %>><#WLANConfig11b_WirelessCtrl_button1name#></option>
													<option class="content_input_fd" value="1"<% nvram_match("fw_pt_h323", "1","selected"); %>>Enabled + NAT helper</option>
												</select>
						</td>
						</tr>

						<tr>
							<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(50,14);"><#NAT_SIP_Passthrough#></a></th>
						<td>
												<select name="fw_pt_sip" class="input_option" OnChange="update_sip_alg_mode_option();">
													<option class="content_input_fd" value="0" <% nvram_match("fw_pt_sip", "0","selected"); %>><#btn_disable#></option>
													<option class="content_input_fd" value="2"<% nvram_match("fw_pt_sip", "2","selected"); %>><#WLANConfig11b_WirelessCtrl_button1name#></option>
													<option class="content_input_fd" value="1"<% nvram_match("fw_pt_sip", "1","selected"); %>>Enabled + NAT helper</option>
												</select>
						</td>
						<tr>
										<th id="fw_pt_sip_mode_th">SIP Passthrough mode<!--untranslated--></th>
										<td id="fw_pt_sip_mode_td">
												<select name="fw_pt_sip_mode" class="input_option">
													<option class="content_input_fd" value="0" <% nvram_match("fw_pt_sip_mode", "0","selected"); %>>Original<!--untranslated--></option>
													<option class="content_input_fd" value="1"<% nvram_match("fw_pt_sip_mode", "1","selected"); %>>Cisco<!--untranslated--></option>
												</select>
										</td>
						</tr>
						</tr>

										<tr>
							<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,11);"><#PPPConnection_x_PPPoERelay_itemname#></a></th>
    	       					<td>
												<select name="fw_pt_pppoerelay" class="input_option" onChange="update_pppoerelay_option();">
													<option class="content_input_fd" value="0" <% nvram_match("fw_pt_pppoerelay", "0","selected"); %>><#btn_disable#></option>
													<option class="content_input_fd" value="1"<% nvram_match("fw_pt_pppoerelay", "1","selected"); %>><#WLANConfig11b_WirelessCtrl_button1name#></option>
												</select>
						</td>
						</tr>
						<tr>
										<th id="pppoerelay_unit_th">PPPoE Relay interface<!--untranslated--></th>
										<td id="pppoerelay_unit_td">
												<select name="pppoerelay_unit" class="input_option">
													<option class="content_input_fd" value="0" <% nvram_match("pppoerelay_unit", "0","selected"); %>><#dualwan_primary#></option>
													<option class="content_input_fd" value="1"<% nvram_match("pppoerelay_unit", "1","selected"); %>><#dualwan_secondary#></option>
												</select>
										</td>
										</tr>
										<tr>
											<th><#FTP_ALG_port#></th>
											<td>
												<input type="text" maxlength="5" name="vts_ftpport" class="input_6_table" value="<% nvram_get("vts_ftpport"); %>" onkeypress="return validator.isNumber(this,event);" autocorrect="off" autocapitalize="off">
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
    <td width="10" align="center" valign="top">&nbsp;</td>
	</tr>
</table>
</form>
<div id="footer"></div>
</body>
</html>
