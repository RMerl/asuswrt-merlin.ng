<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="X-UA-Compatible" content="IE=Edge" />
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">

<title><#Web_Title#> - Monthly</title>
<link rel="stylesheet" type="text/css" href="index_style.css">
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<script language="JavaScript" type="text/javascript" src="/js/jquery.js"></script>
<script language="JavaScript" type="text/javascript" src="/js/chart.min.js"></script>
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/help.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script language="JavaScript" type="text/javascript" src="/js/httpApi.js"></script>
<script language="JavaScript" type="text/javascript" src="/js/trafmon.js"></script>
<style>
.chartCanvas {
	cursor: crosshair;
	border-radius: 10px;
	width: 100% !important;
	height: 370px;
	display: block;
}
</style>

<script type='text/javascript'>
var nvram = httpApi.nvramGet(["rstats_enable"]);

var monthly_history = [];
var barDataUl, barDataDl, barLabels;
var chartObj;

var scale = 2;
var months = [];

if (isSupport("UI4")){
	var labelsColor = "#1C1C1E";
	var gridColor = "#CCC";
	var ticksColor = "#1C1C1E";
	var rxBackgroundColor = "#4C8FC0";
	var rxBorderColor = "#000000";
	var txBackgroundColor = "#4CC08F";
	var txBorderColor = "#000000";
	var chartBackgroundColor = getComputedStyle(document.querySelector(":root")).getPropertyValue("--color-bg-card");
	var tableLabelColor = "#006CE1";
	var tableValueColor = "#1C1C1E";
} else {
	var labelsColor = "#CCC";
	var gridColor = "#282828";
	var ticksColor = "#CCC";
	var rxBackgroundColor = "#4C8FC0";
	var rxBorderColor = "#000000";
	var txBackgroundColor = "#4CC08F";
	var txBorderColor = "#000000";
	var chartBackgroundColor = "#2f3e44";
	var tableLabelColor = "#FFCC00";
	var tableValueColor = "white";
}

function init(){
	var scaleCookie;

	if (nvram.rstats_enable != '1') return;

	months = generateMonthsLabels();
	document.getElementById("chart").style.backgroundColor = chartBackgroundColor;

	if ((scaleCookie = window.localStorage.getItem('monthly')) != null) {
		var parsedScale = parseInt(scaleCookie);
		if (!isNaN(parsedScale) && parsedScale >= 0 && parsedScale <= 2)
			scale = parsedScale;
	}
	document.getElementById('scale').value = scale;

	update_traffic();

	if(bwdpi_support){
		document.getElementById('content_title').innerHTML = "<#traffic_monitor#>";
	}
}

function ymdText(yr, mo){
	return yr + '-' + (mo + 1).toString().padStart(2, '0');
}

function changeScale(newscale){
	scale = parseInt(newscale);
	window.localStorage.setItem('monthly', scale, 366);
	display_data();
}

function update_traffic(){
	$.ajax({
		url: '/update.cgi',
		dataType: 'script',
		data: {'output': 'bandwidth', 'arg0': 'monthly'},
		error: function(xhr) {
			setTimeout("update_traffic()", 1000);
		},
		success: function(response){
			if (typeof(monthly_history) == 'undefined') {
				setTimeout("update_traffic()", 1000);
				return;
			}
			monthly_history.sort(function(a, b){return parseInt(b) - parseInt(a);});
			display_data();
		}
	});
}

function display_data(){
	var htmldata;
	var rows = 0;
	var ymd;
	var now;
	var lastt, lastu = 0, lastd = 0;
	var getYMD = function(n){
		return [(((n >> 16) & 0xFF) + 1900), ((n >>> 8) & 0xFF), (n & 0xFF)];
	}

	barDataUl = [];
	barDataDl = [];
	barLabels = [];

	htmldata = '<table width="730px" class="FormTable_table">' +
	           "<tr><th style=\"height:30px;\"><#Date#></th>" +
	           "<th><#tm_reception#></th>" +
	           "<th><#tm_transmission#></th>" +
	           "<th><#Total#></th></tr>";

	for (i = 0; i < monthly_history.length-1; ++i) {
		var entry = monthly_history[i];
		ymd = getYMD(entry[0]);
		year = (((entry[0] >> 16) & 0xFF) + 1900);
		month = ((entry[0] >>> 8) & 0xFF);

		++rows;

		htmldata += '<tr><td>' + ymdText(ymd[0], ymd[1]) + '</td>' +
		            '<td class="dl">' + rescale_value(entry[1], scale) + '</td>' +
		            '<td class="ul">' + rescale_value(entry[2], scale) + '</td>' +
		            '<td class="total">' + rescale_value(entry[1] + entry[2], scale) + '</td></tr>';

		barDataDl.unshift(entry[1] / scaleFactors[scale]);
		barDataUl.unshift(entry[2] / scaleFactors[scale]);
		barLabels.unshift(months[month] + ' ' + year);
	}

	if(rows == 0)
		htmldata +='<tr><td class="hint-color" colspan="4"><#IPConnection_VSList_Norule#></td></tr>';

	document.getElementById('bwm-monthly-grid').innerHTML = htmldata + '</table>';
	drawChart();
}

function drawChart(){
	if (barLabels.length == 0) return;

	if (chartObj != undefined) chartObj.destroy();
	var ctx = document.getElementById("chart").getContext("2d");

	chartObj = new Chart(ctx, {
		type: 'bar',
		data:  {
			labels: barLabels,
			datasets: [
				{
					data: barDataDl,
					label: "<#tm_reception#> (" + scaleNames[scale] + ")",
					borderWidth: 1,
					backgroundColor: rxBackgroundColor,
					borderColor: rxBorderColor
				},
				{
					data: barDataUl,
					label: "<#tm_transmission#> (" + scaleNames[scale] +")",
					borderWidth: 1,
					backgroundColor: txBackgroundColor,
					borderColor: txBorderColor
				}
			]
		},
		options: {
			segmentShowStroke: false,
			animationEasing: "easeOutQuart",
			animationSteps: 100,
			animateScale: true,
			responsive: true,
			interaction: {
				mode: 'index',
				intersect: false
			},
			plugins: {
				tooltip: {
					position: 'nearest',
					intersect: false,
					mode: 'index',
					callbacks: {
						label: function (context) { return context.parsed.y.toLocaleString(ui_locale, { minimumFractionDigits: 2, maximumFractionDigits: 2 }) + " " + scaleNames[scale]; },
					}
				},
				legend: {
					display: true,
					position: "top",
					labels: {color: labelsColor}
				},
			},
			scales: {
				x: {
					grid: { display: false },
					ticks: {color: ticksColor },
				},
				y: {
					grid: { color: gridColor },
					ticks: {
						color: ticksColor,
						callback: function(value, index, values) {
							return value.toLocaleString(ui_locale) + " " + scaleNames[scale];
						}
					}
				}
			}
		},
	});
}

</script>
</head>

<body onload="show_menu();init();" class="bg" >

<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" action="apply.cgi" target="hidden_frame">
<input type="hidden" name="current_page" value="Main_TrafficMonitor_monthly.asp">
<input type="hidden" name="next_page" value="Main_TrafficMonitor_monthly.asp">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="action_wait" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">

<table class="content" align="center" cellpadding="0" cellspacing="0">
<tr>
	<td width="23">&nbsp;</td>

<!--=====Beginning of Main Menu=====-->
	<td valign="top" width="202">
		<div id="mainMenu"></div>
		<div id="subMenu"></div>
	</td>

    <td valign="top">
		<div id="tabMenu" class="submenuBlock"></div>
<!--===================================Beginning of Main Content===========================================-->
		<table width="98%" border="0" align="left" cellpadding="0" cellspacing="0">
		<tr>
			<td align="left"  valign="top">
				<table width="100%" border="0" cellpadding="4" cellspacing="0" class="FormTitle" id="FormTitle">
				<tbody>
					<tr>
						<td bgcolor="#4D595D" valign="top">
							<table width="740px" border="0" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3">
								<tr><td><table width="100%">
									<tr>
										<td  class="formfonttitle" align="left">
											<div id="content_title" style="margin-top:5px;"><#Menu_TrafficManager#> - <#traffic_monitor#></div>
										</td>
										<td>
											<div align="right">
												<select id="page_select" onchange="tm_switchPage(this.options[this.selectedIndex].value, '4')" class="input_option">
													<option value="1"><#menu4_2_1#></option>
													<option value="2"><#menu4_2_2#></option>
													<option value="3"><#menu4_2_3#></option>
													<option value="4" selected>Monthly</option>
													<option value="5">Settings</option>
												</select>
											</div>
										</td>
									</tr>
							</table></td></tr>
					<tr>
						<td height="5"><div class="splitLine"></div></td>
					</tr>
					<tr>
						<td bgcolor="#4D595D">
							<table width="730"  border="1" align="left" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
							<thead>
								<tr>
									<td colspan="2"><#t2BC#></td>
								</tr>
							</thead>
							<tbody>
								<tr>
									<th width="40%"><#Scale#></th>
									<td>
										<select style="width:70px" class="input_option" onchange='changeScale(this.value)' id='scale'>
											<option value="0">KB</option>
											<option value="1">MB</option>
											<option value="2" selected>GB</option>
										</select>
									</td>
								</tr>
							</tbody>
							</table>
						</td>
					</tr>
					<tr>
						<td>
							<canvas id="chart" class="chartCanvas"></canvas>
						</td>
					</tr>
					<tr>
						<td>
							<div id="bwm-monthly-grid"></div>
						</td>
					</tr>
					</table>
					</td>
					</tr>
			</tbody>
			</table>
			</td>
		</tr>
		</table>
		</div>
	</td>

	<td width="10" align="center" valign="top">&nbsp;</td>
</tr>
</table>
<div id="footer"></div>
</body>
</html>
