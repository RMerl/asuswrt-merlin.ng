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
<title><#Web_Title#> - <#fan_tuning#></title>
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
<script type="text/javascript" src="/js/jquery.js"></script>
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/help.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script language="JavaScript" type="text/javascript" src="/validator.js"></script>
<script type='text/javascript'>
var temp_base = 40;
var d_temp = 20;
var rpm_base = 1500;
var d_rpm = 250;
var fanctrl_info = <% get_fanctrl_info(); %>;
var curr_cpuTemp = "<% get_cpu_temperature(); %>";
var cpuTemp = new Array();
cpuTemp = [curr_cpuTemp];
var curr_rxData = fanctrl_info[3];
var rxData = new Array();
rxData = [(curr_rxData - rpm_base) / (d_rpm / d_temp) + temp_base];
var curr_coreTmp_2 = fanctrl_info[1];
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
var fanctrl_trip_points = "<% nvram_get("fanctrl_trip_points"); %>".replace(/&#62/g, ">").replace(/&#60/g, "<");
var fanctrl_inact_time = "<% nvram_get("fanctrl_inact_time"); %>".replace(/&#58/g, ":").replace(/&#60/g, "<");
var get_wan_lan_status = <% get_wan_lan_status(); %>;
var sfp_status = get_wan_lan_status["portSpeed"]["10G SFP+"];
var sfp_hmsg = "It's better to turn on FAN at lower temperature due to maximum operating temperature of SFP transceiver module is 70°C in general.";	/* untranslated */
						
function initial(){
	show_menu();
	showclock();

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

	if(!power_support || Qcawifi_support){
		inputHideCtrl(document.form.wl0_TxPower, 0);
		inputHideCtrl(document.form.wl1_TxPower, 0);
		document.getElementById("formDesc").innerHTML = "Adjust fan speed depending on your system status.";/* untranslated */
	}

	if(based_modelid == "GT-AXY16000" || based_modelid == "RT-AX89U"){
		var dc = new Array();
		var dcDesc = new Array();
		var temp_hyst = fanctrl_trip_points.split("<");
		var hour_minute = fanctrl_inact_time.split("<");
		cur = '<% nvram_get("fanctrl_dutycycle"); %>';
		dc = [0, -1, 1, 2, 3];	/* auto, off, state 1~3 */
		dcDesc = ["<#Auto#>", "<#btn_disable#>", "<#Low#>", "<#Medium#>", "<#High#>"];
		add_options_x2(document.form.fanctrl_dutycycle, dcDesc, dc, cur);
		if (temp_hyst.length == 4) {
			var th0 = temp_hyst[0].split(">");	/* OFF: 	temp>hyst */
			var th1 = temp_hyst[1].split(">");	/* Low: 	temp>hyst */
			var th2 = temp_hyst[2].split(">");	/* Medium:	temp>hyst */
			var th3 = temp_hyst[3].split(">");	/* High:	temp>hyst */
			if (th0.length == 2 && th1.length == 2 && th2.length == 2 && th3.length == 2
			 && th0[0] >= 0 && th1[0] > 0 && th2[0] > 0 && th3[0] > 0
			 && th0[1] >= 0 && th1[1] > 0 && th2[1] > 0 && th3[1] > 0
			 && th0[0] <= 95 && th1[0] <= 95 && th2[0] <= 95 && th3[0] <= 100
			 && (parseInt(th0[0]) - parseInt(th0[1])) >= 0
			 && (parseInt(th1[0]) - parseInt(th1[1])) > parseInt(th0[0])
			 && (parseInt(th2[0]) - parseInt(th2[1])) > parseInt(th1[0])
			 && (parseInt(th3[0]) - parseInt(th3[1])) > parseInt(th2[0])
			) {
				document.form.trip0.value = th0[0];
				document.form.ltrip0.value = th0[0] - th0[1];
				document.form.trip1.value = th1[0];
				document.form.ltrip1.value = th1[0] - th1[1];
				document.form.trip2.value = th2[0];
				document.form.ltrip2.value = th2[0] - th2[1];
				document.form.trip3.value = th3[0];
				document.form.ltrip3.value = th3[0] - th3[1];
			}
		}
		if (hour_minute.length == 2) {
			var s_time = hour_minute[0].split(":");
			var e_time = hour_minute[1].split(":");
			if (s_time.length == 2 && e_time.length == 2
			 && s_time[0] >= 0 && s_time[0] <= 23
			 && s_time[1] >= 0 && s_time[1] <= 59
			 && e_time[0] >= 0 && e_time[0] <= 23
			 && e_time[1] >= 0 && e_time[1] <= 59
			) {
				document.form.s_hour.value = s_time[0];
				document.form.s_minute.value = s_time[1];
				document.form.e_hour.value = e_time[0];
				document.form.e_minute.value = e_time[1];
			}
		}

		var c_trip1 = parseInt(document.form.trip1.value);
		if (document.form.fanctrl_fullspeed_temp_unit.value == 1) {
			sfp_hmsg = "It's better to turn on FAN at lower temperature due to maximum operating temperature of SFP transceiver module is 158°F in general.";	/* untranslated */

			document.form.trip0.value = Math.round(document.form.trip0.value*9/5)+32;
			document.form.ltrip0.value = Math.round(document.form.ltrip0.value*9/5)+32;
			document.form.trip1.value = Math.round(document.form.trip1.value*9/5)+32;
			document.form.ltrip1.value = Math.round(document.form.ltrip1.value*9/5)+32;
			document.form.trip2.value = Math.round(document.form.trip2.value*9/5)+32;
			document.form.ltrip2.value = Math.round(document.form.ltrip2.value*9/5)+32;
			document.form.trip3.value = Math.round(document.form.trip3.value*9/5)+32;
			document.form.ltrip3.value = Math.round(document.form.ltrip3.value*9/5)+32;
		}
		if (document.form.fanctrl_dutycycle.value == "0") {
			document.getElementById('fan_inact_time').style.display = "";
			document.getElementById('fan_start_temp').style.display = "";
			document.getElementById('fan_stop_temp').style.display = "";
			document.form.fanctrl_trip_points.disabled = false;
			document.form.fanctrl_inact_time.disabled = false;
		}
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

function showclock(){
	JS_timeObj.setTime(systime_millsec);
	systime_millsec += 1000;
	JS_timeObj2 = JS_timeObj.toString();
	JS_timeObj2 = JS_timeObj2.substring(0,3) + ", " +
	              JS_timeObj2.substring(4,10) + "  " +
				  checkTime(JS_timeObj.getHours()) + ":" +
				  checkTime(JS_timeObj.getMinutes()) + ":" +
				  checkTime(JS_timeObj.getSeconds()) + "  " +
				  /*JS_timeObj.getFullYear() + " GMT" +
				  timezone;*/ // Viz remove GMT timezone 2011.08
				  JS_timeObj.getFullYear();
	document.getElementById("system_time").innerHTML = JS_timeObj2;
	setTimeout("showclock()", 1000);
	if(navigator.appName.indexOf("Microsoft") >= 0)
		document.getElementById("textarea").style.width = "99%";
    //document.getElementById("banner3").style.height = "13";
}

function show_hide_auto_mode_opts(value){
	if (based_modelid == "GT-AXY16000" || based_modelid == "RT-AX89U") {
		if (value == "0") {
			/* auto mode */
			document.getElementById('fan_inact_time').style.display = "";
			document.getElementById('fan_start_temp').style.display = "";
			document.getElementById('fan_stop_temp').style.display = "";
			document.form.fanctrl_trip_points.disabled = false;
			document.form.fanctrl_inact_time.disabled = false;
		} else {
			/* off, low, medium, high... */
			document.getElementById('fan_inact_time').style.display = "none";
			document.getElementById('fan_start_temp').style.display = "none";
			document.getElementById('fan_stop_temp').style.display = "none";
			document.form.fanctrl_trip_points.disabled = true;
			document.form.fanctrl_inact_time.disabled = true;
		}
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
	if (based_modelid == "GT-AXY16000" || based_modelid == "RT-AX89U") {
		var c_trips = new Array(parseInt(document.form.trip0.value), parseInt(document.form.trip1.value), parseInt(document.form.trip2.value), parseInt(document.form.trip3.value));
		var c_ltrips = new Array(parseInt(document.form.ltrip0.value), parseInt(document.form.ltrip1.value), parseInt(document.form.ltrip2.value), parseInt(document.form.ltrip3.value));
		var c_hysts = new Array();
		var s_time, e_time;
		var s_hour = parseInt(document.form.s_hour.value);
		var e_hour = parseInt(document.form.e_hour.value);
		var s_minute = parseInt(document.form.s_minute.value);
		var e_minute = parseInt(document.form.e_minute.value);

		// 'F ==> 'C if necessary, calc hysts
		for (var i = 0; i < c_trips.length; ++i) {
			if (document.form.fanctrl_fullspeed_temp_unit.value == 1) {
				c_trips[i] = Math.round((c_trips[i]-32)*5/9);
				c_ltrips[i] = Math.round((c_ltrips[i]-32)*5/9);
			}

			if (i == 1 && c_trips[i] > 93)
				c_trips[i] = 93;
			if (i == 2 && c_trips[i] > 95)
				c_trips[i] = 95;
			else if (i == 3 && c_trips[i] > 100)
				c_trips[i] = 100;

			if (c_ltrips[i] >= c_trips[i])
				c_ltrips[i] = c_trips[i] - 1;
			c_hysts[i] = c_trips[i] - c_ltrips[i];
		}

		// if c_trips[0] < 0 or c_trips[1~3] < 1, set it as 0,1 respectively
		if (c_hysts[0] < 0)
			c_hysts[0] = 0;
		for (var i = 1; i < c_hysts.length; ++i) {
			if (c_hysts[i] < 1)
				c_hysts[i] = 1;
		}

		// trip0/hyst0 is hidden. Make sure c_trips[0] - c_hysts[0] >= 0 and keep c_trips[1~3] - c_hysts[1~3] > c_trips[0~2].
		// The page will be reloaded after we submit it. No need to update new values to form unless it violates another rules.
		if (c_trips[0] >= (c_trips[1] - c_hysts[1]))
			c_trips[0] = c_trips[1] - c_hysts[1] - 1;
		if (c_trips[0] <= 0)
			c_trips[0] = 0;
		if (c_trips[0] - c_hysts[0] < 0)
			c_hysts[0] = 0;
		for (var i = 1; i < c_trips.length; ++i) {
			if ((c_trips[i] - c_hysts[i]) <= c_trips[i-1]) {
				c_hysts[i] = c_trips[i] - c_trips[i-1] - 1;
				if (c_hysts[i] < 1) {
					c_hysts[i] = 1;
					c_trips[i] = c_trips[i-1] + c_hysts[i] + 1;
				}
			}
		}

		// min. c_hysts[0] could be zero due to FAN is off at this trip-point.
		if (c_trips[0] <  0 || c_trips[1] <= 0 || c_trips[2] <= 0 || c_trips[3] <= 0
		 || c_trips[0] > 95 || c_trips[1] > 95 || c_trips[2] > 95 || c_trips[3] > 100
		 || c_hysts[0] <  0 || c_hysts[1] <= 0 || c_hysts[2] <= 0 || c_hysts[3] <= 0
		 || (c_trips[0] - c_hysts[0]) < 0
		 || (c_trips[1] - c_hysts[1]) <= c_trips[0]
		 || (c_trips[2] - c_hysts[2]) <= c_trips[1]
		 || (c_trips[3] - c_hysts[3]) <= c_trips[2]
		) {
			console.log("Invalid trip-points temp,hyst: tp0 " + c_trips[0] + "," + c_hysts[0] + " tp1 " + c_trips[1] + "," + c_hysts[1] + " tp2 " + c_trips[2] + "," + c_hysts[2] + " tp3 " + c_trips[3] + "," + c_hysts[3]);
			return false;
		}

		if (s_hour < 0 || s_hour > 23)
			s_hour = 0;
		if (e_hour < 0 || e_hour > 23)
			e_hour = 0;
		if (s_minute < 0 || s_minute > 59)
			s_minute = 0;
		if (e_minute < 0 || e_minute > 59)
			e_minute = 0;
		if (s_hour == e_hour && s_minute == e_minute && (s_hour != 0 || s_minute != 0))
			s_hour = e_hour = s_minute = e_minute = 0;

		s_time = s_hour * 60 + s_minute;
		e_time = e_hour * 60 + e_minute;

		// Alert user if one of below condition true due to maximum operating temp. of SFP is 70'C in general.
		// 1. auto FAN speed and temperature of low trip-point >= 70'C
		// 2. FAN off
		if (sfp_status != undefined && sfp_status != "X"
		 && ((c_trips[1] >= 70 && document.form.fanctrl_dutycycle.value == "0")
		  || document.form.fanctrl_dutycycle.value == "-1")
		  || (s_time != 0 && s_time == e_time)
		)
				alert(sfp_hmsg);

		document.form.fanctrl_trip_points.value = c_trips[0].toString() + ">" + c_hysts[0].toString() + "<" + c_trips[1].toString() + ">" + c_hysts[1].toString() + "<" + c_trips[2].toString() + ">" + c_hysts[2].toString() + "<" + c_trips[3].toString() + ">" + c_hysts[3].toString();

		document.form.fanctrl_inact_time.value = s_hour.toString() + ":" + s_minute.toString() + "<" + e_hour.toString() + ":" + e_minute.toString();

		if (document.form.fanctrl_trip_points.value != fanctrl_trip_points
		 || document.form.fanctrl_dutycycle.value != document.form.fanctrl_dutycycle_orig.value
		 || document.form.fanctrl_inact_time.value != document.form.fanctrl_inact_time_orig.value
		) {
			if(document.form.action_script.value != "")
				document.form.action_script.value += ";";
			document.form.action_script.value += "restart_fanctrl";
		}
	} else {
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

		if(validator.numberRange(document.form.fanctrl_fullspeed_temp, 25, 70)
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
		}
	}
	showLoading();
	document.form.submit();
}

function changeTempUnit(num){
	cookie.set("CoreTmpUnit", num, 365);
	refreshpage();
}

function setCookie(num){
	cookie.set("CoreTmpUnit", num, 365);
}

function getCookie(c_name)
{
	return cookie.get("CoreTmpUnit");
}
</script>
</head>

<body onload="initial();" class="bg">
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
<input type="hidden" name="fanctrl_trip_points" value="" disabled>
<input type="hidden" name="fanctrl_inact_time_orig" value="<% nvram_get("fanctrl_inact_time"); %>" disabled>
<input type="hidden" name="fanctrl_inact_time" value="" disabled>
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
									  <div class="formfonttitle"><#menu5_6#> - <#fan_tuning#></div>
									  <div style="margin:10px 0 10px 5px;" class="splitLine"></div>
									  <div class="formfontdesc" id="formDesc"><#fan_tuning_desc#></div>
									</td>
					  		</tr>

								<tr style="height:10px">
									<td bgcolor="#4D595D" valign="top">
										<table width="99%" border="0" align="center" cellpadding="0" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
											<thead>
											<tr>
												<td colspan="2"><#fan_tuning_temperature#></td>
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
												<td colspan="2"><#sys_adjustment#></td>
											</tr>
											</thead>
											<tr>
												<th width="20%"><#General_x_SystemTime_itemname#></th>
												<td><span id="system_time" class="devicepin" style="color:#FFFFFF;"></span>
													<br><span id="dstzone" style="display:none;margin-left:5px;color:#FFFFFF;"></span>
												</td>
											</tr>
											
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
														<option class="content_input_fd" value="0" <% nvram_match("fanctrl_mode", "0", "selected"); %>>Auto</option>
														<option class="content_input_fd" value="1" <% nvram_match("fanctrl_mode", "1", "selected"); %>>Manually</option>
													</select>			
												</td>
											</tr>

											<tr>
												<th><#sys_temp_unit#></th>
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
												<th><#sys_fan_speed#></th>
												<td> 
													<select name="fanctrl_dutycycle" class="input_option" onChange="show_hide_auto_mode_opts(this.value);">
														<option class="content_input_fd" value="0" <% nvram_match("fanctrl_dutycycle", "1", "selected"); %>><#Auto#></option>
														<option class="content_input_fd" value="1" <% nvram_match("fanctrl_dutycycle", "1", "selected"); %>>50%</option>
														<option class="content_input_fd" value="2" <% nvram_match("fanctrl_dutycycle", "2", "selected"); %>>67%</option>
														<option class="content_input_fd" value="3" <% nvram_match("fanctrl_dutycycle", "3", "selected"); %>>75%</option>
														<option class="content_input_fd" value="4" <% nvram_match("fanctrl_dutycycle", "4", "selected"); %>>80%</option>
													</select>										
												</td> 
											</tr>

											<tr id="fan_inact_time" style="display:none"><th>Inactive time</th>
											<td><input style="text" name="s_hour" maxlength="3" class="input_3_table" value="00" onKeyPress="return validator.isNumber(this,event);" autocorrect="off" autocapitalize="off">&ensp;:
												<input style="text" name="s_minute" maxlength="3" class="input_3_table" value="00" onKeyPress="return validator.isNumber(this,event);" autocorrect="off" autocapitalize="off">&ensp;~
												<input style="text" name="e_hour" maxlength="3" class="input_3_table" value="00" onKeyPress="return validator.isNumber(this,event);" autocorrect="off" autocapitalize="off">&ensp;:
												<input style="text" name="e_minute" maxlength="3" class="input_3_table" value="00" onKeyPress="return validator.isNumber(this,event);" autocorrect="off" autocapitalize="off">
											</td></tr>
											<tr id="fan_start_temp" style="display:none"><th>Start temperature</th>
												<td><input style="text;display:none;" name="trip0" maxlength="3" class="input_3_table" value="25" onKeyPress="return validator.isNumber(this,event);" autocorrect="off" autocapitalize="off"><#Low#>
												    <input style="text" name="trip1" maxlength="3" class="input_3_table" value="50" onKeyPress="return validator.isNumber(this,event);" autocorrect="off" autocapitalize="off">&ensp;<#Medium#>
													<input style="text" name="trip2" maxlength="3" class="input_3_table" value="93" onKeyPress="return validator.isNumber(this,event);" autocorrect="off" autocapitalize="off">&ensp;<#High#>
													<input style="text" name="trip3" maxlength="3" class="input_3_table" value="100" onKeyPress="return validator.isNumber(this,event);" autocorrect="off" autocapitalize="off">
											</td></tr>
											<tr id="fan_stop_temp" style="display:none"><th>Stop temperature</th>
												<td><input style="text;display:none;" name="ltrip0" maxlength="3" class="input_3_table" value="20" onKeyPress="return validator.isNumber(this,event);" autocorrect="off" autocapitalize="off"><#Low#>
												    <input style="text" name="ltrip1" maxlength="3" class="input_3_table" value="45" onKeyPress="return validator.isNumber(this,event);" autocorrect="off" autocapitalize="off">&ensp;<#Medium#>
													<input style="text" name="ltrip2" maxlength="3" class="input_3_table" value="85" onKeyPress="return validator.isNumber(this,event);" autocorrect="off" autocapitalize="off">&ensp;<#High#>
													<input style="text" name="ltrip3" maxlength="3" class="input_3_table" value="94" onKeyPress="return validator.isNumber(this,event);" autocorrect="off" autocapitalize="off">
											</td></tr>

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
