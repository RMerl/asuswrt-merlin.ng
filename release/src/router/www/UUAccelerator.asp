<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=Edge"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png"><title><#Web_Title#> - <#UU_Accelerator#></title>
<link rel="stylesheet" type="text/css" href="index_style.css">
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="usp_style.css">
<link rel="stylesheet" type="text/css" href="app_installation.css">
<script type="text/javascript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/js/httpApi.js"></script>
<script type="text/javascript" src="/switcherplugin/jquery.iphone-switch.js"></script>
<style>
.uu-card{
	display:flex;
	border: 2px solid #41484a;
	padding: 18px 6px;
	align-items: center;
	border-radius: 4px;
	margin: 12px 6px;
}

.uu-card-divider{
	width: 1px;
	height: 120px;
	background-color: #929EA1;
}

.uu-card-content{
	flex: 1;
	min-width: 0;
	margin: 0 12px;
}

.uu-card-header{
	display:flex;
	justify-content: space-between;
	margin-bottom: 12px;
}

.uu-card-title{
	font-size: 16px;
}

.uu-card-faq{
	margin-right: 12px;
}

.uu-card-desc{
	color: var(--wrt-badge-warning-color,#FFCC00);
}

.uu-card-action{
	width: 122px;
	flex: 0 0 122px;
	text-align: center;
}

.uu-switch-holder{
	width: 74px;
	margin: 0 auto 12px;
	position: relative;
	overflow: hidden;
	height: 32px;
}

.uu-go-btn{
	width: 122px;
	min-width: 122px;
	box-sizing: border-box;
}

.uu-go-btn:disabled{
	opacity: 0.5;
	cursor: default;
}

.uu-switch-busy{
	pointer-events: none;
	opacity: 0.5;
}
</style>
<script>
	var label_mac = <% get_label_mac(); %>;
var uuSwitchState = httpApi.nvramGet(["uuplugin_enable"]).uuplugin_enable || "0";

function initial(){
	show_menu();
	initUuSwitch();
	updateUuGoState(uuSwitchState == "1");

}
function uuRegister(mac){
	var _mac = mac.toLowerCase();
	window.open('https://router.uu.163.com/asus/pc.html#/acce?gwSn=' + _mac + '&type=asuswrt', '_blank');
}

function updateUuGoState(enabled){
	var goButton = document.getElementById("uu_go_btn");
	if(goButton)
		goButton.disabled = !enabled;
	uuSwitchState = enabled ? "1" : "0";
}

function applyUuSwitchState(enabled){
	var switchHolder = $("#uu_switch");
	var nextState = enabled ? "1" : "0";

	updateUuGoState(enabled);
	switchHolder.addClass("uu-switch-busy");
	showLoading();

	httpApi.nvramSet({
		action_mode: "apply",
		rc_service: "restart_uuplugin",
		uuplugin_enable: nextState
	}, function(){
		hideLoading();
		switchHolder.removeClass("uu-switch-busy");
		uuSwitchState = nextState;
	});
}

function initUuSwitch(){
	$("#uu_switch").iphoneSwitch(uuSwitchState,
		function() {
			applyUuSwitchState(true);
		},
		function() {
			applyUuSwitchState(false);
		}
	);
}
</script>
</head>
<body onload="initial();" onunload="unload_body();" class="bg">
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" width="0" height="0" frameborder="0" scrolling="no"></iframe>
<form method="post" name="form" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="action_wait" value="">
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
				<div id="tabMenu" class="submenuBlock"></div>
				<br>
		<!--=====Beginning of Main Content=====-->
				<div class="app_table app_table_usb" id="FormTitle">
					<table>
						<tr>
							<td class="formfonttitle">
								<div style="width:730px">
									<table width="730px">
										<tr>
											<td align="left">
												<span class="formfonttitle"><#UU_Accelerator#></span>
											</td>
										</tr>
									</table>
								</div>
							</td>
						</tr> 
						<tr>
							<td><div class="splitLine"></div></td>
						</tr> 
					</table>
					<div class="uu-card">
						<div style="margin: 0 12px;">
							<img src="/images/UU_Logo_RGB.svg" alt="" style="max-width: 200px;">
						</div>
						<div class="uu-card-divider"></div>
						<div class="uu-card-content">
							<div class="uu-card-header">
								<div class="uu-card-title"><#UU_Accelerator#></div>
								<div class="uu-card-faq"><a href="https://uu.163.com/router/" target="_blank">FAQ</a></div>
							</div>
							<div class="uu-card-desc"><#UU_Accelerator_desc#></div>
						</div>
						<div class="uu-card-divider"></div>
						<div class="uu-card-action">
							<div id="uu_switch" class="uu-switch-holder"></div>
							<input type="button" id="uu_go_btn" class="button_gen uu-go-btn" value="<#btn_go#>" onclick="uuRegister(label_mac);">
						</div>
					</div>
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

