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
<title>Alexa & IFTTT</title>
<link rel="stylesheet" type="text/css" href="index_style.css">
<link rel="stylesheet" type="text/css" href="form_style.css">
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/form.js"></script>
<script language="JavaScript" type="text/javascript" src="/help.js"></script>
<script language="JavaScript" type="text/javascript" src="/js/jquery.js"></script>
<style>
.div_table{
	display:table;
}
.div_tr{
	display:table-row;
}
.div_td{
	display:table-cell;
}
.div_desc{
	position:relative;
	vertical-align:top;
}

.div_img {
	text-align:center;
	padding-top: 35px;
	padding-left:25px;
}

.step_1{
	width:30px;
	height:30px;
	background-position:center;
	background-attachment:fixed;
	background:url(images/New_ui/smh_step_1.png) no-repeat center;
	margin:auto;
}

.step_2{
	width:30px;
	height:30px;
	background-position:center;
	background-attachment:fixed;
	background:url(images/New_ui/smh_step_2.png) no-repeat center;
	margin:auto;
}

.step_2_text{
	width:101px;
	height:22px;
	background-position:center;
	background-attachment:fixed;
	background:url(images/New_ui/smh_step_2_text.png) no-repeat center;
	margin:auto 10px;
}

.step_3{
	width:30px;
	height:30px;
	background-position:center;
	background-attachment:fixed;
	background:url(images/New_ui/smh_step_3.png) no-repeat center;
	margin:auto;
}

.step_3_acclink{
	width:219px;
	height:181px;
	background-position:center;
	background-attachment:fixed;
	background:url(images/New_ui/smh_step_3_acclink.png) no-repeat center;
	margin:auto;
	background-size: 219px 181px;
}

.and_you_can{
	width:421px;
	height:337px;
	background-position:center;
	background-attachment:fixed;
	background:url(images/New_ui/smh_step_4_flow.png) no-repeat center;
	margin:auto;
	background-size: 421px 337px;
}

.smh_asus_router{
	width:146px;
	height:146px;
	background-position:center;
	background-attachment:fixed;
	background:url(images/New_ui/smh_asus_router.png) no-repeat center;
	margin:41px 0px 0px 287px;
	background-size: 146px 146px;
	position: absolute;
}

.alertpin{
	width:400px;
	height:auto;
	position:absolute;
	background: rgba(0,0,0,0.95);
	z-index:10;
	margin-left:100px;
	border-radius:10px;
	padding:10px;
	display: none;
}
</style>
<script>

var remaining_time = 120;
var remaining_time_min;
var remaining_time_sec;
var remaining_time_show;
var countdownid;

function initial(){
	show_menu();

	if(!ifttt_support){
		document.getElementById("divSwitchMenu").style.display = "none";
		document.getElementById("formfonttitle").innerHTML = "Amazon Alexa"
	}
	if('<% nvram_get("fw_lw_enable_x"); %>' == '1')
		document.getElementById("network_services_Remind").style.display = "";
}

function gen_new_pincode(){
	require(['/require/modules/makeRequest.js'], function(makeRequest){
		makeRequest.start('/get_IFTTTPincode.cgi', show_alert_pin, function(){});
	});
}

function show_alert_pin(xhr){

	var response = JSON.parse(xhr.responseText);
	remaining_time = 120;

	cal_panel_block("alert_pin");
	$('#alert_pin').fadeIn(300);

	document.getElementById("gen_pin").innerHTML = response.ifttt_pincode;

	countdownfunc();
	countdownid = window.setInterval(countdownfunc,1000);
}

function close_alert_pin(){
	clearInterval(countdownid);
	$('#alert_pin').fadeOut(100);
}

function checkTime(i){
	if (i<10){
		i="0" + i
	}
	return i
}

function cal_panel_block(obj){
	var blockmarginLeft;
	if (window.innerWidth)
		winWidth = window.innerWidth;
	else if ((document.body) && (document.body.clientWidth))
		winWidth = document.body.clientWidth;

	if (document.documentElement  && document.documentElement.clientHeight && document.documentElement.clientWidth){
		winWidth = document.documentElement.clientWidth;
	}

	if(winWidth >1050){
		winPadding = (winWidth-1050)/2;
		winWidth = 1105;
		blockmarginLeft= (winWidth*0.2)+winPadding;
	}
	else if(winWidth <=1050){
		blockmarginLeft= (winWidth)*0.2 + document.body.scrollLeft;
	}

	if(obj == "alert_pin"){
		document.getElementById(obj).style.marginLeft = (blockmarginLeft - 190)+"px";
	}
	else
		document.getElementById(obj).style.marginLeft = blockmarginLeft+"px";
}

function countdownfunc(){
	remaining_time_min = checkTime(Math.floor(remaining_time/60));
	remaining_time_sec = checkTime(Math.floor(remaining_time%60));
	remaining_time_show = remaining_time_min +":"+ remaining_time_sec;
	document.getElementById("rtime").innerHTML = remaining_time_show;
	if (remaining_time<0){
		clearInterval(countdownid);
		setTimeout("close_alert_pin();", 2000);
	}
	remaining_time--;
}

function clipboard(ID_value)
{
	var input = document.createElement('textarea');
	document.body.appendChild(input);
	if(document.getElementById(ID_value).value == undefined)
		input.value = document.getElementById(ID_value).innerHTML;
	else
		input.value = document.getElementById(ID_value).value;
	input.focus();
	input.select();
	document.execCommand('Copy');
	input.remove();
}

</script>
</head>
<body onload="initial();" onunLoad="return unload_body();">
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="current_page" value="Advanced_Smart_Home_Alexa.asp">
<input type="hidden" name="next_page" value="Advanced_Smart_Home_Alexa.asp">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_wait" value="5">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>" disabled>
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
					<td valign="top" >
						<table width="760px" border="0" cellpadding="4" cellspacing="0" class="FormTitle" id="FormTitle">
							<tbody>
							<tr>
								<td bgcolor="#4D595D" valign="top">
									<div>&nbsp;</div>
									<div class="formfonttitle">Alexa & IFTTT - Amazon Alexa</div>
									<div id="divSwitchMenu" style="margin-top:-40px;float:right;"><div style="width:110px;height:30px;float:left;border-top-left-radius:8px;border-bottom-left-radius:8px;" class="block_filter_pressed"><div class="tab_font_color" style="text-align:center;padding-top:5px;font-size:14px">Amazon Alexa</div></div><div style="width:110px;height:30px;float:left;border-top-right-radius:8px;border-bottom-right-radius:8px;" class="block_filter"><a href="Advanced_Smart_Home_IFTTT.asp"><div class="block_filter_name">IFTTT</div></a></div></div>
									<div style="margin-left:5px;margin-top:10px;margin-bottom:10px"><img src="/images/New_ui/export/line_export.png"></div>

									<div class="div_table">
											<div class="div_tr">
												<div class="div_td div_desc" style="width:55%">
													<div style="font-weight:bolder;font-size:16px;padding:25px 40px">
														Control ASUS Router via Amazon Alexa
													</div>
													<div style="padding:0px 40px;font-family:Arial, Helvetica, sans-serif;font-size:13px;">
														<span>Control your ASUS Router with simple voice commands using Amazon Alexa and ASUS Router skill. When paired with Amazon Alexa, you can ask ASUS Router to perform various tasks and no need to open browser login with administrator or ASUS Router app on your phone, for example:
														</span>
														<p style="font-size:13px;padding-top: 20px;font-style:italic;">Try Saying:</p>
														<p style="font-size:13px;padding-left: 20px;font-style:italic;">“Alexa, ask ASUS ROUTER to turn on Guest Network for two hours”</p>
														<p style="font-size:13px;padding-left: 20px;font-style:italic;">“Alexa, ask ASUS ROUTER to turn off Guest Network”</p>
														<p style="font-size:13px;padding-left: 20px;font-style:italic;">“Alexa, ask ASUS ROUTER to pause the Internet”</p>
														<p style="font-family:Arial, Helvetica, sans-serif;font-size:13px;padding-top: 2px;padding-left: 20px;font-style:italic;text-decoration: underline;cursor:pointer;">More Skills</p>
														<p id="network_services_Remind" style="font-size:13px;padding-top: 10px;font-style:italic;color:#FFCC00;font-size:13px;display: none;">
														Remind : The policy of network services filter you are using in firewall will be covered once you saying “Alexa, ask ASUS Router to block devices from internet access”
														</p>
													</div>
													<div style="text-align:center;padding-top:60px;font-family:Arial, Helvetica, sans-serif;font-style:italic;font-weight:lighter;font-size:18px;">
														Create a smarter home with ASUS Router and Alexa!
													</div>
													<div class="div_img">
														<table style="width:99%">
															<div class="div_td" style="vertical-align:middle;">
																<div class="div_tr">
																	<div class="div_td" style="vertical-align:middle;">
																		<div class="step_1"></div>
																	</div>
																	<div class="div_td" style="vertical-align:middle;">
																		<div style="text-align:left;margin-top:5px;padding-left:3px;">
																					<iframe width="180" height="56" frameBorder="0" src="https://nwep.asus.com/loginWithAmazon.html" ></iframe>
																		</div>
																	</div>
																</div>
																<div class="div_tr">
																	<div class="div_td" style="vertical-align:middle;padding-top:30px;">
																		<div class="step_2"></div>
																	</div>
																	<div class="div_td" style="vertical-align:middle;padding-top:30px;">
																		<div>
																			<div class="div_td" style="vertical-align:middle;">
																				<div class="step_2_text"></div>
																			</div>
																			<div class="div_td" style="vertical-align:middle;">
																				<div style="text-align:right;">
																			<input class="button_gen_short" type="button" onclick="window.open('http://alexa.amazon.com/spa/index.html#skills/beta/amzn1.ask.skill.ea91f4d4-81a2-463a-bbba-8ac4b4506401');" value="GO">
																				</div>
																			</div>
																		</div>
																	</div>
																</div>
																<div class="div_tr">
																	<div class="div_td" style="vertical-align:top;padding-top:40px;">
																		<div class="step_3"></div>
																	</div>
																	<div class="div_td" style="vertical-align:middle;padding-top:30px;">
																		<div class="step_3_acclink"></div>
																	</div>
																</div>
															</div>
															<div class="div_td" style="vertical-align:middle;padding-left:23px;padding-top: 23px;">
																	<div class="smh_asus_router"></div>
																	<div class="and_you_can"></div>
															</div>
														</table>
													</div>
													<div id="alert_pin" class="alertpin">
														<table style="width:99%">
															<tr>
																<th colspan="2">
																	<div style="font-size:14px;padding-bottom:8px;">Manually paired ASUS router and your Amazon Alexa, please copy&paste below activate PIN to Link ASUS router and Alexa webpage.</div>		
																</th>
															</tr>
															<tr>
																<td colspan="2">
																	<table class="FormTable" width="60%" border="1" align="center" cellpadding="4" cellspacing="0">
																		<tr>
																			<td>
																				<div style="text-align: center;">
																					<span style="line-height:30px;" id="gen_pin"><span>
																				</div>
																			</td>
																			<div style="text-align: right;padding-top: 10px;margin:0px 23px -30px 0px;" id='rtime'></div>
																		</tr>
																	</table>
																</td>
															</tr>
															<tr>
																<td>
																	<div style="text-align:right;padding:20px 10px 0px 0px;">
																		<input class="button_gen" type="button" onclick="clipboard('gen_pin');" value="Copy">
																	</div>
																</td>
																<td>
																	<div style="text-align:left;padding:20px 0px 0px 10px;">
																		<input class="button_gen" type="button" onclick="close_alert_pin();" value="<#CTL_close#>">
																	</div>
																</td>
															</tr>
														</table>
													</div>
												</div>
											</div>
											<div style="padding:102px 19px 0px 0px;text-align:right;">
												<span style="cursor:pointer;font-family:Arial, Helvetica, sans-serif;font-style:italic;font-weight:lighter;font-size:14px;text-decoration: underline;" onclick="gen_new_pincode();" >Advanced</span>
											</div>
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