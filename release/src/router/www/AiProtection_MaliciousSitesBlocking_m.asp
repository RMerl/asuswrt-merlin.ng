<!DOCTYPE html>
<html>
<head>
	<meta charset="UTF-8">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<meta http-equiv="X-UA-Compatible" content="ie=edge">
	<title><#Web_Title#> - <#AiProtection_sites_blocking#></title>
	<link rel="stylesheet" href="index_style.css"> 
	<link rel="stylesheet" href="form_style.css">
	<link rel="stylesheet" href="/css/adaptive_mobile.css">

	<script src="/js/jquery.js"></script>
	<script src="/state.js"></script>
	<script src="/popup.js"></script>
	<script src="/general.js"></script>
	<script src="/help.js"></script>
	<script src="/disk_functions.js"></script>
	<script src="/form.js"></script>
	<script src="/client_function.js"></script>
	<script src="/js/Chart.js"></script>
	<script src="/js/httpApi.js"></script>
	<script src="/validator.js"></script>

<style>

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
var ctf_disable = '<% nvram_get("ctf_disable"); %>';
var ctf_fa_mode = '<% nvram_get("ctf_fa_mode"); %>';

function initial(){
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
}

function getEventTime(timestamp){
	var time = document.form.wrs_mals_t.value*1000;
	if(timestamp){
		time = timestamp*1000;
	}

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

	var date_format = year + "/" + month + "/" + date + " " + hour + ":" + minute;
	return date_format;
}

var mals_count = 0;
function getMalsCount(){
	$.ajax({
		url: '/getAiProtectionEvent.asp',
		dataType: 'script',
		crossDomain: true,
		error: function(xhr) {
			setTimeout("getMalsCount();", 1000);
		},
		success: function(response){
			mals_count = event_count.mals_n;
			$("#mals_count").html(mals_count);
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
	info_bar = [];
	for(i=0;i<data.length;i++){
		var mac = data[i][0];
		var ip = ""
		var hit = data[i][1];
		var name = mac;
		if(clientList[mac]){
			name = (clientList[mac].nickName == "") ? clientList[mac].name : clientList[mac].nickName;
			ip = clientList[mac].ip;
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
	var dataLength = (info_bar.length <= 5) ? info_bar.length : 5;
	
	if(info_bar == ''){
		$('#recountIcon').hide();
		code += '<div class="no-data"><#AiProtection_eventnodetected#></div>';
	}
	else{
		$('#recountIcon').show();
		for(i=0;i<dataLength;i++){
			var targetObj = info_bar[info_bar[i]];
			if(hit_count_all == 0){
				var percent = 0;
			}
			else{
				var percent = parseInt((targetObj.hit/hit_count_all)*100);
				if(percent == 0)
					percent = 0.5;
			}

			code += '<div class="list-margin">';
			code += '<div>'+ targetObj.name +'</div>';
			code += '<div class="flexbox flex-a-center">';
			code += '<div class="bar-container"><div class="bar-cal" style="width:'+ percent +'%;"></div></div>';
			code += '<div class="list-count">'+ targetObj.hit +'</div>';
			code += '</div></div>';
		}
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
	drawLineChart(date_label, hight_array);
}

function drawLineChart(date_label, high_array){
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
		responsive: true,
		maintainAspectRatio: false,
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
			var data_array = JSON.parse(data);
			if(data_array != ""){
				
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
			}
			else{
				dataObject = {};
			}

			generateDetailTable(dataObject);
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

	if(Object.entries(dataObj) != ''){
		$('#deleteData').show();
		code += '<div class="table-header">';
		code += '<div class="table-cell"><#diskUtility_time#></div>';
		code += '<div class="table-cell"><#AiProtection_event_Threat#></div>';
		code += '<div class="table-cell"><#AiProtection_event_Source#></div>';
		code += '<div class="table-cell"><#AiProtection_event_Destination#></div>';
		code += '</div>';
		for(i=0; i<Object.keys(dataObj).length; i++){
			var eventID = Object.keys(dataObj)[i];
			var cat_id_index = "_" + dataObj[eventID].threat;
			code += '<div class="table-row">';
			code += '<div class="table-cell time-stamp">';
			code += '<div class="flexbox flex-a-center">';
			code += dataObj[eventID].time[0];
			code += '<div id="'+ eventID +'_arrow" class="arrow-field arrow-right" onclick="showDetailEvent(this, \''+ eventID +'\');"></div>';
			code += '</div></div>'; 
			code += '<div class="table-cell" data-title="<#AiProtection_event_Threat#>">'+ cat_id_array[cat_id_index].description +'</div>';
			var _name = dataObj[eventID].source[0];
			if(clientList[_name]){
				_name = clientList[_name].name;
			}
			code += '<div class="table-cell" data-title="<#AiProtection_event_Source#>">'+ _name +'</div>';
			code += '<div class="table-cell" data-title="<#AiProtection_event_Destination#>">'+ dataObj[eventID].destination[0] +'</div>';
			code += '</div>';
		}
	}
	else{
		$('#deleteData').hide();
		code += '<div class="no-data"><#AiProtection_eventnodetected#></div>';
	}
	
	$("#detail_info_table").html(code);
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
		code += '<div class="count-field">Count: <span class="count-hit">'+ data.source.length +'</span></div>';
		for(i=0; i<data.time.length; i++){
			code += '<div class="detail-content" data-title="Time">'+ data.time[i] +'</div>';
		}

		temp.innerHTML = code;
		target.appendChild(temp);
		document.getElementById(event + '_arrow').className = 'arrow-field arrow-down';
	}
}

function recount(){
	var timestamp = new Date().getTime();
	
	if(document.form.wrs_mals_enable.value == "1"){
		httpApi.nvramSet({
			'wrs_mals_t': timestamp.toString().substring(0, 10),
			'action_mode': 'apply',
			'rc_service': 'restart_wrs;restart_firewall'
		},
		function(){
			setTimeout(function(){
				getMalsCount();
				getEventTime(timestamp.toString().substring(0, 10));
				getIPSData("mals", "mac");
				hideConfirm();
			}, 1000);	
		});
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

function eraseDatabase(){
	var timestamp = new Date().getTime();
	
	httpApi.nvramSet({
		'wrs_mals_t': timestamp.toString().substring(0, 10), 
		'action_mode': 'apply',
		'rc_service': 'reset_mals_db'
	},
	function(){
		setTimeout(function(){
			var t = new Date();
			var timestamp = t.getTime();
			var date = timestamp.toString().substring(0, 10);
			getMalsCount();
			getEventTime(date);
			getIPSData("mals", "mac");
			getIPSChart("mals", date);
			getIPSDetailData("mals", "all");
			hideConfirm();
		}, 1000);	
	});
}

function quickAddWhitelist(domain){
	showWhitelistField();
	$("#newDomain").val(domain);
}

function addWhitelist(){
	var _url = $("#newDomain").val();
	$('#domainErrMessage').hide();
	if(_url == ''){
		$('#domainErrMessage').html('<#AiProtection_ErrMsg_blank#>');
		$('#domainErrMessage').show();
		return false;
	}

	if(!validator.domainName_flag(_url) && !validator.ipv4_addr(_url)){
		$('#domainErrMessage').html('<#AiProtection_ErrMsg_wrong_format#>');
		$('#domainErrMessage').show();
		return false;
	}

	if(whitelist.data.length >= 64){
		$('#domainErrMessage').html('<#AiProtection_ErrMsg_full#>');
		$('#domainErrMessage').show();
		return false;
	}

	for(i=0;i<whitelist.data.length;i++){
		if(_url == whitelist.data[i]){
			$('#domainErrMessage').html('<#AiProtection_ErrMsg_duplicate#>');
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

function showConfirm(_id){
	var _paddingTop = window.scrollY + 60;
	$('#confirm_field').css('top', _paddingTop + 'px');
	$('#confirm_field').show();
	if(_id == 'recountIcon'){
		$('#confirmed').click(function(){
			recount();
			
		});
	}
	else if(_id == 'deleteData'){
		$('#confirmed').click(function(){
			eraseDatabase();
		});
	}
}
function hideConfirm(){
	$('#confirm_field').hide();
	$('#confirmed').unbind('click');
}
</script>
</head>
<body onload="initial();" onunload="unload_body();" class="bg">
<div id="TopBanner"></div>
<div id="hiddenMask" class="popup_bg" style="z-index:999;">
	<table cellpadding="5" cellspacing="0" id="dr_sweet_advise" class="dr_sweet_advise" align="center"></table>
	<!--[if lte IE 6.5.]><script>alert("<#ALERT_TO_CHANGE_BROWSER#>");</script><![endif]-->
</div>
<div id="confirm_field" class="card-lv1-bg confirm-block" style="display:none;">
	<div class="confirm-title">Erase Database</div>
	<div class="confirm-content">Are you sure that you want to erase database?</div>
	<div class="confirm-control-block">
		<div class="confirm-control-panel" onclick="hideConfirm()">CANCEL</div>
		<div id="confirmed" class="confirm-control-panel" onclick="">OK</div>
	</div>
</div>
<iframe name="hidden_frame" id="hidden_frame" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="AiProtection_MaliciousSitesBlocking_m.asp">
<input type="hidden" name="next_page" value="AiProtection_MaliciousSitesBlocking_m.asp">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_wait" value="5">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="restart_wrs;restart_firewall">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>" disabled>
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="wrs_mals_enable" value="<% nvram_get("wrs_mals_enable"); %>">
<input type="hidden" name="wrs_mals_t" value="<% nvram_get("wrs_mals_t"); %>">
<input type="hidden" name="wrs_protect_enable" value="<% nvram_get("wrs_protect_enable"); %>">
<div>
	<div class="card-bg">
		<div class="page-title"><#AiProtection_sites_blocking#></div>
		<div class="split-Line"></div>
		<div class="description-bar">
			<div class="description-title"><#Description#></div>
			<div class="description"><#AiProtection_sites_block_desc#></div>
		</div>
		<div class="tm-logo">
			<img src="images/New_ui/TrendMirco_logo.svg" alt="" class="tm-logo-size">
		</div>
	</div>

	<div class="card-bg category">
		<div class="category-block">	
			<div class="flexbox flex-a-center flex-j-spaceB">
				<div class="page-title"><#AiProtection_event#></div>
				<img id="recountIcon" src="images/New_ui/recount.svg" alt="" class="icon-size24" onclick="showConfirm(this.id);" style="display:none;">
			</div>
		
			<div class="flexbox flex-a-center card-lv1 card-lv1-bg" >
				<div class="event-title">Since</div>
				<div class="event-time" id="mals_time"></div>
				<div class="event-count" id="mals_count"></div>
				<div class="event-hit"><#AiProtection_scan_rHits#></div>
			</div>

			<div class="card-lv1 card-lv1-bg">
				<div class="flexbox flex-a-center flex-j-spaceB">
					<div class="description"><#AiProtection_TopClient#></div>
					<div><img src="images/New_ui/mals.svg" alt="" class="icon-size24"></div>
				</div>

				<div id="vp_bar_table"></div>
				<!-- <div class="list-margin">
					<div>Jieming</div>
					<div class="flexbox flex-a-center">
						<div class="bar-container">
							<div class="bar-cal"></div>
						</div>
						<div class="list-count">999999</div>
					</div>
				</div> -->
			</div>
		</div>


		<!-- Threat Activities -->
		<div class="category-block">		
			<div class="flexbox flex-a-center category-title">
				<div class="page-title canvas-title"><#AiProtection_activity#></div>
			</div>
			<div class="canvas-bg">
				<div class="chart-title"><#AiProtection_scan_rHits#></div>
				<div class="canvas-chart">
					<canvas id="canvas"></canvas>
				</div>
			</div>
		</div>		
	</div>

	<div class="card-bg">
		<div class="flexbox flex-a-center flex-j-spaceB flex-wrap">
			<div class="page-title"><#AiProtection_eventdetails#></div>
			<div>
				<!-- <img src="images/save.svg" alt="" style="width:24px;height:24px;"> -->
				<img id="deleteData" src="images/delete.svg" alt="" class="icon-size24" style="display:none" onclick="showConfirm(this.id);">
				<!-- <img src="images/edit.svg" alt="" style="width:24px;height:24px;"> -->
			</div>
		</div>
		
		<div>
			<div class="table" id="detail_info_table"></div>
		</div>
	</div>

</div>
<div id="footer"></div>
</form>
</body>
</html>
