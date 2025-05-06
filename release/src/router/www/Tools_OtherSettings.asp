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
<title><#Web_Title#> - Tweaks</title>
<link rel="stylesheet" type="text/css" href="index_style.css">
<link rel="stylesheet" type="text/css" href="form_style.css">
<script language="JavaScript" type="text/javascript" src="/js/jquery.js"></script>
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/validator.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script language="JavaScript" type="text/javascript" src="/help.js"></script>
<script language="JavaScript" type="text/javascript" src="/tmmenu.js"></script>
<script>

function initial() {
	show_menu();
	initConntrackValues()
}

function initConntrackValues(){

	tcp_array = document.form.ct_tcp_timeout.value.split(" ");

	document.form.tcp_established.value = tcp_array[1];
	document.form.tcp_syn_sent.value = tcp_array[2];
	document.form.tcp_syn_recv.value = tcp_array[3];
	document.form.tcp_fin_wait.value = tcp_array[4];
	document.form.tcp_time_wait.value = tcp_array[5];
	document.form.tcp_close.value = tcp_array[6];
	document.form.tcp_close_wait.value = tcp_array[7];
	document.form.tcp_last_ack.value = tcp_array[8];

	udp_array = document.form.ct_udp_timeout.value.split(" ");

	document.form.udp_unreplied.value = udp_array[0];
	document.form.udp_assured.value = udp_array[1];

}


function applyRule(){
	if (!validator.numberRange(document.form.ct_max, 256, 300000) ||
	    !validator.numberRange(document.form.tcp_established, 1, 432000) ||
	    !validator.numberRange(document.form.tcp_syn_sent, 1, 86400) ||
	    !validator.numberRange(document.form.tcp_syn_recv, 1, 86400) ||
	    !validator.numberRange(document.form.tcp_fin_wait, 1, 86400) ||
	    !validator.numberRange(document.form.tcp_time_wait, 1, 86400) ||
	    !validator.numberRange(document.form.tcp_close, 1, 86400) ||
	    !validator.numberRange(document.form.tcp_close_wait, 1, 86400) ||
	    !validator.numberRange(document.form.tcp_last_ack, 1, 86400) ||
	    !validator.numberRange(document.form.udp_assured, 1, 86400) ||
	    !validator.numberRange(document.form.udp_unreplied, 1, 86400))
		return false;

	showLoading();

	document.form.ct_tcp_timeout.value = "0 "+
		document.form.tcp_established.value +" " +
		document.form.tcp_syn_sent.value +" " +
		document.form.tcp_syn_recv.value +" " +
		document.form.tcp_fin_wait.value +" " +
		document.form.tcp_time_wait.value +" " +
		document.form.tcp_close.value +" " +
		document.form.tcp_close_wait.value +" " +
		document.form.tcp_last_ack.value +" 0";

	document.form.ct_udp_timeout.value = document.form.udp_unreplied.value + " "+document.form.udp_assured.value;

	if (based_modelid != "RT-AC86U") {
		if (getRadioValue(document.form.cstats_enable) != "<% nvram_get("cstats_enable"); %>") {
			if ( (getRadioValue(document.form.cstats_enable) == 1) && ("<% nvram_get("ctf_disable"); %>" == 0) )
				FormActions("start_apply.htm", "apply", "reboot", "<% get_default_reboot_time(); %>");
			else
				document.form.action_script.value += ";restart_firewall;restart_cstats";
		} else {
			document.form.action_script.value += ";restart_cstats";
		}
	}

	if (getRadioValue(document.form.dns_local_cache) != "<% nvram_get("dns_local_cache"); %>")
		document.form.action_script.value += ";restart_dnsmasq";

	document.form.submit();
}


</script>
</head>

<body onload="initial();" onunLoad="return unload_body();" class="bg">
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="current_page" value="Tools_OtherSettings.asp">
<input type="hidden" name="next_page" value="Tools_OtherSettings.asp">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="restart_conntrack">
<input type="hidden" name="action_wait" value="5">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="SystemCmd" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="ct_tcp_timeout" value="<% nvram_get("ct_tcp_timeout"); %>">
<input type="hidden" name="ct_udp_timeout" value="<% nvram_get("ct_udp_timeout"); %>">
<table class="content" align="center" cellpadding="0" cellspacing="0">
  <tr>
    <td width="17">&nbsp;</td>
    <td valign="top" width="202">
      <div id="mainMenu"></div>
      <div id="subMenu"></div></td>
    <td valign="top">
        <div id="tabMenu" class="submenuBlock"></div>

      <!--===================================Beginning of Main Content===========================================-->
      <table width="98%" border="0" align="left" cellpadding="0" cellspacing="0">
        <tr>
          <td valign="top">
            <table width="760px" border="0" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTitle" id="FormTitle">
                <tbody>
                <tr bgcolor="#4D595D">
                <td valign="top">
			<div>&nbsp;</div>
			<div class="formfonttitle"><#menu5_6#> - Tweaks</div>
			<div style="margin:10px 0 10px 5px;" class="splitLine"></div>
				<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable" style="margin-top:8px;">
					<thead>
						<tr>
							<td colspan="2">TCP/IP settings</td>
						</tr>
					</thead>
					<tr>
						<th>TCP connections limit</th>
						<td>
							<input type="text" maxlength="6" class="input_12_table" name="ct_max" onKeyPress="return validator.isNumber(this,event);" value="<% nvram_get("ct_max"); %>">
						</td>
					</tr>

					<tr>
						<th>TCP Timeout: Established</th>
						<td>
							<input type="text" maxlength="6" class="input_6_table" name="tcp_established" onKeyPress="return validator.isNumber(this,event);" value="">
							<span id="ct_established_default">Default: 2400</span>
						</td>

					</tr>

					<tr>
						<th>TCP Timeout: syn_sent</th>
						<td>
							<input type="text" maxlength="5" class="input_6_table" name="tcp_syn_sent" onKeyPress="return validator.isNumber(this,event);" value="">
							<span>Default: 120</span>
						</td>
					</tr>

					<tr>
						<th>TCP Timeout: syn_recv</th>
						<td>
							<input type="text" maxlength="5" class="input_6_table" name="tcp_syn_recv" onKeyPress="return validator.isNumber(this,event);" value="">
							<span>Default: 60</span>
						</td>
					</tr>

					<tr>
						<th>TCP Timeout: fin_wait</th>
						<td>
							<input type="text" maxlength="5" class="input_6_table" name="tcp_fin_wait" onKeyPress="return validator.isNumber(this,event);" value="">
							<span>Default: 120</span>
						</td>
					</tr>

					<tr>
						<th>TCP Timeout: time_wait</th>
						<td>
							<input type="text" maxlength="5" class="input_6_table" name="tcp_time_wait" onKeyPress="return validator.isNumber(this,event);" value="">
							<span>Default: 120</span>
						</td>
					</tr>

					<tr>
						<th>TCP Timeout: close</th>
						<td>
							<input type="text" maxlength="5" class="input_6_table" name="tcp_close" onKeyPress="return validator.isNumber(this,event);" value="">
							<span>Default: 10</span>
						</td>
					</tr>

					<tr>
						<th>TCP Timeout: close_wait</th>
						<td>
							<input type="text" maxlength="5" class="input_6_table" name="tcp_close_wait" onKeyPress="return validator.isNumber(this,event);" value="">
							<span>Default: 60</span>
						</td>
					</tr>

					<tr>
						<th>TCP Timeout: last_ack</th>
						<td>
							<input type="text" maxlength="5" class="input_6_table" name="tcp_last_ack" onKeyPress="return validator.isNumber(this,event);" value="">
							<span>Default: 30</span>
						</td>
					</tr>

					<tr>
						<th>UDP Timeout: Assured</th>
						<td>
							<input type="text" maxlength="5" class="input_6_table" name="udp_assured" onKeyPress="return validator.isNumber(this,event);" value="">
							<span>Default: 180</span>
						</td>
					</tr>

					<tr>
						<th>UDP Timeout: Unreplied</th>
						<td>
							<input type="text" maxlength="5" class="input_6_table" name="udp_unreplied" onKeyPress="return validator.isNumber(this,event);" value="">
							<span>Default: 30</span>
						</td>
					</tr>
				</table>
				<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable" style="margin-top:8px;">
                                        <thead>
						<tr>
							<td colspan="2">Advanced Tweaks and Hacks</td>
						</tr>
					</thead>
					<tr>
						<th>Redirect webui access to www.asusrouter.com</th>
						<td>
							<input type="radio" name="http_dut_redir" value="1" <% nvram_match_x("","http_dut_redir","1", "checked"); %> ><#checkbox_Yes#>
							<input type="radio" name="http_dut_redir" value="0" <% nvram_match_x("","http_dut_redir","0", "checked"); %> ><#checkbox_No#>
						</td>
					</tr>
					<tr>
						<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(50,4);">Firewall: Drop IPv6 neighbour solicitation broadcasts (default: No)</a></th>
						<td>
							<input type="radio" name="ipv6_ns_drop" class="input" value="1" <% nvram_match_x("", "ipv6_ns_drop", "1", "checked"); %>><#checkbox_Yes#>
							<input type="radio" name="ipv6_ns_drop" class="input" value="0" <% nvram_match_x("", "ipv6_ns_drop", "0", "checked"); %>><#checkbox_No#>
						</td>
					</tr>
					<tr>
						<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(50,27);">Wan: Use local caching DNS server as system resolver (default: No)</a></th>
						<td>
							<input type="radio" name="dns_local_cache" class="input" value="1" <% nvram_match_x("", "dns_local_cache", "1", "checked"); %>><#checkbox_Yes#>
							<input type="radio" name="dns_local_cache" class="input" value="0" <% nvram_match_x("", "dns_local_cache", "0", "checked"); %>><#checkbox_No#>
						</td>
					</tr>
					<tr>
						<th>Disable Asusnat tunnel</th>
						<td>
							<input type="radio" name="aae_disable_force" class="input" value="1" <% nvram_match_x("", "aae_disable_force", "1", "checked"); %>><#checkbox_Yes#>
							<input type="radio" name="aae_disable_force" class="input" value="0" <% nvram_match_x("", "aae_disable_force", "0", "checked"); %>><#checkbox_No#>
						</td>
					</tr>
					<tr>
						<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(50,21);">dhcpd: send empty WPAD with a carriage return</a></th>
						<td>
							<input type="radio" name="dhcpd_send_wpad" class="input" value="1" <% nvram_match_x("", "dhcpd_send_wpad", "1", "checked"); %>><#checkbox_Yes#>
							<input type="radio" name="dhcpd_send_wpad" class="input" value="0" <% nvram_match_x("", "dhcpd_send_wpad", "0", "checked"); %>><#checkbox_No#>
						</td>
					</tr>
				</table>
				<div class="apply_gen">
					<input name="button" type="button" class="button_gen" onclick="applyRule();" value="<#CTL_apply#>"/>
			        </div>
			</td></tr>
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

</body>
</html>

