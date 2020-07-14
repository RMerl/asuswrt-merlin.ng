<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=Edge"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png"><title><#Web_Title#> - <#Network_Printer_Server#></title>
<link rel="stylesheet" type="text/css" href="index_style.css">
<link rel="stylesheet" type="text/css" href="usp_style.css">
<link rel="stylesheet" type="text/css" href="form_style.css">
<style type="text/css"> 
	div.wrapper { margin: 0 auto; width: 730px;}
	td.sidenav { width:200px;}
	body {font-family: Verdana, Tohoma, Arial, Helvetica, sans-serif;padding:0;margin:0;}
	.wrapperDesc { margin: 0 auto; width: 570px;}
</style> 
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/js/httpApi.js"></script>
<script type="text/javascript" src="/switcherplugin/jquery.iphone-switch.js"></script>
<style type="text/css">
.printerServer_table{
	width:740px;
	padding:5px; 
	padding-top:20px; 
	margin-top:-16px; 
	position:relative;
	-webkit-border-radius: 3px;
	-moz-border-radius: 3px;
	border-radius: 3px;
	height: 760px;
}
.line_export{
	height:20px;
	width:736px;
}
.desctitle{
	font-size: 14px;
	font-weight: bolder;
	margin-left: 20px;
	margin-top: 15px;
}
.desc{
	margin-left: 20px;
	margin-top: 10px;
}
.descimage{
	margin-left: 20px;
	margin-top: 5px;
}
.statusBar{
	margin:auto;
}
.MethodDesc{
	font-style: italic;
	color: #999;
}
.imdShade{
	-moz-box-shadow: 15px 15px 10px #333;
	-webkit-box-shadow: 15px 15px 10px #333;
	box-shadow: 15px 15px 10px #333;
}
</style>
<script>
var faq_href1 = "https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Faq&lang="+ui_lang+"&kw=&num=146";
var faq_href2 = "https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Faq&lang="+ui_lang+"&kw=&num=147";
var faq_href3 = "https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Faq&lang="+ui_lang+"&kw=&num=148";

function initial(){
	show_menu();
	document.getElementById("faq1").href=faq_href1;
	document.getElementById("faq2").href=faq_href2;
	document.getElementById("faq3").href=faq_href3;
}

function showMethod(flag1, flag2){
	document.getElementById("method1").style.display = flag1;
	document.getElementById("method1Title").style.display = flag1;
	document.getElementById("method2").style.display = flag2;
	document.getElementById("method2Title").style.display = flag2;
	if(flag1 == ""){
		document.getElementById("help1").style.color = "#FFF";
		document.getElementById("help2").style.color = "gray";
	}
	else{
		document.getElementById("help1").style.color = "gray";
		document.getElementById("help2").style.color = "#FFF";
	}
}
</script>
</head>

<body onload="initial();" onunload="unload_body();" class="bg">
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" width="0" height="0" frameborder="0" scrolling="no"></iframe>
<form method="post" name="form" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="current_page" value="PrinterServer.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="restart_lpd;restart_u2ec;">
<input type="hidden" name="action_wait" value="5">
<input type="hidden" name="usb_printer" value="<% nvram_get("usb_printer"); %>">
</form>

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
<div class="printerServer_table content_bg" id="FormTitle">
<table>
  <tr>
  	<td class="formfonttitle"><#Network_Printer_Server#>
			<img onclick="go_setting('/APP_Installation.asp')" align="right" style="cursor:pointer;margin-right:10px;margin-top:-10px" title="<#Menu_usb_application#>" src="/images/backprev.png" onMouseOver="this.src='/images/backprevclick.png'" onMouseOut="this.src='/images/backprev.png'">
		</td>
  </tr> 
  <tr>
  	<td><div class="splitLine"></div></td>
  </tr>
  <tr>
   	<td><div class="formfontdesc"><#Network_Printer_desc#></div></td> 
  </tr>
  <tr>
   	<td>
		<div id="mainbody">
			<div class="wrapper">
				<div class="shadow-l">
					<div class="shadow-r">
						<table class="" cellspacing="0" cellpadding="0">
							<tbody>
								<tr valign="top">
								<td class="">
									<div class="padding">
										<div class="">
											<ul class="">
												<li>
													<a id="faq1" href="" target="_blank" style="text-decoration:underline;font-size:14px;font-weight:bolder;color:#FFF"><#asus_ez_print_share#> FAQ</a>&nbsp;&nbsp;
													<a href="http://dlcdnet.asus.com/pub/ASUS/LiveUpdate/Release/Wireless/Printer.zip" style="text-decoration:underline;font-size:14px;font-weight:bolder;color:#FC0"><#Download_now#></a>
												</li>
												<li style="margin-top:10px;">
													<a id="faq2" href="" target="_blank" style="text-decoration:underline;font-size:14px;font-weight:bolder;color:#FFF"><#LPR_print_share#> FAQ (Windows)</a>&nbsp;&nbsp;
												</li>
												<li style="margin-top:10px;">
													<a id="faq3" href="" target="_blank" style="text-decoration:underline;font-size:14px;font-weight:bolder;color:#FFF"><#LPR_print_share#> FAQ (MAC)</a>&nbsp;&nbsp;
												</li>
											</ul>	
										</div>	
										<span class="article_seperator">&nbsp;</span>
									</div>
								</td>
								</tr>
								<tr>
									<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
											<tr>
												<th><#Network_Printer_Server#></th>
												<td>
												<div class="left" style="width:94px; float:left; cursor:pointer;" id="network_printer_enable"></div>
												<div class="iphone_switch_container" style="height:32px; width:74px; position: relative; overflow: hidden">
												<script type="text/javascript">
													$('#network_printer_enable').iphoneSwitch(document.form.usb_printer.value,
														function() {
															document.form.usb_printer.value = "1";
															showLoading();
															document.form.submit();
														},
														 function() {
															document.form.usb_printer.value = "0";
															showLoading();
															document.form.submit();
														}
													);
												</script>
												</div>
												</td>
											</tr>
									</table>
								</tr>
					</tbody></table>
				</div>
			</div>
		</div>
		</div>

		</td>
  </tr>
  </table>

<!--=====End of Main Content=====-->
		</td>

		<td width="20" align="center" valign="top"></td>
	</tr>
</table>

<div id="footer"></div>
</body>
</html>

