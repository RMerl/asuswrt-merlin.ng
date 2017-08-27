<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<meta http-equiv="X-UA-Compatible" content="IE=Edge" />
<meta name="svg.render.forceflash" content="false" />	
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title><#Web_Title#> - Performance tuning</title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<style type="text/css">
.btnDesc{
	font-size: 14px;
	font-family: Segoe UI;
}
.btnTitle{
	text-align:left;
	font-size: 18px;
	font-weight: bold;
	color: #5AD;
	line-height: 35px;
	font-family: Segoe UI;
	text-shadow: 1px 1px 0px #000;
}
</style>
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/help.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script language="JavaScript" type="text/javascript" src="/validator.js"></script>
<script type="text/javascript" src="/js/jquery.js"></script>
<script type='text/javascript'>var fanctrl_info = [<% get_fanctrl_info(); %>];
var cpuTemp = [<% get_cpu_temperature(); %>];
if(typeof cpuTemp[0] != "undefined")
	fanctrl_info = ["0", cpuTemp[0], cpuTemp[0], "0"];
var curr_rxData = fanctrl_info[3];
var curr_coreTmp_2 = convertTemp(fanctrl_info[1], fanctrl_info[2], 0);
//var curr_coreTmp_2 = fanctrl_info[1];
var curr_coreTmp_5 = fanctrl_info[2];
var coreTmp_2 = new Array();
var coreTmp_5 = new Array();
coreTmp_2 = [curr_coreTmp_2];
coreTmp_5 = [curr_coreTmp_5];
var wl_control_channel = <% wl_control_channel(); %>;

var MaxTxPower_2;
var MaxTxPower_5;
var flag = 0;;
var HW_MAX_LIMITATION_2 = 101;
var HW_MIN_LIMITATION_2 = 9;
var HW_MAX_LIMITATION_5 = 251;
var HW_MIN_LIMITATION_5 = 9;
var fanctrl_fullspeed_temp_orig = convertTemp('<% nvram_get("fanctrl_fullspeed_temp"); %>', 0, 0);
var fanctrl_period_temp_orig = convertTemp('<% nvram_get("fanctrl_period_temp"); %>', 0, 0);
var fanctrl_fullspeed_temp_orig_F = Math.round(fanctrl_fullspeed_temp_orig*9/5+32);
var fanctrl_period_temp_orig_F = Math.round(fanctrl_period_temp_orig*9/5+32);
						
function initial(){
	show_menu();

	if(fanctrl_info.length != 0)
		update_coretmp();

	if(cookie.get("CoreTmpUnit") == 1){
		document.getElementById("unitDisplay1").innerHTML = "°F";
		document.getElementById("unitDisplay2").innerHTML = "°F";
		document.form.fanctrl_fullspeed_temp.value = fanctrl_fullspeed_temp_orig_F;
		document.form.fanctrl_period_temp.value = fanctrl_period_temp_orig_F;
	}		
	else{
		document.getElementById("unitDisplay1").innerHTML = "°C";
		document.getElementById("unitDisplay2").innerHTML = "°C";
		document.form.fanctrl_fullspeed_temp.value = fanctrl_fullspeed_temp_orig;
		document.form.fanctrl_period_temp.value = fanctrl_period_temp_orig;
	}
	document.form.fanctrl_fullspeed_temp_unit.selectedIndex = cookie.get("CoreTmpUnit");

	if(!power_support){
		inputHideCtrl(document.form.wl0_TxPower, 0);
		inputHideCtrl(document.form.wl1_TxPower, 0);
	}

	if(based_modelid == "RT-AC68U" || based_modelid == "RT-AC68A" || based_modelid == "DSL-AC68U" || based_modelid == "4G-AC68U"){
		document.form.selLED.onchange = function(){
			document.form.btn_led_mode.value = 0;
			document.form.selCLK.checked = false;
			$("#btnDescTr").fadeOut(100);
		}
	
		document.form.selCLK.onchange = function(){
			document.form.btn_led_mode.value = 1;
			document.form.selLED.checked = false;
			$("#btnDescTr").fadeIn(300);
			scrollTo(1000, 1000);
			setTimeout('document.getElementById("alertHint").style.visibility="hidden"', 500);
			setTimeout('document.getElementById("alertHint").style.visibility=""', 1000);
			setTimeout('document.getElementById("alertHint").style.visibility="hidden"', 1500);
			setTimeout('document.getElementById("alertHint").style.visibility=""', 2000);
			setTimeout('document.getElementById("alertHint").style.visibility="hidden"', 2500);
			setTimeout('document.getElementById("alertHint").style.visibility=""', 3000);
		}
	
		document.getElementById("btnCtrlTr").style.display = "";
		document.getElementById("btnDescTr").style.display = "";
		if(document.form.btn_led_mode.value == 1)
			document.form.selCLK.click();
		else
			document.form.selLED.click();
	}
}

function update_coretmp(e){
  $.ajax({
    url: '/ajax_coretmp.asp',
    dataType: 'script', 
	
    error: function(xhr){
      update_coretmp();
    },
    success: function(response){
			if(fanctrl_info.length != 0)
				updateNum(curr_coreTmp_2, curr_coreTmp_5);
			setTimeout("update_coretmp();", 5000);
		}    
  });
}

function convertTemp(__coreTmp_2, __coreTmp_5, _method){
	if(_method == 0)
		return parseInt(__coreTmp_2)*0.5+20;
	else
		return (parseInt(__coreTmp_2)-20)*2;
}

function updateNum(_coreTmp_2, _coreTmp_5){
	curr_coreTmp_2 = convertTemp(_coreTmp_2, _coreTmp_5, 0);

	if(document.form.fanctrl_fullspeed_temp_unit.value == 1){
		document.getElementById("coreTemp_2").innerHTML = Math.round(_coreTmp_2*9/5+32) + " °F";
		document.getElementById("coreTemp_5").innerHTML = Math.round(_coreTmp_5*9/5+32) + " °F";
	}
	else{
		document.getElementById("coreTemp_2").innerHTML = _coreTmp_2 + " °C";
		document.getElementById("coreTemp_5").innerHTML = _coreTmp_5 + " °C";
	}
}

function applyRule(){
	if(parseInt(document.form.wl0_TxPower.value) > HW_MAX_LIMITATION_2){
		document.getElementById("TxPowerHint_2").style.display = "";
		document.form.wl0_TxPower.focus();
		return false;
	}

	var wlcountry = '<% nvram_get("wl0_country_code"); %>';
	if(wlcountry == 'US' || wlcountry == 'CN' || wlcountry == 'TW')
		HW_MAX_LIMITATION_5 = 501;
	else
		HW_MAX_LIMITATION_5 = 251;

	if(parseInt(document.form.wl1_TxPower.value) > HW_MAX_LIMITATION_5){
		document.getElementById("TxPowerHint_5").style.display = "";
		document.form.wl1_TxPower.focus();
		return false;
	}

	if(parseInt(document.form.wl0_TxPower.value) > 80 && flag < 2){
		document.getElementById("TxPowerHint_2").style.display = "";
		document.form.wl0_TxPower.focus();
		flag++;
		return false;
	}
	else
		document.getElementById("TxPowerHint_2").style.display = "none";
		
	if(parseInt(document.form.wl1_TxPower.value) > 80 && flag < 2){
		document.getElementById("TxPowerHint_5").style.display = "";
		document.form.wl1_TxPower.focus();
		flag++;
		return false;
	}
	else
		document.getElementById("TxPowerHint_5").style.display = "none";

	if(parseInt(document.form.wl0_TxPower.value) > parseInt(document.form.wl0_TxPower_orig.value) 
		|| parseInt(document.form.wl1_TxPower.value) > parseInt(document.form.wl1_TxPower_orig.value))
	  FormActions("start_apply.htm", "apply", "set_wltxpower;reboot", "<% get_default_reboot_time(); %>");
	else{
		if(document.form.wl0_TxPower.value != document.form.wl0_TxPower_orig.value 
			|| document.form.wl1_TxPower.value != document.form.wl1_TxPower_orig.value)
			document.form.action_script.value = "restart_wireless";
	
		if(document.form.fanctrl_mode.value != document.form.fanctrl_mode_orig.value 
			|| document.form.fanctrl_fullspeed_temp.value != document.form.fanctrl_fullspeed_temp_orig.value
			|| document.form.fanctrl_period_temp.value != document.form.fanctrl_period_temp_orig.value
			|| document.form.fanctrl_dutycycle.value != document.form.fanctrl_dutycycle_orig.value){
			if(document.form.action_script.value != "")
				document.form.action_script.value += ";";
			document.form.action_script.value += "restart_fanctrl";
		}
	}
	
	if(parseInt(document.form.fanctrl_period_temp.value) > parseInt(document.form.fanctrl_fullspeed_temp.value)){
		alert("This value could not exceed "+document.form.fanctrl_fullspeed_temp.value);
		document.form.fanctrl_period_temp.focus();
		return false;
	}

	if(document.form.fanctrl_fullspeed_temp_unit.value == "1"){
		document.form.fanctrl_fullspeed_temp.value = Math.round((document.form.fanctrl_fullspeed_temp.value-32)*5/9);
		document.form.fanctrl_period_temp.value = Math.round((document.form.fanctrl_period_temp.value-32)*5/9);
	}
	
	/*if(validator.numberRange(document.form.fanctrl_fullspeed_temp, 25, 70) 
		&& validator.numberRange(document.form.fanctrl_period_temp, 25, 55)){
		document.form.fanctrl_fullspeed_temp.value = convertTemp(document.form.fanctrl_fullspeed_temp.value, 0, 1);
		document.form.fanctrl_period_temp.value = convertTemp(document.form.fanctrl_period_temp.value, 0, 1);
		Math.round(document.form.fanctrl_fullspeed_temp.value);
		Math.round(document.form.fanctrl_period_temp.value);
		showLoading();
		document.form.submit();
	}
	else{
		if(document.form.fanctrl_fullspeed_temp_unit.value == "1"){
			document.form.fanctrl_fullspeed_temp.value = fanctrl_fullspeed_temp_orig_F;
			document.form.fanctrl_period_temp.value = fanctrl_period_temp_orig_F;
		}
		else{
			document.form.fanctrl_fullspeed_temp.value = fanctrl_fullspeed_temp_orig;
			document.form.fanctrl_period_temp.value = fanctrl_period_temp_orig;
		}
	}*/
	showLoading();
	document.form.submit();
}

function changeTempUnit(num){
	cookie.set("CoreTmpUnit", num, 365);
	refreshpage();
}
</script>
</head>

<body onload="initial();">
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>
<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="current_page" value="Advanced_PerformanceTuning_Content.asp">
<input type="hidden" name="next_page" value="Advanced_PerformanceTuning_Content.asp">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_wait" value="5">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="wl_ssid" value="<% nvram_get("wl_ssid"); %>">
<input type="hidden" name="wl0_TxPower_orig" value="<% nvram_get("wl0_TxPower"); %>" disabled>
<input type="hidden" name="wl1_TxPower_orig" value="<% nvram_get("wl1_TxPower"); %>" disabled>
<input type="hidden" name="fanctrl_mode_orig" value="<% nvram_get("fanctrl_mode"); %>" disabled>
<input type="hidden" name="fanctrl_fullspeed_temp_orig" value="<% nvram_get("fanctrl_fullspeed_temp"); %>" disabled>
<input type="hidden" name="fanctrl_period_temp_orig" value="<% nvram_get("fanctrl_period_temp"); %>" disabled>
<input type="hidden" name="fanctrl_dutycycle_orig" value="<% nvram_get("fanctrl_dutycycle"); %>" disabled>
<input type="hidden" name="btn_led_mode" value="<% nvram_get("btn_led_mode"); %>">

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
					<td valign="top">
						<table width="760px" border="0" cellpadding="4" cellspacing="0" class="FormTitle" id="FormTitle">
							<tbody>
                <tr bgcolor="#4D595D" style="height:10px">
	                <td valign="top">
									  <div>&nbsp;</div>
									  <div class="formfonttitle"><#menu5_6#> - Performance tuning</div>
									  <div style="margin-left:5px;margin-top:10px;margin-bottom:10px"><img src="/images/New_ui/export/line_export.png"></div>
									  <div class="formfontdesc">Fine tune the radio power to enhance/decrease the coverage and change the cooler spin mode.Please note: If the output power is increased for long distance signal transmission, the client also need to use high power card to get the best performance.</div>
									</td>
					  		</tr>

								<tr style="height:10px">
									<td bgcolor="#4D595D" valign="top">
										<table width="99%" border="0" align="center" cellpadding="0" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
											<thead>
											<tr>
												<td colspan="2">Cooler status</td>
											</tr>
											</thead>
											
											<tr>
												<td valign="top">
													<div style="margin-left:-10px;">
														<!--========= svg =========-->
														<!--[if IE]>
															<div id="svg-table" align="left">
															<object id="graph" src="fan.svg" classid="image/svg+xml" width="740" height="300">
															</div>
														<![endif]-->
														<!--[if !IE]>-->
															<object id="graph" data="fan.svg" type="image/svg+xml" width="740" height="300">
														<!--<![endif]-->
															</object>
											 			<!--========= svg =========-->
													</div>
												</td>
											</tr>
										</table>
									</td>
					  		</tr>

								<tr style="display:none;" style="height:10px">
									<td bgcolor="#4D595D" valign="top">
						    	 	<table width="735px" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable_NWM">
								  		<tr>
								  			<th style="text-align:center; width:35%;height:25px;">2.4GHz</th>
								  			<th style="text-align:center; width:35%;">5GHz</th>
								  			<th style="text-align:center; width:30%;">Unit</th>
								  		</tr>
		
								  		<tr>
								  			<td style="text-align:center; background-color:#111;"><span id="coreTemp_2" style="font-weight:bold;color:#FF9000"></span></td>
								 				<td style="text-align:center; background-color:#111;"><span id="coreTemp_5" style="font-weight:bold;color:#33CCFF"></span></td>
								  			<td style="text-align:center; background-color:#111;">
													<!--select name="fanctrl_fullspeed_temp_unit" class="input_option" onchange="changeTempUnit(this.value)" style="background-color:#111;">
														<option class="content_input_fd" value="0">°C</option>
														<option class="content_input_fd" value="1">°F</option>
													</select-->			
												</td>
								    	</tr>
										</table>
									</td>
					  		</tr>

								<tr style="height:10px">
									<td bgcolor="#4D595D" valign="top">
										<table width="99%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
											<thead>
											<tr>
												<td colspan="2">System adjustment</td>
											</tr>
											</thead>
											
											<tr>
												<th>2.4GHz Transmit radio power</th>
												<td>
													<input type="text" name="wl0_TxPower" maxlength="3" class="input_3_table" value="<% nvram_get("wl0_TxPower"); %>" autocorrect="off" autocapitalize="off"> mW
													<span id="TxPowerHint_2" style="margin-left:10px;display:none;">This value could not exceed 80</span>
												</td>
											</tr>
				            
											<tr>
												<th>5GHz Transmit radio power</th>
												<td>
													<input type="text" name="wl1_TxPower" maxlength="3" class="input_3_table" value="<% nvram_get("wl1_TxPower"); %>" autocorrect="off" autocapitalize="off"> mW
													<span id="TxPowerHint_5" style="margin-left:10px;display:none;">This value could not exceed 80</span>
												</td> 
											</tr>

											<tr style="display:none">
												<th>Cooler rotate mode</th>
												<td>
													<select name="fanctrl_mode" class="input_option">
														<option class="content_input_fd" value="0" <% nvram_match("fanctrl_mode", "0", "selected"); %>><#Automatic_cooler#></option>
														<option class="content_input_fd" value="1" <% nvram_match("fanctrl_mode", "1", "selected"); %>>Manually</option>
													</select>			
												</td>
											</tr>

											<tr>
												<th>Temperature unit</th>
												<td>
													<select name="fanctrl_fullspeed_temp_unit" class="input_option" onchange="changeTempUnit(this.value)">
														<option class="content_input_fd" value="0">°C</option>
														<option class="content_input_fd" value="1">°F</option>
													</select>			
												</td>
											</tr>
											
											<tr style="display:none">
												<th>Cooler full speed spin</th>
												<td>Temperature over
													<input type="text" name="fanctrl_fullspeed_temp" maxlength="3" class="input_3_table" value="<% nvram_get("fanctrl_fullspeed_temp"); %>" autocorrect="off" autocapitalize="off">
													<span style="color:#FFF" id="unitDisplay1">°C</span>
												</td>
											</tr>

											<tr style="display:none">
												<th>Cooler periodically spin</th>
												<td>Temperature over
													<input type="text" name="fanctrl_period_temp" maxlength="3" class="input_3_table" value="<% nvram_get("fanctrl_period_temp"); %>" autocorrect="off" autocapitalize="off">
													<span style="color:#FFF" id="unitDisplay2">°C</span>
												</td>
											</tr>
				            
											<tr>
												<th>Spin duty cycle</th>
												<td> 
													<select name="fanctrl_dutycycle" class="input_option">
														<option class="content_input_fd" value="0" <% nvram_match("fanctrl_dutycycle", "1", "selected"); %>><#Auto#></option>
														<option class="content_input_fd" value="1" <% nvram_match("fanctrl_dutycycle", "1", "selected"); %>>50%</option>
														<option class="content_input_fd" value="2" <% nvram_match("fanctrl_dutycycle", "2", "selected"); %>>67%</option>
														<option class="content_input_fd" value="3" <% nvram_match("fanctrl_dutycycle", "3", "selected"); %>>75%</option>
														<option class="content_input_fd" value="4" <% nvram_match("fanctrl_dutycycle", "4", "selected"); %>>80%</option>
													</select>										
												</td> 
											</tr>

										</table>
									</td>
					  		</tr>

								<tr valign="top" style="height:10px;display:none;" id="btnCtrlTr">
									<td bgcolor="#4D595D" valign="top">
										<table width="99%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
											<thead>
											<tr>
												<td colspan="2">LED button Behavior</td>
											</tr>
											</thead>
											
											<tr>
												<th style="height:120px"><div align="center"><img src="/images/position.png"></div></th>
												<td>
													<div style="cursor:pointer;" onclick="document.form.selLED.click();"><input type="radio" name="selLED" class="input" <% nvram_match("wl_ap_isolate", "1", "checked"); %>>
														LED: <span style="color:#FC0">Press to turn on and off the LED.</span>
													</div>
													<br>
													<div style="cursor:pointer;" onclick="document.form.selCLK.click();"><input type="radio" name="selCLK" class="input" <% nvram_match("wl_ap_isolate", "0", "checked"); %>>
														OverClock: <span style="color:#FC0">Press the button to turn on overclock, release the button to turn off.</span>
													</div>
												</td>
											</tr>
										</table>

									</td>
					  		</tr>

								<tr valign="top" style="height:1px;display:none;" id="btnDescTr">
									<td bgcolor="#4D595D" valign="top" align="center">
										<br/>
										<table style="width:90%">
											<tr height="10px">
												<td width="20%" valign="center" align="right">
													<img src="/images/btnReleased.png">
												</td>
												<td width="5%"></td>
												<td align="left" width="75%" valign="center">
													<table>
														<tr height="30px">
															<td valign="middle">
																<div class="btnTitle">Released</div>
															</td>
														</tr>	
														<tr height="50px">
															<td valign="top">
																<div id="btnReleased" class="btnDesc">Release the button to turn off overclock, <#Web_Title2#> will reboot automatically.</div>		
															</td>
														</tr>	
													</table>
												</td>
											</tr>

											<tr height="10px"></tr>

											<tr height="10px">
												<td width="20%" valign="center" align="right">
													<img src="/images/btnPressed.png">
												</td>
												<td width="5%"></td>
												<td align="left" width="75%" valign="center">
													<table>
														<tr height="30px">
															<td valign="middle">
																<div class="btnTitle">Pressed</div>		
															</td>
														</tr>	
														<tr height="90px">
															<td valign="top">
																<div id="btnPressed" class="btnDesc">
																	Press the button to turn on overclock, this process will increase the clock frequency of your <#Web_Title2#> to 1000Mhz and reboot automatically.
																	<div id='alertHint' style='color: #FF1F00;'>If <#Web_Title2#> does not respond when you turn on overclock, please turn off overclock, power off and on to reboot <#Web_Title2#>.</div>
																</div>		
															</td>
														</tr>	
													</table>
												</td>
											</tr>

										</table>
									</td>
					  		</tr>

								<tr valign="top" style="height:10px">
									<td bgcolor="#4D595D" valign="top">
										<div class="apply_gen">
											<input class="button_gen" onclick="applyRule();" type="button" value="<#CTL_apply#>"/>
										</div>
									</td>
					  		</tr>

								<tr valign="top">
									<td bgcolor="#4D595D" valign="top">
										<div>
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
