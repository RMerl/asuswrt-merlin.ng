﻿<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=Edge"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png"><title><#Web_Title#> - <#Game_Boost#></title>
<link rel="stylesheet" type="text/css" href="index_style.css">
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="usp_style.css">
<link rel="stylesheet" type="text/css" href="device-map/device-map.css">
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/form.js"></script>
<script type="text/javascript" src="/js/httpApi.js"></script>
<script language="JavaScript" type="text/javascript" src="/js/asus_eula.js"></script>
<script type="text/javascript" src="/client_function.js"></script>
<script type="text/javascript" src="/form.js"></script>
<script type="text/javascript" src="/validator.js"></script>
<style>
*{
	box-sizing: content-box;
}
body{
	margin: 0;
	color:#FFF;
}
.switch{
	position: relative;
	width: 200px;
	height: 70px;
}

.switch input{
	cursor: pointer;
	height: 100%;
	opacity: 0;
	position: absolute;
	width: 100%;
	z-index: 100;
	left:0;
}
.container{
	background-color: #444;
	width:100%;
	height:100%;
}
.container::after{
	content: '';
	background-color:#999;
	width:50px;
	height:40px;
	position:absolute;
	left: 0;
	top: 0;
	border-top-left-radius:5px;
	border-bottom-left-radius:5px;
}
@-moz-document url-prefix(){ 		/*Firefox Hack*/
	.container::after{
		top:0;
	}
}
@supports (-ms-accelerator:true) {		/*Edge Browser Hack, not work on Edge 38*/
  	.container::after{
		top:0;
	}
}

.switch input:checked~.container{
	background: #D30606;
}
.switch input:checked~.container::after{
	left: 50px;
	border-top-right-radius:5px;
	border-bottom-right-radius:5px;
}
@media all and (-ms-high-contrast:none)
{
    *::-ms-backdrop, .container::after { margin-top: 0px} /* IE11 */
}
.btn{
	background-color: #990000;
	color: #EBE8E8;
}
.btn:hover{
	background-color: #D30606;
	color: #FFF;
}

#gameList_block{
	position: absolute;
	width: 700px;
	height: 600px;
	background-color: #444f53;
	z-index: 199;
	padding: 12px 18px;
	overflow-y: auto;
	margin-top: -40px;
}

.qr_code{
	margin: 8px auto;
	background-image: url('images/New_ui/asus_router_android_qr.png');
	width: 124px;
	height: 124px;
	background-size: 100%;
}
.qr_android{
	background-image: url('images/New_ui/asus_router_android_qr.png');
}
.qr_android_cn{
	background-image: url('images/New_ui/asus_router_android_qr_cn.png');
}
</style>

<script>
window.onresize = function() {
	cal_panel_block("gameList_block", 0.23);
}
var fc_disable_orig = '<% nvram_get("fc_disable"); %>';
var runner_disable_orig = '<% nvram_get("runner_disable"); %>';
var ctf_disable = '<% nvram_get("ctf_disable"); %>';
var ctf_fa_mode = '<% nvram_get("ctf_fa_mode"); %>';
var bwdpi_app_rulelist = "<% nvram_get("bwdpi_app_rulelist"); %>".replace(/&#60/g, "<");
function initial(){
	show_menu();
	/*if((document.form.qos_enable.value == '1') && (document.form.qos_type.value == '1') && (bwdpi_app_rulelist.indexOf('game') != -1)){
		document.getElementById("game_boost_enable").checked = true;
	}
	else{
		document.getElementById("game_boost_enable").checked = false;
	}*/

	if((document.form.qos_enable.value == '1') && (document.form.qos_type.value == '0')){
		document.getElementById("game_priority_enable").checked = true;
	}
	else{
		document.getElementById("game_priority_enable").checked = false;
	}

	if(is_CN || document.form.preferred_lang.value == 'CN'){
		$('#android_qr').removeClass('qr_android').addClass('qr_android_cn');
		$('#android_link').hide();
		$('#android_cn_link').show();
	}

	if(wtfast_support){
		$('#wtfast_1').show();
		$('#wtfast_2').show();
		$('#wtfast_3').show();
	}

	if(!ASUS_EULA.status("tm"))
		ASUS_EULA.config(eula_confirm, cancel);

	setTimeout("showDropdownClientList('setClientIP', 'mac', 'all', 'ClientList_Block_PC', 'pull_arrow', 'all');", 500);
	genGameList();	
}

function sign_eula(){
	if(document.getElementById("game_boost_enable").checked){
		if(!reset_wan_to_fo.check_status()) {
			document.getElementById("game_boost_enable").checked = false;
			return false;
		}
	}

	if(ASUS_EULA.check("tm")){
		check_game_boost();
	}
}

function check_game_boost(){
	if(document.getElementById("game_boost_enable").checked){
		document.form.qos_enable.value = '1';
		document.form.qos_type.value = '1';
		document.form.bwdpi_app_rulelist.disabled = false;
		document.form.bwdpi_app_rulelist.value = "9,20<8<4<0,5,6,15,17<4,13<13,24<1,3,14<7,10,11,21,23<game";
	}
	else{
		document.form.qos_enable.value = '0';
		document.form.bwdpi_app_rulelist.disabled = true;
	}

	if(ctf_disable == 1){
		document.form.action_script.value = "restart_qos;restart_firewall";
	}
	else{
		if(ctf_fa_mode == "2"){
			FormActions("start_apply.htm", "apply", "reboot", "<% get_default_reboot_time(); %>");
		}
		else{
			if(document.form.qos_type.value == 0)
				FormActions("start_apply.htm", "apply", "reboot", "<% get_default_reboot_time(); %>");
			else{
				document.form.action_script.value = "restart_qos;restart_firewall";
			}
		}
	}

	if(reset_wan_to_fo.change_status)
		reset_wan_to_fo.change_wan_mode(document.form);

	document.form.submit();
}

function eula_confirm(){
	document.form.TM_EULA.value = 1;
	document.form.action_wait.value = "15";
	check_game_boost();
}

function cancel(){
	refreshpage();
}

function setClientIP(macaddr){
	document.getElementById('client').value = macaddr;
	hideClients_Block();
}

function hideClients_Block(){
	document.getElementById("pull_arrow").src = "/images/arrow-down.gif";
	document.getElementById('ClientList_Block_PC').style.display='none';
}

function pullLANIPList(obj){
	var element = document.getElementById('ClientList_Block_PC');
	var isMenuopen = element.offsetWidth > 0 || element.offsetHeight > 0;

	if(isMenuopen == 0){		
		obj.src = "/images/arrow-top.gif"
		element.style.display = 'block';		
		document.getElementById('client').focus();		
	}
	else
		hideClients_Block();
}
var gameList = '<% nvram_get("rog_clientlist"); %>'.replace(/&#60/g, "<");;
function genGameList(){
	var list_array = gameList.split('<');
	var code = '';
	code += '<thead><tr><td colspan="4">Game Device List&nbsp;(<#List_limit#>&nbsp;64)</td></tr></thead>';
	code += '<tr>';
	code += '<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,10);"><#Client_Name#> (<#PPPConnection_x_MacAddressForISP_itemname#>)</a></th>';
	code += '<th><#list_add_delete#></th>';
	code += '</tr>';
	code += '<tr>';
	code += '<td width="40%">';
	code += '<input type="text" class="input_20_table" maxlength="17" id="client" style="margin-left:-12px;width:255px;" onKeyPress="return validator.isHWAddr(this,event)" onClick="hideClients_Block();" autocorrect="off" autocapitalize="off" placeholder="ex: <% nvram_get("lan_hwaddr"); %>">';
	code += '<img id="pull_arrow" height="14px;" src="/images/arrow-down.gif" style="position:absolute;*margin-left:-3px;*margin-top:1px;" onclick="pullLANIPList(this);" title="<#select_MAC#>">';
	code += '<div id="ClientList_Block_PC" class="clientlist_dropdown" style="margin-left:138px;"></div>';
	code += '</td>';
	code += '<td width="10%">';
	code += '<div><input type="button" class="add_btn" onClick="addGameList(64);"></div>';
	code += '</td>';
	code += '</tr>';
		
	if(list_array.length == '0'){
		code += '<tr><td colspan="2" style="color:#FFCC00;">No data in table.</td></tr>';
	}
	else{
		for(i=1; i<list_array.length; i++){
			code += '<tr>';
			code += '<td>';
			code += '<div style="display:flex;align-items: center;justify-content: center;padding-left:30px;">';
			code += '<div style="width:20%;"><div id="clientIcon_3497F683C346" class="clientIcon type34" ></div></div>';
			code += '<div style="text-align: left;line-height: 18px;width:50%;">';
			var mac = list_array[i];
			var name = (clientList[mac] != undefined) ? clientList[mac].name : '';
			var ip = (clientList[mac] != undefined) ? clientList[mac].ip : '';
			code += '<div>'+ name +'</div>';		// NAME
			code += '<div>'+ mac +'</div>';		// MAC
			code += '<div>'+ ip +'</div>';		// IP
			code += '</div>';
			code += '</div>';
			code += '</td>';
			code += '<td width="10%">';
			code += '<div><input type="button" class="remove_btn" onClick="delGameList(\''+ mac +'\')"></div>';
			code += '</td>';
			code += '</tr>';
		}
	}
	
	$('#game_list').html(code);
}

function addGameList(){
	var mac = $('#client').val();
	var list_array = gameList.split('<');
	var maximum = '64';
	if(mac == ''){
		alert('client can not be empty!');
		return false;
	}

	if(list_array.length > maximum){
		alert("<#JS_itemlimit1#> " + maximum + " <#JS_itemlimit2#>");
		return false;
	}

	// check mac is whether in the list
	for(i=1; i<list_array.length; i++){
		if(list_array[i] == mac){
			alert('Already is the list');
			return false;
		}
	}

	gameList = '<' + mac + gameList;
	$.ajax({
		url: '/rog_first_qos.cgi',
		dataType: 'json',
		data: {
			rog_mac: mac,
			action: 'add'
		},
		error: function(){},
		success: function(response){
			genGameList();
		}
	});

	setTimeout("showDropdownClientList('setClientIP', 'mac', 'all', 'ClientList_Block_PC', 'pull_arrow', 'all');", 500);
}

function delGameList(target){
	var mac = target;
	var list_array = gameList.split('<');
	var temp = '';
	for(i=1; i<list_array.length; i++){
		if(list_array[i] != mac){
			temp += '<' + list_array[i];
		}	
	}

	gameList = temp;
	$.ajax({
		url: '/rog_first_qos.cgi',
		dataType: 'json',
		data: {
			rog_mac: mac,
			action: 'delete'
		},
		error: function(){},
		success: function(response){
			genGameList();
		}
	});

	setTimeout("showDropdownClientList('setClientIP', 'mac', 'all', 'ClientList_Block_PC', 'pull_arrow', 'all');", 500);
}

function showGameListField(){
	$('#gameList_block').show();
	cal_panel_block("gameList_block", 0.23);
}

function hideGameListField(){
	$('#gameList_block').hide();
}

function enableGamePriority(){
	if(document.form.qos_enable.value == '0'){		// OFF -> ON
		if(document.form.qos_obw.value == '0' || document.form.qos_obw.value == ''){
			document.form.qos_obw.disabled = false;
			document.form.qos_obw.value = '2048000';
		}

		if(document.form.qos_ibw.value == '0' || document.form.qos_ibw.value == ''){
			document.form.qos_ibw.disabled = false;
			document.form.qos_ibw.value = '1024000';
		}
	}
	
	if(document.getElementById("game_priority_enable").checked){
		document.form.qos_enable.value = '1';
		document.form.qos_type.value = '0';
		document.form.action_script.value = 'reboot';

		if(document.form.qos_type.value == 0 && !lantiq_support){
			FormActions("start_apply.htm", "apply", "reboot", "<% get_default_reboot_time(); %>");
		}
	}
	else{
		document.form.qos_enable.value = '0';
	}

	if(ctf_disable == 1 || (fc_disable_orig != '' && runner_disable_orig != '')){
		document.form.action_script.value = "restart_qos;restart_firewall";
	}
	else{
		if(ctf_fa_mode == "2"){
			FormActions("start_apply.htm", "apply", "reboot", "<% get_default_reboot_time(); %>");
		}
		else{
			if(document.form.qos_type.value == 0 && !lantiq_support){
				FormActions("start_apply.htm", "apply", "reboot", "<% get_default_reboot_time(); %>");
			}	
			else{
				document.form.action_script.value = "restart_qos;restart_firewall";
			}
		}
	}

	if(reset_wan_to_fo.change_status)
		reset_wan_to_fo.change_wan_mode(document.form);

	document.form.submit();
}

function applyRule(){
	document.form.submit();
}
</script>
</head>
<body onload="initial();" onunload="unload_body();">
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>
<div id="hiddenMask" class="popup_bg" style="z-index:999;">
	<table cellpadding="5" cellspacing="0" id="dr_sweet_advise" class="dr_sweet_advise" align="center"></table>
	<!--[if lte IE 6.5.]><script>alert("<#ALERT_TO_CHANGE_BROWSER#>");</script><![endif]-->
</div>
<div id="gameList_block" style="display:none">
	<div style="display:flex;justify-content: space-between;align-items: center;">
		<div>
			<input type="button" class="button_gen" value="<#CTL_apply#>" onclick="applyRule();">
		</div>
		<div style="width:28px;height:28px;background-image:url('images/New_ui/cancel.svg');cursor:pointer" onclick="hideGameListField();"></div>
	</div>
	
	<table id="game_list" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable_table" style="margin-top:8px;"></table> 
</div>
<iframe name="hidden_frame" id="hidden_frame" width="0" height="0" frameborder="0" scrolling="no"></iframe>
<form method="post" name="form" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="current_page" value="GameBoost.asp">
<input type="hidden" name="next_page" value="GameBoost.asp">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="restart_qos;restart_firewall">
<input type="hidden" name="action_wait" value="5">
<input type="hidden" name="qos_enable" value="<% nvram_get("qos_enable"); %>">
<input type="hidden" name="qos_type" value="<% nvram_get("qos_type"); %>">
<input type="hidden" name="qos_type_ori" value="<% nvram_get("qos_type"); %>">
<input type="hidden" name="bwdpi_app_rulelist" value="<% nvram_get("bwdpi_app_rulelist"); %>">
<input type="hidden" name="TM_EULA" value="<% nvram_get("TM_EULA"); %>">
<input type="hidden" name="qos_obw" value="<% nvram_get("qos_obw"); %>" disabled>
<input type="hidden" name="qos_ibw" value="<% nvram_get("qos_ibw"); %>" disabled>
</form>
<div>
	<table class="content" align="center" cellspacing="0" style="margin:auto;">
		<tr>
			<td width="17">&nbsp;</td>
			<!--=====Beginning of Main Menu=====-->
			<td valign="top" width="202">
				<div id="mainMenu"></div>
				<div id="subMenu"></div>
			</td>
			<td valign="top">
				<div id="tabMenu" style="*margin-top: -160px;"></div>
				<br>
		<!--=====Beginning of Main Content=====-->
				<div id="FormTitle" style="background:url('images/New_ui/mainimage_img_Game.jpg');background-repeat: no-repeat;margin-top:-15px;border-radius:3px;background-size: cover;">
					<table style="padding-left:10px;">
						<tr>
							<td class="formfonttitle">
								<div style="width:730px;padding-top:10px;">
									<table width="730px">
										<tr>
											<td align="left">

												<div style="display:table-cell;background:url('/images/New_ui/game.svg');width:77px;height:77px;"></div>
												<div class="formfonttitle" style="display:table-cell;font-size:26px;font-weight:bold;color:#EBE8E8;vertical-align:middle">Game</div>
											</td>
										</tr>
									</table>
								</div>
							</td>
						</tr>
					<!-- Service table -->
						<tr>
							<td valign="top" height="0px">
								<div>
									<table style="border-collapse:collapse;width:100%">
										<tbody>
											<!-- Game Boost (Adaptive QoS) -->	
											<!-- <tr>
												<td>
													<div style="padding: 5px 0;font-size:20px;">Game Boost</div>
												</td>
												<td colspan="2">
													<div style="padding: 5px 10px;font-size:20px;color:#FFCC66;"><#Game_Boost_lan_title#></div>
												</td>
											</tr>
											<tr>
												<td colspan="3">
													<div style="width:100%;height:1px;background-color:#D30606"></div>
												</td>
											</tr>
											<tr>
												<td align="center" style="width:85px;">
													<div style="width:97px;height:71px;background:url('images/New_ui/GameBoost_QoS.png');no-repeat"></div>
												</td>
												<td style="width:400px;height:120px;">
													<div style="height:60px;font-size:16px;color:#A0A0A0;padding-left:10px;">
														<#Game_Boost_lan_desc#>
													</div>
												</td>
												<td>
													<div class="switch" style="margin:auto;width:100px;height:40px;text-align:center;line-height:40px;font-size:18px">
														<input id="game_boost_enable" type="checkbox" onclick="sign_eula();">
														<div class="container" style="display:table;border-radius:5px;">
															<div style="display:table-cell;width:50%;">
																<div>ON</div>
															</div>
															<div style="display:table-cell">
																<div>OFF</div>
															</div>
														</div>
													</div>
												</td>
											</tr> -->
											<!-- Gear Accelerator -->
											<!-- <tr style="height:50px;"></tr> -->
											<tr>
												<td style="width:200px">
													<div style="padding: 5px 0;font-size:20px;">Gear Accelerator</div>
												</td>
												<td colspan="2">
													<div style="padding: 5px 10px;font-size:20px;color:#FFCC66">Game Device Prioritizing</div>
												</td>
											</tr>
											<tr>
												<td colspan="3">
													<div style="width:100%;height:1px;background-color:#D30606"></div>
												</td>
											</tr>
											<tr>
												<td align="center">
													<div style="width:85px;height: 85px;background-image: url('images/New_ui/GameBoost_gamePriority.svg');background-size: 100%;"></div>													
												</td>
												<td style="width:400px;height:120px;">
													<div style="font-size:16px;color:#949393;padding-left:10px;">Prioritizing your game devices for the best gaming experience.</div>
													<div onclick="showGameListField();" class="btn" style="margin: 12px 0;width:100px;height:40px;line-height: 40px;text-align: center;border-radius: 5px;font-size:18px;"><#CTL_add#></div>
												</td>
												<td>
													<div class="switch" style="margin:auto;width:100px;height:40px;text-align:center;line-height:40px;font-size:18px">
														<input id="game_priority_enable" type="checkbox" onclick="enableGamePriority();">
														<div class="container" style="display:table;border-radius:5px;">
															<div style="display:table-cell;width:50%;">
																<div>ON</div>
															</div>
															<div style="display:table-cell">
																<div>OFF</div>
															</div>
														</div>
													</div>
												</td>
											</tr>
											<!-- Mobile Game mode -->
											<tr style="height:50px;"></tr>
											<tr>
												<td style="width:200px">
													<div style="padding: 5px 0;font-size:20px;">Mobile Game mode</div>
												</td>
												<td colspan="2">
													<div style="padding: 5px 10px;font-size:20px;color:#FFCC66">Boost your Mobile Game Play</div>
												</td>
											</tr>
											<tr>
												<td colspan="3">
													<div style="width:100%;height:1px;background-color:#D30606"></div>
												</td>
											</tr>
											<tr>
												<td align="center">
													<div style="width:85px;height: 85px;background-image: url('images/New_ui/GameBoost_mobileGame.svg');background-size: 100%;"></div>
													<!-- <img style="padding-right:10px;;" src="/images/New_ui/GameBoost_WTFast.png" > -->
												</td>
												<td style="width:400px;height:120px;">
													<div style="font-size:16px;color:#949393;padding-left:10px;">Download and install ASUS Router App. Enable mobile game mode in app to ensure the best mobile gaming experiences.</div>
												</td>
												<td>
													<div style="display:flex;align-items: center;">
														<div style="margin: 0 12px">
															<div id="android_qr" class="qr_code qr_android"></div>	
															<a id="android_link" href="https://play.google.com/store/apps/details?id=com.asus.aihome" target="_blank">
																<div style="width:124px;height:36px;background:url('images/googleplay.png') no-repeat;;background-size:100%;"></div>
															</a>
															<a id="android_cn_link" style="display:none" href="https://dlcdnets.asus.com/pub/ASUS/LiveUpdate/Release/Wireless/ASUSRouter_Android_Release.apk" target="_blank">
																<div style="width:124px;height:36px;border:1px solid #BDBDBD;border-radius: 6px;text-align: center;line-height: 36px;font-size:20px;">Android</div>
															</a>
														</div>
														<div style="margin: 0 12px">
															<div class="qr_code" style="background-image: url('images/New_ui/asus_router_ios_qr.png');">
															</div>
															<a href="https://itunes.apple.com/tw/app/asus-router/id1033794044" target="_blank">
																<div style="width:124px;height:36px;background:url('images/AppStore.png') no-repeat;background-size:100%;"></div>
															</a>				
														</div>		
													</div>
												</td>
											</tr>
											

											<!-- OPEN NAT -->
											<tr style="height:50px;"></tr>
											<tr>
												<td style="width:200px">
													<div style="padding: 5px 0;font-size:20px;">Open NAT</div>
												</td>
												<td colspan="2">
													<div style="padding: 5px 10px;font-size:20px;color:#FFCC66">3-steps Port Forwarding Setup</div>
												</td>
											</tr>
											<tr>
												<td colspan="3">
													<div style="width:100%;height:1px;background-color:#D30606"></div>
												</td>
											</tr>
											<tr>
												<td align="center">
													<div style="width:85px;height: 85px;background-image: url('images/New_ui/GameBoost_openNAT.svg');background-size: 100%;"></div>
												</td>
												<td style="width:400px;height:120px;">
													<div style="font-size:16px;color:#949393;padding-left:10px;">Open NAT offers a hassle-free way to create port forwarding rules for online games and optimizes the routing packets from your game console to the modem with an optimized gaming experience.</div>
												</td>
												<td>
													<div class="btn" style="margin:auto;width:100px;height:40px;text-align:center;line-height:40px;font-size:18px;cursor:pointer;border-radius:5px;" onclick="location.href='GameProfile.asp';"><#btn_go#></div>
												</td>
											</tr>

											<!-- WTFast -->
											<tr style="height:50px;"></tr>
											<tr id='wtfast_1' style="display:none">
												<td style="width:200px">
													<div style="padding: 5px 0;font-size:20px;"><#Game_Boost_internet#></div>
												</td>
												<td colspan="2">
													<div style="padding: 5px 10px;font-size:20px;color:#FFCC66">WTFast GPN</div>
												</td>
											</tr>
											<tr id='wtfast_2' style="display:none">
												<td colspan="3">
													<div style="width:100%;height:1px;background-color:#D30606"></div>
												</td>
											</tr>
											<tr id='wtfast_3' style="display:none">
												<td align="center" style="width:85px">
													<img style="padding-right:10px;;" src="/images/New_ui/GameBoost_WTFast.png" >
												</td>
												<td style="width:400px;height:120px;">
													<div style="font-size:16px;color:#949393;padding-left:10px;"><#Game_Boost_desc#></div>
												</td>
												<td>
													<div class="btn" style="margin:auto;width:100px;height:40px;text-align:center;line-height:40px;font-size:18px;cursor:pointer;border-radius:5px;" onclick="location.href='Advanced_WTFast_Content.asp';"><#btn_go#></div>
												</td>
											</tr>										
										</tbody>
									</table>
								</div>
							</td>
						</tr>
					</table>
				</div>
		<!--=====End of Main Content=====-->
			</td>
			<td width="20" align="center" valign="top"></td>
		</tr>
	</table>
</div>

<div id="footer"></div>
</body>
</html>
