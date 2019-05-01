<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="X-UA-Compatible" content="IE=Edge">
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title><#Network_Tools#> - Netstat</title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script language="JavaScript" type="text/javascript" src="/help.js"></script>
<script language="JavaScript" type="text/javascript" src="/js/jquery.js"></script>
<script>
function init(){
	show_menu();
}

var netoolApi = {
	start: function(queryStr){
		$.getJSON("/netool.cgi", queryStr)
			.done(function(data){
				$("#loadingIcon").show();
				$("#cmdBtn").hide();

				if(data.successful != "0") setTimeout(function(){
					netoolApi.check(data.successful)
				}, 1000)
			})
			.fail(function(e){
				netoolApi.show('Fail to start')
			});
	},
	check: function(fileName){
		$.get("/netool.cgi", {"type":0, "target": fileName})
			.done(function(data){
				netoolApi.show(data)

				if(data.search("XU6J03M6") == -1){
					setTimeout(function(){
						netoolApi.check(fileName);
					}, 500);				
				}
				else{
					$("#loadingIcon").hide();
					$("#cmdBtn").show();
				}
			})
			.fail(function(e){
				netoolApi.show('Fail to get data')
			});
	},
	show: function(content){
		$("#textarea").val(content.replace("XU6J03M6", "").replace('{"result":', "").replace('}', ""));
	}
}

function hideCNT(obj){
	return false;
}
</script>
</head>
<body onload="init();">
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>
<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="POST" name="form" action="" target="hidden_frame">
<input type="hidden" name="current_page" value="Main_Netstat_Content.asp">
<input type="hidden" name="next_page" value="Main_Netstat_Content.asp">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">

<table class="content" align="center" cellpadding="0" cellspacing="0">
	<tr>
		<td width="17">&nbsp;</td>
		<td valign="top" width="202">
			<div id="mainMenu"></div>
			<div id="subMenu"></div>
		</td>	
		<td valign="top">
			<div id="tabMenu" class="submenuBlock"></div>		
			<table width="98%" border="0" align="left" cellpadding="0" cellspacing="0">
				<tr>
					<td align="left" valign="top">				
						<table width="760px" border="0" cellpadding="5" cellspacing="0" bordercolor="#6b8fa3"  class="FormTitle" id="FormTitle">		
							<tr>
								<td bgcolor="#4D595D" colspan="3" valign="top">
									<div>&nbsp;</div>
									<div class="formfonttitle"><#Network_Tools#> - Netstat</div>
									<div style="margin: 10px 0 10px 5px;" class="splitLine"></div>
									<div class="formfontdesc" id="cmdDesc"><#NetworkTools_Info#></div>
									<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
										<tr>
											<th width="20%"><#NetworkTools_Method#></th>
											<td>
												<select id="cmdMethod" class="input_option" name="cmdMethod" onchange="hideCNT(this);">
													<option value="6" selected>Netstat</option>
 												</select>
											</td>										
										</tr>										
										<tr>
											<th width="20%"><#NetworkTools_option#></th>
											<td>
												<input type="checkbox" class="options" value="00000001"><#sockets_listening#>
												<br>
												<input type="checkbox" class="options" value="00000010"><#sockets_all#>
												<br>
												<input type="checkbox" class="options" value="00001000"><#sockets_TCP#>
												<br>
												<input type="checkbox" class="options" value="00010000"><#sockets_UDP#>
												<br>
												<input type="checkbox" class="options" value="00100000"><#sockets_RAW#>
												<br>
												<input type="checkbox" class="options" value="01000000"><#sockets_UNIX#>
												<br>
												<input type="checkbox" class="options" value="10000000"><#Display_routingtable#>
												<br>
												<input type="checkbox" class="options" value="00000100"><#sockets_not_resolve_names#>
											</td>			
										</tr>
									</table>

									<div class="apply_gen" style="height:40px">
										<input class="button_gen" id="cmdBtn" onClick="onSubmitCtrl(this, ' Refresh ')" type="button" value="<#NetworkTools_Diagnose_btn#>">
										<script>
											$("#cmdBtn").click(function(){
												var options = 0;
												$(".options").each(function(){
													if(this.checked) options += parseInt($(this).val());
												})

												$("#textarea").val("");
												netoolApi.start({
													"type": $("#cmdMethod").val(),
													"netst": "0x00" + parseInt(options, 2).toString(16)
												})
											})
										</script>

										<img id="loadingIcon" style="display:none;" src="/images/InternetScan.gif"></span>
									</div>

									<div style="margin-top:8px" id="logArea">
										<textarea cols="63" rows="27" wrap="off" readonly="readonly" id="textarea" class="textarea_ssh_table" style="width:99%;font-family:Courier New, Courier, mono; font-size:11px;"><% nvram_dump("syscmd.log","syscmd.sh"); %></textarea>
									</div>
								</td>
							</tr>
						</table>
					</td>
				</tr>
			</table>
			<!--===================================Ending of Main Content===========================================-->
		</td>
		<td width="10" align="center" valign="top"></td>
	</tr>
</table>
</form>

<div id="footer"></div>
</body>
<script type="text/javascript">
<!--[if !IE]>-->
	(function($){
		var textArea = document.getElementById('textarea');
		textArea.scrollTop = textArea.scrollHeight;
	})(jQuery);
<!--<![endif]-->
</script>
</html>
