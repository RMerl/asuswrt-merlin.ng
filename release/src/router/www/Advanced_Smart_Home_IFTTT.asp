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
	padding-top: 30px;
	padding-left:25px;
}

.step_1{
	width:30px;
	height:30px;
	background-position:center;
	background-attachment:fixed;
	background:url(images/New_ui/smh_step_1.png) no-repeat center;
	margin-top:2px;
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

.smh_ifttt_asus_channel{
	width:256px;
	height:170px;
	background-position:center;
	background-attachment:fixed;
	background:url(images/New_ui/smh_ifttt_asus_channel.png) no-repeat center;
	margin:auto;
	background-size: 96%;
	margin-right:23px;
}

.smh_ifttt{
	width:405px;
	height:170px;
	background-position:center;
	background-attachment:fixed;
	background:url(images/New_ui/smh_ifttt.png) no-repeat center;
	margin:auto;
	background-size: 93%;
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

	if(!alexa_support){
		document.getElementById("divSwitchMenu").style.display = "none";
		document.getElementById("formfonttitle").innerHTML = "IFTTT"
	}
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
									<div class="formfonttitle">Alexa & IFTTT - IFTTT</div>
									<div id="divSwitchMenu" style="margin-top:-40px;float:right;"><div style="width:110px;height:30px;float:left;border-top-left-radius:8px;border-bottom-left-radius:8px;" class="block_filter"><a href="Advanced_Smart_Home_Alexa.asp"><div class="block_filter_name">Amazon Alexa</div></a></div><div style="width:110px;height:30px;float:left;border-top-right-radius:8px;border-bottom-right-radius:8px;" class="block_filter_pressed"><div class="tab_font_color" style="text-align:center;padding-top:5px;font-size:14px">IFTTT</div></div></div>
									<div style="margin-left:5px;margin-top:10px;margin-bottom:10px"><img src="/images/New_ui/export/line_export.png"></div>

									<div class="div_table">
											<div class="div_tr">
												<div class="div_td div_desc" style="width:55%">
													<div style="font-weight:bolder;font-size:16px;padding:25px 40px">
														Using IFTTT with ASUS Router
													</div>
													<div style="padding:0px 40px;font-family:Arial, Helvetica, sans-serif;font-size:13px;">
														<span>IFTTT short for "if this then that." It allows you connect several websites, apps, and Internet-connected devices with each other through rules (or “Applets”). The Applet use specific condition to perform all kinds of custom tasks for you. Connect ASUS Router to IFTTT and start a automation life with creative applets on ASUS Router channel, for example:
														</span>
														<p style="font-size:13px;padding-top: 20px;padding-left: 20px;font-style:italic;">“If closes to dinner time or bedtime, then turn off wifi”</p>
														<p style="font-size:13px;padding-left: 20px;font-style:italic;">“If my daughter comes home, then send me and email, text, or phone call”</p>
														<p style="font-size:13px;padding-left: 20px;font-style:italic;">“If my wifi comes home, then play my favorite song”</p>
														<p style="font-size:13px;padding-left: 20px;font-style:italic;">“If I am playing game, then boost the game speed”</p>
														<p style="font-size:13px;padding-top: 2px;padding-left: 20px;font-style:italic;text-decoration: underline;cursor:pointer;">More Applets</p>
													</div>
													<div style="text-align:center;padding-top:60px;font-family:Arial, Helvetica, sans-serif;font-style:italic;font-weight:lighter;font-size:18px;">
														Start a automation life with ASUS Router and IFTTT!
													</div>
													<div class="div_img">
														<table style="width:99%">
															<div style="font-size:20px;color:#c0c0c0;padding-bottom:20px;">Get started :</div>
															<div class="div_td" style="vertical-align:middle;">
																<div class="div_tr" style="height: 45px;">
																	<div class="div_td" style="vertical-align:top;">
																		<div class="step_1"></div>
																	</div>
																	<div class="div_td" style="font-size:16px;padding:5px 0px 0px 10px;">
																		<div style="color:#c0c0c0;">
																		<a style="font-weight:bolder;text-decoration:underline;color:#FFCC00;" href="https://ifttt.com/login" target="_blank">Sign in</a>
														 				with your IFTTT account.
																		</div>
																	</div>
																</div>
																<div class="div_tr" style="height: 45px;">
																	<div class="div_td" style="vertical-align:top;padding-top:25px;">
																		<div class="step_2"></div>
																	</div>
																	<div class="div_td" style="width:250px;padding:29px 0px 0px 10px;">
																			<div class="div_td" style="font-size:16px;">
																				<div style="color:#c0c0c0">Go to the 
																				<a style="font-weight:bolder;text-decoration:underline;color:#FFCC00;" href="https://ifttt.com/asusrouter" target="_blank">ASUS Router Channel</a>
																				 and click Connect</div>
																			</div>
																	</div>
																</div>
																<div class="div_tr">
																	<div class="div_td" style="vertical-align:top;padding-top:40px;">
																		<div class=""></div>
																	</div>
																	<div class="div_td" style="vertical-align:middle;padding-top:10px;">
																		<div class="smh_ifttt_asus_channel" style="cursor:pointer;" onclick="window.open('https://ifttt.com/asusrouter');" target="_blank"></div>
																	</div>
																</div>
															</div>
															<div class="div_td" style="padding-left:10px;">
																<div class="div_tr" style="height: 45px;">
																	<div class="div_td" style="vertical-align:top;">
																		<div class="step_3"></div>
																	</div>
																	<div class="div_td" style="font-size:16px;vertical-align:middle;padding:5px 0px 0px 10px;width:300px;">
																		<div style="color:#c0c0c0">Choose or create an IFTTT applet on  
																		<a style="font-weight:bolder;text-decoration:underline;color:#FFCC00;" href="https://ifttt.com/asusrouter" target="_blank">ASUS Router Channel</a>
																		</div>
																	</div>
																</div>
																<div style="font-weight:bolder;font-size:20px;color:#c0c0c0;padding-top:57px;padding-left:15px;">And you can..</div>
																<div class="smh_ifttt" style="cursor:pointer;" onclick="window.open('https://ifttt.com/asusrouter');" target="_blank"></div>
															</div>
														</table>
													</div>
													<div id="alert_pin" class="alertpin">
														<table style="width:99%">
															<tr>
																<th colspan="2">
																	<div style="font-size:14px;padding-bottom:8px;">Manually paired ASUS router and your IFTTT account, please copy&paste below activate PIN to link ASUS router and IFTTT webpage.</div>
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
