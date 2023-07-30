﻿<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=Edge"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title><#Web_Title#> - <#Adaptive_History#></title>
<link rel="stylesheet" type="text/css" href="index_style.css">
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="usp_style.css">
<link rel="stylesheet" type="text/css" href="css/element.css">
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/client_function.js"></script>
<script type="text/javascript" src="/switcherplugin/jquery.iphone-switch.js"></script>
<script type="text/javascript" src="/js/httpApi.js"></script>
<script language="JavaScript" type="text/javascript" src="/js/asus_eula.js"></script>
<style>
*{
	box-sizing: content-box;
}	
.transition_style{
	-webkit-transition: all 0.2s ease-in-out;
	-moz-transition: all 0.2s ease-in-out;
	-o-transition: all 0.2s ease-in-out;
	transition: all 0.2s ease-in-out;
}
</style>
<script>

function initial(){
	show_menu();
	if(document.form.bwdpi_wh_enable.value == 1){
		document.getElementById("log_field").style.display = "";
		getWebHistory("all", "1");
		getWebHistoryList();
	}
	else{
		document.getElementById("log_field").style.display = "none";
	}

	if(!ASUS_EULA.status("tm")){
		ASUS_EULA.config(eula_confirm, cancel);
	}	
}

var htmlEnDeCode = (function() {
    var charToEntityRegex,
        entityToCharRegex,
        charToEntity,
        entityToChar;

    function resetCharacterEntities() {
        charToEntity = {};
        entityToChar = {};
        // add the default set
        addCharacterEntities({
            '&amp;'     :   '&',
            '&gt;'      :   '>',
            '&lt;'      :   '<',
            '&quot;'    :   '"',
            '&#39;'     :   "'"
        });
    }

    function addCharacterEntities(newEntities) {
        var charKeys = [],
            entityKeys = [],
            key, echar;
        for (key in newEntities) {
            echar = newEntities[key];
            entityToChar[key] = echar;
            charToEntity[echar] = key;
            charKeys.push(echar);
            entityKeys.push(key);
        }
        charToEntityRegex = new RegExp('(' + charKeys.join('|') + ')', 'g');
        entityToCharRegex = new RegExp('(' + entityKeys.join('|') + '|&#[0-9]{1,5};' + ')', 'g');
    }

    function htmlEncode(value){
        var htmlEncodeReplaceFn = function(match, capture) {
            return charToEntity[capture];
        };

        return (!value) ? value : String(value).replace(charToEntityRegex, htmlEncodeReplaceFn);
    }

    function htmlDecode(value) {
        var htmlDecodeReplaceFn = function(match, capture) {
            return (capture in entityToChar) ? entityToChar[capture] : String.fromCharCode(parseInt(capture.substr(2), 10));
        };

        return (!value) ? value : String(value).replace(entityToCharRegex, htmlDecodeReplaceFn);
    }

    resetCharacterEntities();

    return {
        htmlEncode: htmlEncode,
        htmlDecode: htmlDecode
    };
})();

var data_array = new Array();
function parsingAjaxResult(rawData){
	var match = 0;
	for(i=0;i<rawData.length;i++){
		var thisRawData = rawData[i];
		thisRawData[2] = htmlEnDeCode.htmlEncode(rawData[i][2]);

		for(j=0;j<data_array.length;j++){
			if((data_array[j][0] == thisRawData[0])
			&& (data_array[j][1] == thisRawData[1])
			&& (data_array[j][2].toUpperCase() == thisRawData[2].toUpperCase())){
				match = 1;
				break;
			}
		}

		if(match == 0)
			data_array.push(thisRawData);

		match = 0;
	}

	var code = "";
	code += "<tr>";
	code += "<th style='width:20%;text-align:left'><#Access_Time#></th>";
	code += "<th style='width:30%;text-align:left'><#PPPConnection_x_MacAddressForISP_itemname#> / <#Client_Name#></th>";
	code += "<th style='width:50%;text-align:left'><#Domain_Name#></th>";
	code += "</tr>";
	for(var i=0; i<data_array.length; i++){
		var thisLog = {
			macAddr: data_array[i][0],
			timeStamp: data_array[i][1],
			hostName: data_array[i][2]
		}

		code += "<tr style='line-height:15px;'>";
		code += "<td>" + convertTime(thisLog.timeStamp) + "</td>";
		if(clientList[thisLog.macAddr] != undefined) {
			var clientName = (clientList[thisLog.macAddr].nickName == "") ? clientList[thisLog.macAddr].name : clientList[thisLog.macAddr].nickName;
			code += "<td title="+ thisLog.macAddr + ">" + clientName + "</td>";
		}
		else
			code += "<td>" + thisLog.macAddr + "</td>";

		code += "<td>" + thisLog.hostName + "</td>";
		code += "</tr>";
	}

	document.getElementById('log_table').innerHTML = code;
	data_array = [];
}

function convertTime(t){
	var time = new Date();
	var time_string = "";
	time.setTime(t*1000);

	time_string = time.getFullYear() + "-" + (time.getMonth() + 1) + "-";
	time_string += transform_time_format(time.getDate());
	time_string += "&nbsp&nbsp" + transform_time_format(time.getHours()) + ":" + transform_time_format(time.getMinutes()) + ":" + transform_time_format(time.getSeconds());

	return time_string;
}

function transform_time_format(time){
	var string = "";
	if(time < 10)
		string += "0" + time;
	else
		string += time;

	return string;
}

var history_array = new Array();
function getWebHistory(mac, page){
	var page_count = page;
	var client = "?client=" + mac + "&page=" + page_count;

	$.ajax({
		url: '/getWebHistory.asp' + client,
		dataType: 'script',
		error: function(xhr){
			setTimeout("getWebHistory(mac, page);", 1000);
		},
		success: function(response){
			history_array = array_temp;
			parsingAjaxResult(array_temp);
			if(page_count == "1"){
				document.getElementById('previous_button').style.visibility = "hidden";
			}
			else{
				document.getElementById('previous_button').style.visibility = "visible";
			}

			if(history_array.length < 50){
				document.getElementById('next_button').style.visibility = "hidden";
			}
			else{
				document.getElementById('next_button').style.visibility = "visible";
			}

			if(page_count == "1" && history_array.length < 50){
				document.getElementById('current_page').style.visibility = "hidden";
			}
			else{
				document.getElementById('current_page').style.visibility = "visible";
			}

			document.getElementById('current_page').value = page_count;
		}
	});
}

function getWebHistoryList(){
	$.ajax({
		url: '/getDBList.asp?type=WebHistory',
		dataType: 'script',
		error: function(xhr){
			setTimeout("getWebHistoryList();", 1000);
		},
		success: function(response){
			genClientListOption();
		}
	});
}

function genClientListOption(){
	var _list = dbList[0];

	if(_list == undefined || _list.length == 0){
		setTimeout("genClientListOption();", 1000);
		return false;
	}

	document.getElementById("clientListOption").options.length = 1;
	for(var i=0; i<_list.length; i++){
		var mac = _list[i];
		if(clientList[_list[i]]){
			var clientObj = clientList[_list[i]];
			var clientName = (clientObj.nickName == "") ? clientObj.name : clientObj.nickName;
			var newItem = new Option(clientName, clientObj.mac);
		}
		else{
			var newItem = new Option(mac, mac);
		}
				
		document.getElementById("clientListOption").options.add(newItem);
	}
}

function change_page(flag, target){
	var current_page = document.getElementById('current_page').value;
	var page = 1;
	if(flag == "next"){
		page = parseInt(current_page) + 1;
		getWebHistory(target, page);
	}
	else{
		page = parseInt(current_page) - 1;
		if(page < 1)
			page = 1;

		getWebHistory(target, page);
	}
}
function applyRule(){
	if(reset_wan_to_fo.change_status)
		reset_wan_to_fo.change_wan_mode(document.form);

	if(document.form.bwdpi_wh_enable.value == 1) {
		var t = new Date();
		var timestamp = t.getTime().toString().substring(0,10);
		document.form.bwdpi_wh_stamp.value = timestamp;
	}
	document.form.submit();
}
function eula_confirm(){
	document.form.TM_EULA.value = 1;
	document.form.bwdpi_wh_enable.value = 1;
	document.form.action_wait.value = "30";
	applyRule();
}

function cancel(){
	curState = 0;
	$('#iphone_switch').animate({backgroundPosition: -37}, "slow", function() {});
	document.form.action_wait.value = "30";
	document.form.action_script.value = "restart_qos;restart_firewall";
}
function switch_control(_status){
	if(_status) {
		if(reset_wan_to_fo.check_status()) {
			if(ASUS_EULA.check("tm")){
				document.form.bwdpi_wh_enable.value = 1;
				applyRule();
			}
		}
		else
			cancel();
	}
	else {
		document.form.bwdpi_wh_enable.value = 0;
		applyRule();
	}
}
function cal_panel_block(obj){
	var blockmarginLeft;
	if (window.innerWidth)
		winWidth = window.innerWidth;
	else if ((document.body) && (document.body.clientWidth))
		winWidth = document.body.clientWidth;

	if (document.documentElement  && document.documentElement.clientHeight && document.documentElement.clientWidth){
		winWidth = document.documentElement.clientWidth;
	}

	if(winWidth >1050){
		winPadding = (winWidth-1050)/2;
		winWidth = 1105;
		blockmarginLeft= (winWidth*0.2)+winPadding;
	}
	else if(winWidth <=1050){
		blockmarginLeft= (winWidth)*0.2 + document.body.scrollLeft;
	}

	if(obj == "demo_background")
		document.getElementById(obj).style.marginLeft = (blockmarginLeft + 25)+"px";
	else
		document.getElementById(obj).style.marginLeft = blockmarginLeft+"px";
}
function updateWebHistory() {
	setTimeout(function() {
		getWebHistory(document.form.clientList.value, '1');
	}, 200);
}
function exportWebHistoryLog(){
	var all_array = [];
	var mac = document.form.clientList.value;
	var page = 1;

	var get_history = function(_mac, _page){
		 $.get("/appGet.cgi?hook=bwdpi_history()&client=" + _mac + "&page=" + _page + "", function(data){
			var result = jQuery.parseJSON(data);
			var len = result.bwdpi_history.length;
			if(len > 0){
				all_array = $.merge(all_array, result.bwdpi_history);
				if(len == 50){
					page++;
					get_history(mac, page);
				}
				else{
					export_CSV(all_array);
				}
			}
			else{
				export_CSV(all_array);
			}
		});
	};
	get_history(mac, page);
}
function export_CSV(export_array) {
	var data = [["<#Access_Time#>", "<#MAC_Address#>", "<#Client_Name#>".replace(/&#39;/g, "'"), "<#Domain_Name#>"]];
	var tempArray = new Array();

	var setArray = function(array) {
		for(var i = 0; i < array.length; i += 1) {
			var thisLog = {
				macAddr: array[i][0],
				timeStamp: array[i][1],
				hostName: array[i][2]
			}

			tempArray = [];
			tempArray[0] = (convertTime(thisLog.timeStamp)).replace("&nbsp&nbsp", "  ");
			if(clientList[thisLog.macAddr] != undefined) {
				var clientName = (clientList[thisLog.macAddr].nickName == "") ? clientList[thisLog.macAddr].name : clientList[thisLog.macAddr].nickName;
				tempArray[1] = thisLog.macAddr;
				tempArray[2] = clientName;
			}
			else
				tempArray[2] = tempArray[1] = thisLog.macAddr;

			tempArray[3] = thisLog.hostName;
			data.push(tempArray);
		}
	};
	setArray(export_array);

	var csvContent = '';
	data.forEach(function (infoArray, index) {
		var dataString = infoArray.join(',');
		csvContent += index < data.length ? dataString + '\n' : dataString;
	});

	var download = function(content, fileName, mimeType) {
		var a = document.createElement('a');
		mimeType = mimeType || 'application/octet-stream';

		if (navigator.msSaveBlob) { // IE10
			return navigator.msSaveBlob(new Blob([content], { type: mimeType }), fileName);
		}
		else if ('download' in a) { //html5 A[download]
			a.href = 'data:' + mimeType + ',' + "%EF%BB%BF" + encodeURIComponent(content);
			a.setAttribute('download', fileName);
			document.getElementById("log_table").appendChild(a);
			setTimeout(function() {
				a.click();
				document.getElementById("log_table").removeChild(a);
			}, 66);
			return true;
		}
		else { //do iframe dataURL download (old ch+FF):
			var f = document.createElement('iframe');
			document.getElementById("log_table").appendChild(f);
			f.src = 'data:' + mimeType + ',' + "%EF%BB%BF" + encodeURIComponent(content);

			setTimeout(function() {
				document.getElementById("log_table").removeChild(f);
				}, 333);
			return true;
		}
	};

	download(csvContent, 'Web_History.csv', 'data:text/csv;charset=utf-8');
}
</script>
</head>
<body onload="initial();" onunload="unload_body();" class="bg">
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>
<div id="hiddenMask" class="popup_bg" style="z-index:999;">
	<table cellpadding="5" cellspacing="0" id="dr_sweet_advise" class="dr_sweet_advise" align="center"></table>
	<!--[if lte IE 6.5]><iframe class="hackiframe"></iframe><![endif]-->
</div>
<iframe name="hidden_frame" id="hidden_frame" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="current_page" value="/AdaptiveQoS_WebHistory.asp">
<input type="hidden" name="next_page" value="/AdaptiveQoS_WebHistory.asp">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="restart_qos;restart_firewall">
<input type="hidden" name="action_wait" value="30">
<input type="hidden" name="flag" value="">
<input type="hidden" name="bwdpi_wh_enable" value="<% nvram_get("bwdpi_wh_enable"); %>">
<input type="hidden" name="bwdpi_wh_stamp" value="<% nvram_get("bwdpi_wh_stamp"); %>">
<input type="hidden" name="TM_EULA" value="<% nvram_get("TM_EULA"); %>">
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
									<div id="content_title" class="formfonttitle"><#menu5_3_2#> - <#Adaptive_History#></div>
									<div style="margin:10px 0 10px 5px;" class="splitLine"></div>
									<div class="formfontdesc">
										<#Adaptive_History_desc#>
									</div>
									<div style="margin:5px">
										<table style="margin-left:0px;" width="95%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
											<th><#Adaptive_History#></th>
											<td>
												<div align="center" class="left" style="width:94px; float:left; cursor:pointer;" id="bwdpi_wh_enable"></div>
															<script type="text/javascript">
																$('#bwdpi_wh_enable').iphoneSwitch('<% nvram_get("bwdpi_wh_enable"); %>',
																	function(){
																		switch_control(1);
																	},
																	function(){
																		switch_control(0);
																	}
																);
															</script>
											</td>
										</table>
									</div>
									<div id="log_field">
										<div style="margin:10px 5px">
											<select id="clientListOption" class="input_option" name="clientList" onchange="getWebHistory(this.value, '1');">
												<option value="all" selected><#All_Client#></option>
											</select>
											<label style="margin: 0 5px 0 20px;visibility:hidden;cursor:pointer" id="previous_button" onclick="change_page('previous', document.getElementById('clientListOption').value);">Previous</label>
											<input class="input_3_table" value="1" id="current_page"></input>
											<label style="margin-left:5px;cursor:pointer" id="next_button" onclick="change_page('next', document.getElementById('clientListOption').value);">Next</label>
										</div>
										<div class="web_frame" style="height:600px;overflow:auto;margin:5px">
											<table style="width:100%" id="log_table"></table>
										</div>
										<div class="apply_gen">
											<input class="button_gen" onClick="httpApi.cleanLog('web_history', updateWebHistory);" type="button" value="<#CTL_clear#>" >
											<input class="button_gen" onClick="getWebHistory(document.form.clientList.value, '1')" type="button" value="<#CTL_refresh#>">
											<input class="button_gen" onClick="exportWebHistoryLog()" type="button" value="<#btn_Export#>" >
										</div>
									</div>
								</td>
							</tr>
						</table>
					</td>
				</tr>
			</table>
			<!--===================================End of Main Content===========================================-->
		</td>
	</tr>
</table>
</form>
<div id="footer"></div>
</body>
</html>
