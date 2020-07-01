<!DOCTYPE html>
<html>
<head>
	<meta charset="UTF-8">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<meta http-equiv="X-UA-Compatible" content="ie=edge">
	<title><#Web_Title#> - <#AiProtection_two-way_IPS#></title>
	<link rel="stylesheet" href="index_style.css"> 
	<link rel="stylesheet" href="form_style.css">
	<link rel="stylesheet" href="/css/adaptive_mobile.css">

	<script src="/js/httpApi.js"></script>
	<script src="/state.js"></script>
	<script src="/popup.js"></script>
	<script src="/general.js"></script>
	<script src="/help.js"></script>
	<script src="/js/jquery.js"></script>
	<script src="/disk_functions.js"></script>
	<script src="/form.js"></script>
	<script src="/client_function.js"></script>
	<script src="/js/Chart.js"></script>
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
</style>
<script>
var ctf_disable = '<% nvram_get("ctf_disable"); %>';
var ctf_fa_mode = '<% nvram_get("ctf_fa_mode"); %>';

function initial(){
	if(document.form.wrs_protect_enable.value == '1' && document.form.wrs_vp_enable.value == '1'){
		vulnerability_check('1');
	}
	else{
		vulnerability_check('0');
	}

	getIPSCount();
	getEventTime();
	getIPSData("vp", "mac");
	var t = new Date();
	var timestamp = t.getTime();
	var date = timestamp.toString().substring(0, 10);
	getIPSChart("vp", date);
	getIPSDetailData("vp", "all");
}

function getEventTime(timestamp){
	var time = document.form.wrs_vp_t.value*1000;
	if(timestamp){
		time = timestamp*1000;
	}

	var vp_date = transferTimeFormat(time);
	$("#ips_time").html(vp_date);
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
var ips_count = 0;
function getIPSCount(){
	$.ajax({
		url: '/getAiProtectionEvent.asp',
		dataType: 'script',	
		error: function(xhr) {
			setTimeout("getIPSCount();", 1000);
		},
		success: function(response){
			ips_count = event_count.vp_n;
			$("#ips_count").html(ips_count);
		}
	});
}

function getIPSData(type, event){
	$.ajax({
		url: '/getIPSEvent.asp?type=' + type + '&event=' + event,
		dataType: 'script',	
		error: function(xhr) {
			setTimeout("getIPSData('vp', event);", 1000);
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
		 url: '/getIPSChart.asp?type=' + type + '&date='+ date,
		dataType: 'script',	
		error: function(xhr) {
			setTimeout("getIPSChart('vp', date);", 1000);
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
			strokeColor: "#ED1C24",
			pointColor: "#ED1C24",
			pointHighlightFill: "#FFF",
			pointHighlightStroke: "#ED1C24",
			data: high_array
		}, {
			fillColor: "rgba(255,255,255,0)",
			strokeColor: "#FFE500",
			pointColor: "#FFE500",
			pointHighlightFill: "#FFF",
			pointHighlightStroke: "#FFE500",
			data: medium_array
		},
		{
			fillColor: "rgba(255,255,255,0)",
			strokeColor: "#59CA5E",
			pointColor: "#59CA5E",
			pointHighlightFill: "#FFF",
			pointHighlightStroke: "#59CA5E",
			data: low_array
		}]
	}


	var ctx = document.getElementById("canvas").getContext("2d");
	window.myLine = new Chart(ctx).Line(lineChartData, {
		responsive: true,
		maintainAspectRatio: false,
	});
}

var dataObject = new Object;
var csvContent = 'Time,Level,Type,Source,Destination,Threat';
function getIPSDetailData(type, event){
	$.ajax({
		url: '/getIPSDetailEvent.asp?type=' + type + '&event=' + event,
		dataType: 'script',	
		error: function(xhr) {
			setTimeout(function(){getIPSDetailData('vp', event);}, 1000);
		},
		success: function(response){
			dataObject = {};
			var data_array = JSON.parse(data);
			if(data_array != ""){
				
				for(i=0; i<data_array.length; i++){
					csvContent += '\n';
					csvContent += data_array[i][0] + ',' + data_array[i][1] + ',' + direct_type[data_array[i][5]] + ',' + data_array[i][2] + ',' + data_array[i][3] + ',' + data_array[i][4];

					var _index = data_array[i][4].split(' ')[0] + '_' + data_array[i][3];
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
							level: '',
							attackType: '',
							title: '',
							catId: ''
						}

						dataObject[_index].source.push(data_array[i][2]);
						dataObject[_index].destination.push(data_array[i][3]);
						dataObject[_index].time.push(data_array[i][0]);
						dataObject[_index].level = data_array[i][1];
						dataObject[_index].attackType = data_array[i][5];
						dataObject[_index].title = data_array[i][4];
						dataObject[_index].catId = (data_array[i][7]) ? data_array[i][7] : "0";
					}

				}
				
				var _array = new Array;
				for(i=0; i<Object.keys(dataObject).length; i++){
					var eventID = Object.keys(dataObject)[i];
					var time = dataObject[eventID].time[0];
					time = time.split(' ');
					time = time[0].split('-').concat(time[1].split(':'));
					var t = new Date(time[0], parseInt(time[1])-1, time[2], time[3], time[4], time[5]).getTime();
					_array[i] = [
						t, 
						dataObject[eventID].time[0], 
						dataObject[eventID].level, 
						dataObject[eventID].source[0], 
						dataObject[eventID].destination[0], 
						dataObject[eventID].title, 
						dataObject[eventID].attackType, 
						eventID, 
						dataObject[eventID].time.length, 
						dataObject[eventID].catId
					];
				}
				
				_array.sort(function(a, b){
					return b[0] - a[0];
				});
			}
			else{
				dataObject = {};
			}

			generateDetailTable(dataObject);
		}
	});
}

var direct_type = ["", "Device Infected", "External Attacks"];
function generateDetailTable(dataObj){
	var code = '';
	code += '<div class="table-header">';
	code += '<div class="table-cell"><#diskUtility_time#></div>';
	code += '<div class="table-cell"><#AiProtection_level_th#></div>';
	code += '<div class="table-cell">Type</div>';
	code += '<div class="table-cell"><#AiProtection_event_Source#></div>';
	code += '<div class="table-cell"><#AiProtection_event_Destination#></div>';
	code += '<div class="table-cell"><#AiProtection_event_Threat#></div>';
	code += '</div>';

	if(Object.entries(dataObj) != ''){
		$('#deleteData').show();
		for(i=0; i<Object.keys(dataObj).length; i++){
			var eventID = Object.keys(dataObj)[i];
			code += '<div class="table-row">';
			code += '<div class="table-cell time-stamp">';
			code += '<div class="flexbox flex-a-center">';
			code += dataObj[eventID].time[0];
			code += '<div id="'+ eventID +'_arrow" class="arrow-field arrow-right" onclick="showDetailEvent(this, \''+ eventID +'\');"></div>';
			code += '</div></div>';
			code += '<div class="table-cell" data-title="<#AiProtection_level_th#>">'+ dataObj[eventID].level +'</div>';
			code += '<div class="table-cell" data-title="Type">'+ direct_type[dataObj[eventID].attackType] +'</div>';
			code += '<div class="table-cell" data-title="<#AiProtection_event_Source#>">'+ dataObj[eventID].source[0] +'</div>';
			code += '<div class="table-cell" data-title="<#AiProtection_event_Destination#>">'+ dataObj[eventID].destination[0] +'</div>';
			code += '<div class="table-cell" data-title="<#AiProtection_event_Threat#>">'+ dataObj[eventID].title +'</div>';
			code += '</div>';
		}
	}
	else{
		$('#deleteData').hide();
		code += '<div class="no-data"><#IPConnection_VSList_Norule#></div>';
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

function threatQuery(id, keyword){
/*
	var keywordMappingTable = {
		"0": "",
		"1": "Misc",
		"2": "Web Attack",
		"4": "Buffer Overflow",
		"8": "Backdoor/Trojan",
		"10": "Access Control",
		"80": "Virus/Worm",
		"100": "Botnet",
		"200": "DoS/DDoS",
		"400": "Scan",
		"800": "File Transfer"
	}
*/
	var url = 'http://nw-dlcdnet.asus.com/trend/' + id + "?q=" + keyword/*8keywordMappingTable[catId]*/;
	window.open(url, '_blank');
}

function recount(obj){
	var timestamp = new Date().getTime();

	if(document.form.wrs_vp_enable.value == "1"){	
		httpApi.nvramSet({
			'wrs_vp_t': timestamp.toString().substring(0, 10),
			'action_mode': 'apply',
			'rc_service': 'restart_wrs;restart_firewall'
		},
		function(){
			setTimeout(function(){
				getIPSCount();
				getEventTime(timestamp.toString().substring(0, 10));
				getIPSData("vp", "mac");
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

	showLoading();	
	document.form.submit();
}

function vulnerability_check(active){
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
		'vp_mals_t': timestamp.toString().substring(0, 10), 
		'action_mode': 'apply',
		'rc_service': 'reset_vp_db'
	},
	function(){
		setTimeout(function(){
			var t = new Date();
			var timestamp = t.getTime();
			var date = timestamp.toString().substring(0, 10);
			getIPSCount();
			getEventTime(date);
			getIPSData("vp", "mac");
			getIPSChart("vp", date);
			getIPSDetailData("vp", "all");
			hideConfirm();
		}, 1000);	
	});
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
<div id="Loading" class="popup_bg"></div>
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
<input type="hidden" name="current_page" value="AiProtection_IntrusionPreventionSystem_m.asp">
<input type="hidden" name="next_page" value="AiProtection_IntrusionPreventionSystem_m.asp">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_wait" value="5">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="restart_wrs;restart_firewall">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>" disabled>
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="wrs_mals_enable" value="<% nvram_get("wrs_mals_enable"); %>">
<input type="hidden" name="wrs_cc_enable" value="<% nvram_get("wrs_cc_enable"); %>">
<input type="hidden" name="wrs_vp_enable" value="<% nvram_get("wrs_vp_enable"); %>">
<input type="hidden" name="wan0_upnp_enable" value="<% nvram_get("wan0_upnp_enable"); %>" disabled>
<input type="hidden" name="wan1_upnp_enable" value="<% nvram_get("wan1_upnp_enable"); %>" disabled>
<input type="hidden" name="misc_http_x" value="<% nvram_get("misc_http_x"); %>" disabled>
<input type="hidden" name="misc_ping_x" value="<% nvram_get("misc_ping_x"); %>" disabled>
<input type="hidden" name="dmz_ip" value="<% nvram_get("dmz_ip"); %>" disabled>
<input type="hidden" name="autofw_enable_x" value="<% nvram_get("autofw_enable_x"); %>" disabled>
<input type="hidden" name="vts_enable_x" value="<% nvram_get("vts_enable_x"); %>" disabled>
<input type="hidden" name="wps_enable" value="<% nvram_get("wps_enable"); %>" disabled>
<input type="hidden" name="wps_sta_pin" value="<% nvram_get("wps_sta_pin"); %>" disabled>
<input type="hidden" name="TM_EULA" value="<% nvram_get("TM_EULA"); %>">
<input type="hidden" name="PM_SMTP_SERVER" value="<% nvram_get("PM_SMTP_SERVER"); %>">
<input type="hidden" name="PM_SMTP_PORT" value="<% nvram_get("PM_SMTP_PORT"); %>">
<input type="hidden" name="PM_MY_EMAIL" value="<% nvram_get("PM_MY_EMAIL"); %>">
<input type="hidden" name="PM_SMTP_AUTH_USER" value="<% nvram_get("PM_SMTP_AUTH_USER"); %>">
<input type="hidden" name="PM_SMTP_AUTH_PASS" value="">
<input type="hidden" name="wrs_mail_bit" value="<% nvram_get("wrs_mail_bit"); %>">
<input type="hidden" name="st_ftp_force_mode" value="<% nvram_get("st_ftp_force_mode"); %>" disabled>
<input type="hidden" name="st_ftp_mode" value="<% nvram_get("st_ftp_mode"); %>" disabled>
<input type="hidden" name="st_samba_force_mode" value="<% nvram_get("st_samba_force_mode"); %>" disabled>
<input type="hidden" name="st_samba_mode" value="<% nvram_get("st_samba_mode"); %>" disabled>
<input type="hidden" name="wrs_vp_t" value="<% nvram_get("wrs_vp_t"); %>">
<input type="hidden" name="wrs_protect_enable" value="<% nvram_get("wrs_protect_enable"); %>">
<div>
	<div class="card-bg">
		<div class="page-title"><#AiProtection_two-way_IPS#></div>
		<div class="split-Line"></div>
		<div class="description-bar">
			<div class="description-title"><#Description#></div>
			<div class="description"><#AiProtection_two-way_IPS_desc#></div>
		</div>
		<div class="tm-logo">
			<img src="images/New_ui/TrendMirco_logo.svg" alt="" class="tm-logo-size">
		</div>
	</div>

	<div class="card-bg category">
		<div class="category-block">	
			<div class="flexbox flex-a-center flex-j-spaceB">
				<div class="page-title"><#AiProtection_event#></div>
				<img id="recountIcon" src="images/New_ui/recount.svg" alt="" class="icon-size24" onclick="recount(this);" style="display:none;">
			</div>
		
			<div class="flexbox flex-a-center card-lv1 card-lv1-bg" >
				<div class="event-title">Since</div>
				<div class="event-time" id="ips_time"></div>
				<div class="event-count" id="ips_count"></div>
				<div class="event-hit"><#AiProtection_scan_rHits#></div>
			</div>

			<div class="card-lv1 card-lv1-bg">
				<div class="flexbox flex-a-center flex-j-spaceB">
					<div><#AiProtection_TopClient#></div>
					<div><img src="images/New_ui/mals.svg" alt="" class="icon-size24"></div>
				</div>
				<div id="vp_bar_table"></div>
			</div>
		</div>


		<!-- Threat Activities -->
		<div class="category-block">		
			<div class="flexbox flex-a-center category-title">
				<div class="page-title"><#AiProtection_level#></div>
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
