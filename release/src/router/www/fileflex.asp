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
<title><#Web_Title#> - FileFlex</title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="other.css">
<link rel="stylesheet" type="text/css" href="app_installation.css">
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/validator.js"></script>
<script type="text/javascript" src="/disk_functions.js"></script>
<script language="JavaScript" type="text/javascript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/switcherplugin/jquery.iphone-switch.js"></script>
<script type="text/javascript" src="/form.js"></script>
<script type="text/javascript" src="/js/httpApi.js"></script>
<style>
.fileflex_icon{
	position: relative;
	background-image: url(images/New_ui/USBExt/app_list_active.svg);
	background-size: cover;
	background-repeat: no-repeat;
	background-position: 0% 63%;
	width: 80px;
	height: 80px;
	margin: 0 auto;
}
.fileflex_icon::before{
	content: "";
	position: absolute;
	top: -20px;
	right: -20px;
	background-image: url(images/New_ui/USBExt/circle.svg);
	background-size: cover;
	background-position: 0% 100%;
	width: 120px;
	height: 120px;
}
.text_link{
	text-decoration: underline;
	cursor: pointer;
	font-weight: bolder;
	font-size: 16px;
}
.text_link.yellow{
	color: #FC0;
}
</style>
<script>
function initial(){
	show_menu();
}
function createAcc(){
	window.open('https://asus.fileflex.com/fbweb/app/public/view/register', '_blank');
}
function loginAcc(){
	window.open('https://asus.fileflex.com', '_blank');
}
function installFAQ(){
	window.open('https://fileflex.com/support/faqs/', '_blank');
}
function introduction(){
	window.open('https://asus-info.fileflex.com/', '_blank');
}
</script>
</head>

<body onload="initial();" onunLoad="return unload_body();" class="bg">
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>
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
					<td align="left" valign="top">
						<table width="760px" border="0" cellpadding="5" cellspacing="0" class="FormTitle" id="FormTitle" style="border-radius:3px;">
							<tbody>
								<tr>
									<td bgcolor="#4D595D" valign="top">
										<div>&nbsp;</div>
										<div>
											<table width="100%">
												<tr>
													<td align="left">
														<span class="formfonttitle">FileFlex Connector</span><!-- untranslated -->
													</td>
													<td align="right">
														<img onclick="go_setting('/APP_Installation.asp')" align="right" style="cursor:pointer;position:absolute;margin-left:-40px;margin-top:-30px;" title="Back to USB Extension" src="/images/backprev.png" onMouseOver="this.src='/images/backprevclick.png'" onMouseOut="this.src='/images/backprev.png'">
													</td>
												</tr>
											</table>
										</div>
										<div style="margin:5px;" class="splitLine"></div>
										<div>
											<table>
												<tr>
													<td style="text-align:center;width:200px;">
														<div class="fileflex_icon"></div>
													</td>
													<td>
														<div class="formfonttitle">FileFlex Connector Installation Successful !</div><!-- untranslated -->
														<div class="formfontdesc" style="font-style:italic;font-size:14px;">
															FileFlex provides the cloud functionality of secure remote access, sharing and streaming to the router's USB attached storage or the storage of router-networked devices from a smart phone, tablet or remote computer. It also provides automatic back up of photos and videos from smart phones and tablets to your router's USB attached storage or the storage of router-networked devices.<!-- untranslated -->
														</div>
														<br>
														<div class="formfonttitle">
															<div style="margin-top:10px;float:left;">
																<span class="text_link yellow" onclick="loginAcc();">Login FileFlex now</span><!-- untranslated -->
															</div>
															<div style="margin-left:10px;float:left;"><img src="images/New_ui/aidisk/steparrow.png"></div>
															<div style="clear:both;"></div>
														</div>
													</td>
												</tr>
											</table>
										</div>
										<div style="margin:5px;" class="splitLine"></div>
										<ul style="line-height:30px;">
											<li>
												<span class="text_link" onclick="createAcc();">Create a new FileFlex account</span><!-- untranslated -->
											</li>
											<li>
												<span class="text_link" onclick="installFAQ();">FileFlex Installation FAQ</span><!-- untranslated -->
											</li>
											<li>
												<span class="text_link" onclick="introduction();">FileFlex Introduction</span><!-- untranslated -->
											</li>
										</ul>
									</td>
								</tr>
							</tbody>
						</table>
					</td>
				</tr>
			</table>
			<!--===================================End of Main Content===========================================-->
		</td>
		<td width="10" align="center" valign="top">&nbsp;</td>
	</tr>
</table>
<div id="footer"></div>
</body>
</html>
