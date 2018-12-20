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
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script language="JavaScript" type="text/javascript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/js/httpApi.js"></script>
<script language="JavaScript" type="text/javascript" src="/js/asus_eula.js"></script>
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
	padding: 30px 0px 50px 25px;
	position: absolute;
}

.and_you_can{
	width:421px;
	height:337px;
	background-position:center;
	background-attachment:fixed;
	background:url(images/New_ui/smh_step_4_flow.svg) no-repeat center;
	margin:auto;
	background-size: 365px 292px;
}

.alertpin{
	width:400px;
	height:auto;
	position:absolute;
	background: rgba(0,0,0,0.95);
	z-index:10;
	margin:-215px;
	border-radius:10px;
	padding:10px;
	display: none;
}

.title_num_div {
	vertical-align:top;
	padding-top:34px;
}

.title_num {
	display: block;
	width: 21px;
	height: 21px;
	line-height: 22px;
	-moz-border-radius: 50%;
	-webkit-border-radius: 50%;
	border-radius: 50%;
	background-color: #C0C0C0;
	text-align: center;
	font-weight: 700;
	font-family: sans-serif;
	color: #4D595D;
	font-size: 16px;
}

.step_div {
	vertical-align:middle;
	padding-top:35px;
	font-size:16px;
	padding-left:8px;
	color:#c0c0c0;
}
</style>
<script>

var remaining_time = 120;
var remaining_time_min;
var remaining_time_sec;
var remaining_time_show;
var countdownid;
var MAX_RETRY_NUM = 5;
var external_ip_retry_cnt = MAX_RETRY_NUM;
var external_ip = -1;
var AAE_MAX_RETRY_NUM = 3;
var flag = '<% get_parameter("flag"); %>';
var realip_state = "";

var StatusList = {
	"NoInetrnet": "<#Alexa_Status_Disconnect#>",
	"SvrFail": "Server connection failed",
	"StepAccount": "<#Alexa_Status_Account#>",
	"EnableRemoteCtrl": "<#Alexa_Register1#>",
	"Success": "Amazon Alexa account is registered"
}

var AccLinkStatus = {
	"RemoteStatus":{
		"ddns_enable_x":'<% nvram_get("ddns_enable_x"); %>',
		"ddns_hostname_x":'<% nvram_get("ddns_hostname_x"); %>',
		"ddns_server_x":'<% nvram_get("ddns_server_x"); %>',
		"misc_http_x":'<% nvram_get("misc_http_x"); %>',
		"link_internet":'<% nvram_get("link_internet"); %>',
	},
	"getoken":{
		"ifttt_token":('<% nvram_match_x("","ifttt_token", "", "1"); %>' == '1')?false:true,
	},
	"AAE_SIP":{
		"aae_sip_connected":'<% nvram_get("aae_sip_connected"); %>',
	}
}

var Amazon_URLs = [
	{ "Region": "United States", "WebSite": "https://www.amazon.com/ASUS-ROUTER/dp/B07285G1RK" },
	{ "Region": "United Kingdom and Ireland", "WebSite": "https://www.amazon.co.uk/ASUS-ROUTER/dp/B07285G1RK" },
	{ "Region": "Canada", "WebSite": "https://www.amazon.ca/ASUS-ROUTER/dp/B07285G1RK" },
	{ "Region": "Germany and Austria", "WebSite": "https://www.amazon.de/ASUS-ROUTER/dp/B07285G1RK" },
	{ "Region": "India", "WebSite": "https://www.amazon.in/ASUS-ROUTER/dp/B07285G1RK" },
	{ "Region": "Japan", "WebSite": "https://www.amazon.co.jp/ASUS-ROUTER/dp/B07285G1RK" },
	{ "Region": "Australia and New Zealand", "WebSite": "https://www.amazon.com.au/ASUS-ROUTER/dp/B07285G1RK" },
]

function initial(){
	show_menu();

	httpApi.faqURL("1033393", function(url){document.getElementById("faq").href=url;});
	create_AmazonRegion_select();

	if(!ifttt_support){
		$(document).attr("title","Alexa");
		document.getElementById("divSwitchMenu").style.display = "none";
		document.getElementById("formfonttitle").innerHTML = "Amazon Alexa Skill";
	}
	if('<% nvram_get("fw_lw_enable_x"); %>' == '1')
		document.getElementById("network_services_Remind").style.display = "";

	if(flag == 'from_endpoint'){
		AAE_MAX_RETRY_NUM = 10;
		get_activation_code();
	}

	tag_control();
	setTimeout("get_real_ip();", 1000);
	setTimeout("update_acc_link_status();", 1000);

	if(isSupport("amazon_avs")){
		document.getElementById("amazon_avs_div").style.display = "";
	}
}

function create_AmazonRegion_select(){
	var select = document.getElementById("service_region");
	var text = "";
	var selected = false;

	if(Amazon_URLs.length > 0){
		select.length = 0;
		for(var i = 0; i < Amazon_URLs.length; i++){
			text = Amazon_URLs[i].Region;
			if(Amazon_URLs[i].Region == "United States")
				selected = true;
			else
				selected = false;

			var option = new Option(text, Amazon_URLs[i].WebSite, false, selected);
			select.options.add(option);
		}
	}
}

function tag_control(){
	var obj;
	if((obj = document.getElementById('remote_control_here')) != null){
		obj.style="text-decoration: underline;cursor:pointer;";
		obj.onclick=function(){
			ASUS_EULA.config(enable_remote_control, function(){});
			if(ASUS_EULA.check('asus')){
				enable_remote_control();
			}
		};
	}
}

function get_real_ip(){
	if(AccLinkStatus.RemoteStatus.link_internet == '2'){
		$.ajax({
			url: 'get_real_ip.asp',
			dataType: 'script',
			error: function(xhr){
				setTimeout("get_real_ip();", 3000);
			},
			success: function(response){
				external_ip_retry_cnt--;
				if(realip_state != "2" && external_ip_retry_cnt > 0){
					setTimeout("get_real_ip();", 3000);
				}
			}
		});
	}else{
		external_ip_retry_cnt--;
		setTimeout("get_real_ip();", 3000);
	}
}

function enable_remote_control(){
	if(confirm("<#Alexa_Register_confirm#>")){
		require(['/require/modules/makeRequest.js'], function(makeRequest){
			makeRequest.start('/enable_remote_control.cgi',hide_remote_control , function(){});
		});
	}
}

function hide_remote_control(){
	stopFlag = 1;
	showLoading(5);
	setTimeout("location.href=document.form.current_page.value", 5000);
}

function send_gen_pincode(){

	close_alert('alert_ASUS_EULA');

	if(flag == 'from_endpoint')
		location.href = "/send_IFTTTPincode.cgi";
	else
		gen_new_pincode();
}

function detcet_aae_state(){
	$.ajax({
		url: '/appGet.cgi?hook=nvram_get(aae_enable)',
		dataType: 'json',
		error: function(xhr){
		setTimeout("detcet_aae_state()", 1000);
		},
		success: function(response){
			if(response.aae_enable != '0')
				send_gen_pincode();
			else{
				AAE_MAX_RETRY_NUM--;
				if(AAE_MAX_RETRY_NUM == 0)
					send_gen_pincode();
				else
					setTimeout("detcet_aae_state()", 1000);
			}
		}
	});
}

function get_activation_code(){
	close_alert('alert_pin');
	ASUS_EULA.config(get_activation_code, function(){});
	if(ASUS_EULA.check("asus"))
		gen_new_pincode();
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

function close_alert(name){
	if(name == 'alert_pin'){
		clearInterval(countdownid);
	}
	$('#'+name).fadeOut(100);
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

	document.getElementById(obj).style.marginLeft = (blockmarginLeft-400)+"px";
}

function countdownfunc(){
	remaining_time_min = checkTime(Math.floor(remaining_time/60));
	remaining_time_sec = checkTime(Math.floor(remaining_time%60));
	remaining_time_show = remaining_time_min +":"+ remaining_time_sec;
	document.getElementById("rtime").innerHTML = remaining_time_show;
	if (remaining_time<0){
		clearInterval(countdownid);
		setTimeout("close_alert('alert_pin');", 2000);
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
	input.select();
	document.execCommand('Copy');
	input.remove();
}

function update_acc_link_status(){

	AccLinkStatus.RemoteStatus = httpApi.nvramGet(["ddns_enable_x", "ddns_hostname_x", "ddns_server_x", "misc_http_x", "link_internet"],true);
	AccLinkStatus.AAE_SIP = httpApi.nvramGet(["aae_sip_connected"],true);
	AccLinkStatus.getoken = httpApi.nvram_match_x("ifttt_token","","1");

	setTimeout("update_acc_link_status()", 5000);
	show_account_state();
}

function show_account_state(){

	var RetDDNSstatus = function(){

		if(AccLinkStatus.RemoteStatus.ddns_enable_x == '0' || AccLinkStatus.RemoteStatus.ddns_hostname_x == '' || AccLinkStatus.RemoteStatus.misc_http_x == '0')
			return false;
		else
			return true;
	}

	var RetAccLink = {
		"AccLink":(AccLinkStatus.getoken.ifttt_token == '1')?false:true,
		"AAE_SIP":(AccLinkStatus.AAE_SIP.aae_sip_connected == "1")?true:false,
		"DDNSLink":RetDDNSstatus()
	}

	var RetStatus;
	if(AccLinkStatus.RemoteStatus.link_internet != "2"){
		RetStatus = StatusList.NoInetrnet;
	}
	else if(external_ip == 1){	//public ip
		if(RetAccLink.DDNSLink)
			RetStatus = (RetAccLink.AccLink)?StatusList.Success:StatusList.StepAccount;
		else
			RetStatus = StatusList.EnableRemoteCtrl;
	}
	else{
		if(RetAccLink.AccLink)
			RetStatus = (RetAccLink.AAE_SIP)?StatusList.Success:StatusList.SvrFail;
		else
			RetStatus = StatusList.StepAccount;
	}

	document.getElementById("acc_link_status").innerHTML = RetStatus;

	if(RetStatus == StatusList.EnableRemoteCtrl)
		tag_control();
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
									<div id="formfonttitle" class="formfonttitle">Alexa & IFTTT - Amazon Alexa Skill</div>
									<div id="divSwitchMenu" style="margin-top:-40px;float:right;"><div style="width:110px;height:30px;float:left;border-top-left-radius:8px;border-bottom-left-radius:8px;" class="block_filter_pressed"><div class="tab_font_color" style="text-align:center;padding-top:5px;font-size:14px">Amazon Alexa Skill</div></div><div style="width:110px;height:30px;float:left;border-top-right-radius:8px;border-bottom-right-radius:8px;" class="block_filter"><a href="Advanced_Smart_Home_IFTTT.asp"><div class="block_filter_name">IFTTT</div></a></div></div>
									<div style="margin:10px 0 10px 5px;" class="splitLine"></div>
									<div class="div_table">
											<div class="div_tr">
												<div class="div_td div_desc" style="width:55%">
													<div style="font-weight:bolder;font-size:16px;padding:20px 40px"><#Alexa_Desc1#></div>
													<div style="padding:0px 40px;font-family:Arial, Helvetica, sans-serif;font-size:13px;">
														<span><#Alexa_Desc2#></span>
														<p style="font-size:13px;padding-top: 20px;font-style:italic;"><#Alexa_Example0#></p>
														<p style="font-size:13px;padding-left: 20px;font-style:italic;">“<#Alexa_Example1#>”</p>
														<p style="font-size:13px;padding-left: 20px;font-style:italic;">“<#Alexa_Example2#>”</p>
														<p style="font-size:13px;padding-left: 20px;font-style:italic;">“<#Alexa_Example3#>”</p>
														<a id="faq" href="" style="font-family:Arial, Helvetica, sans-serif;font-size:13px;padding-top: 2px;padding-left: 20px;font-style:italic;text-decoration: underline;cursor:pointer;" target="_blank"><#Alexa_More_Skill#></a>
														<p id="network_services_Remind" style="font-size:13px;padding-top: 10px;font-style:italic;color:#FFCC00;font-size:13px;display: none;">WARNING: The current network service filter policy for firewall will be overwritten once you say “Alexa, ask ASUS Router to pause the Internet</p>
													</div>
													<div style="text-align:center;padding-top:35px;font-family:Arial, Helvetica, sans-serif;font-style:italic;font-weight:lighter;font-size:18px;"><#Alexa_Register0#></div>
													<div id="acc_link_status" style="text-align:center;padding-top:10px;font-size:15px;color:#FFCC00;font-weight:bolder; height:20px;"></div>
													<div class="div_img">
														<table style="width:99%">
															<div class="div_td" style="padding-top:20px; width:45%">
																<div class="div_tr">
																	<div class="div_td">
																		<div class="title_num">1</div>
																	</div>
																	<div class="div_td step_div" style="padding-top:0px;">
																		Select and go to your preferred Amazon country/region website
																		<select class="input_option" id="service_region" name="service_region" style="margin-top: 10px;">
																		</select>
																		<input class="button_gen" style="margin-top: 10px;" type="button" onclick="window.open(document.form.service_region.value);" value="GO">
																	</div>
																</div>
																<div class="div_tr">
																	<div class="div_td title_num_div">
																		<div class="title_num">2</div>
																	</div>
																	<div class="div_td step_div">
																		<span style="text-decoration:underline; cursor: pointer;" onclick="get_activation_code();"><#Get_Activation_Code#></span>
																	</div>
																</div>
																<div class="div_tr">
																	<div class="div_td title_num_div">
																		<div class="title_num">3</div>
																	</div>
																	<div class="div_td step_div"><#Link_Amazon_and_Router#></div>
																	</div>
																<div id="amazon_avs_div" class="div_tr" style="display: none;">
																	<div class="div_td title_num_div">
																		<div class="title_num">4</div>
																	</div>
																	<div class="div_td step_div">
																		In order to use Amazon Alexa Skill, Lyra Voice needs to bind with Amazon account first. Please setup/sign-in to Amazon via <span id="app_span" style="text-decoration: underline;cursor:pointer;">ASUS Router app</span>
																	</div>
																</div>
															</div>
															<div class="div_td" style="vertical-align:middle; width: 55%;">
																	<div class="and_you_can"></div>
															</div>
														</table>
													</div>
													<div id="alert_pin" class="alertpin">
														<table style="width:99%">
															<tr>
																<th colspan="2">
																	<div style="font-size:14px;padding-bottom:8px;"><#Alexa_pin_desc#></div>
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
																		<input class="button_gen" type="button" onclick="close_alert('alert_pin');" value="<#CTL_close#>">
																	</div>
																</td>
															</tr>
														</table>
													</div>
												</div>
											</div>
									</div>
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
</form>
<div id="footer"></div>
</body>
</html>
