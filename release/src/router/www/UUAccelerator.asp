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
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/validator.js"></script>
<script type="text/javascript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/switcherplugin/jquery.iphone-switch.js"></script>
<script>
	var label_mac = <% get_label_mac(); %>;
function initial(){
	show_menu();
	uu_check_status();
}
function uuRegister(mac){
	var _mac = mac.toLowerCase();
	window.open('https://router.uu.163.com/asus/pc.html#/acce?gwSn=' + _mac + '&type=asuswrt-merlin', '_blank');
}
function uu_version_check(){
	document.getElementById("btn_check").disabled = true;
	document.uu_update.submit();
	document.getElementById("uu_status").innerHTML = "MD5: 检查中...";
	document.getElementById("uu_update_scan").style.display = "";
	setTimeout("uu_check_status();", 3000);
}
function uu_check_status(){
	﻿md5 = '<% nvram_get("netease_uu_md5"); %>';
	uu_enabled = '<% nvram_get("netease_uu_enable"); %>';
	document.getElementById("uu_status").innerHTML = "MD5: "+md5;
	if(uu_enabled == 1)
		document.getElementById("btn_check").disabled = false;
	else
		document.getElementById("btn_check").disabled = true;
	document.getElementById("uu_update_scan").style.display = "none";
}
function applyRule() {
	showLoading();
	document.form.submit();
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
<input type="hidden" name="current_page" value="UUAccelerator.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="start_uu_check">
<input type="hidden" name="action_wait" value="2">
<input type="hidden" name="netease_uu_enable" value="<% nvram_get("netease_uu_enable"); %>">
</form>
<form method="post" name="uu_update" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="start_uu_check">
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
											<td align="right">
												<div align="center" class="left" style="width:94px; float:right; cursor:pointer;" id="uu_enable"></div>
												<script type="text/javascript">
													$('#uu_enable').iphoneSwitch('<% nvram_get("netease_uu_enable"); %>',
														function(){
															document.form.netease_uu_enable.value = 1;
															document.getElementById("btn_check").disabled = false;
															applyRule();
														},
														function(){
															document.form.netease_uu_enable.value = 0;
															document.getElementById("btn_check").disabled = true;
															applyRule();
														}
													);
												</script>
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
					<div style="display:flex;border: 2px solid #41484a;padding: 18px 6px;align-items: center;border-radius:4px;margin: 12px 6px;">
						<div style="margin: 0 12px;">
							<img src="/images/uu_accelerator.png" alt="">
						</div>
						<div style="width:1px;height: 120px;background-color: #929EA1"></div>
						<div style="width:350px;margin: 0 12px;">
							<div style="display:flex;justify-content: space-between;margin-bottom:12px;">
								<div style="font-size: 16px;"><#UU_Accelerator#></div>
								<div style="margin-right:12px;"><a href="https://uu.163.com/router/" target="_blank">FAQ</a></div>
							</div>
							<div style="color: #FC0;margin-bottom:12px;"><#UU_Accelerator_desc#></div>
							<div>
								<span id="uu_status" style="color: #FC0;"></span>
								<img id="uu_update_scan" style="display:none;" src="images/InternetScan.gif">
							</div>
						</div>
						<div style="width:1px;height: 120px;background-color: #929EA1"></div>
						<div style="width:122px;margin: auto;">
							<input type="button" class="button_gen" onclick="uuRegister(label_mac);" value="<#btn_go#>">
							<div style="margin-top: 12px;">
								<input type="button" id="btn_check" name="btn_check" class="button_gen" onclick="uu_version_check();" value="<#liveupdate#>">
							</div>
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

