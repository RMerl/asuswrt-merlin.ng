<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="X-UA-Compatible" content="IE=Edge"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title></title>
<link href="/NM_style.css" rel="stylesheet" type="text/css" />
<link href="/form_style.css" rel="stylesheet" type="text/css" />
<link href="/js/table/table.css" rel="stylesheet" type="text/css" >
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/js/table/table.js"></script>
<style type="text/css">
.title{
	font-size:16px;
	text-align:center;
	font-weight:bolder;
	margin-bottom:5px;
}

.ram_table{
	height:30px;
	text-align:center;
}

.ram_table td{
	width:33%;
}

.loading_bar{
	width:150px;
}

.loading_bar > div{
	margin-left:5px;
	background-color: #333333;
	border-radius:15px;
	padding:2px;
}

.status_bar{
	height:12px;
	border-radius:15px;
}

#ram_bar{
	background-color:#0096FF;
}

#tx_bar{
	background-color:#3CF;
}

#rx_bar{
	background-color:#118811;
}

#cpu0_bar{
	background-color:#FF9000;	
}

#cpu1_bar{
	background-color:#3CF;
}

#cpu2_bar{
	background-color:#FC0;
}

#cpu3_bar{
	background-color:#FF44FF;
}

.percentage_bar{
	width:60px;
	text-align:center;
}

.cpu_div{
	margin-top:-5px;
}

.status_bar{
  -webkit-transition: all 0.5s ease-in-out;
  -moz-transition: all 0.5s ease-in-out;
  -o-transition: all 0.5s ease-in-out;
  transition: all 0.5s ease-in-out;

}
.tableApi_table th {
	height: 20px;
}
.data_tr {
	height: 30px;
}
</style>
<script>
if(parent.location.pathname.search("index") === -1) top.location.href = "../"+'<% networkmap_page(); %>';

/*Initialize array*/
var cpu_info_old = new Array();
var core_num = '<%cpu_core_num();%>';
var cpu_usage_array = new Array();
var array_size = 46;
for(i=0;i<core_num;i++){
	cpu_info_old[i] = {
		total:0,
		usage:0
	}
	
	cpu_usage_array[i] = new Array();
	for(j=0;j<array_size;j++){
		cpu_usage_array[i][j] = 101;
	}
}
var ram_usage_array = new Array(array_size);
for(i=0;i<array_size;i++){
	ram_usage_array[i] = 101;
}
/*End*/

var last_rx = 0;
var last_tx = 0;
var max_rx = 100 * 1024;
var max_tx = 100 * 1024;
var current_rx = 0;
var current_tx = 0;
var qos_enable = "<% nvram_get("qos_enable"); %>";
var qos_ibw = "<% nvram_get("qos_ibw"); %>";
var qos_obw = "<% nvram_get("qos_obw"); %>";

function initial(){
	generate_cpu_field();
	if((parent.sw_mode == 2 || parent.sw_mode == 4) && '<% nvram_get("wlc_band"); %>' == '<% nvram_get("wl_unit"); %>')
		document.form.wl_subunit.value = 1;
	else
		document.form.wl_subunit.value = -1;

	if(parent.lyra_hide_support){
		document.getElementById("t0").style.display = "";
		document.getElementById("span0").innerHTML = "<#tm_wireless#>";
	}
	else{
		if(parent.band5g_support){
			document.getElementById("t0").style.display = "";
			document.getElementById("t1").style.display = "";

			if(parent.wl_info.band5g_2_support)
				tab_reset(0);

			if(parent.wl_info.band60g_support)
				tab_reset(0);

			if(parent.smart_connect_support && (parent.isSwMode("rt") || parent.isSwMode("ap")))
				change_smart_connect('<% nvram_get("smart_connect_x"); %>');
			
			// disallow to use the other band as a wireless AP
			if(parent.sw_mode == 4 && !localAP_support){
				for(var x=0; x<parent.wl_info.wl_if_total;x++){
					if(x != '<% nvram_get("wlc_band"); %>')
						document.getElementById('t'+parseInt(x)).style.display = 'none';
				}
			}
		}
		else{
			document.getElementById("t0").style.display = "";
		}
	}

	if(parent.wlc_express == '1' && parent.sw_mode == '2'){
		document.getElementById("t0").style.display = "none";
	}
	else if(parent.wlc_express == '2' && parent.sw_mode == '2'){
		document.getElementById("t1").style.display = "none";
	}

	detect_CPU_RAM();
	get_ethernet_ports();

	var table_height = document.getElementById("rt_table").clientHeight;
	if(table_height != "0" || table_height != "")
		set_NM_height(table_height);
	else {
		document.body.style.overflow = "hidden";
		var errorCount = 0;
		var readyStateCheckInterval = setInterval(function() {
			table_height = document.getElementById("rt_table").clientHeight;
			if (table_height != "0" || table_height != "") {
				clearInterval(readyStateCheckInterval);
				set_NM_height(table_height);
			}
			else {
				if(errorCount > 5) {
					clearInterval(readyStateCheckInterval);
					table_height = parent.document.getElementById("NM_table").style.height;
					set_NM_height(table_height);
				}
				errorCount++;
			}
		}, 10);
	}

	if (qos_enable > 0 && qos_ibw > 0 && qos_obw > 0) {
		max_rx = qos_ibw * 1024 / 8;
		max_tx = qos_obw * 1024 / 8;
	}
	update_traffic();
}

function tabclickhandler(wl_unit){
	if(wl_unit == "status"){
		location.href = "router_status.asp";
	}
/*	else if (wl_unit == "compatibility") {
		location.href = "compatibility.asp";
	}*/
	else{
		if((parent.sw_mode == 2 || parent.sw_mode == 4) && '<% nvram_get("wlc_band"); %>' == wl_unit)
			document.form.wl_subunit.value = 1;
		else
			document.form.wl_subunit.value = -1;

		if(parent.wlc_express != '0' && parent.wlc_express != '')
			document.form.wl_subunit.value = 1;

		document.form.wl_unit.value = wl_unit;
		document.form.current_page.value = "device-map/router.asp?time=" + Math.round(new Date().getTime()/1000);
		FormActions("/apply.cgi", "change_wl_unit", "", "");
		document.form.target = "hidden_frame";
		document.form.submit();
	}
}

function render_RAM(total, free, used){
	var used_percentage = total_MB = free_MB = used_MB = 0;
	total_MB = Math.round(total/1024);
	free_MB = Math.round(free/1024);
	used_MB = Math.round(used/1024);
	
	$("#ram_total_info").html(total_MB + "MB");
	$("#ram_free_info").html(free_MB + "MB");
	$("#ram_used_info").html(used_MB + "MB");

	used_percentage = Math.round((used/total)*100);
	$("#ram_bar").css("width", used_percentage +"%");
	$("#ram_quantification").html(used_percentage +"%");
}

function render_CPU(cpu_info_new){
	var pt = "";
	var percentage = total_diff = usage_diff = 0;
	var length = Object.keys(cpu_info_new).length;

	for(i=0;i<length;i++){
		pt = "";
		total_diff = (cpu_info_old[i].total == 0)? 0 : (cpu_info_new["cpu"+i].total - cpu_info_old[i].total);
		usage_diff = (cpu_info_old[i].usage == 0)? 0 : (cpu_info_new["cpu"+i].usage - cpu_info_old[i].usage);
		
		if(total_diff == 0)
			percentage = 0;
		else	
			percentage = parseInt(100*usage_diff/total_diff);
	
		$("#cpu"+i+"_bar").css("width", percentage +"%");
		$("#cpu"+i+"_quantification").html(percentage +"%");
		cpu_usage_array[i].push(100 - percentage);
		cpu_usage_array[i].splice(0,1);
		for(j=0;j<array_size;j++){
			pt += j*6 +","+ cpu_usage_array[i][j] + " ";	
		}

		document.getElementById('cpu'+i+'_graph').setAttribute('points', pt);
		cpu_info_old[i].total = cpu_info_new["cpu"+i].total;
		cpu_info_old[i].usage = cpu_info_new["cpu"+i].usage;
	}
}

function detect_CPU_RAM(){
	if(parent.isIE8){
		require(['/require/modules/makeRequest.js'], function(makeRequest){
			makeRequest.start('/cpu_ram_status.asp', function(xhr){				
				render_CPU(cpuInfo);
				render_RAM(memInfo.total, memInfo.free, memInfo.used);
				setTimeout("detect_CPU_RAM();", 2000);
			}, function(){});
		});
	}
	else{
		$.ajax({
	    	url: '/cpu_ram_status.asp',
	    	dataType: 'script',
	    	error: detect_CPU_RAM,
	    	success: function(data){			
				render_CPU(cpuInfo);
				render_RAM(memInfo.total, memInfo.free, memInfo.used);
				setTimeout("detect_CPU_RAM();", 2000);
			}
		});
	}
}

function tab_reset(v){
	var tab_array1 = document.getElementsByClassName("tab_NW");
	var tab_array2 = document.getElementsByClassName("tabclick_NW");
	var tab_width = Math.floor(270/(parent.wl_info.wl_if_total+1));

/*	if (Bcmwifi_support && band5g_11ax_support) {
		tab_width = "60";
	}*/

	var i = 0;
	while(i < tab_array1.length){
		/*if (tab_array1[i].id == "t_compatibility") {
			tab_array1[i].style.width = '93px';
		}
		else {*/
			tab_array1[i].style.width = tab_width + 'px';
		//}

		tab_array1[i].style.display = "";
		i++;
	}
	
	if(typeof tab_array2[0] != "undefined"){
		tab_array2[0].style.width=tab_width+'px';
		tab_array2[0].style.display = "";
	}
	
	if(v == 0){
		document.getElementById("span0").innerHTML = "2.4GHz";
		if(parent.wl_info.band5g_2_support){
			document.getElementById("span1").innerHTML = "5GHz-1";
			document.getElementById("span2").innerHTML = "5GHz-2";
		}else{
			document.getElementById("span1").innerHTML = "5GHz";
			document.getElementById("t2").style.display = "none";
		}

		if(!parent.wl_info.band60g_support){
			document.getElementById("t3").style.display = "none";
		}		
	}else if(v == 1){	//Smart Connect
		if(isSupport("triband") && dwb_info.mode) {
			document.getElementById("span0").innerHTML = "2.4GHz and 5GHz-1";
			document.getElementById("span2").innerHTML = "5GHz-2";
			document.getElementById("t3").style.display = "none";
			document.getElementById("t1").style.display = "none";
			document.getElementById("t0").style.width = "142px";
			document.getElementById("span0").style.padding = "5px 0px 0px 8px";
		}
		else {
			if(based_modelid == "RT-AC5300" || based_modelid == "RT-AC3200" || based_modelid == "GT-AC5300" || based_modelid == "GT-AX11000" || based_modelid == "RT-AX92U" || based_modelid == "RT-AX95Q")
				document.getElementById("span0").innerHTML = "2.4GHz, 5GHz-1 and 5GHz-2";
			else if(based_modelid == "RT-AC88U" || based_modelid == "RT-AX88U" || based_modelid == "RT-AC86U" || based_modelid == "AC2900" || based_modelid == "RT-AC3100" || based_modelid == "BLUECAVE" || based_modelid == "RT-AX58U" || based_modelid == "TUF-AX3000" || based_modelid == "RT-AX82U" || based_modelid == "RT-AX56U" || based_modelid == "RT-AX56_XD4" || based_modelid == "RT-AX86U" || based_modelid == "RT-AX5700" || based_modelid == "RT-AX68U")
				document.getElementById("span0").innerHTML = "2.4GHz and 5GHz";

			document.getElementById("t1").style.display = "none";
			document.getElementById("t2").style.display = "none";
			document.getElementById("t3").style.display = "none";
			document.getElementById("t0").style.width = (tab_width*parent.wl_info.wl_if_total+10) +'px';
		}
	}
	else if(v == 2){ //5GHz Smart Connect
		document.getElementById("span0").innerHTML = "2.4GHz";
		document.getElementById("span1").innerHTML = "5GHz-1 and 5GHz-2";
		document.getElementById("t3").style.display = "none";
		document.getElementById("t2").style.display = "none";	
		document.getElementById("t1").style.width = "143px";
		document.getElementById("span1").style.padding = "5px 4px 5px 7px";
	}
}

function change_smart_connect(v){
	switch(v){
		case '0':
			tab_reset(0);	
			break;
		case '1': 
			tab_reset(1);
			break;
		case '2': 
			tab_reset(2);
			break;
	}
}

function generate_cpu_field(){
	var code = "";
	for(i=0;i<core_num;i++){
		code += "<tr><td><div class='cpu_div'><table>";
		code += "<tr><td><div><table>";
		code += "<tr>";		
		code += "<td class='loading_bar' colspan='2'>";
		code += "<div>";
		code += "<div id='cpu"+i+"_bar' class='status_bar'></div>";
		code += "</div>";
		code += "</td>";	
		code += "<td>";
		code += "<div>Core "+parseInt(i+1)+"</div>";
		code += "</td>";		
		code += "<td class='percentage_bar'>";
		code += "<div id='cpu"+i+"_quantification'>0%</div>";
		code += "</td>";		
		code += "</tr>";
		code += "</table></div></td></tr>";
		code += "</table></div></td></tr>";

		document.getElementById('cpu'+i+'_graph').style.display = "";
	}

	if(parent.getBrowser_info().ie == "9.0" || parent.getBrowser_info().ie == "8.0")
		document.getElementById('cpu_field').outerHTML = code;
	else
		document.getElementById('cpu_field').innerHTML = code;
}

function get_ethernet_ports() {
	$.ajax({
		url: '/ajax_ethernet_ports.asp',
		async: false,
		dataType: 'script',
		error: function(xhr) {
			setTimeout("get_ethernet_ports();", 1000);
		},
		success: function(response) {
			var wanLanStatus = get_wan_lan_status["portSpeed"];
			var wanCount = get_wan_lan_status["portCount"]["wanCount"];
			//parse nvram to array
			var parseStrToArray = function(_array) {
				var speedMapping = new Array();	
				speedMapping["M"] = "100 Mbps";
				speedMapping["G"] = "1 Gbps";
				speedMapping["Q"] = "2.5 Gbps";
				speedMapping["X"] = "<#Status_Unplugged#>";
				
				var parseArray = [];
				for (var prop in _array) {
					if (_array.hasOwnProperty(prop)) {
						var newRuleArray = new Array();
						var port_name = prop;
						if(wanCount != undefined) {
							if(port_name.substr(0, 3) == "WAN") {
								if(parseInt(wanCount) > 1) {
									var port_idx = port_name.split(" ");
									port_name = port_idx[0] + " " + (parseInt(port_idx[1]) + 1);
								}
								else {
									port_name = "WAN";
								}
							}
						}

						newRuleArray.push(port_name);
						newRuleArray.push(speedMapping[_array[prop]]);
						parseArray.push(newRuleArray);
					}
				}
				return parseArray;
			};

			//set table Struct
			var tableStruct = {
				data: parseStrToArray(wanLanStatus),
				container: "tableContainer",
				header: [ 
					{
						"title" : "<#Status_Ports#>",
						"width" : "50%"
					},
					{
						"title" : "<#Status_Str#>",
						"width" : "50%"
					}
				]
			}

			if(tableStruct.data.length) {
				$("#tr_ethernet_ports").css("display", "");
				tableApi.genTableAPI(tableStruct);
			}

			setTimeout("get_ethernet_ports();", 3000);
		}
	});
}

function update_traffic() {
	$.ajax({
	url: '/update.cgi',
	dataType: 'script',
	data: {'output': 'netdev'},
	error: function(xhr) {
			setTimeout("update_traffic();", 1000);
	},
	success: function(response){
		var diff_rx = 0;
		if(last_rx != 0){
			if((current_rx - last_rx) < 0){
				diff_rx = 1;
			}
			else{
				diff_rx = (current_rx - last_rx)/2;
			}
		}

		last_rx = current_rx;
		if(netdev.INTERNET){
			current_rx = netdev.INTERNET.rx;
		}
		else{
			current_rx = netdev.WIRED.rx + netdev.WIRELESS0.rx + netdev.WIRELESS1.rx;
			if( netdev.WIRELESS2){
				current_rx += netdev.WIRELESS2.rx;
			}
		}

		var diff_tx = 0;
		if(last_tx != 0){
			if((current_tx - last_tx) < 0){
				diff_tx = 1;
			}
			else{
				diff_tx = (current_tx - last_tx)/2;
			}
		}

		last_tx = current_tx;
		if(netdev.INTERNET){
			current_tx = netdev.INTERNET.tx;
		}
		else{
			current_tx = netdev.WIRED.tx + netdev.WIRELESS0.tx + netdev.WIRELESS1.tx;
			if( netdev.WIRELESS2){
				current_tx += netdev.WIRELESS2.tx;
			}
		}

		document.getElementById('rx-current').innerHTML = adjust_unit(diff_rx);
		document.getElementById('tx-current').innerHTML = adjust_unit(diff_tx);

		if (diff_rx > max_rx)
			max_rx = diff_rx;

		used_percentage = Math.round((diff_rx/max_rx)*100);
		$("#rx_bar").css("width", used_percentage +"%");

		if (diff_tx > max_tx)
			max_tx = diff_tx;

		used_percentage = Math.round((diff_tx/max_tx)*100);
		$("#tx_bar").css("width", used_percentage +"%");

		setTimeout("update_traffic();", 2000);
	}
	});
}

function adjust_unit(value) {
	value = value / 1024;
	unit = " KB/s";

	if (value > 1024) {
		value = value / 1024;
		unit = " MB/s";
	}
	return value.toFixed(2) + unit;
}

</script>
</head>
<body class="statusbody" onload="initial();">
<iframe name="hidden_frame" id="hidden_frame" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" id="form" action="/start_apply2.htm">
<input type="hidden" name="current_page" value="device-map/router_status.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="action_wait" value="">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="wl_unit" value="<% nvram_get("wl_unit"); %>">
<input type="hidden" name="wl_subunit" value="-1">

<table border="0" cellpadding="0" cellspacing="0" id="rt_table">
<tr>
	<td>		
		<table width="100px" border="0" align="left" style="margin-left:8px;" cellpadding="0" cellspacing="0">
			<td>
				<div id="t0" class="tab_NW" align="center" style="font-weight: bolder;display:none; margin-right:2px;" onclick="tabclickhandler(0)">
					<span id="span0" style="cursor:pointer;font-weight: bolder;">2.4GHz</span>
				</div>
			</td>
			<td>
				<div id="t1" class="tab_NW" align="center" style="font-weight: bolder;display:none; margin-right:2px;" onclick="tabclickhandler(1)">
					<span id="span1" style="cursor:pointer;font-weight: bolder;">5GHz</span>
				</div>
			</td>
			<td>
				<div id="t2" class="tab_NW" align="center" style="font-weight: bolder;display:none; margin-right:2px;" onclick="tabclickhandler(2)">
					<span id="span2" style="cursor:pointer;font-weight: bolder;">5GHz-2</span>
				</div>
			</td>
			<td>
				<div id="t3" class="tab_NW" align="center" style="font-weight: bolder;display:none; margin-right:2px;" onclick="tabclickhandler(3)">
					<span id="span3" style="cursor:pointer;font-weight: bolder;">60GHz</span>
				</div>
			</td>
			<!--td>
				<div id="t_compatibility" class="tab_NW" align="center" style="font-weight: bolder; margin-right:2px;" onclick="tabclickhandler('compatibility')">
					<span style="cursor:pointer;font-weight: bolder;">Compatibility</span>
				</div>
			</td-->
			<td>
				<div id="t_status" class="tabclick_NW" align="center" style="font-weight: bolder; margin-right:2px;" onclick="tabclickhandler('status')">
					<span id="span_status" style="cursor:pointer;font-weight: bolder;"><#Status_Str#></span>
				</div>
			</td>
		</table>
	</td>
</tr>

<tr>
	<td>
                <table width="96%" border="1" align="center" cellpadding="4" cellspacing="0" class="table1px" id="net" style="margin: 0px 8px;">
			<tr>
				<td colspan="3">
					<div class="title">Internet Traffic</div>
					<div style="margin-top: 5px;*margin-top:-70px;" class="line_horizontal"></div>
				</td>
			</tr>
			<tr>
				<td class="loading_bar">
					<div>
						<div id="rx_bar" class="status_bar"></div>
					</div>
				</td>
				<td>
					<div>Down:</div>
				</td>
				<td style="text-align:right;padding-right:10px;">
					<div id="rx-current">-- KB/s</div>
				</td>
			</tr>
			<tr>
				<td class="loading_bar">
					<div>
						<div id="tx_bar" class="status_bar"></div>
					</div>
				</td>
				<td>
					<div>Up:</div>
				</td>
				<td style="text-align:right;padding-right:10px;">
					<div id="tx-current">-- KB/s</div>
				</td>
			</tr>
			<tr>
				<td colspan="3" style="border-bottom:5px #2A3539 solid;padding:0px 10px 5px 10px;"></td>
			</tr>
		</table>
	</td>
</tr>

<tr>
	<td>
		<div>
		<table width="96%" border="1" align="center" cellpadding="4" cellspacing="0" class="table1px" id="cpu" style="margin: 0px 8px;">
			<tr>
				<td >
					<div class="title"><#Status_CPU#></div>
					<div style="margin-top: 5px;*margin-top:-70px;" class="line_horizontal"></div>
				</td>
			</tr >
			<tr>
				<td>
					<table id="cpu_field"></table>
				</td>
			</tr>
			
			<tr style="height:100px;" class="IE8HACK">
				<td colspan="3">
					<div style="margin:0px 11px 0px 11px;background-color:black;">
						<svg width="270px" height="100px">
							<g>
								<line stroke-width="1" stroke-opacity="1"   stroke="rgb(255,255,255)" x1="0" y1="0%"   x2="100%" y2="0%" />
								<line stroke-width="1" stroke-opacity="0.2" stroke="rgb(255,255,255)" x1="0" y1="25%"  x2="100%" y2="25%" />
								<line stroke-width="1" stroke-opacity="0.2" stroke="rgb(255,255,255)" x1="0" y1="50%"  x2="100%" y2="50%" />
								<line stroke-width="1" stroke-opacity="0.2" stroke="rgb(255,255,255)" x1="0" y1="75%"  x2="100%" y2="75%" />
								<line stroke-width="1" stroke-opacity="1"   stroke="rgb(255,255,255)" x1="0" y1="100%" x2="100%" y2="100%" />
							</g>							
							<g>
								<text font-family="Verdana" fill="#FFFFFF" font-size="8" x="0" y="98%">0%</text>
								<text font-family="Verdana" fill="#FFFFFF" font-size="8" x="0" y="55%">50%</text>
								<text font-family="Verdana" fill="#FFFFFF" font-size="8" x="0" y="11%">100%</text>
							</g>							
							<line stroke-width="1" stroke-opacity="1"   stroke="rgb(0,0,121)"   x1="0"   y1="0%" x2="0"   y2="100%" id="tick1" />
							<line stroke-width="1" stroke-opacity="0.3" stroke="rgb(40,255,40)" x1="30"  y1="0%" x2="30"  y2="100%" id="tick2" />
							<line stroke-width="1" stroke-opacity="0.3" stroke="rgb(40,255,40)" x1="60"  y1="0%" x2="60"  y2="100%" id="tick3" />
							<line stroke-width="1" stroke-opacity="0.3" stroke="rgb(40,255,40)" x1="90"  y1="0%" x2="90"  y2="100%" id="tick4" />
							<line stroke-width="1" stroke-opacity="0.3" stroke="rgb(40,255,40)" x1="120" y1="0%" x2="120" y2="100%" id="tick5" />
							<line stroke-width="1" stroke-opacity="0.3" stroke="rgb(40,255,40)" x1="150" y1="0%" x2="150" y2="100%" id="tick6" />
							<line stroke-width="1" stroke-opacity="0.3" stroke="rgb(40,255,40)" x1="180" y1="0%" x2="180" y2="100%" id="tick7" />
							<line stroke-width="1" stroke-opacity="0.3" stroke="rgb(40,255,40)" x1="210" y1="0%" x2="210" y2="100%" id="tick8" />
							<line stroke-width="1" stroke-opacity="0.3" stroke="rgb(40,255,40)" x1="240" y1="0%" x2="240" y2="100%" id="tick9" />						
							<line stroke-width="1" stroke-opacity="1"   stroke="rgb(0,0,121)"   x1="270" y1="0%" x2="270" y2="100%" id="tick10" />

							<polyline id="cpu0_graph" style="fill:none;stroke:#FF9000;stroke-width:1;width:200px;"  points=""></polyline>
							<polyline id="cpu1_graph" style="fill:none;stroke:#3CF;stroke-width:1;width:200px;display:none;"  points=""></polyline>
							<polyline id="cpu2_graph" style="fill:none;stroke:#FC0;stroke-width:1;width:200px;display:none;"  points=""></polyline>
							<polyline id="cpu3_graph" style="fill:none;stroke:#FF44FF;stroke-width:1;width:200px;display:none;"  points=""></polyline>
						</svg>
					</div>
				</td>
			</tr>			
			<tr>
				<td style="border-bottom:5px #2A3539 solid;padding:0px 10px 5px 10px;"></td>
			</tr>
 		</table>
		</div>
  	</td>
</tr>

<tr>
	<td> 
		<div>
			<table width="96%" border="1" align="center" cellpadding="4" cellspacing="0" class="table1px" style="margin: 0px 8px;">	
			<tr>
				<td colspan="3">		
					<div class="title"><#Status_RAM#></div>
					<div style="margin-top: 5px;*margin-top:-70px;" class="line_horizontal"></div>
				</td>
			</tr>
			<tr class="ram_table">
				<td>
					<div><#Status_Used#></div>	  			
					<div id="ram_used_info"></div>	
				</td>
				<td>
					<div><#Status_Free#></div>
					<div id="ram_free_info"></div>	  			
				</td>
				<td>
					<div><#Status_Total#></div>	  			
					<div id="ram_total_info"></div>	  			
				</td>
			</tr>  
			<tr>
				<td colspan="3">
					<div>
						<table>
							<tr>
								<td class="loading_bar" colspan="2">
									<div>
										<div id="ram_bar" class="status_bar"></div>				
									</div>
								</td>
								<td>
									<div style="width:39px;"></div>
								</td>
								<td class="percentage_bar">
									<div id="ram_quantification">0%</div>
								</td>
							</tr>
						</table>
					</div>
				</td>
			</tr>
			</table>
		</div>
	</td>
</tr>
<tr id="tr_ethernet_ports" style="display:none;">
	<td> 
		<div>
			<table width="96%" border="1" align="center" cellpadding="4" cellspacing="0" class="table1px" style="margin: 0px 8px;">	
				<tr>
					<td style="border-bottom:5px #2A3539 solid;padding:0px 10px 5px 10px;"></td>
				</tr>
				<tr>
					<td>
						<div class="title"><#Status_Ethernet_Ports#></div>
						<div style="margin-top: 5px;*margin-top:-70px;" class="line_horizontal"></div>
					</td>
				</tr>
				<tr>
					<td>
						<div style="overflow-x:hidden;height:190px;">
							<div id="tableContainer" style="margin-top:-10px;"></div>
						</div>
					</td>
				</tr>
			</table>
		</div>
	</td>
</tr>
</table>			
</form>
</body>
</html>
