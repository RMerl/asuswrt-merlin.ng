<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=Edge"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<title><#Web_Title#> - <#AiProtection_sites_blocking#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/disk_functions.js"></script>
<script type="text/javascript" src="/form.js"></script>
<script type="text/javascript" src="/client_function.js"></script>
<script type="text/javascript" src="/js/Chart.js"></script>
<script type="text/javascript" src="/js/httpApi.js"></script>
<script type="text/javascript" src="/validator.js"></script>
<style>
#googleMap > div{
	border-radius: 10px;
}
.detail-table{
	background-color: #3C3C3C;
	margin: 6px 3px;
	padding: 6px 18px;
	border-radius: 6px;
}
.arrow-field{
	cursor:pointer;
	background-size: 100%;
	background-repeat: no-repeat;
	transition: linear 0.3s;
}
.arrow-right{
	padding: 0 4px;
	margin-left: 10px;
	background-image: url('images/New_ui/arrow_right.svg');
}
.arrow-down{
	padding: 0 7px;
	margin-left: 5px;
	background-image: url('images/New_ui/arrow_down.svg');
}
.confirm{
	width:650px;
	height:200px;
	position:absolute;
	background: #293438;
	z-index:10;
	margin: 0 0 0 300px;
	border-radius:10px;
	display: none;
}
.confirm-button{
	background: linear-gradient(#233438 0%, #0F1011 100%);
	border-radius: 8px;
	height:33px;
	cursor: pointer;
	min-width:120px;
	line-height: 33px;
	text-align: center;
	margin: 0 12px;
}
.icon-container{
	width: 24px;
	height: 24px;
	cursor:pointer;
	background-size: 100%;
}
.icon-save{
	background:url('images/save.svg');
}
.icon-save:hover{
	background:url('images/save_hover.svg');
}
.icon-save:active{
	background:url('images/save_active.svg');
}
.icon-delete{
	background:url('images/delete.svg');
}
.icon-delete:hover{
	background:url('images/delete_hover.svg');
}
.icon-delete:active{
	background:url('images/delete_active.svg');
}
.icon-edit{
	background:url('images/edit.svg');
}
.icon-edit:hover{
	background:url('images/edit_hover.svg');
}
.icon-edit:active{
	background:url('images/edit_active.svg');
}
</style>
<script>
window.onresize = function() {
	if(document.getElementById("erase_confirm").style.display == "block") {
		cal_panel_block("erase_confirm", 0.25);
	}
}
<% get_AiDisk_status(); %>
var AM_to_cifs = get_share_management_status("cifs");  // Account Management for Network-Neighborhood
var AM_to_ftp = get_share_management_status("ftp");  // Account Management for FTP

var ctf_disable = '<% nvram_get("ctf_disable"); %>';
var ctf_fa_mode = '<% nvram_get("ctf_fa_mode"); %>';

function initial(){
	show_menu();
	if(document.form.wrs_protect_enable.value == '1' && document.form.wrs_mals_enable.value == '1'){
		mals_check('1');
	}
	else{
		mals_check('0');
	}
	
	getMalsCount();
	getEventTime();
	getIPSData("mals", "mac");
	var t = new Date();
	var timestamp = t.getTime();
	var date = timestamp.toString().substring(0, 10);
	getIPSChart("mals", date);
	getIPSDetailData("mals", "all");

	$('#newDomain').on({
		search: function(e){	
			var inputValue = e.currentTarget.value;
			quickAdd(inputValue);
		},
		keyup: function(e){
			var inputValue = e.currentTarget.value;
			if(e.keyCode != '13'){
				query(inputValue);
			}
		}
	});
}

function getEventTime(){
	var time = document.form.wrs_mals_t.value*1000;
	var mals_date = transferTimeFormat(time);
	$("#mals_time").html(mals_date);
}

function transferTimeFormat(time){
	if(time == 0){
		return '';
	}

	var t = new Date();
	t.setTime(time);
	var year = t.getFullYear();
	var month = t.getMonth() + 1;
	if(month < 10){
		month  = "0" + month;
	}
	
	var date = t.getDate();
	if(date < 10){
		date = "0" + date;
	}
	
	var hour = t.getHours();
	if(hour < 10){
		hour = "0" + hour;
	}
			
	var minute = t.getMinutes();
	if(minute < 10){
		minute = "0" + minute;
	}

	var date_format = "Since " + year + "/" + month + "/" + date + " " + hour + ":" + minute;
	return date_format;
}

var mals_count = 0;
function getMalsCount(){
	$.ajax({
		url: '/getAiProtectionEvent.asp',
		dataType: 'script',	
		error: function(xhr) {
			setTimeout("getMalsCount();", 1000);
		},
		success: function(response){
			var code = ""
			mals_count = event_count.mals_n;
			code += mals_count;
			code += '<span style="font-size: 16px;padding-left: 5px;"><#AiProtection_scan_rHits#></span>';
			$("#mals_count").html(code);
		}
	});
}

function getIPSData(type, event){
	$.ajax({
		url: '/getIPSEvent.asp?type=' + type + '&event=' + event,
		dataType: 'script',	
		error: function(xhr) {
			setTimeout("getIPSData('mals', event);", 1000);
		},
		success: function(response){
			if(data != ""){
				var data_array = JSON.parse(data);
				collectInfo(data_array);
			}
		}
	});
}

var info_bar = new Array();
var hit_count_all = 0;
function collectInfo(data){
	for(i=0;i<data.length;i++){
		var mac = data[i][0];
		var ip = ""
		var hit = data[i][1];
		var name = "";
		if(clientList[mac]){
			name = (clientList[mac].nickName == "") ? clientList[mac].name : clientList[mac].nickName;
			ip = clientList[mac].ip;
		}
		else{
			name = mac;
		}

		hit_count_all += parseInt(hit);
		info_bar.push(mac);
		info_bar[mac] = new targetObject(ip, name, hit, mac);
	}

	generateBarTable();
}

function targetObject(ip, name, hit, mac){
	this.ip = ip;
	this.name = name;
	this.hit = hit;
	this.mac = mac;
}

function generateBarTable(){
	var code = '';
	for(i=0;i<info_bar.length;i++){
		var targetObj = info_bar[info_bar[i]];
		code += '<div style="margin:10px;">';
		code += '<div style="display:inline-block;width:130px;">'+ targetObj.name +'</div>';
		code += '<div style="display:inline-block;width:150px;">';
		if(hit_count_all == 0){
			var percent = 0;
		}
		else{
			var percent = parseInt((targetObj.hit/hit_count_all)*100);
			if(percent > 85)
				percent = 85;
		}

		code += '<div style="width:'+ percent +'%;background-color:#FC0;height:13px;border-radius:1px;display:inline-block;vertical-align:middle"></div>';
		code += '<div style="display:inline-block;padding-left:5px;">'+ targetObj.hit +'</div>';
		code += '</div>';
		code += '</div>';
	}

	if(code == ''){
		code += '<div style="font-size:16px;text-align:center;margin-top:70px;color:#FC0"><#AiProtection_eventnodetected#></div>';		
	}

	$("#vp_bar_table").html(code);
}

function getIPSChart(type, date){
	$.ajax({
		 url: '/getNonIPSChart.asp?type=' + type + '&date='+ date,
		dataType: 'script',	
		error: function(xhr) {
			setTimeout("getIPSChart('mals', date);", 1000);
		},
		success: function(response){
			collectChart(data, date);
		}
	});
}

function collectChart(data, date){
	var timestamp = date*1000;
	var t = new Date(timestamp);
	t.setHours(23);
	t.setMinutes(59);
	t.setSeconds(59);
	var timestamp_new = t.getTime();
	var date_label = new Array();
	var month = "";
	var date = "";
	
	for(i=0;i<7;i++){
		var temp = new Date(timestamp_new);
		var date_format = "";
		month = temp.getMonth() + 1;
		date = temp.getDate();
		date_format = month + '/' + date;
		timestamp_new -= 86400000;
		date_label.unshift(date_format);
	}

	var high_array = new Array();
	var medium_array = new Array();
	var low_array =  new Array();
	hight_array = data[0];
	medium_array = data[1];
	low_array = data[2];

	drawLineChart(date_label, hight_array, medium_array, low_array);
}

function drawLineChart(date_label, high_array, medium_array, low_array){
	var lineChartData = {
		labels: date_label,
		datasets: [{
			fillColor: "rgba(255,255,255,0)",
			strokeColor: "#FFE500",
			pointColor: "#FFE500",
			pointHighlightFill: "#FFF",
			pointHighlightStroke: "#FFE500",
			data: high_array
		}]
	}

	var ctx = document.getElementById("canvas").getContext("2d");
	window.myLine = new Chart(ctx).Line(lineChartData, {
		responsive: true
	});
}

var dataObject = new Object;
var csvContent = 'Time,Threat,Source,Destination';
function getIPSDetailData(type, event){
	$.ajax({
		url: '/getIPSDetailEvent.asp?type=' + type + '&event=' + event,
		dataType: 'script',	
		error: function(xhr) {
			setTimeout("getIPSDetailData('mals', event);", 1000);
		},
		success: function(response){
			if(data != ""){
				var data_array = JSON.parse(data);
				for(i=0; i<data_array.length; i++){
					csvContent += '\n';
					csvContent += data_array[i][0] + ',' + data_array[i][1] + ',' + data_array[i][2] + ',' + data_array[i][3];

					var _index = data_array[i][3] + '_' + data_array[i][2];
					if(dataObject[_index]){
						dataObject[_index].source.push(data_array[i][2]);
						dataObject[_index].destination.push(data_array[i][3]);
						dataObject[_index].time.push(data_array[i][0]);
					}
					else{					
						dataObject[_index] = {
							source: [],
							destination: [],
							time: [],
							threat: ''
						}

						dataObject[_index].source.push(data_array[i][2]);
						dataObject[_index].destination.push(data_array[i][3]);
						dataObject[_index].time.push(data_array[i][0]);
						dataObject[_index].threat = data_array[i][1];
					}
				}

				generateDetailTable(dataObject);
			}
		}
	});
}

var cat_id_index = [["39", "Proxy Avoidance"], ["73", "Malicious Software"], ["74", "Spyware"], ["75", "Phishing"], ["76", "Spam"], 
					["77", "Adware"], ["78", "Malware Accomplic"], ["79", "Disease Vector"], ["80", "Cookies"], ["81", "Dialers"], 
					["82", "Hacking"], ["83", "Joke Program"], ["84", "Password Cracking Apps"], ["85", "Remote Access"], ["86", "Made for AdSense sites"],
					["91", "C&C Server"], ["92", "Malicious Domain"], ["94", "Scam"], ["95", "Ransomware"]];
var cat_id_array = new Array();
for(i=0; i<cat_id_index.length;i++){
	var index = "_" + cat_id_index[i][0];
	cat_id_array.push(index);
	cat_id_array[index] = new catID_Object(cat_id_index[i][0], cat_id_index[i][1]);
}


function catID_Object(id, description){
	this.id = id;
	this.description = description;
	return this;
}

function generateDetailTable(dataObj){
	var code = '';
	code += '<div style="font-size:14px;font-weight:bold;border-bottom: 1px solid #797979">';
	code += '<div style="display:table-cell;width:180px;padding-right:5px;"><#diskUtility_time#></div>';
	code += '<div style="display:table-cell;width:80px;padding-right:5px;"><#AiProtection_event_Threat#></div>';
	code += '<div style="display:table-cell;width:170px;padding-right:5px;"><#AiProtection_event_Source#></div>';
	code += '<div style="display:table-cell;width:200px;padding-right:5px;"><#AiProtection_event_Destination#></div>';
	code += '<div style="display:table-cell;width:50px;padding-right:5px;"></div>';
	code += '</div>';

	if(dataObj == ""){
		code += '<div style="text-align:center;font-size:16px;color:#FC0;margin-top:90px;"><#IPConnection_VSList_Norule#></div>';
	}
	else{
		for(i=0; i<Object.keys(dataObj).length; i++){
			code += '<div style="word-break:break-all;border-bottom: 1px solid #797979">';
			code += '<div style="display:table-cell;width:180px;height:30px;vertical-align:middle;padding-right:5px;">';
			var eventID = Object.keys(dataObj)[i];

			code += dataObj[eventID].time[0];
			if(dataObj[eventID].time.length != 1){
				code += '<span id="'+ eventID +'_arrow" class="arrow-field arrow-right" onclick="showDetailEvent(this, \''+ eventID +'\');"></span>';
			}
	
			code += '</div>';

			var cat_id_index = "_" + dataObj[eventID].threat;
			code += '<div style="display:table-cell;width:80px;height:30px;vertical-align:middle;padding-right:5px;">'+ cat_id_array[cat_id_index].description +'</div>';
			var _name = dataObj[eventID].source[0];
			if(clientList[dataObj[eventID].source[0]]){
				_name = clientList[dataObj[eventID].source[0]].name;
			}

			code += '<div style="display:table-cell;width:170px;height:30px;vertical-align:middle;padding-right:5px;">'+ _name +'</div>';
			code += '<div style="display:table-cell;width:200px;height:30px;vertical-align:middle;padding-right:5px;text-decoration:underline;cursor:pointer;" title="<#AiProtection_DetailTable_Click_Title#>" onclick="checkEventSafe(\''+ dataObj[eventID].destination[0] +'\');">'+ dataObj[eventID].destination[0] +'</div>';
			code += '<div style="display:table-cell;width:50px;padding-right:5px;vertical-align:middle" onclick="quickAddWhitelist(\''+ dataObj[eventID].destination[0] +'\')"><img src="images/New_ui/add.svg" style="width:21px;height:21px;"></div>';
			code += '</div>';
		}
	}
	
	$("#detail_info_table").html(code);
}

function checkEventSafe(url){
	httpApi.isItSafe_trend(url);
}

function showDetailEvent(obj, event){
	if(document.getElementById(event)){
		document.getElementById(event).remove();
		document.getElementById(event + '_arrow').className = 'arrow-field arrow-right';
	}
	else{		// show detail table
		var target = obj.parentNode.parentNode;
		var temp  = document.createElement('div');
		temp.id = event;
		temp.className = 'detail-table';
		var code = '';
		var data = dataObject[event];
		code += '<div style="border-bottom: 1px solid #C0C0C0;">Count: <span style="color:#FC0;">'+ data.source.length +'</span></div>';
		for(i=0; i<data.time.length; i++){
			code += '<div style="display:flex;padding:2px 0;">';
			code += '<div style="width: 150px;">'+ data.time[i] +'</div>';
			code += '<div style="width: 150px;">'+ data.source[i] +'</div>';
			code += '<div>'+ data.destination[i] +'</div>';
			code += '</div>';
		}

		temp.innerHTML = code;
		target.appendChild(temp);
		document.getElementById(event + '_arrow').className = 'arrow-field arrow-down';
	}
}

function recount(){
	var t = new Date();
	var timestamp = t.getTime()

	if(document.form.wrs_mals_enable.value == "1"){												
		document.form.wrs_mals_t.value = timestamp.toString().substring(0, 10);
	}
	
	if(document.form.wrs_mals_enable.value == "1"){
		document.form.action_wait.value = "1";
		applyRule();
	}
}

function applyRule(){
	if(ctf_disable == 0 && ctf_fa_mode == 2){
		if(!confirm(Untranslated.ctf_fa_hint)){
			return false;
		}	
		else{
			document.form.action_script.value = "reboot";
			document.form.action_wait.value = "<% nvram_get("reboot_time"); %>";
		}	
	}

	showLoading();	
	document.form.submit();
}

function mals_check(active){
	if(active == "1"){
		$("#bar_shade").css("display", "none");
		$("#chart_shade").css("display", "none");
		$("#info_shade").css("display", "none");
	}
	else{
		$("#bar_shade").css("display", "");
		$("#chart_shade").css("display", "");
		$("#info_shade").css("display", "");
	}
}

function recountHover(flag){
	if(flag == 1){
		$("#vulner_delete_icon").css("background","url('images/New_ui/recount_hover.svg')");
	}
	else{
		$("#vulner_delete_icon").css("background","url('images/New_ui/recount.svg')");
	}
}

function eraseDatabase(){
	document.form.action_script.value = 'reset_mals_db';
	document.form.action_wait.value = "1";
	applyRule();
}

function showEraseConfirm(){
	$('#model_name').html(based_modelid)
	cal_panel_block("erase_confirm", 0.25);
	$('#erase_confirm').fadeIn(300);
}

function hideConfirm(){
	$('#erase_confirm').fadeOut(100);
}

function quickAddWhitelist(domain){
	showWhitelistField();
	$("#newDomain").val(domain);
}


function addWhitelist(){
	var _url = $("#newDomain").val();
	$('#domainErrMessage').hide();
	if(_url == ''){
		$('#domainErrMessage').html('The domain name can not be blank');
		$('#domainErrMessage').show();
		return false;
	}

	if(!validator.domainName_flag(_url) && !validator.ipv4_addr(_url)){
		$('#domainErrMessage').html('wrong domain name / IPv4 address format');
		$('#domainErrMessage').show();
		return false;
	}

	if(whitelist.data.length >= 64){
		$('#domainErrMessage').html('The whitelist is reaching maximum, the maximum size is 64');
		$('#domainErrMessage').show();
		return false;
	}

	for(i=0;i<whitelist.data.length;i++){
		if(_url == whitelist.data[i]){
			$('#domainErrMessage').html('The domain name is already in the whitelist');
			$('#domainErrMessage').show();
			return false;
		}
	}

	$.ajax({
		url: "/wrs_wbl.cgi?action=add&type=0&url_type=url&url=" + _url,
		type: "POST",
		success: function(response){
			whitelist.data.push(_url);
			genWhitelist(whitelist);
		}
	});

	$("#newDomain").val('');
}

function deleteWhitelist(domain){
	var _url = domain;
	$.ajax({
		url: "/wrs_wbl.cgi?action=del&type=0&url_type=url&url="+ _url,
		type: "POST",
		error: function(xhr) {
			deleteWhitelist(_url);
		},
		success: function(response){
			for(i=0;i< whitelist.data.length;i++){
				if(whitelist.data[i] == _url){
					whitelist.data.splice(i,1);
					i--;
				}
			}
			
			genWhitelist(whitelist);
		}
	});
}

var whitelist = new Object;
function getWhitelist(){
	$.ajax({
		url: "/wrs_wbl.cgi?action=get&type=0",
		type: "POST",
		error: function(xhr) {
			getWhitelist();
		},
		success: function(response){
			whitelist = JSON.parse(response);
			genWhitelist(whitelist);
		}
	}); 
}

function genWhitelist(list){
	var code = '';
	var _list = list.data;
	var _list_length = Object.keys(_list).length;
	for(var i=0; i< _list_length; i++){
		code += '<div style="display:flex;margin: 12px 6px;justify-content: space-between;align-items: center;padding:0 6px 6px 0;border-bottom: 1px solid #667881 ">';
		code += '<div style="font-size:14px;word-break:break-all">'+ _list[i] +'</div>';
		code += '<div class="icon-container icon-delete" onclick="deleteWhitelist(\''+ _list[i] +'\')"></div>';
		code += '</div>';
	}

	$('#whitelistTable').html(code);
	$('#list_count').html(_list_length);
}

function showWhitelistField(){
	getWhitelist();
	$('#whitelistField').show();
}

function hideWhitelistField(){
	$('#whitelistField').hide();
}

var download = function(content, fileName, mimeType) {
	var a = document.createElement('a');
	mimeType = mimeType || 'application/octet-stream';
	if (navigator.msSaveBlob) { // IE10
		return navigator.msSaveBlob(new Blob([content], { type: mimeType }), fileName);
	} 
	else if ('download' in a) { //html5 A[download]
		a.href = 'data:' + mimeType + ',' + encodeURIComponent(content);
		a.setAttribute('download', fileName);
		document.getElementById("save_icon").appendChild(a);
		setTimeout(function() {
			document.getElementById("save_icon").removeChild(a);
			a.click();
		}, 66);
		return true;
	} 
	else { //do iframe dataURL download (old ch+FF):
		var f = document.createElement('iframe');
		document.getElementById("save_icon").appendChild(f);
		f.src = 'data:' + mimeType + ',' + encodeURIComponent(content);
		setTimeout(function() {
			document.getElementById("save_icon").removeChild(f);
			}, 333);
		return true;
	}
};

function query(value){
	var code = '';
	if(value == ''){
		$('#query_list').hide();
	}
	else{
		var _array = Object.keys(dataObject);
		var _obj_list = new Array;
		for(i=0;i<_array.length;i++){
			var _name = dataObject[_array[i]].destination[0]
			if(_name.indexOf(value) != -1 && _obj_list[_name] == undefined){
				code += '<li onclick="quickAdd(\''+ _name +'\')">'+ _name +'</li>';
				_obj_list[_name] = '';
			}

		}
	}

	if(code != ''){
		$('#query_list').show();
	}

	$('#query_list').html(code);
}

function quickAdd(value){
	$('#newDomain').val(value);
	$('#query_list').hide();
	//genLogTable(value);
}
</script>
</head>
<body onload="initial();" onunload="unload_body();" class="bg">
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>
<div id="erase_confirm" class="confirm">
	<div style="margin: 16px 24px;font-size:24px;"><span id="model_name"></span> says</div>
	<div style="margin: 16px 24px;font-size:16px;">Are you sure want to permanently delete events.</div>
	<div style="display:flex;justify-content: flex-end;margin: 36px 24px;">
		<div class="confirm-button" onclick="hideConfirm();">Cancel</div>
		<div class="confirm-button" onclick="eraseDatabase();">OK</div>
	</div>
</div>
<div id="hiddenMask" class="popup_bg" style="z-index:999;">
	<table cellpadding="5" cellspacing="0" id="dr_sweet_advise" class="dr_sweet_advise" align="center"></table>
	<!--[if lte IE 6.5.]><script>alert("<#ALERT_TO_CHANGE_BROWSER#>");</script><![endif]-->
</div>
<iframe name="hidden_frame" id="hidden_frame" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="AiProtection_MaliciousSitesBlocking.asp">
<input type="hidden" name="next_page" value="AiProtection_MaliciousSitesBlocking.asp">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_wait" value="5">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="restart_wrs;restart_firewall">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>" disabled>
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="wrs_mals_enable" value="<% nvram_get("wrs_mals_enable"); %>">
<input type="hidden" name="wrs_mals_t" value="<% nvram_get("wrs_mals_t"); %>">
<input type="hidden" name="wrs_protect_enable" value="<% nvram_get("wrs_protect_enable"); %>">

<table class="content" align="center" cellpadding="0" cellspacing="0" >
	<tr>
		<td width="17">&nbsp;</td>		
		<td valign="top" width="202">				
			<div  id="mainMenu"></div>	
			<div  id="subMenu"></div>		
		</td>					
		<td valign="top">
			<div id="tabMenu" class="submenuBlock"></div>	
		<!--===================================Beginning of Main Content===========================================-->		
			<table width="98%" border="0" align="left" cellpadding="0" cellspacing="0" >
				<tr>
					<td valign="top" >		
						<table width="730px" border="0" cellpadding="4" cellspacing="0" class="FormTitle" id="FormTitle">
							<tbody>
							<tr>
								<td class="content_bg" valign="top">
									<div>&nbsp;</div>
									<div>
										<table width="730px">
											<tr>
												<td align="left">
													<span class="formfonttitle"><#AiProtection_title#> - <#AiProtection_sites_blocking#></span>
												</td>
											</tr>
										</table>
									</div>									
									<div style="margin:10px 0 10px 5px;" class="splitLine"></div>
									<div id="PC_desc">
										<table width="700px" style="margin-left:25px;">
											<tr>
												<td style="font-size:14px;">
													<div><#AiProtection_sites_block_desc#></div>
												</td>
											</tr>									
										</table>
									</div>

									<!--=====Beginning of Main Content=====-->
									<div style="margin-top:5px;">
										<div style="display:table;margin: 10px 15px">

											<div style="display:table-cell;width:370px;height:350px;">
												<div style="display:table-row">
													<div style="font-size:16px;margin:0 0 5px 5px;text-align:center"><#AiProtection_event#></div>
												</div>
												<div id="vulner_table" style="background-color:#444f53;width:350px;height:340px;border-radius: 10px;display:table-cell;position:relative;">
													<div id="bar_shade" style="position:absolute;width:330px;height:330px;background-color:#505050;opacity:0.6;margin:5px;display:none"></div>
													<div>
														<div style="display:table-cell;width:50px;padding: 10px;">
															<div style="width:35px;height:35px;background:url('images/New_ui/mals.svg');margin: 0 auto;"></div>

														</div>	
														<div style="display:table-cell;width:200px;padding: 10px;vertical-align:middle;text-align:center;">
															<div id="mals_count" style="margin: 0 auto;font-size:26px;font-weight:bold;color:#FC0"></div>
															<div id="mals_time" style="margin: 5px  auto 0;"></div>
														</div>	
														<div style="display:table-cell;width:50px;padding: 10px;">
															<div id="vulner_delete_icon" style="width:32px;height:32px;margin: 0 auto;cursor:pointer;background:url('images/New_ui/recount.svg');" onclick="recount();" onmouseover="recountHover('1')" onmouseout="recountHover('0')"></div>
														</div>	
													</div>
													<div style="height:240px;margin-top:0px;">
														<div style="text-align:center;font-size:16px;"><#AiProtection_TopClient#></div>
														<div id="vp_bar_table" style="height:235px;margin: 0 10px;border-radius:10px;overflow:auto"></div>
													</div>
												</div>
											</div>

											<div style="display:table-cell;width:370px;height:350px;padding-left:10px;">
												<div style="font-size:16px;margin:0 0 5px 5px;text-align:center;"><#AiProtection_activity#></div>

												<!-- Line Chart -Block-->
												<div style="background-color:#444f53;width:350px;height:340px;border-radius: 10px;display:table-cell;padding-left:10px;position:relative">
													<div id="chart_shade" style="position:absolute;width:350px;height:330px;background-color:#505050;opacity:0.6;margin:5px 0 5px -5px;display:none"></div>
													<div>
														<div style="display:inline-block;margin:5px 10px"><#AiProtection_scan_rHits#></div>		
													</div>			
													<div style="width:90%">
														<div>
															<canvas id="canvas"></canvas>
														</div>
													</div>	

												</div>

												<!-- End Line Chart Block -->

											</div>
										</div>


										<!--div style="margin: 10px auto;width:720px;height:500px;">
											<div id="googleMap" style="height:100%;">

											</div>
										</div-->
										<div style="margin: 0 24px;">
											<div style="display:flex;justify-content: space-between;align-content: center;">											
												<div style="text-align:center;font-size:16px;"><#AiProtection_eventdetails#></div>
												<div style="display: flex;">
													<div style="margin: 0 8px;"><div id="save_icon" class="icon-container icon-save" title="<#CTL_onlysave#>" onclick="download(csvContent, 'MaliciousSitesBlocking.csv', 'data:text/csv;charset=utf-8');"></div></div>
													<div style="margin: 0 8px;"><div id="delete_icon" class="icon-container icon-delete" onclick="showEraseConfirm();" title="<#CTL_del#>"></div></div>
													<div style="margin: 0 8px;"><div id="edit_icon" class="icon-container icon-edit" onclick="showWhitelistField();" title="Manage Whitelist"></div></div>
												</div>
											</div>
											<div id="whitelistField" style="position: absolute;width:600px;height:650px;background-color: rgba(47,62,68,1);margin:-350px 0 0 35px;z-index:30;padding: 32px 24px 24px 24px;display:none;">
												<div style="display:flex;justify-content: space-between;align-items: center;">
													<div style="font-size: 24px;">Whitelist</div>
													<div onclick="hideWhitelistField();"><img src="images/New_ui/icon_close.svg" alt="" style="width:32px;height:32px;cursor:pointer;"></div>
												</div>
												<div style="color: #CCCCCC;font-size: 16px;margin: 12px 0 24px 0;">Add a domain you trust and unlock it with AiProtection</div>
												<div>
													<div style="font-size: 14px;padding: 0 0 2px 4px;">Add domain to whitelist</div>
													<div>
														<input id="newDomain" style="font-size: 14px;" type="text" maxlength="32" size="22" class="input_32_table" autocomplete="off" autocorrect="off" autocapitalize="off">
														<ul id="query_list" class="query_list"></ul>						
													</div>
													<div id="domainErrMessage" style="color: #FC0;margin: 4px 0 2px 4px;"></div>
												</div>
												<div style="margin: 12px 0;text-align: right;">
													<input type="button" class="button_gen" value="<#CTL_add#>" onclick="addWhitelist();">
												</div>
												<div style="width:100%;" class="line_horizontal"></div>
												<div style="margin-top:24px;">
													<span style="font-size: 24px;padding-right:12px;">My whitelist</span>
													<span style="font-size: 14px;">Count: <b id="list_count">0</b> (<#List_limit#>&nbsp;64)</span>
												</div>
												<div id="whitelistTable" style="overflow: auto;height:420px;"></div>
											</div>
										</div>

										<div style="margin: 10px auto;width:720px;height:500px;background:#444f53;border-radius:10px;position:relative;overflow:auto">
											
											<div id="info_shade" style="position:absolute;width:710px;height:490px;background-color:#4b3b3b;opacity:0.6;margin:5px;display:none"></div>
											<div id="detail_info_table" style="padding: 10px 15px;">
												<div style="font-size:14px;font-weight:bold;border-bottom: 1px solid #797979">
													<div style="display:table-cell;width:180px;padding-right:5px;"><#diskUtility_time#></div>
													<div style="display:table-cell;width:80px;padding-right:5px;"><#AiProtection_event_Threat#></div>
													<div style="display:table-cell;width:170px;padding-right:5px;"><#AiProtection_event_Source#></div>
													<div style="display:table-cell;width:200px;padding-right:5px;"><#AiProtection_event_Destination#></div>
													<div style="display:table-cell;width:50px;padding-right:5px;"></div>
												</div>											
											</div>
										</div>
									</div>
									<div style="width:135px;height:55px;margin: 10px 0 0 600px;background-image:url('images/New_ui/tm_logo_power.png');"></div>
								</td>
							</tr>
							</tbody>	
						</table>
					</td>         
				</tr>
			</table>				
		<!--===================================Ending of Main Content===========================================-->		
		</td>		
		<td width="10" align="center" valign="top">&nbsp;</td>
	</tr>
</table>
<div id="footer"></div>
</form>
</body>
</html>
