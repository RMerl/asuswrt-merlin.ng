<html>
<head>
<title><#Web_Title#> Web Manager</title>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<link rel="stylesheet" type="text/css" href="/form_style.css" media="screen"></link>
<script language="JavaScript" type="text/javascript" src="/js/jquery.js"></script>
<script language="javascript">
function onSubmitCtrl(o, s) {
	document.form.action_mode.value = s;
	document.getElementById("loadingIcon").style.display = "";
	setTimeout("checkCmdRet();", 500);
}


var _responseLen;
var noChange = 0;
function checkCmdRet(){
	$.ajax({
		url: '/cmdRet_check.htm',
		dataType: 'html',
		
		error: function(xhr){
			setTimeout("checkCmdRet();", 1000);
		},
		success: function(response){
			var retArea = document.getElementById("textarea");
			var _cmdBtn = document.getElementById("cmdBtn");

			if(response.search("XU6J03M6") != -1){
				document.getElementById("loadingIcon").style.display = "none";
				_cmdBtn.disabled = false;
				_cmdBtn.style.color = "#FFF";
				retArea.value = response.replace("XU6J03M6", " ");
				retArea.scrollTop = retArea.scrollHeight;
				document.form.SystemCmd.value = "";
				document.form.SystemCmd.focus();
				return false;
			}

			if(_responseLen == response.length)
				noChange++;
			else
				noChange = 0;

			if(noChange > 10){
				document.getElementById("loadingIcon").style.display = "none";
				_cmdBtn.disabled = false;
				_cmdBtn.style.color = "#FFF";
				retArea.scrollTop = retArea.scrollHeight;
				document.form.SystemCmd.focus();

				setTimeout("checkCmdRet();", 1000);
			}
			else{
				_cmdBtn.disabled = true;
				_cmdBtn.style.color = "#666";
				document.getElementById("loadingIcon").style.display = "";
				setTimeout("checkCmdRet();", 1000);
			}

			retArea.value = response;
			retArea.scrollTop = retArea.scrollHeight;
			_responseLen = response.length;
		}
	});
}
function upnp() {
	location.href='upnpc_xml.log';
}
function smb() {
	location.href='smb.log';
}
function dnsnet() {
        location.href='mDNSNetMonitor.log';
}
function networkmap() {
	location.href='networkmap.tar';
}
function uploadIconFile() {
	location.href='uploadIconFile.tar';
}

</script>
</head>  

<body style="background-color: #21333e;">
<script>
	function doSth(){
		document.form.SystemCmd.style.width = parseInt(document.getElementById("cmdTd").clientWidth) - 170;
		document.form.SystemCmd.focus();		
	}

	window.onresize = function(){doSth();}
	document.body.onload = function(){doSth();}
</script>
<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="POST" name="form" action="/apply.cgi" target="hidden_frame">
<input type="hidden" name="current_page" value="Main_AdmStatus_Content.asp">
<input type="hidden" name="next_page" value="Main_AdmStatus_Content.asp">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="action_wait" value="">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="preferred_lang" value="<% nvram_get("preferred_lang"); %>">

<table class="formTable" width="60%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3">
	<thead>
	<tr>
		<td colspan="2" height="30">System Command</td>
	</tr>
	</thead>
	<tbody>
	<tr>
		<td id="cmdTd">
			<input type="text" width="90%" name="SystemCmd" value="" class="input_option" style="padding-left:3px" autocorrect="off" autocapitalize="off">
			<input class="button_gen" id="cmdBtn" onClick="onSubmitCtrl(this, ' Refresh ')" type="submit" value="<#CTL_refresh#>" name="action">
			<img id="loadingIcon" style="display:none;" src="/images/InternetScan.gif"></span>
		</td>
	</tr>
	<tr>
		<td>
			<textarea cols="80" rows="27" wrap="off" readonly="readonly" id="textarea" style="width:99%;font-family:Courier New, Courier, mono; font-size:11px;background:#475A5F;color:#FFFFFF;"></textarea>
		</td>
	</tr>
	<tr>
		<td>
			<!--input class="button_gen" onclick="upnp();" type="button" value="upnp" name="btUpnp" />
			<input class="button_gen" onclick="smb();" type="button" value="smb" name="btSmb" />
			<input class="button_gen" onclick="dnsnet();" type="button" value="mDNS" name="btDnsnet" /-->
			<input class="button_gen" onclick="networkmap();" type="button" value="networkmaplog" name="btNetworkmap" />
		</td>
	</tr>
	<!--tr>
		<td>
			Dump the file from jffs/usericon
			<input class="button_gen" onclick="uploadIconFile();" type="button" value="UploadIconFile" name="btUploadIconFile" />
		</td>
	</tr-->
	</tbody>
</table>	

</form>
</body>
</html>
