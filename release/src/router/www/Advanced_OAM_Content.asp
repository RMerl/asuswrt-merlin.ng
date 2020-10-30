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
<title><#Web_Title#> - <#menu_dsl_setting#></title>
<link rel="stylesheet" type="text/css" href="index_style.css">
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="other.css">
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/validator.js"></script>
<script>

function initial(){
	show_menu();
	hide_md_id0(document.form.oam0_mode.value);
	hide_md_id1(document.form.oam1_mode.value);
}

function hide_md_id0(_value){
	document.getElementById("tr_md_id0").style.display = (_value == "1") ? "none" : "";
}

function hide_md_id1(_value){
	document.getElementById("tr_md_id1").style.display = (_value == "1") ? "none" : "";
}

function applyRule(){
	if(valid_form()){
		showLoading();
		document.form.submit();
	}
}

function valid_form(){
	// MEG ID
	if(document.form.oam0_id.value.length <= 0) {
		document.form.oam0_id.focus();
		return false;
	}
	if(document.form.oam1_id.value.length <= 0) {
		document.form.oam1_id.focus();
		return false;
	}

	// MD Name
	if(document.form.oam0_mode.value == 0)
	{
		if(document.form.oam0_md_name.value.length <= 0) {
			document.form.oam0_id.focus();
			return false;
		}
	}
	if(document.form.oam1_mode.value == 0)
	{
		if(document.form.oam1_md_name.value.length <= 0) {
			document.form.oam1_id.focus();
			return false;
		}
	}

	// Local MEP ID
	if(document.form.oam0_lmep_id.value.length <= 0) {
		document.form.oam0_lmep_id.focus();
		return false;
	}
	else
	{
		if(!validator.numberRange(document.form.oam0_lmep_id, 1, 8191)) {
			document.form.oam0_lmep_id.focus();
			return false;
		}
	}
	if(document.form.oam1_lmep_id.value.length <= 0) {
		document.form.oam1_lmep_id.focus();
		return false;
	}
	else
	{
		if(!validator.numberRange(document.form.oam1_lmep_id, 1, 8191)) {
			document.form.oam1_lmep_id.focus();
			return false;
		}
	}

	// Local MEP VLAN ID
	if(document.form.oam0_lmep_vid.value > 0) {
		if(!validator.numberRange(document.form.oam0_lmep_vid, 1, 4094)) {
			document.form.oam0_lmep_vid.focus();
			return false;
		}
	}
	if(document.form.oam1_lmep_vid.value > 0) {
		if(!validator.numberRange(document.form.oam1_lmep_vid, 1, 4094)) {
			document.form.oam1_lmep_vid.focus();
			return false;
		}
	}

	// Remote MEP ID
	if(document.form.oam0_rmep_id.value > 0) {
		if(!validator.numberRange(document.form.oam0_rmep_id, 1, 8191)) {
			document.form.oam0_rmep_id.focus();
			return false;
		}
	}
	if(document.form.oam1_rmep_id.value > 0) {
		if(!validator.numberRange(document.form.oam1_rmep_id, 1, 8191)) {
			document.form.oam1_rmep_id.focus();
			return false;
		}
	}

	// OAM ID
	if(document.form.oam_3ah_id.value.length <= 0) {
		document.form.oam_3ah_id.focus();
		return false;
	}
	else
	{
		if(!validator.numberRange(document.form.oam_3ah_id, 1, 2147483647)) {
			document.form.oam_3ah_id.focus();
			return false;
		}
	}

	return true;
}

</script>
</head>

<body onload="initial();" onunLoad="return unload_body();">
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
<input type="hidden" name="current_page" value="Advanced_OAM_Content.asp">
<input type="hidden" name="next_page" value="Advanced_OAM_Content.asp">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="restart_oam">
<input type="hidden" name="action_wait" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">

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
		<td bgcolor="#4D595D" valign="top"  >
		<div>&nbsp;</div>
		<div class="formfonttitle"><#menu5_6#> - Ethernet OAM</div>
		<div style="margin: 10px 0 10px 5px;" class="splitLine"></div>
		<div class="formfontdesc">Ethernet OAM</div>
		<!-- First OAM-->
		<table width="99%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable">
			<thead>
			<tr>
				<td colspan="2">First Service OAM</td>
			</tr>
			</thead>
			<tr>
				<th>
					Enable OAM Service
				</th>
				<td>
					<input type="radio" name="oam0_srv_enable" class="input" value="1" <% nvram_match("oam0_srv_enable", "1", "checked"); %>><#checkbox_Yes#>
					<input type="radio" name="oam0_srv_enable" class="input" value="0" <% nvram_match("oam0_srv_enable", "0", "checked"); %>><#checkbox_No#>
				</td>
			</tr>
			<tr>
				<th>
					Specification
				</th>
				<td>
					<input type="radio" name="oam0_mode" class="input" value="1" onclick="hide_md_id0(this.value);" <% nvram_match("oam0_mode", "1", "checked"); %>>ITU-T Y.1731
					<input type="radio" name="oam0_mode" class="input" value="0" onclick="hide_md_id0(this.value);" <% nvram_match("oam0_mode", "0", "checked"); %>>IEEE 802.1ag
				</td>
			</tr>
			<tr>
				<th>
					MEG / MD Level
				</th>
				<td>
					<select class="input_option" name="oam0_level">
						<option value="0" <% nvram_match("oam0_level", "0", "selected"); %>>0</option>
						<option value="1" <% nvram_match("oam0_level", "1", "selected"); %>>1</option>
						<option value="2" <% nvram_match("oam0_level", "2", "selected"); %>>2</option>
						<option value="3" <% nvram_match("oam0_level", "3", "selected"); %>>3</option>
						<option value="4" <% nvram_match("oam0_level", "4", "selected"); %>>4</option>
						<option value="5" <% nvram_match("oam0_level", "5", "selected"); %>>5</option>
						<option value="6" <% nvram_match("oam0_level", "6", "selected"); %>>6</option>
						<option value="7" <% nvram_match("oam0_level", "7", "selected"); %>>7</option>
					</select>
				</td>
			</tr>
			<tr id="tr_md_id0" style="display:none">
				<th>
					MD Name
				</th>
				<td>
					<input type="text" name="oam0_md_name" maxlength="45" class="input_32_table" value="<% nvram_get("oam0_md_name"); %>">
				</td>
			</tr>
			<tr>
				<th>
					MEG / MA ID
				</th>
				<td>
					<input type="text" name="oam0_id" maxlength="45" class="input_32_table" value="<% nvram_get("oam0_id"); %>">
				</td>
			</tr>
			<tr>
				<th>
					Local MEP ID
				</th>
				<td>
					<input type="text" name="oam0_lmep_id" maxlength="4" class="input_6_table" value="<% nvram_get("oam0_lmep_id"); %>" onKeyPress="return validator.isNumber(this,event);"> ( 1 ~ 8191 )
				</td>
			</tr>
			<tr>
				<th>
					Local MEP VLAN ID
				</th>
				<td>
					<input type="text" name="oam0_lmep_vid" maxlength="4" class="input_6_table" value="<% nvram_get("oam0_lmep_vid"); %>" onKeyPress="return validator.isNumber(this,event);"> ( 1 ~ 4094, 0 means no VLAN tag)
				</td>
			</tr>
			<tr>
				<th>
					Remote MEP ID
				</th>
				<td>
					<input type="text" name="oam0_rmep_id" maxlength="4" class="input_6_table" value="<% nvram_get("oam0_rmep_id"); %>" onKeyPress="return validator.isNumber(this,event);"> ( 1 ~ 8191, 0 means no Remote MEP)
				</td>
			</tr>
			<tr>
				<th>
					CCM Transmission Interval
				</th>
				<td>
					<select class="input_option" name="oam0_ccm_itvl">
						<option value="0" <% nvram_match("oam0_ccm_itvl", "0", "selected"); %>><#btn_Disabled#></option>
						<option value="1" <% nvram_match("oam0_ccm_itvl", "1", "selected"); %>>3.3 ms</option>
						<option value="2" <% nvram_match("oam0_ccm_itvl", "2", "selected"); %>>10 ms</option>
						<option value="3" <% nvram_match("oam0_ccm_itvl", "3", "selected"); %>>100 ms</option>
						<option value="4" <% nvram_match("oam0_ccm_itvl", "4", "selected"); %>>1 sec</option>
						<option value="5" <% nvram_match("oam0_ccm_itvl", "5", "selected"); %>>10 sec</option>
						<option value="6" <% nvram_match("oam0_ccm_itvl", "6", "selected"); %>>1 min</option>
						<option value="7" <% nvram_match("oam0_ccm_itvl", "7", "selected"); %>>10 min</option>
					</select>
				</td>
			</tr>
		</table>

		<!-- Second OAM-->
		<table width="99%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable" style="margin-top:10px;">
			<thead>
			<tr>
				<td colspan="2">Second Service OAM</td>
			</tr>
			</thead>
			<tr>
				<th>
					Enable OAM Service
				</th>
				<td>
					<input type="radio" name="oam1_srv_enable" class="input" value="1" <% nvram_match("oam1_srv_enable", "1", "checked"); %>><#checkbox_Yes#>
					<input type="radio" name="oam1_srv_enable" class="input" value="0" <% nvram_match("oam1_srv_enable", "0", "checked"); %>><#checkbox_No#>
				</td>
			</tr>
			<tr>
				<th>
					Specification
				</th>
				<td>
					<input type="radio" name="oam1_mode" class="input" value="1" onclick="hide_md_id1(this.value);" <% nvram_match("oam1_mode", "1", "checked"); %>>ITU-T Y.1731
					<input type="radio" name="oam1_mode" class="input" value="0" onclick="hide_md_id1(this.value);" <% nvram_match("oam1_mode", "0", "checked"); %>>IEEE 802.1ag
				</td>
			</tr>
			<tr>
				<th>
					MEG / MD Level
				</th>
				<td>
					<select class="input_option" name="oam1_level">
						<option value="0" <% nvram_match("oam1_level", "0", "selected"); %>>0</option>
						<option value="1" <% nvram_match("oam1_level", "1", "selected"); %>>1</option>
						<option value="2" <% nvram_match("oam1_level", "2", "selected"); %>>2</option>
						<option value="3" <% nvram_match("oam1_level", "3", "selected"); %>>3</option>
						<option value="4" <% nvram_match("oam1_level", "4", "selected"); %>>4</option>
						<option value="5" <% nvram_match("oam1_level", "5", "selected"); %>>5</option>
						<option value="6" <% nvram_match("oam1_level", "6", "selected"); %>>6</option>
						<option value="7" <% nvram_match("oam1_level", "7", "selected"); %>>7</option>
					</select>
				</td>
			</tr>
			<tr id="tr_md_id1" style="display:none">
				<th>
					MD Name
				</th>
				<td>
					<input type="text" name="oam1_md_name" maxlength="45" class="input_32_table" value="<% nvram_get("oam1_md_name"); %>">
				</td>
			</tr>
			<tr>
				<th>
					MEG / MA ID
				</th>
				<td>
					<input type="text" name="oam1_id" maxlength="45" class="input_32_table" value="<% nvram_get("oam1_id"); %>">
				</td>
			</tr>
			<tr>
				<th>
					Local MEP ID
				</th>
				<td>
					<input type="text" name="oam1_lmep_id" maxlength="4" class="input_6_table" value="<% nvram_get("oam1_lmep_id"); %>" onKeyPress="return validator.isNumber(this,event);"> ( 1 ~ 8191 )
				</td>
			</tr>
			<tr>
				<th>
					Local MEP VLAN ID
				</th>
				<td>
					<input type="text" name="oam1_lmep_vid" maxlength="4" class="input_6_table" value="<% nvram_get("oam1_lmep_vid"); %>" onKeyPress="return validator.isNumber(this,event);"> ( 1 ~ 4094, 0 means no VLAN tag)
				</td>
			</tr>
			<tr>
				<th>
					Remote MEP ID
				</th>
				<td>
					<input type="text" name="oam1_rmep_id" maxlength="4" class="input_6_table" value="<% nvram_get("oam1_rmep_id"); %>" onKeyPress="return validator.isNumber(this,event);"> ( 1 ~ 8191, 0 means no Remote MEP)
				</td>
			</tr>
			<tr>
				<th>
					CCM Transmission Interval
				</th>
				<td>
					<select class="input_option" name="oam1_ccm_itvl">
						<option value="0" <% nvram_match("oam1_ccm_itvl", "0", "selected"); %>><#btn_Disabled#></option>
						<option value="1" <% nvram_match("oam1_ccm_itvl", "1", "selected"); %>>3.3 ms</option>
						<option value="2" <% nvram_match("oam1_ccm_itvl", "2", "selected"); %>>10 ms</option>
						<option value="3" <% nvram_match("oam1_ccm_itvl", "3", "selected"); %>>100 ms</option>
						<option value="4" <% nvram_match("oam1_ccm_itvl", "4", "selected"); %>>1 sec</option>
						<option value="5" <% nvram_match("oam1_ccm_itvl", "5", "selected"); %>>10 sec</option>
						<option value="6" <% nvram_match("oam1_ccm_itvl", "6", "selected"); %>>1 min</option>
						<option value="7" <% nvram_match("oam1_ccm_itvl", "7", "selected"); %>>10 min</option>
					</select>
				</td>
			</tr>
		</table>

		<table width="99%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable" style="margin-top:10px;">
			<thead>
			<tr>
				<td colspan="2">IEEE 802.3ah</td>
			</tr>
			</thead>
			<tr>
				<th>
					Enable Link OAM
				</th>
				<td>
					<input type="radio" name="oam_3ah_enable" class="input" value="1" <% nvram_match("oam_3ah_enable", "1", "checked"); %>><#checkbox_Yes#>
					<input type="radio" name="oam_3ah_enable" class="input" value="0" <% nvram_match("oam_3ah_enable", "0", "checked"); %>><#checkbox_No#>
				</td>
			</tr>
			<tr>
				<th>
					OAM ID
				</th>
				<td>
					<input type="text" name="oam_3ah_id" maxlength="10" class="input_12_table" value="<% nvram_get("oam_3ah_id"); %>" onKeyPress="return validator.isNumber(this,event);"> (Positive integer)
				</td>
			</tr>
			<tr>
				<th>
					Auto Event
				</th>
				<td>
					<input type="radio" name="oam_auto_event" class="input" value="1" <% nvram_match("oam_auto_event", "1", "checked"); %>><#checkbox_Yes#>
					<input type="radio" name="oam_auto_event" class="input" value="0" <% nvram_match("oam_auto_event", "0", "checked"); %>><#checkbox_No#>
				</td>
			</tr>
			<tr>
				<th>
					Variable Retrieval
				</th>
				<td>
					<input type="radio" name="oam_var_retrvl" class="input" value="1" <% nvram_match("oam_var_retrvl", "1", "checked"); %>><#checkbox_Yes#>
					<input type="radio" name="oam_var_retrvl" class="input" value="0" <% nvram_match("oam_var_retrvl", "0", "checked"); %>><#checkbox_No#>
				</td>
			</tr>
			<tr>
				<th>
					Link Events
				</th>
				<td>
					<input type="radio" name="oam_link_event" class="input" value="1" <% nvram_match("oam_link_event", "1", "checked"); %>><#checkbox_Yes#>
					<input type="radio" name="oam_link_event" class="input" value="0" <% nvram_match("oam_link_event", "0", "checked"); %>><#checkbox_No#>
				</td>
			</tr>
			<tr>
				<th>
					Remote Loopback
				</th>
				<td>
					<input type="radio" name="oam_remote_lb" class="input" value="1" <% nvram_match("oam_remote_lb", "1", "checked"); %>><#checkbox_Yes#>
					<input type="radio" name="oam_remote_lb" class="input" value="0" <% nvram_match("oam_remote_lb", "0", "checked"); %>><#checkbox_No#>
				</td>
			</tr>
			<tr>
				<th>
					Active Mode
				</th>
				<td>
					<input type="radio" name="oam_active_mode" class="input" value="1" <% nvram_match("oam_active_mode", "1", "checked"); %>><#checkbox_Yes#>
					<input type="radio" name="oam_active_mode" class="input" value="0" <% nvram_match("oam_active_mode", "0", "checked"); %>><#checkbox_No#>
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
			<!--===================================End of Main Content===========================================-->
	</td>

	<td width="10" align="center" valign="top">&nbsp;</td>
  </tr>
</table>
</form>

<div id="footer"></div>
</body>
</html>
