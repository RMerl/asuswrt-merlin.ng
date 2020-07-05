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
#updateProgress_bg{
	margin-top:5px;
	height:30px;
	width:97%;
	background: #596E74;
	border-top: 1px solid #929EA1;
	border-bottom: 1px solid #929eA1;
}
</style>
<script type="text/javascript" src="../require/require.min.js"></script>
<script type="text/javascript" src="../js/jquery.js"></script>
<script type="text/javascript" src="../validator.js"></script>
<script>
if(parent.location.pathname.search("index") === -1) top.location.href = "../index.asp";

var diskmon_usbport = '<% nvram_get("diskmon_usbport"); %>';
var diskOrder = parent.getSelectedDiskOrder();
var progressBar;

function initial(){
	if(diskmon_usbport != parent.usbPorts[diskOrder-1].node){
		document.usbUnit_form.diskmon_usbport.value = parent.usbPorts[diskOrder-1].node;
		document.usbUnit_form.submit();
	}

	set_disk_info(parent.usbPorts[diskOrder-1]);

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
	$('#diskTab').html(parent.gen_tab_menu(disk_list_array, "format"));
}
function disk_scan_status() {
	require(['/require/modules/diskList.js'], function(diskList){
		diskList.list(function(){
	 		$.each(parent.usbPorts, function(i, curPort){
		 		$.each(diskList.list(), function(j, usbDevice){
	 				if(curPort.node == usbDevice.node)
	 					parent.usbPorts[i] = usbDevice;
		 		});
	 		});

			set_disk_info(parent.usbPorts[diskOrder-1]);
			update_deviceDec(parent.usbPorts[diskOrder-1]);
		})
	});
}
function set_disk_info(device) {
	var disk_system = "";
	for(var i = 0; i < device.partition.length; i += 1) {
		if(device.partition[i].format != "unknown") {
			document.form.disk_name.value = device.partition[i].partName;
			disk_system = device.partition[i].format;
		}
	}
	
	switch(disk_system) {
		case "tfat" :
			document.form.disk_name.maxLength = 11;
			disk_system = "tfat";
			break;
		case "tntfs" :
			document.form.disk_name.maxLength = 32;
			disk_system = "tntfs";
			break;
		case "thfsplus" :
			document.form.disk_name.maxLength = 30;
			disk_system = "thfsplus";
			break;
		case "ext4" :
			document.form.disk_name.maxLength = 16;
			disk_system = "ext4";
			break;
		default :
			document.form.disk_name.maxLength = 11;
			disk_system = "tfat";
	}

	var usb_fatfs_mod = '<% nvram_get("usb_fatfs_mod"); %>';
	var usb_ntfs_mod = '<% nvram_get("usb_ntfs_mod"); %>';
	var usb_hfs_mod = '<% nvram_get("usb_hfs_mod"); %>';
	if(usb_fatfs_mod != "tuxera")
		$("#disk_system option[value='tfat']").remove();
	if(usb_ntfs_mod != "tuxera")
		$("#disk_system option[value='tntfs']").remove();
	if(usb_hfs_mod != "tuxera")
		$("#disk_system option[value='thfsplus']").remove();

	var selected_disk_system = document.form.disk_system;
	selected_disk_system.selectedIndex = 0; 
	for(var i = 0; i < selected_disk_system.length; i += 1) {
		if(selected_disk_system.options[i].value == disk_system) {
			selected_disk_system.value = disk_system;
			break;
		}
	}
}
function update_deviceDec(device) {
	var dec_html_code = '';
	var percentbar = 0;

	if(device.mountNumber > 0){
		var simpleNum2 = function(num) {
			if(typeof(num) == "string" && num.length == 0)
				return 0;

			return parseInt(num*1000)/1000;
		};
		percentbar = simpleNum2((device.totalSize - device.totalUsed)/device.totalSize*100);
		percentbar = Math.round(100 - percentbar);	
		dec_html_code += '<div id="diskquota" align="left" style="margin-top:5px;margin-bottom:10px;">\n';
		dec_html_code += '<div class="quotabar" style="width:' + percentbar + ';height:13px;"></div>';
		dec_html_code += '</div>\n';
	}
	else{
		dec_html_code += '<div class="style1"><strong id="diskUnmount'+ device.usbPath +'"><#DISK_UNMOUNTED#></strong></div>\n';
	}

	parent.document.getElementById("deviceDec_"+device.usbPath).innerHTML = dec_html_code;
}
function go_format() {
	var Block_chars = function(obj, keywordArray) {
		// bolck ascii code 32~126 first
		var invalid_char = "";
		for(var i = 0; i < obj.value.length; ++i) {
			if(obj.value.charCodeAt(i) < '32' || obj.value.charCodeAt(i) > '126') {
				invalid_char += obj.value.charAt(i);
			}
		}
		if(invalid_char != "") {
			alert('<#JS_validstr2#>" '+ invalid_char +'" !');
			obj.focus();
			return false;
		}

		// check if char in the specified array
		if(obj.value) {
			invalid_char = "";
			for(var i=0; i<keywordArray.length; i++) {
				if( obj.value.indexOf(keywordArray[i]) >= 0) {
					if(keywordArray[i] == " ")
						invalid_char += "Space";
					else 
						invalid_char += keywordArray[i];
					
				}
			}
			if(invalid_char != "") {
				alert(invalid_char+ " <#JS_invalid_chars#>");
				obj.focus();
				return false;
			}
		}
		return true;
	};

	document.form.disk_name.value = document.form.disk_name.value.trim();
	if(!validator.isEmpty(document.form.disk_name))
		return false;

	var disk_system = document.form.disk_system.value;
	var temp_label = document.form.disk_name.value;
	if(disk_system == "tfat") {
		if(temp_label.length > 12)
			document.form.disk_name.value = temp_label.substr(0, 11);
	}
	else if(disk_system == "thfsplus") {
		if(temp_label.length > 31)
			document.form.disk_name.value = temp_label.substr(0, 30);
	}
	else if(disk_system == "ext4") {
		if(temp_label.length > 17)
			document.form.disk_name.value = temp_label.substr(0, 16);
	}

	if(!Block_chars(document.form.disk_name, ["~", "`", "!", "#", "$", "%", "^", "&", "*", "(", ")", "+", "=", "{", "[", "}", "]", "|", "\\", ":", ";", "\"", "'", "<", ">", ",", ".", "?", "/", " "]))
		return false;

	if(!confirm("<#format_confirm_alert#>")) {
		document.getElementById('scan_status_field').style.display = "";
		document.getElementById('progressBar').style.display = "none";
		return false;
	}

	$('#textarea_disk0').text('');
	document.getElementById("updateProgress").style.width = "0%";
	document.getElementById('progress_bar_no').innerHTML = "0%";
	document.getElementById('loadingIcon').style.display = "";
	document.getElementById('progressBar').style.display = "";
	document.getElementById('scan_status_field').style.display = "none";
	document.getElementById('btn_format').disabled = true;
	
	document.form.diskformat_file_system.value = document.form.disk_system.value;
	document.form.diskformat_label.value = document.form.disk_name.value;
	document.form.submit();
}
function change_disk_system() {
	var disk_system = document.form.disk_system.value;
	var temp_label = document.form.disk_name.value;
	if(disk_system == "tfat") {
		document.form.disk_name.maxLength = 11;
		if(temp_label.length > 12)
			document.form.disk_name.value = temp_label.substr(0, 11);
	}
	else if(disk_system == "tntfs") {
		document.form.disk_name.maxLength = 32;
	}
	else if(disk_system == "thfsplus") {
		document.form.disk_name.maxLength = 30;
		if(temp_label.length > 31)
			document.form.disk_name.value = temp_label.substr(0, 30);
	}
	else if(disk_system == "ext4") {
		document.form.disk_name.maxLength = 16;
	}
}
function show_loadingBar_field(){
	document.getElementById('loadingIcon').style.display = "none";
	showLoadingUpdate();
	progressBar = 1;

	parent.document.getElementById('ring_USBdisk_'+diskOrder).style.display = "";
	parent.document.getElementById('ring_USBdisk_'+diskOrder).style.backgroundImage = "url(/images/New_ui/networkmap/backgroud_move_8P_2.0.gif)";
	if(parent.rog_support){
		parent.document.getElementById('ring_USBdisk_'+diskOrder).style.backgroundRepeat = "no-repeat";
		parent.document.getElementById('ring_USBdisk_'+diskOrder).style.backgroundPosition = '34px -3px';
	}
	else
		parent.document.getElementById('ring_USBdisk_'+diskOrder).style.backgroundPosition = '-1px -1px';
}
function showLoadingUpdate(){
	$.ajax({
		url: '../ajax_disk_format.xml?diskmon_usbport=' + parent.usbPorts[diskOrder-1].node,
		dataType: 'xml',
		error: function(xhr) {
			showLoadingUpdate();
		},
		success: function(xml) {
			var disk_format_flag = $(xml).find('disk_flag').text();
			var disk_format_log = $(xml).find('disk_log').text();
			if(disk_format_flag == "0") {
				if(progressBar >= 5)
					progressBar = 5;
				document.getElementById('scan_message').innerHTML = "Initializing disk format…";
			}
			else if(disk_format_flag == "1") {
				if(progressBar <= 5)
					progressBar = 6;
				else if (progressBar >= 15)
					progressBar = 15;
				document.getElementById('scan_message').innerHTML = "<#diskUtility_umount#>";
			}
			else if(disk_format_flag == "2") {
				if(progressBar <= 15)
					progressBar = 16;
				else if (progressBar >= 70)
					progressBar = 70;
				document.getElementById('scan_message').innerHTML = "Disk formating ...";
			}
			else if(disk_format_flag == "3") {
				if(progressBar <= 70)
					progressBar = 71;
				else if (progressBar >= 95)
					progressBar = 95;
				document.getElementById('scan_message').innerHTML = "<#diskUtility_reMount#>";
			}
			else if(disk_format_flag == "4") {
				if(progressBar <= 95)
					progressBar = 96;
				document.getElementById('scan_message').innerHTML = "Finishing disk formating...";
			}
			else if(disk_format_flag == "-1") {
				document.getElementById("updateProgress").style.width = progressBar + "%";
				document.getElementById('progress_bar_no').innerHTML = progressBar + "%";
				document.getElementById('scan_message').innerHTML = "Disk format error!";
				if(!parent.rog_support) {
					parent.document.getElementById('ring_USBdisk_'+diskOrder).style.backgroundImage = "url(/images/New_ui/networkmap/white_04.gif)";
					parent.document.getElementById('ring_USBdisk_'+diskOrder).style.backgroundPosition = '0px -184px';
					parent.document.getElementById('iconUSBdisk_'+diskOrder).style.backgroundPosition = '1px -206px';
				}
				document.getElementById('btn_format').disabled = false;
				$('#textarea_disk0').html($(xml).find('disk_log').text());
				return false;
			}
			progressBar++;
			document.getElementById("updateProgress").style.width = progressBar + "%";
			document.getElementById('progress_bar_no').innerHTML = progressBar + "%";
			$('#textarea_disk0').html($(xml).find('disk_log').text());
			if(progressBar > 100) {
				document.getElementById('progressBar').style.display = "none";
				document.getElementById('scan_status_field').style.display = "";
				document.getElementById('btn_format').disabled = false;
				if(!parent.rog_support) {
					parent.document.getElementById('ring_USBdisk_'+diskOrder).style.backgroundImage = "url(/images/New_ui/networkmap/white_04.gif)";
					parent.document.getElementById('ring_USBdisk_'+diskOrder).style.backgroundPosition = '0% 0%';
				}
				else
					parent.document.getElementById('ring_USBdisk_'+diskOrder).style.backgroundImage = "";

				if(parent.rog_support)
					parent.document.getElementById('iconUSBdisk_'+diskOrder).style.backgroundPosition = '1px -95px';
				else
					parent.document.getElementById('iconUSBdisk_'+diskOrder).style.backgroundPosition = '1px -105px';
				disk_scan_status();
				return false;
			}

			setTimeout("showLoadingUpdate();", 100);		
		}
	});
}

</script>
</head>
<iframe name="hidden_frame" id="hidden_frame" width="0" height="0" frameborder="0" scrolling="no"></iframe>
<form method="post" name="usbUnit_form" action="/apply.cgi" target="hidden_frame">
<input type="hidden" name="action_mode" value="change_diskmon_unit">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="action_wait" value="1">
<input type="hidden" name="diskmon_usbport" value="">
<input type="hidden" name="current_page" value="">
<input type="hidden" name="next_page" value="">
</form>
<body class="statusbody" onload="initial();">
<form name="form" method="post" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="next_page" value="/device-map/disk_format.asp">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="start_diskformat">
<input type="hidden" name="action_wait" value="1">
<input type="hidden" name="diskformat_file_system" value="tfat">
<input type="hidden" name="diskformat_label" value="">
<div id="diskTab" class='tab_table'></div>
<table width="95%" align="center" cellspacing="0">
  <tr >
    <td class="list_bg">
		<div id="scan_status_field" style="margin-top:10px;margin-left:10px;">
			<table>
				<tr>
					<td><#format_desc#></td>
				</tr>
			</table>	
		</div>
		<div id="progressBar" style="margin-left:9px;;margin-top:10px;display:none">
			<div id="scan_message"></div>
			<div id="updateProgress_bg">
				<div>
					<span id="progress_bar_no" style="position:absolute;margin-left:130px;margin-top:7px;" ></span>
					<div id="updateProgress" class="quotabar" style="width:0%;height:30px;"></div>
				</div>
			</div>
		</div>
		<div style="margin-top:5px;*margin-top:-10px;width:283px;" class="line_horizontal"></div>
		<div style="padding:7px;">
			<div class="formfonttitle_nwm"><#DiskLabel#> :</div>
			<div>
				<input type="text" maxlength="32" class="input_25_table" name="disk_name" value="" onkeypress="return validator.isString(this, event)" autocorrect="off" autocapitalize="off">
			</div>

		</div>
		<div style="margin-top:5px;*margin-top:-10px;width:283px;" class="line_horizontal"></div>
		<div style="padding:7px;">
			<div class="formfonttitle_nwm"><#format_type#> :</div>
			<div>
				<select name="disk_system" id="disk_system" class="input_option" style="margin-left:2px;" onChange="change_disk_system();">
					<option value="tntfs">NTFS</option>
					<option value="tfat">FAT</option>
					<option value="thfsplus">HFS</option>
					<option value="ext4">EXT4</option>
				</select>
			</div>
		</div>
		<div style="margin-top:5px;*margin-top:-10px;width:283px;" class="line_horizontal"></div>
		<div class="formfonttitle_nwm" style="margin-left:10px;margin-bottom:5px;margin-top:10px;"><#format_results#></div>
		<span id="log_field" >
			<textarea cols="15" rows="13" readonly="readonly" id="textarea_disk0" class="textarea_bg" style="resize:none;display:;width:93%; font-family:'Courier New', Courier, mono; font-size:11px;margin-left:8px;color:#FFFFFF;"></textarea>
		</span>
		<div style="margin-top:20px;margin-bottom:10px;"align="center">
			<input id="btn_format" type="button" class="button_gen" onclick="go_format();" value="<#CTL_format#>">
			<img id="loadingIcon" style="display:none;margin-right:10px;" src="/images/InternetScan.gif">
		</div>
    </td>
  </tr>
</table>
</form>
</body>
</html>
