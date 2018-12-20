<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title></title>
<link href="../form_style.css" rel="stylesheet" type="text/css" />
<link href="../NM_style.css" rel="stylesheet" type="text/css" />
<style>
a:link {
	text-decoration: underline;
	color: #FFFFFF;
}
a:visited {
	text-decoration: underline;
	color: #FFFFFF;
}
a:hover {
	text-decoration: underline;
	color: #FFFFFF;
}
a:active {
	text-decoration: none;
	color: #FFFFFF;
}
</style>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/js/httpApi.js"></script>
<script type="text/javascript" src="/switcherplugin/jquery.iphone-switch.js"></script>
<script>
if(parent.location.pathname.search("index") === -1) top.location.href = "../"+'<% networkmap_page(); %>';

var diskOrder = parent.getSelectedDiskOrder();

<% get_AiDisk_status(); %>

var apps_array = <% apps_info("asus"); %>;

function initial(){
	if(!parent.media_support)
		document.getElementById("mediaserver_hyperlink").style.display = "none";
	
	showDiskUsage(parent.usbPorts[diskOrder-1]);

	if(sw_mode == "2" || sw_mode == "3" || sw_mode == "4")
		document.getElementById("aidisk_hyperlink").style.display = "none";
		
	if(noaidisk_support)
		document.getElementById("aidisk_hyperlink").style.display = "none";
	
	if((based_modelid == "DSL-AC68U" || based_modelid == "RT-AC3200" || based_modelid == "RT-AC87U" || based_modelid == "RT-AC68U" || based_modelid == "RT-AC68A" || based_modelid == "RT-AC56S" || based_modelid == "RT-AC56U" || based_modelid == "RT-AC55U" || based_modelid == "RT-AC55UHP" || based_modelid == "RT-N18U" || based_modelid == "RT-AC88U" || based_modelid == "RT-AC86U" || based_modelid == "AC2900" || based_modelid == "RT-AC3100" || based_modelid == "RT-AC5300" || based_modelid == "RP-AC68U" || based_modelid == "RT-AC58U" || based_modelid == "RT-AC82U" || based_modelid == "MAP-AC3000" || based_modelid == "RT-AC85U" || based_modelid == "RT-AC85P" || based_modelid == "RT-AC65U"|| based_modelid == "4G-AC68U" || based_modelid == "BLUECAVE") && parent.currentUsbPort == 0){
		document.getElementById('reduce_usb3_table').style.display = "";
	}
	else if((based_modelid == "RT-AC88Q" || based_modelid == "RT-AD7200" || based_modelid == "RT-N65U" || based_modelid == "GT-AC5300" || based_modelid == "RT-AX88U" || based_modelid == "RT-AX95U" || based_modelid == "BRT-AC828") && (parent.currentUsbPort == 0 || parent.currentUsbPort == 1)){
		document.getElementById('reduce_usb3_table').style.display = "";
	}

	var disk_list_array = new Array();
	var usb_fatfs_mod = '<% nvram_get("usb_fatfs_mod"); %>';
	var usb_ntfs_mod = '<% nvram_get("usb_ntfs_mod"); %>';
	var usb_hfs_mod = '<% nvram_get("usb_hfs_mod"); %>';

	disk_list_array = { "info" : ["<#diskUtility_information#>", "disk.asp"], "health" : ["<#diskUtility#>", "disk_utility.asp"], "format" : ["<#CTL_format#>", "disk_format.asp"]};
	if(!parent.diskUtility_support) {
		delete disk_list_array.health;
		delete disk_list_array.format;
	}
	if(usb_fatfs_mod != "tuxera" && usb_ntfs_mod != "tuxera" && usb_hfs_mod != "tuxera") {
		delete disk_list_array.format;
	}
	$('#diskTab').html(parent.gen_tab_menu(disk_list_array, "info"));

	//short term solution for brt-ac828
	if(based_modelid == "BRT-AC828") {
		document.getElementById("mediaserver_hyperlink").style.display = "none";
		document.getElementById("aidisk_hyperlink").style.display = "none";
	}

	//complete SMBv1_FAQ link
	document.getElementById('SMBv1_FAQ').target="_blank";
	document.getElementById('SMBv1_FAQ').style.textDecoration="underline";
	httpApi.faqURL("1037477", function(url){document.getElementById("SMBv1_FAQ").href=url;});

	reset_NM_height();
}

var thisForeignDisksIndex;
function showDiskUsage(device){
	document.getElementById("disk_model_name").innerHTML = device.deviceName;

	if(device.mountNumber > 0){
		showtext(document.getElementById("disk_total_size"), simpleNum(device.totalSize) + " GB");		
		showtext(document.getElementById("disk_avail_size"), simpleNum(device.totalSize - device.totalUsed) +" GB");
		document.getElementById("mounted_item1").style.display = "";		
		document.getElementById("unmounted_refresh").style.display = "none";
	}
	else{
		document.getElementById("mounted_item1").style.display = "none";
		document.getElementById("unmounted_refresh").style.display = "";
	}

	for(var i = 0; i < apps_array.length; i++){
		if(apps_array[i][0] == "downloadmaster" && apps_array[i][4] == "yes" && apps_array[i][3] == "yes"){
			if(device.hasAppDev){
				document.getElementById("dmLink").style.display = "";
			}
			break;
		}
	}

	thisForeignDisksIndex = device.node;
}

function goUPnP(){
	parent.location.href = "/mediaserver.asp";
}

function gotoAidisk(){
	parent.location.href = "/aidisk.asp";
}

function gotoDM(){
	var dm_http_port = '<% nvram_get("dm_http_port"); %>';
	if(dm_http_port == "")
		dm_http_port = "8081";

	var dm_url = "";
	var header_info = [<% get_header_info(); %>];
	dm_url = "http://" + header_info[0].host + ":" + dm_http_port;
	window.open(dm_url);
}

function remove_disk_call(){
	top.remove_disk(thisForeignDisksIndex)
}

function switchUSBType(){
	document.form.submit();
}
</script>
</head>

<body class="statusbody" onload="initial();">
<div id="diskTab" class='tab_table'></div>
<table width="95%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="table1px">
	<tr>
    <td style="padding:5px 10px 0px 15px;">
    	<p class="formfonttitle_nwm"><#Modelname#>:</p>
			<p class="tab_info_bg" style="padding-left:10px; margin-top:3px;line-height:20px; color:#FFFFFF;" id="disk_model_name"></p>
    	<div style="margin-top:5px;" class="line_horizontal"></div>
    </td>
  </tr>
</table>

<table id="mounted_item1" width="95%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="table1px">
  <tr>
    <td style="padding:5px 10px 0px 15px;">
    	<p class="formfonttitle_nwm"><#Availablespace#>:</p>
    	<p class="tab_info_bg" style="padding-left:10px; margin-top:3px;line-height:20px; color:#FFFFFF;" id="disk_avail_size"></p>
    	<div style="margin-top:5px;" class="line_horizontal"></div>
    </td>
  </tr>

  <tr>
    <td style="padding:5px 10px 0px 15px;">
    	<p class="formfonttitle_nwm"><#Totalspace#>:</p>
    	<p class="tab_info_bg" style="padding-left:10px; margin-top:3px;line-height:20px; color:#FFFFFF;" id="disk_total_size"></p>
		<div style="margin-top:5px;" class="line_horizontal"></div>
    </td>
  </tr>

  <tr id="mediaserver_hyperlink">
    <td style="padding:10px 15px 0px 15px;;">
    	<p class="formfonttitle_nwm" style="float:left;width:138px;"><#UPnPMediaServer#>:</p>
      <input type="button" class="button_gen" onclick="goUPnP();" value="<#btn_go#>" >
		<div style="margin-top:5px;" class="line_horizontal"></div>
    </td>
  </tr>

  <tr id="aidisk_hyperlink">
    <td height="50" style="padding:10px 15px 0px 15px;">
    	<p class="formfonttitle_nwm" style="float:left;width:138px;"><#AiDiskWizard#>:</p>
    	<input type="button" class="button_gen" onclick="gotoAidisk();" value="<#btn_go#>" >
    	<div style="margin-top:5px;" class="line_horizontal"></div>
    </td>
  </tr>

  <tr id="dmLink" style="display:none;">
    <td height="50" style="padding:10px 15px 0px 15px;">
    	<p class="formfonttitle_nwm" style="float:left;width:138px;">Download Master</p>
    	<input type="button" class="button_gen" onclick="gotoDM();" value="<#btn_go#>" >
		<div style="margin-top:5px;" class="line_horizontal"></div>
    </td>
  </tr>
</table>

<table width="95%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="table1px">
  <tr>
    <td height="50" style="padding:10px 15px 0px 15px;">
    	<p class="formfonttitle_nwm" style="float:left;width:138px; "><#Safelyremovedisk_title#>:</p>
    	<input id="show_remove_button" class="button_gen" type="button" class="button" onclick="remove_disk_call();" value="<#btn_remove#>">
    	<div id="show_removed_string" style="display:none;"><#Safelyremovedisk#></div>
    	<div style="margin-top:5px;" class="line_horizontal"></div>
    </td>
  </tr>
</table>

<table width="95%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="table1px" id="reduce_usb3_table" style="display:none">
	<tr>
		<td height="50" style="padding:10px 15px 0px 15px;">
			<p class="formfonttitle_nwm" style="float:left;width:138px; " onmouseover="parent.overHint(24);" onmouseout="parent.nd();">USB Mode</p>
			<form method="post" name="form" action="/start_apply.htm" target="hidden_frame">
				<input type="hidden" name="current_page" value="/">
				<input type="hidden" name="next_page" value="/">
				<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
				<input type="hidden" name="action_mode" value="apply">
				<input type="hidden" name="action_script" value="reboot">
				<input type="hidden" name="action_wait" value="<% get_default_reboot_time(); %>">
				<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
				<div align="center" class="left" style="width:120px; float:left; cursor:pointer;margin-top:-7px;" id="reduce_usb3_enable">
					<select class="input_option" name="usb_usb3" >
						<option value="0" <% nvram_match("usb_usb3", "0", "selected"); %>>USB 2.0</option>
						<option value="1" <% nvram_match("usb_usb3", "1", "selected"); %>>USB 3.0</option>
					</select>
				</div>
			</form>
		</td>
	</tr>
	<tr>
		<td height="50" style="padding:10px 15px 0px 15px;">
			<#ADSL_FW_note#><br><#SMBv1_enable_hint#>
		</td>
	</tr>
	<tr>
		<td>
			<div style="margin-top:5px;" class="line_horizontal"></div>
			<div class="apply_gen">
				<input class="button_gen" onclick="switchUSBType();" type="button" value="Apply">
			</div>
		</td>
	</tr>
</table>

<div id="unmounted_refresh" style="padding:5px 0px 5px 25px; display:none">
<ul style="font-size:11px; font-family:Arial; padding:0px; margin:0px; list-style:outside; line-height:150%;">
	<li><#DiskStatus_refresh1#><a href="/" target="_parent"><#DiskStatus_refresh2#></a><#DiskStatus_refresh3#></li>
</ul>
</div>

<form method="post" name="diskForm" action="">
<input type="hidden" name="disk" value="">
</form>
</body>
</html>
