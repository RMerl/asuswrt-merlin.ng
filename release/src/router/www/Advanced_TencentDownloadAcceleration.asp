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
<title><#Web_Title#> - <#TimeMach#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="other.css">
<link rel="stylesheet" type="text/css" href="app_installation.css">
<script type="text/javascript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/validator.js"></script>
<script type="text/javascript" src="/disk_functions.js"></script>
<script type="text/javascript" src="/switcherplugin/jquery.iphone-switch.js"></script>
<script type="text/javascript" src="/form.js"></script>
<script type="text/javascript" src="/js/httpApi.js"></script>
<script language="JavaScript" type="text/javascript" src="/js/asus_policy.js"></script>
<style>
.MainContent{
	background-color: #4D595D;
	width: 99%;
	-webkit-border-radius: 3px;
	-moz-border-radius: 3px;
	border-radius:3px;
}

.nohover:hover{
	background-color: #293438;
	*background-color: #293438;
}
</style>
<script>
window.onresize = function() {
	if(document.getElementById("folderTree_panel").style.display == "block") {
		cal_panel_block("folderTree_panel", 0.25);
	}
} 

var orig_tencent_download_enable = httpApi.nvramGet(["tencent_download_enable"], true).tencent_download_enable;
var orig_tencent_download_device = httpApi.nvramGet(["tencent_download_device"], true).tencent_download_device;

function initial(){
	if(re_mode == "1"){
		$("#FormTitle").addClass("perNode_MainContent");
		$("#returnBtn").addClass("perNode_returnBtn");
		$("#apply_btn").addClass("perNode_apply_gen");
		show_loading_obj();
	}
	else{
		$("#content_table").addClass("content");
		$("#FormTitle").addClass("FormTitle MainContent");
		$("#returnBtn").addClass("returnBtn");
		$("#apply_btn").addClass("apply_gen");
		show_menu();
	}

	$("#FormTitle").css("display", "");

	if(policy_status.PP == 0 || policy_status.PP_time == ''){
        const policyModal = new PolicyModalComponent({
            policy: "PP",
            agreeCallback: applyRule,
            disagreeCallback: refreshpage
        });
        policyModal.show();
    }else{
        applyRule();
    }

	if(orig_tencent_download_device != '')
		document.getElementById("downloadPath").innerHTML = '/mnt/'+ orig_tencent_download_device;
	else
		document.getElementById("downloadPath").innerHTML = '<div style="margin-left:5px;color:#FC0"><#DM_Install_partition#></div>';
	
	if(document.form.tencent_download_enable.value == "0"){
		document.getElementById("backupPath_tr").style.display = "none";
	}
	else{
		document.getElementById("backupPath_tr").style.display = "";
	}

	setInterval(show_partition, 2000);

	if(orig_tencent_download_device != ''){
		for(var x=0; x<usbDevicesList.length;x++){
			for(var y=0; y<usbDevicesList[x].partition.length;y++){
				if(usbDevicesList[x].partition[y].partName == orig_tencent_download_device){
					var selected_accessable_size = simpleNum(usbDevicesList[x].partition[y].size-usbDevicesList[x].partition[y].used);
					var selected_total_size = simpleNum(usbDevicesList[x].partition[y].size);

					setPart(usbDevicesList[x].partition[y].partName, selected_accessable_size, selected_total_size, true);
					return;
				}
			}

		}
	}
}

function selPartition(){
	show_partition();
	cal_panel_block("folderTree_panel", 0.25);
	$("#folderTree_panel").fadeIn(300);
}

function cancel_folderTree(){
	$("#folderTree_panel").fadeOut(300);
}

function show_partition(){
 	require(['/require/modules/diskList.js?hash=' + Math.random().toString()], function(diskList){
		var htmlcode = "";
		var mounted_partition = 0;
		
		htmlcode += '<table align="center" style="margin:auto;border-collapse:collapse;">';

 		var usbDevicesList = diskList.list();
		for(var i=0; i < usbDevicesList.length; i++){
			for(var j=0; j < usbDevicesList[i].partition.length; j++){
				var all_accessable_size = simpleNum(usbDevicesList[i].partition[j].size-usbDevicesList[i].partition[j].used);
				var all_total_size = simpleNum(usbDevicesList[i].partition[j].size);

				if(usbDevicesList[i].partition[j].status== "unmounted")
					continue;

				if(usbDevicesList[i].partition[j].isAppDev){
					if(all_accessable_size > 1)
						htmlcode += '<tr style="cursor:pointer;" onclick="setPart(\''+ usbDevicesList[i].partition[j].partName +'\', \''+ all_accessable_size +'\', \''+ all_total_size +'\');"><td class="app_table_radius_left"><div class="iconUSBdisk"></div></td><td class="app_table_radius_right" style="width:250px;">\n';
					else
						htmlcode += '<tr><td class="app_table_radius_left"><div class="iconUSBdisk_noquota"></div></td><td class="app_table_radius_right" style="width:250px;">\n';
					htmlcode += '<div class="app_desc"><b>'+ usbDevicesList[i].partition[j].partName + ' <span style="color:#FC0;">(active)</span></b></div>';
				}
				else{
					if(all_accessable_size > 1)
						htmlcode += '<tr style="cursor:pointer;" onclick="setPart(\''+ usbDevicesList[i].partition[j].partName +'\', \''+ all_accessable_size +'\', \''+ all_total_size +'\');"><td class="app_table_radius_left"><div class="iconUSBdisk"></div></td><td class="app_table_radius_right" style="width:250px;">\n';
					else
						htmlcode += '<tr><td class="app_table_radius_left"><div class="iconUSBdisk_noquota"></div></td><td class="app_table_radius_right" style="width:250px;">\n';
					htmlcode += '<div class="app_desc"><b>'+ usbDevicesList[i].partition[j].partName + '</b></div>'; 
				}

				if(all_accessable_size > 1)
					htmlcode += '<div class="app_desc"><#Availablespace#>: <b>'+ all_accessable_size+" GB" + '</b></div>'; 
				else
					htmlcode += '<div class="app_desc"><#Availablespace#>: <b>'+ all_accessable_size+" GB <span style=\'color:#FFCC00\'>(Disk quota can not less than 1GB)" + '</span></b></div>'; 

				htmlcode += '<div class="app_desc"><#Totalspace#>: <b>'+ all_total_size+" GB" + '</b></div>'; 
				htmlcode += '</div><br/><br/></td></tr>\n';
				mounted_partition++;
			}
		}

		if(mounted_partition == 0){
				htmlcode += '<tr height="300px"><td colspan="2" class="nohover"><span class="app_name" style="line-height:100%"><#no_usb_found#></span></td></tr>\n';
		}

		document.getElementById("partition_div").innerHTML = htmlcode;
	});
}

var totalSpace;
var availSpace;
function setPart(_part, _avail, _total, _get){
	document.getElementById("downloadPath").innerHTML = "/mnt/" + _part;
	document.form.tencent_download_device.value = _part;
	cancel_folderTree();
	totalSpace = _total;
	availSpace = _avail;
}

function apply_eula_check(){
	if(document.form.tencent_download_enable.value == "1" && (policy_status.PP == 0 || policy_status.PP_time == ''){
		return false;
	}

	applyRule();
}

function applyRule(){
	if(document.form.tencent_download_device.value == "" && document.form.tencent_download_enable.value == "1"){
		alert("Change the Storage path button to \"Select\" and the text \"Select the USB storage device that you want to access.\"");
		return false;
	}

	showLoading(); 
	document.form.submit();
}
</script>
</head>

<body onload="initial();" onunLoad="return unload_body();" class="bg">
<div id="TopBanner"></div>
<!-- floder tree-->
<div id="folderTree_panel" class="panel_folder">
	<table>
		<tr>
			<td>
				<div style="width:450px;font-family:Arial;font-size:13px;font-weight:bolder; margin-top:23px;margin-left:30px;"><#DM_Install_partition#> :</div>
			</td>
		</tr>
	</table>
	<div id="partition_div" class="folder_tree" style="margin-top:15px;height:335px;">
		<#no_usb_found#>
	</div>
	<div style="background-image:url(images/Tree/bg_02.png);background-repeat:no-repeat;height:90px;margin-top:5px;">
		<input class="button_gen" type="button" style="margin-left:40%;margin-top:18px;" onclick="cancel_folderTree();" value="<#CTL_Cancel#>">
	</div>
</div>

<div id="hiddenMask" class="popup_bg">
	<table cellpadding="5" cellspacing="0" id="dr_sweet_advise" class="dr_sweet_advise" align="center">
		<tr>
		<td>
			<div class="drword" id="drword" style="height:110px;"><#Main_alert_proceeding_desc4#> <#Main_alert_proceeding_desc1#>...
				<br/>
				<br/>
	    </div>
		  <div class="drImg"><img src="images/alertImg.png"></div>
			<div style="height:70px;"></div>
		</td>
		</tr>
	</table>
<!--[if lte IE 6.5]><iframe class="hackiframe"></iframe><![endif]-->
</div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame" autocomplete="off">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="Advanced_TencentDownloadAcceleration.asp">
<input type="hidden" name="next_page" value="Advanced_TencentDownloadAcceleration.asp">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="action_wait" value="5">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="tencent_download_enable" value="<% nvram_get("tencent_download_enable"); %>">
<input type="hidden" name="tencent_download_device" value="<% nvram_get("tencent_download_device"); %>">

<table id="content_table" align="center" cellpadding="0" cellspacing="0">
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
		<div id="FormTitle" style="display: none;">
			<table  border="0" cellpadding="5" cellspacing="0">
				<tbody>
				<tr>
					<td>
					<div style="width: 99%; margin-top: 30px; margin-bottom: 5px;">
						<span class="formfonttitle">Tencent Download Acceleration</span>
						<span id="returnBtn">
							<img onclick="go_setting('/APP_Installation.asp')"  title="Back to USB Extension" src="/images/backprev.png" onMouseOver="this.src='/images/backprevclick.png'" onMouseOut="this.src='/images/backprev.png'">
						</span>
					</div>
					<div id="splitLine" class="splitLine"></div>


					<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable" style="margin-top:8px">
						<thead>
						<tr>
							<td colspan="2"><#t2BC#></td>
						</tr>
						</thead>
						<tr>
							<th>Tencent Download Acceleration Enable</th>
							<td>
								<div class="left" style="width:94px; float:left; cursor:pointer;" id="radio_tencent_download_enable"></div>
								<div class="iphone_switch_container" style="height:32px; width:74px; position: relative; overflow: hidden">
								<script type="text/javascript">
									$('#radio_tencent_download_enable').iphoneSwitch('<% nvram_get("tencent_download_enable"); %>',
										 function() {
											document.form.tencent_download_enable.value = "1";
											$("#backupPath_tr").fadeIn(500);
										 },
										 function() {
											document.form.tencent_download_enable.value = "0";
											$("#backupPath_tr").fadeOut(300);
										 }
									);
								</script>
								</div>

							</td>
						</tr>
						<tr id="backupPath_tr">
							<th>Storage path</a></th>
							<td>
								<input class="button_gen" onclick="selPartition()" type="button" value="<#Select_btn#>"/>
								<span id="downloadPath" style="font-family: Lucida Console;"></span>
							</td>
						</tr>
					</table>

					<div id="apply_btn">
						<input class="button_gen" onclick="apply_eula_check()" type="button" value="<#CTL_apply#>"/>
					</div>
					</td>
				</tr>
				</tbody>
			</table>
		</div>
			<!--===================================End of Main Content===========================================-->
	</td>
  <td width="10" align="center" valign="top">&nbsp;</td>
	</tr>
</table>
</form>

<div id="footer"></div>
</body>
</html>
