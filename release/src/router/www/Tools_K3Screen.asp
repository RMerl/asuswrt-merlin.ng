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
<title><#Web_Title#> - <#K3Screen#></title>
<link rel="stylesheet" type="text/css" href="index_style.css">
<link rel="stylesheet" type="text/css" href="form_style.css">
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/validator.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script language="JavaScript" type="text/javascript" src="/help.js"></script>
<script language="JavaScript" type="text/javascript" src="/js/jquery.js"></script>
<script language="JavaScript" type="text/javascript" src="/switcherplugin/jquery.iphone-switch.js"></script>
<script language="JavaScript" type="text/javascript" src="/js/table/table.js"></script>
<script>

function applyRule(){
	showLoading();
	document.form.submit();
}

function done_validating(action){
	refreshpage();
}

function initial(){
	show_menu();
}

</script>
</head>
<body onload="initial();" onunLoad="return unload_body();">

<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">

<input type="hidden" name="current_page" value="Tools_K3Screen.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="apply_new">
<input type="hidden" name="action_wait" value="2">
<input type="hidden" name="action_script" value="restart_screen">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="screen_enable" value="<% nvram_get("screen_enable"); %>">

<table class="content" align="center" cellpadding="0" cellspacing="0">
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
		  <div class="formfonttitle"><#Tools#> - <#K3Screen#></div>
		  <div style="margin:10px 0 10px 5px;" class="splitLine"></div>
		  <div class="formfontdesc"><#tools_screen_desc#></div>

	<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
	<thead>
		<tr>
			<td colspan="2"><#screen_info#></td>
		</tr>
	</thead>
		<tr>
			<th><#mcu_version#></th>
			<td><% nvram_get("mcu_version"); %></td>
		</tr>
	<thead>
		<tr>
			<td colspan="2"><#screen_settings#></td>
		</tr>
	</thead>
        <tr>
          <th><#screen_switch#></th>
			<td>
				<div align="center" class="left" style="width:94px; float:left; cursor:pointer;" id="screen_enable"></div>
				<div class="iphone_switch_container" style="height:32px; width:74px; position: relative; overflow: hidden">
					<script type="text/javascript">
						$('#screen_enable').iphoneSwitch('<% nvram_get("screen_enable"); %>',
							function(){
								document.form.screen_enable.value = 1;
								applyRule();
							},
							function(){
								document.form.screen_enable.value = 0;
								applyRule();
							}
						);
					</script>
				</div>
			</td>
        </tr>

        <tr>
          <th><#weather_key#></th>
          <td>
			<input type="text" maxlength="32" class="input_32_table" name="weather_key" value="<% nvram_get("weather_key"); %>" autocorrect="off" autocapitalize="off">
			<div class="formfontdesc" style="color:#FFCC00;margin-top:5px;"><#weather_key_hint#></div>
          </td>
        </tr>

        <tr>
          <th><#weather_city#></th>
			<td>
		      <input type="text" maxlength="16" class="input_20_table" name="weather_city" value="<% nvram_get("weather_city"); %>" autocorrect="off" autocapitalize="off">
		      <span><#weather_note#></span>
			</td>
        </tr>

        <tr>
          <th><#weather_interval#></th>
          <td>
			<select name="weather_interval" class="input_option" >
				<option value="0" <% nvram_match("weather_interval", "0","selected"); %>><#weather_off#></option>
				<option value="3600" <% nvram_match("weather_interval", "3600","selected"); %>><#weather_1h#></option>
				<option value="7200" <% nvram_match("weather_interval", "7200","selected"); %>><#weather_2h#></option>
				<option value="14400" <% nvram_match("weather_interval", "14400","selected"); %>><#weather_4h#></option>
				<option value="21600" <% nvram_match("weather_interval", "21600","selected"); %>><#weather_6h#></option>
				<option value="43200" <% nvram_match("weather_interval", "43200","selected"); %>><#weather_12h#></option>
				<option value="86400" <% nvram_match("weather_interval", "86400","selected"); %>><#weather_24h#></option>
			</select>
          </td>
        </tr>

        <tr id="hidepwd_tr">
          <th><#screen_hidepwd#></th>
			<td>
				<input type="radio" name="screen_hidepwd" class="input" value="1" <% nvram_match_x("", "screen_hidepwd", "1", "checked"); %>><#checkbox_Yes#>
				<input type="radio" name="screen_hidepwd" class="input" value="0" <% nvram_match_x("", "screen_hidepwd", "0", "checked"); %>><#checkbox_No#>
			</td>
        </tr>

        <tr id="hidepwd_visitor_tr">
          <th><#screen_hidepwd_visitor#></th>
			<td>
				<input type="radio" name="screen_hidepwd_visitor" class="input" value="1" <% nvram_match_x("", "screen_hidepwd_visitor", "1", "checked"); %>><#checkbox_Yes#>
				<input type="radio" name="screen_hidepwd_visitor" class="input" value="0" <% nvram_match_x("", "screen_hidepwd_visitor", "0", "checked"); %>><#checkbox_No#>
			</td>
        </tr>

        <tr>
          <th><#screen_timeout#></th>
          <td>
		      <input type="text" maxlength="3" class="input_6_table" name="screen_timeout" onKeyPress="return validator.isNumber(this,event);" value="<% nvram_get("screen_timeout"); %>">
		      <span><#FW_note#> 0 - <#btn_Disabled#></span>
		  </td>
        </tr>

	</table>
            <div class="apply_gen">
				<input name="button" type="button" class="button_gen" onclick="applyRule()" value="<#CTL_apply#>"/>
            </div>

		</td>
	</tr>

	</tbody>

</table>
	</td>
	</form>

        </tr>
      </table>
		<!--===================================Ending of Main Content===========================================-->
	</td>

    <td width="10" align="center" valign="top">&nbsp;</td>
	</tr>
</table>

<div id="footer"></div>

</body>
</html>
