<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=Edge"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title><#Web_Title#> - GRE</title>
<link rel="stylesheet" href="index_style.css">
<link rel="stylesheet" href="form_style.css">

<script src="/state.js"></script>
<script src="/general.js"></script>
<script src="/help.js"></script>
<script src="/popup.js"></script>
<script src="/validator.js"></script>
<script src="/js/jquery.js"></script>

<script>

<% get_l2gre_parameter(); %>
<% get_l3gre_parameter(); %>

function initial(){
	show_menu();
}

function apply_l2gre(){

	if(validForm()){

		showLoading();
		document.form_l2gre.submit();
	}
}

function apply_l3gre(){

	if(validForm()){

		showLoading();
		document.form_l3gre.submit();
	}
}

function validForm(){
	return true;
}

function change_l2gre_unit(unit){
	document.chg_l2gre.l2gre_unit.value=unit.toString();
	document.chg_l2gre.submit();
}

function change_l3gre_unit(unit){
	document.chg_l3gre.l3gre_unit.value=unit.toString();
	document.chg_l3gre.submit();
}

</script>

</head>

<body onload="initial();" onunLoad="return unload_body();" class="bg">
<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>


<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

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
					<div class="formfonttitle">GRE TEST</div>
					<div style="margin:10px 0 10px 5px;" class="splitLine"></div>
					<div id="titl_desc" class="formfontdesc">GRE TEST</div>

<form method="post" name="form_l2gre" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="Advanced_GRE_Content.asp">
<input type="hidden" name="next_page" value="Advanced_GRE_Content.asp">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="restart_l2gre">
<input type="hidden" name="action_wait" value="1">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">

					<table id="L2GRETable" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable">
						<thead>
							<tr>
								<td colspan="2">Layer 2</td>
							</tr>
						</thead>
						<tr id="l2gre_unit_field" class="rept ew">
							<th>Index</th>
							<td>
								<select name="l2gre_unit" class="input_option" onChange="change_l2gre_unit(this.value);">
									<option class="content_input_fd" value="1" <% nvram_match("l2gre_unit", "1","selected"); %>>1</option>
									<option class="content_input_fd" value="2" <% nvram_match("l2gre_unit", "2","selected"); %>>2</option>
								</select>
							</td>
						</tr>
						<tr id="l2gre_enable">
							<th>Enable Layer 2 GRE</th>
							<td>
								<input type="radio" value="1" name="l2gre_enable" class="input" <% nvram_match("l2gre_enable", "1", "checked"); %>><#checkbox_Yes#></input>
								<input type="radio" value="0" name="l2gre_enable" class="input" <% nvram_match("l2gre_enable", "0", "checked"); %>><#checkbox_No#></input>
							</td>
						</tr>
						<tr id="l2gre_remote">
							<th>Remote Address</th>
							<td>
								<input type="text" maxlength="44" name="l2gre_remote" id="l2gre_remote" class="input_32_table" value="<% nvram_get("l2gre_remote"); %>" autocorrect="off" autocapitalize="off"></input>
							</td>
						</tr>
						<tr id="l2gre_vlanid">
							<th>VLAN ID (optional)</th>
							<td>
								<input type="text" maxlength="4" name="l2gre_vlanid" id="l2gre_vlanid" class="input_6_table" value="<% nvram_get("l2gre_vlanid"); %>" autocorrect="off" autocapitalize="off"></input>
							</td>
						</tr>
						<tr id="l2gre_ka_enable">
							<th>Enable Keep Alive</th>
							<td>
								<input type="radio" value="1" name="l2gre_ka_enable" class="input" <% nvram_match("l2gre_ka_enable", "1", "checked"); %>><#checkbox_Yes#></input>
								<input type="radio" value="0" name="l2gre_ka_enable" class="input" <% nvram_match("l2gre_ka_enable", "0", "checked"); %>><#checkbox_No#></input>
							</td>
						</tr>
						<tr id="l2gre_ka_period">
							<th>Peroid of sending Keep Alive</th>
							<td>
								<input type="text" maxlength="5" name="l2gre_ka_period" id="l2gre_ka_period" class="input_6_table" value="<% nvram_get("l2gre_ka_period"); %>" autocorrect="off" autocapitalize="off"></input>
							</td>
						</tr>
						<tr id="l2gre_ka_retries">
							<th>Retry attempts</th>
							<td>
								<input type="text" maxlength="5" name="l2gre_ka_retries" id="l2gre_ka_retries" class="input_6_table" value="<% nvram_get("l2gre_ka_retries"); %>" autocorrect="off" autocapitalize="off"></input>
							</td>
						</tr>
						<tr id="l2gre_state">
							<th>Remote Status</th>
							<td>
								<div id="l2gre_state"><% nvram_get("l2gre_state"); %></div>
							</td>
						</tr>
					</table>
					<div class="apply_gen" id="apply_l2gre">
						<input class="button_gen" onclick="apply_l2gre();" type="button" value="<#CTL_apply#>"/>
					</div>
</form>

					<br>

<form method="post" name="form_l3gre" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="Advanced_GRE_Content.asp">
<input type="hidden" name="next_page" value="Advanced_GRE_Content.asp">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="restart_l3gre">
<input type="hidden" name="action_wait" value="1">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">

					<table id="L3GRETable" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable">
						<thead>
							<tr>
								<td colspan="2">Layer 3</td>
							</tr>
						</thead>
						<tr id="l3gre_unit_field" class="rept ew">
							<th>Index</th>
							<td>
								<select name="l3gre_unit" class="input_option" onChange="change_l3gre_unit(this.value);">
									<option class="content_input_fd" value="1" <% nvram_match("l3gre_unit", "1","selected"); %>>1</option>
									<option class="content_input_fd" value="2" <% nvram_match("l3gre_unit", "2","selected"); %>>2</option>
								</select>
							</td>
						</tr>
						<tr id="l3gre_enable">
							<th>Enable Layer 3 GRE</th>
							<td>
								<input type="radio" value="1" name="l3gre_enable" class="input" <% nvram_match("l3gre_enable", "1", "checked"); %>><#checkbox_Yes#></input>
								<input type="radio" value="0" name="l3gre_enable" class="input" <% nvram_match("l3gre_enable", "0", "checked"); %>><#checkbox_No#></input>
							</td>
						</tr>
						<tr id="l3gre_remote">
							<th>Remote Address</th>
							<td>
								<input type="text" maxlength="44" name="l3gre_remote" id="l3gre_remote" class="input_32_table" value="<% nvram_get("l3gre_remote"); %>" autocorrect="off" autocapitalize="off"></input>
							</td>
						</tr>
						<tr id="l3gre_addr">
							<th>Tunnel Address</th>
							<td>
								<input type="text" maxlength="63" name="l3gre_addr" id="l3gre_addr" class="input_32_table" value="<% nvram_get("l3gre_addr"); %>" autocorrect="off" autocapitalize="off"></input>
							</td>
						</tr>
						<tr id="l3gre_routes">
							<th>Routing Rules</th>
							<td>
								<input type="text" maxlength="1023" name="l3gre_routes" id="l3gre_routes" class="input_32_table" value="<% nvram_get("l3gre_routes"); %>" autocorrect="off" autocapitalize="off"></input>
							</td>
						</tr>
						<tr id="l3gre_ka_enable">
							<th>Enable Keep Alive</th>
							<td>
								<input type="radio" value="1" name="l3gre_ka_enable" class="input" <% nvram_match("l3gre_ka_enable", "1", "checked"); %>><#checkbox_Yes#></input>
								<input type="radio" value="0" name="l3gre_ka_enable" class="input" <% nvram_match("l3gre_ka_enable", "0", "checked"); %>><#checkbox_No#></input>
							</td>
						</tr>
						<tr id="l3gre_ka_period">
							<th>Peroid of sending Keep Alive</th>
							<td>
								<input type="text" maxlength="5" name="l3gre_ka_period" id="l3gre_ka_period" class="input_6_table" value="<% nvram_get("l3gre_ka_period"); %>" autocorrect="off" autocapitalize="off"></input>
							</td>
						</tr>
						<tr id="l3gre_ka_retries">
							<th>Retry attempts</th>
							<td>
								<input type="text" maxlength="5" name="l3gre_ka_retries" id="l3gre_ka_retries" class="input_6_table" value="<% nvram_get("l3gre_ka_retries"); %>" autocorrect="off" autocapitalize="off"></input>
							</td>
						</tr>
						<tr id="l3gre_state">
							<th>Remote Status</th>
							<td>
								<div id="l3gre_state"><% nvram_get("l3gre_state"); %></div>
							</td>
						</tr>
					</table>

					<div class="apply_gen" id="apply_l3gre">
						<input class="button_gen" onclick="apply_l3gre();" type="button" value="<#CTL_apply#>"/>
					</div>
</form>

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

<form method="post" name="chg_l2gre" action="apply.cgi" target="hidden_frame">
	<input type="hidden" name="action_mode" value="chg_l2gre_unit">
	<input type="hidden" name="action_script" value="">
	<input type="hidden" name="action_wait" value="">
	<input type="hidden" name="current_page" value="Advanced_GRE_Content.asp">
	<input type="hidden" name="l2gre_unit" value="">
</form>
<form method="post" name="chg_l3gre" action="apply.cgi" target="hidden_frame">
	<input type="hidden" name="action_mode" value="chg_l3gre_unit">
	<input type="hidden" name="action_script" value="">
	<input type="hidden" name="action_wait" value="">
	<input type="hidden" name="current_page" value="Advanced_GRE_Content.asp">
	<input type="hidden" name="l3gre_unit" value="">
</form>
<div id="footer"></div>
</body>
</html>
