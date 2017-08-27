<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=Edge"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png"><title><#Web_Title#> - <#WiFi_radar#></title>
<link rel="stylesheet" type="text/css" href="index_style.css">
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="usp_style.css">
<link rel="stylesheet" type="text/css" href="app_installation.css">
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/js/jquery.js"></script>
<script>
function initial(){
	show_menu();
}

</script>
</head>
<body onload="initial();" onunload="unload_body();">
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
				<div id="tabMenu" style="*margin-top: -160px;"></div>
				<br>
		<!--=====Beginning of Main Content=====-->
				<div class="app_table" id="applist_table" style="margin-top:-10px;">
					<table>
						<tr>
							<td class="formfonttitle">
								<div style="width:730px">
									<table width="730px">
										<tr>
											<td align="left">
												<span class="formfonttitle"><#WiFi_radar#></span>
											</td>
										</tr>
									</table>
								</div>
							</td>
						</tr> 
						<tr>
							<td class="line_export"><img src="images/New_ui/export/line_export.png" /></td>
						</tr>
						<tr>
							<td>
								<div class="formfontdesc" style="font-size:14px;font-style:italic;"></div>
							</td> 
						</tr>					
					<!-- Service table -->
						<tr>
							<td valign="top" id="app_table_td" height="0px" style="padding-top:20px;">
								<div id="app_table" style="margin-bottom:20px;">
									<table class="appsTable" align="center" style="margin:auto;border-collapse:collapse;">
										<tbody>
											<tr>
												<td align="center" class="app_table_radius_left" style="width:85px">
													<img style="margin-top:0px;" src="/images/New_ui/wifi_survey.png" onclick="window.open('visindex.asp', '_blank')">
												</td>
												<td class="app_table_radius_right" style="width:350px;height:120px;">
													<div class="app_name">
														<a style="text-decoration: underline;" href="visindex.asp" target="_blank"><#WiFi_sitesurvey#></a>
													</div>
													<div class="app_desc" style="height:60px;">
														<li><#WiFi_sitesurvey_desc#></li>
													</div>
												</td>
											</tr>
											<tr style="height:50px;"></tr>
	
											<tr>
												<td align="center" class="app_table_radius_left" style="width:85px;">
													<img style="margin-top:0px;" src="/images/New_ui/wifi_statistic.png" onclick="window.open('channelcapacity.asp', '_blank')">
												</td>
												<td class="app_table_radius_right" style="width:350px;height:120px;">
													<div class="app_name">
														<a style="text-decoration: underline;" href="channelcapacity.asp" target="_blank"><#WiFi_Statistics#></a>
													</div>
													<div class="app_desc" style="height:60px;">
														<li><#WiFi_Statistics_desc#></li>
													</div>
												</td>
											</tr>
											<tr style="height:50px;"></tr>
											<tr>
												<td align="center" class="app_table_radius_left" style="width:85px;">
													<img style="margin-top:0px;" src="/images/New_ui/wifi_troubleShooting.png" onclick="window.open('metrics.asp', '_blank')">
												</td>
												<td class="app_table_radius_right" style="width:350px;height:120px;">
													<div class="app_name">
														<a style="text-decoration: underline;" href="metrics.asp" target="_blank"><#WiFi_TroubleShooting#></a>
													</div>
													<div class="app_desc" style="height:60px;">
														<li><#WiFi_TroubleShooting_desc#></li>
													</div>
												</td>
											</tr>

											<tr style="height:50px;"></tr>
											<tr>
												<td align="center" class="app_table_radius_left" style="width:85px;">
													<img style="margin-top:0px;" src="/images/New_ui/wifi_setting.png" onclick="window.open('configure.asp', '_blank')">
												</td>
												<td class="app_table_radius_right" style="width:350px;height:120px;">
													<div class="app_name">
														<a style="text-decoration: underline;" href="configure.asp" target="_blank"><#Settings#></a>
													</div>
													<div class="app_desc" style="height:60px;">
														<li><#WiFi_radar_desc#></li>
													</div>
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

