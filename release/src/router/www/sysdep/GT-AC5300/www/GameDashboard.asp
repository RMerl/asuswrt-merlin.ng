<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=Edge"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<title><#Web_Title#> - Game Dashboard</title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="tmmenu.css">
<link rel="stylesheet" type="text/css" href="menu_style.css">
<link rel="stylesheet" type="text/css" href="device-map/device-map.css">
<script type="text/javascript" src="js/loader.js"></script>
<script type="text/javascript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/form.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/client_function.js"></script>
<style>
.traffic_bar{
	width: 0%;
	height: 5px;
	border-radius:5px;
}
.traffic_bar_download{
	background-color: #32ADB2;
}
.traffic_bar_upload{
	background-color: #BCBD4D;
}
.transition_style{
	-webkit-transition: all 2s ease-in-out;
	-moz-transition: all 2s ease-in-out;
	-o-transition: all 2s ease-in-out;
	transition: all 2s ease-in-out;
}
.wl_icon{
	display:inline-block;width:48px;height:48px;background:url('images/New_ui/icon_signal.png');
}
.wl0_icon_on{
	background-position: 0px 0px;
}
.wl1_icon_on{
	background-position: 0px 96px;
}
.wl2_icon_on{
	background-position: 0px 48px;
}
.wl0_icon_off{
	background-position: 48px 0px;
}
.wl1_icon_off{
	background-position: 48px 96px;
}
.wl2_icon_off{
	background-position: 48px 48px;
}
.wan_state_icon{
	width: 136px;
	height: 136px;
	margin: 10px 0 0 75px;
}
.wan_icon_connect{
	background: url('images/New_ui/wan-connect.png') no-repeat;
}
.wan_icon_disconnect{
	background: url('images/New_ui/wan-disconnect.png') no-repeat;
}
</style>
<script>
// disable auto log out
AUTOLOGOUT_MAX_MINUTE = 0;
var isDemoMode = ('<% nvram_get("demoMode"); %>' == 1) ? true : false;

google.charts.load('current', {'packages':['corechart']});

var dataArray = new Array();
var time = new Date();
var timestamp = time.getTime();
var t_year = time.getFullYear();
var t_mon = time.getMonth();
var t_date = time.getDate();
var t_hour = time.getHours();
var t_min = time.getMinutes();
var t_sec = time.getSeconds();
var wl0_radio = '<% nvram_get("wl0_radio"); %>';
var wl1_radio = '<% nvram_get("wl1_radio"); %>';
var wl2_radio = '<% nvram_get("wl2_radio"); %>';

for(i=0;i<30;i++){
	var temp = [];
	temp = [new Date(t_year, t_mon, t_date, t_hour, t_min, t_sec),  0, '', 0, ''];
	dataArray.push(temp);
	timestamp -= 3000;
	time = new Date(timestamp);
	getTime();
}

google.charts.setOnLoadCallback(function(){
	drawChart(dataArray);
});  

function getTime(){
	t_year = time.getFullYear();
	t_mon = time.getMonth();
	t_date = time.getDate();
	t_hour = time.getHours();
	t_min = time.getMinutes();
	t_sec = time.getSeconds();
}

function initial(){
	show_menu();
	update_tarffic();
	check_sw_mode();
	check_wireless();

	netoolApi.start({
		"type": 1, 
		"target": "www.google.com"
	});		
}

function check_sw_mode(){
	var mode = "";
	if(sw_mode == "1")
		mode = "<#wireless_router#>";
	else if(sw_mode == "2"){
		if(wlc_express == 1)
			mode = "Express Way (2.4GHz)";
		else if(wlc_express == 2)
			mode = "Express Way (5GHz)";
		else
			mode = "<#OP_RE_item#>";
	}
	else if(sw_mode == "3")
		mode = "<#OP_AP_item#>";
	else if(sw_mode == "4")
		mode = "<#OP_MB_item#>";
	else
		mode = "Unknown";	

	$("#sw_mode_desc").html(mode);
}

function check_wireless(){
	var temp = "";
	//check 2.4 GHz
	temp = (wl0_radio == "1") ? "wl0_icon_on" : "wl0_icon_off"
	$("#wl0_icon").addClass(temp);

	//check 5 GHz-1
	temp = (wl1_radio == "1") ? "wl1_icon_on" : "wl1_icon_off"
	$("#wl1_icon").addClass(temp);

	//check 5 GHz-2
	temp = (wl2_radio == "1") ? "wl2_icon_on" : "wl2_icon_off"
	$("#wl2_icon").addClass(temp);
}

function drawChart(data){
	var dataTable = new google.visualization.DataTable();
	dataTable.addColumn('datetime', 'Date');
	dataTable.addColumn('number', '<#tm_transmission#>');
	dataTable.addColumn({type: 'string', role: 'tooltip'});
	dataTable.addColumn('number', '<#tm_reception#>');
	dataTable.addColumn({type: 'string', role: 'tooltip'});
	dataTable.addRows(data);

	var options = {
		backgroundColor: "transparent",
			title: 'Internet Traffic',
			legend: { position: 'top' },
			legendTextStyle: { color: '#BFBFBF' },
			colors: ['#BCBD4D', '#32ADB2'],
			vAxis: {
				format: '#,### KB/s',
				gridlines: {count: 4},
				textStyle: {color: '#BFBFBF'},
			},
			hAxis: {
				textStyle: {color: '#BFBFBF'}
			}
	}

	var chart = new google.visualization.AreaChart(document.getElementById('area_chart'));   
    chart.draw(dataTable, options);
}
var last_rx = 0
var last_tx = 0
var current_rx = 0;
var current_tx = 0;
var traffic_array = new Array();
function update_tarffic() {
	$.ajax({
    	url: '/update.cgi',
    	dataType: 'script',	
    	data: {'output': 'netdev'},
    	error: function(xhr) {
			setTimeout("update_tarffic();", 1000);
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
    		current_rx = netdev.INTERNET.rx;

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
    		current_tx = netdev.INTERNET.tx;
    		traffic_bar(diff_tx, diff_rx);
    		refineData(diff_tx, diff_rx);
			setTimeout("update_tarffic();drawChart(dataArray);", 2000);
    	}
	});
}

function traffic_bar(tx, rx){
	/*basic unit: Bytes*/
	var temp = "";
	temp = translate_traffic(tx);
	$("#upload_traffic").html(temp[0] + " " + temp[1]);
	$("#upload_traffic_bar").css("width", temp[2]);
	temp = translate_traffic(rx);
	$("#download_traffic").html(temp[0] + " " + temp[1]);
	$("#download_traffic_bar").css("width", temp[2]);
}

function translate_traffic(traffic){
	var value = 0;
	var unit = "Bytes";
	var bar_width = "";
	var bar_value = 0;

	if(traffic != 0)
		bar_value = 1;

	if(traffic > 1024){
		traffic = traffic/1024;
		unit = "KB";
		if(traffic > 1024){
			traffic = traffic/1024;
			unit = "MB";
			bar_value = parseInt(traffic);
			if(bar_value > 100)
				bar_value = 100;
		}
	}

	bar_width = bar_value + "%";
	return [traffic.toFixed(2), unit, bar_width];
}

function refineData(tx, rx){
	time = new Date();
	getTime();
	var tooltip_tx = "";
	var tooltip_rx = "";
	var label_tx = tx + "Bytes/s";
	var label_rx = rx + "Bytes/s";
	rx_temp = rx;
	tx_temp = tx;
	if(tx > 1024){
		label_tx = (tx/1024).toFixed(2) + "KB/s";
		tx = tx/1024;

		if(tx > 1024){
			label_tx = (tx/1024).toFixed(2) + "MB/s";
		}
	}

	if(rx > 1024){
		label_rx = (rx/1024).toFixed(2) + "KB/s";
		rx = rx/1024;

		if(rx > 1024){
			label_rx = (rx/1024).toFixed(2) + "MB/s";
		}
	}	

	tooltip_tx = new Date(t_year, t_mon, t_date, t_hour, t_min, t_sec) + "\n TX: " + label_tx;
	tooltip_rx = new Date(t_year, t_mon, t_date, t_hour, t_min, t_sec) + "\n RX: " + label_rx;
    var temp = [new Date(t_year, t_mon, t_date, t_hour, t_min, t_sec),  tx_temp/1024, tooltip_tx , rx_temp/1024, tooltip_rx];	

    dataArray.push(temp);
    dataArray.shift();
}

var targetData = {}

function initTargetData(){
	var retObj = {};
	retObj["points"] = [];
	retObj["sum"] = 0;
	retObj["avg"] = 0;
	retObj["pingMax"] = 0;
	retObj["pingMin"] = 9999;
	retObj["jitter"] = 0;
	retObj["loss"] = 0;
	retObj["max"] = 0;
	retObj["min"] = 9999;
	retObj["isDoing"] = true;

	retObj["jitters"] = [];
	retObj["jitterSum"] = 0;
	retObj["jitterAvg"] = 0;

	return retObj;
}

var netoolApi = {
	start: function(obj){
		if(!targetData[obj.target]){
			targetData[obj.target] = new initTargetData();
		}

		$.getJSON("/netool.cgi", obj)
			.done(function(data){
				if(data.successful != "0") setTimeout(function(){
					netoolApi.check(obj, data.successful)
				}, 2000)
			})
			.fail(function(e){
				netoolApi.render(obj.target)
			});
	},

	check: function(obj, fileName){
		$.getJSON("/netool.cgi", {"type":0,"target":fileName})
			.done(function(data){
				var thisTarget = targetData[obj.target];
				var pingVal = (data.result[0].ping !== "") ? parseFloat(data.result[0].ping) : 0;
				var jitterVal = (thisTarget.points.length === 0) ? 0 : Math.abs(pingVal - thisTarget.points[thisTarget.points.length-1]).toFixed(3);

				thisTarget.isDoing = (thisTarget.points.length > 110) ? false : thisTarget.isDoing;
				// ping status
				thisTarget.points.push(pingVal);
				thisTarget.pingMax = (thisTarget.pingMax > pingVal) ? thisTarget.pingMax : pingVal;
				thisTarget.pingMin = (thisTarget.pingMin < pingVal) ? thisTarget.pingMin : pingVal;
				thisTarget.sum += pingVal;
				thisTarget.avg = (thisTarget.sum/thisTarget.points.length).toFixed(3);
				thisTarget.jitter = Math.abs(thisTarget.pingMax - thisTarget.pingMin).toFixed(3);
				thisTarget.loss += (parseInt(data.result[0].loss) / 100);

				// jitter status
				thisTarget.jitters.push(jitterVal)
				thisTarget.jitterMax = (thisTarget.jitterMax > jitterVal) ? thisTarget.jitterMax : jitterVal;
				thisTarget.jitterMin = (thisTarget.jitterMin < jitterVal) ? thisTarget.jitterMin : jitterVal;
				thisTarget.jitterSum += parseFloat(jitterVal);
				thisTarget.jitterAvg = (thisTarget.jitterSum/thisTarget.jitters.length).toFixed(3);

				var gap = parseInt(thisTarget.jitter/4) + 2;
				thisTarget.min = parseInt(thisTarget.pingMin/gap)*gap;
				thisTarget.max = thisTarget.min + gap*4;

				if(thisTarget.isDoing){
					netoolApi.render(obj.target);
					netoolApi.start(obj);
				}
			})
			.fail(function(data){
				setTimeout(function(){
					netoolApi.check(obj, fileName);
				}, 500);
			});
	},

	render: function(target){
		var thisTarget = targetData[target];

		// Ping Graph
		var toPosition = function(point){
			return (170-((point-thisTarget.min)/(thisTarget.max-thisTarget.min))*170);
		}

		$(".yAxis")
			.each(function(id){
				$(this).html(thisTarget.min + (thisTarget.max-thisTarget.min)*id/4 + " ms")
			})

		$("#ping_graph")
			.attr("points", function(){
				return thisTarget.points
					.map(function(el, id){return ((id*3) + "," + toPosition(el));})
					.join(" ");
			});

		$("#ping_avg_graph")
			.attr("points", "0," + toPosition(thisTarget.avg) + " 340," + toPosition(thisTarget.avg));

		$("#pingAvg").html(thisTarget.avg + " ms")

		// Jitter Graph
		var toJitterPosition = function(point){
			var graphHeight = (thisTarget.jitter == 0) ? "999" : thisTarget.jitter;
			return (170-(point/graphHeight)*170);
		}

		$(".yAxisJitter")
			.each(function(id){
				$(this).html(Math.abs(thisTarget.jitter*id/4).toFixed(3) + " ms")
			})

		$("#jitter_graph")
			.attr("points", function(){
				return thisTarget.jitters
					.map(function(el, id){return ((id*3) + "," + toJitterPosition(el));})
					.join(" ");
			});

		$("#jitter_avg_graph")
			.attr("points", "0," + toJitterPosition(thisTarget.jitterAvg) + " 340," + toJitterPosition(thisTarget.jitterAvg));

		$("#jitterAvg").html(thisTarget.jitterAvg + " ms")
	},

	reset: function(obj){
		netoolApi.stopAll();
		targetData[obj.target] = new initTargetData();
		$("#ping_graph").attr("points", "0,170");
		$("#ping_avg_graph").attr("points", "0,170");
		$("#jitter_graph").attr("points", "0,170");
	},

	stopAll: function(){
		for(var i in targetData){
			targetData[i].isDoing = false;
		}
	}
}

</script>
</head>

<body onload="initial();" onunload="unload_body();" onselectstart="return false;">
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>
<div id="agreement_panel" class="panel_folder" style="margin-top: -100px;"></div>

<iframe name="hidden_frame" id="hidden_frame" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="GameDashBoard.asp">
<input type="hidden" name="next_page" value="GameDashBoard.asp">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_wait" value="4">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="restart_wrs;restart_firewall">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>" disabled>
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">

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
						<table width="760px" border="0" cellpadding="4" cellspacing="0" class="FormTitle" id="FormTitle">
							<tbody>
							<tr>
								<td style="background:url('images/New_ui/bg_content_ROG.png')" valign="top">
								<div>
									<div style="width:99%;height:300px;margin: 10px auto;">
										<div style="">
											<div style="font-size: 18px;margin: 45px 20px -20px 100px;color:#BFBFBF"><#ROG_Network_Traffic#></div>
										</div>
										<div style="width:100%;">
											<div id="area_chart" style="width: 600px; height: 310px;margin: 0 auto;display:inline-block;"></div>
											<div style="display:inline-block;vertical-align: top;margin:130px 0 0 -50px;width:170px;">
												<div >
													<div>
														<div style="height:5px;padding:2px;background-color:#2A2523;border-radius:10px;width:100px;">
															<div id="download_traffic_bar" class="traffic_bar traffic_bar_download transition_style traffic_bar_boost"></div>
														</div>
													</div>
													<div style="font-size:18px;color:#BFBFBF;margin: 5px 0"><#option_download#></div>
													<div id="download_traffic" style="text-align: right;color:#32ADB2;font-size:22px;margin-bottom:5px;"></div>
												</div>
												<div >
													<div>
														<div style="height:5px;padding:2px;background-color:#2A2523;border-radius:10px;width:100px;">
															<div id="upload_traffic_bar" class="traffic_bar traffic_bar_upload transition_style traffic_bar_boost"></div>
														</div>
													</div>
													<div style="font-size:18px;color:#BFBFBF;margin: 5px 0"><#option_upload#></div>
													<div id="upload_traffic" style="text-align: right;color:#BCBD4D;font-size:22px;margin-bottom:5px;"></div>
												</div>
											</div>
										</div>
									</div>

									<div style="width:99%;height:200px;margin-top:35px;">
										<div style="display: inline-block;width:240px;vertical-align: top;">
											
											<div style="margin: 10px 0 0 30px;">
												<div style="font-size: 18px;color:#BFBFBF"><#ROG_WIRELESS_STATE#></div>
												<div style="text-align: right;margin-right:20px;">
													<div id="wl0_icon" class="wl_icon"></div>
													<div id="wl1_icon" class="wl_icon"></div>
													<div id="wl2_icon" class="wl_icon"></div>
												</div>

											</div>
											<div style="margin: 25px 0 0 90px;text-align: center;">
												<div style="font-size: 18px;color:#BFBFBF"><#ROG_WAN_STATE#></div>
												<div id="wan_state" style="font-size: 18px;margin-top:10px;color:#57BDBA"></div>		
											</div>
										</div>
										<div style="display: inline-block;width:270px;">
											<div style="font-size: 22px;margin-top: 25px;text-align: center;color:#BFBFBF"><#statusTitle_Internet#></div>
											<div id="wan_state_icon" class="wan_state_icon "></div>
										</div>	
										<div style="display: inline-block;width:180px;vertical-align: top;">
											
											<div style="margin: 10px 0 0 30px;">
												<div style="font-size: 18px;color:#BFBFBF"><#DSL_Mode#></div>
												<div id="sw_mode_desc" style="font-size: 18px;margin-top:10px;color:#57BDBA"></div>
											</div>
											<div style="margin-top: 50px;text-align:center;color:#BFBFBF">
												<!--div style="font-size: 18px;">GAME DEVICES</div>
												<div style="margin-top:10px;">
													<span style="font-size: 14px;">Total</span>
													<span style="font-size: 20px;padding:0 10px;color:#9CE239">5</span>
													<span style="font-size: 14px;">ONLINE</span>
												</div-->	
											</div>
										</div>
									</div>

									<div style="width:99%;height:300px;margin: 10px auto;">
										<div style="font-size:18px;margin:45px 120px;position:absolute;width:200px;color:#BFBFBF"><#ROG_Network_PING#></div>
										<div id="svgPingContainer" style="margin:85px 0px 0px 20px;position:absolute;background-color:#221712;">
											<svg width="340px" height="170px">
												<g>
													<line stroke-width="1" stroke-opacity="1"   stroke="#221712" x1="0" y1="0%"   x2="100%" y2="0%" />
													<line stroke-width="1" stroke-opacity="0.2" stroke="rgb(255,255,255)" x1="0" y1="25%"  x2="100%" y2="25%" />
													<line stroke-width="1" stroke-opacity="0.2" stroke="rgb(255,255,255)" x1="0" y1="50%"  x2="100%" y2="50%" />
													<line stroke-width="1" stroke-opacity="0.2" stroke="rgb(255,255,255)" x1="0" y1="75%"  x2="100%" y2="75%" />
													<line stroke-width="1" stroke-opacity="1"   stroke="#221712" x1="0" y1="100%" x2="100%" y2="100%" />
												</g>							
												<g>
													<text class="yAxis" font-family="Verdana" fill="#999" font-size="8" x="0" y="98%">0 ms</text>
													<text class="yAxis" font-family="Verdana" fill="#999" font-size="8" x="0" y="78%">25 ms</text>
													<text class="yAxis" font-family="Verdana" fill="#999" font-size="8" x="0" y="55%">50 ms</text>
													<text class="yAxis" font-family="Verdana" fill="#999" font-size="8" x="0" y="28%">75 ms</text>
													<text class="yAxis" font-family="Verdana" fill="#999" font-size="8" x="0" y="5%">100 ms</text>
												</g>							

												<polyline id="ping_avg_graph" style="fill:none;stroke:#BCCCDC;stroke-width:1;" points="0,250"></polyline>
												<polyline id="ping_graph" style="fill:none;stroke:#57BDBA;stroke-width:2;z-index:9999" points="0,250"></polyline>
											</svg>
										</div>
										<div style="font-size:12px;margin:255px 250px;position:absolute;width:200px;color:#BFBFBF"><#Average_value#> : <span id="pingAvg">0 ms</span></div>

										<div style="font-size:18px;margin:45px 500px;position:absolute;width:200px;color:#BFBFBF"><#ROG_PING_DEVIATION#></div>
										<div id="svgJitterContainer" style="margin:85px 0px 0px 400px;position:absolute;background-color:#221712;"> 
											<svg width="340px" height="170px">
												<g>
													<line stroke-width="1" stroke-opacity="1"   stroke="#221712" x1="0" y1="0%"   x2="100%" y2="0%" />
													<line stroke-width="1" stroke-opacity="0.2" stroke="rgb(255,255,255)" x1="0" y1="25%"  x2="100%" y2="25%" />
													<line stroke-width="1" stroke-opacity="0.2" stroke="rgb(255,255,255)" x1="0" y1="50%"  x2="100%" y2="50%" />
													<line stroke-width="1" stroke-opacity="0.2" stroke="rgb(255,255,255)" x1="0" y1="75%"  x2="100%" y2="75%" />
													<line stroke-width="1" stroke-opacity="1"   stroke="#221712" x1="0" y1="100%" x2="100%" y2="100%" />
												</g>							
												<g>
													<text class="yAxisJitter" font-family="Verdana" fill="#999" font-size="8" x="0" y="98%">0 ms</text>
													<text class="yAxisJitter" font-family="Verdana" fill="#999" font-size="8" x="0" y="78%">0 ms</text>
													<text class="yAxisJitter" font-family="Verdana" fill="#999" font-size="8" x="0" y="55%">0 ms</text>
													<text class="yAxisJitter" font-family="Verdana" fill="#999" font-size="8" x="0" y="28%">0 ms</text>
													<text class="yAxisJitter" font-family="Verdana" fill="#999" font-size="8" x="0" y="5%">0 ms</text>
												</g>							

												<polyline id="jitter_avg_graph" style="fill:none;stroke:#BCCCDC;stroke-width:1;" points="0,250"></polyline>
												<polyline id="jitter_graph" style="fill:none;stroke:#BCBD4D;stroke-width:2;z-index:9999" points="0,250"></polyline>
											</svg>
										</div>
										<div style="font-size:12px;margin:255px 630px;position:absolute;width:200px;color:#BFBFBF"><#Average_value#> : <span id="jitterAvg">0 ms</span></div>
									</div>

									<!--div style="width:99%;height:300px;margin: 10px 0 0 10px;">
										<div style="font-size: 18px;margin: 30px 0 10px 50px;color:#BFBFBF">Gaming Devices</div>
										<div id="game_device_list" style="height:255px;overflow: auto;margin-top:25px;color:#BFBFBF">
											
											<div style="height:50px;margin: 0 10px;">
												<div>
													<div style="display:inline-block;vertical-align: middle;padding: 0 10px;">
														<div style="height:40px;width:40px;cursor:default;" class="clientIcon_no_hover type9" title="Android Device"></div>
													</div>
													<div style="display:inline-block;width:160px;padding: 0 10px;font-size:14px;">ASUS_PC</div>
													<div style="display:inline-block;width:120px">
														<div style="font-size:14px;">Download</div>
														<div>
															<div style="height:5px;padding:2px;background-color:#2A2523;border-radius:10px;width:100px;">
																<div id="download_traffic_bar_0" class="traffic_bar traffic_bar_download transition_style traffic_bar_boost"></div>
															</div>
														</div>
													</div>
													<div style="display:inline-block;width:100px;">
														<div id="dl_traffic_0" style="font-size:16px;color:#32ADB2"></div>
													</div>
													<div style="display:inline-block;width:120px">
														<div style="font-size:14px;">Upload</div>
														<div>
															<div style="height:5px;padding:2px;background-color:#2A2523;border-radius:10px;width:100px;">
																<div id="upload_traffic_bar_0" class="traffic_bar traffic_bar_upload transition_style traffic_bar_boost"></div>
															</div>
														</div>
													</div>
													<div style="display:inline-block;width:100px">
														<div id="ul_traffic_0" style="font-size:16px;color:#BCBD4D"></div>
													</div>													
												</div>
											</div>
											<div style="width:95%;border-bottom: 1px solid #FFF;margin: 0 auto 10px auto"></div>
											<div style="height:50px;margin: 0 10px;">
												<div>
													<div style="display:inline-block;vertical-align: middle;padding: 0 10px;">
														<div style="height:40px;width:40px;cursor:default;" class="clientIcon_no_hover type9" title="Android Device"></div>
													</div>
													<div style="display:inline-block;width:160px;padding: 0 10px;font-size:14px;">ASUS_Laptop</div>
													<div style="display:inline-block;width:120px">
														<div style="font-size:14px;">Download</div>
														<div>
															<div style="height:5px;padding:2px;background-color:#2A2523;border-radius:10px;width:100px;">
																<div id="download_traffic_bar_1" class="traffic_bar traffic_bar_download transition_style traffic_bar_boost"></div>
															</div>
														</div>
													</div>
													<div style="display:inline-block;width:100px;">
														<div id="dl_traffic_1" style="font-size:16px;color:#32ADB2"></div>
													</div>
													<div style="display:inline-block;width:120px">
														<div style="font-size:14px;">Upload</div>
														<div>
															<div style="height:5px;padding:2px;background-color:#2A2523;border-radius:10px;width:100px;">
																<div id="upload_traffic_bar_1" class="traffic_bar traffic_bar_upload transition_style traffic_bar_boost"></div>
															</div>
														</div>
													</div>
													<div style="display:inline-block;width:100px">
														<div id="ul_traffic_1" style="font-size:16px;color:#BCBD4D"></div>
													</div>
												</div>
											</div>



											</div>										
																					
										</div-->									
									</div>
								</div>
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
