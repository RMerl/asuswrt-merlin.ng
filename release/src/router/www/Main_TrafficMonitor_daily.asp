<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="X-UA-Compatible" content="IE=Edge" />
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">

<title><#Web_Title#> - <#menu4_2_3#></title>
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

<script type='text/javascript'>
var nvram = httpApi.nvramGet(["rstats_enable"]);

var daily_history = [];
var barDataUl, barDataDl, barLabels;
var chartObj;

var scale = 2;
var months = [];
const snames = ['KB', 'MB', 'GB'];
const scaleFactors = [1, 1024, 1048576];
const ui_locale = ui_lang.toLowerCase();

function init(){
	var scaleCookie;

	if (nvram.rstats_enable != '1') return;

	months = generateMonthsLabels();

	if ((scaleCookie = cookie.get('daily')) != null) {
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

function switchPage(page){
	if(page == "1")
		location.href = "/Main_TrafficMonitor_realtime.asp";
	else if(page == "2")
		location.href = "/Main_TrafficMonitor_last24.asp";
	else if(page == "4")
		location.href = "/Main_TrafficMonitor_monthly.asp";
	else
		return false;
}

function generateMonthsLabels() {
	for (let i = 0; i < 12; i++) {
		months.push(
			new Date(2000, i, 1).toLocaleString(ui_locale, { month: 'short' })
		);
	}
	return months;
}

function rescale(n){
	return (Number(n / scaleFactors[scale]).toLocaleString(ui_locale, { minimumFractionDigits: 2, maximumFractionDigits: 2 })) + " " + snames[scale];
}

function ymdText(yr, mo, da){
	return yr + '-' + (mo + 1).toString().padStart(2, '0') + '-' + da.toString().padStart(2, '0');
}

function changeScale(newscale){
	scale = parseInt(newscale);
	cookie.set('daily', scale, 366);
	display_data();
}

function update_traffic(){
	$.ajax({
		url: '/update.cgi',
		dataType: 'script',
		data: {'output': 'bandwidth', 'arg0': 'daily'},
		error: function(xhr) {
			setTimeout("update_traffic()", 1000);
		},
		success: function(response){
			if (typeof(daily_history) == 'undefined') {
				setTimeout("update_traffic()", 1000);
				return;
			}
			daily_history.sort(function(a, b){return parseInt(b) - parseInt(a);});
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

	if (daily_history.length > 0) {
		ymd = getYMD(daily_history[0][0]);
		now = new Date((new Date(ymd[0], ymd[1], ymd[2], 12, 0, 0, 0)).getTime() - ((30 - 1) * 86400000));
		document.getElementById('last-dates').innerHTML = '(' + ymdText(now.getFullYear(), now.getMonth(), now.getDate()) + ' ~ ' + ymdText(ymd[0], ymd[1], ymd[2]) + ')';
		lastt = ((now.getFullYear() - 1900) << 16) | (now.getMonth() << 8) | now.getDate();
	}

	barDataUl = [];
	barDataDl = [];
	barLabels = [];

	htmldata = '<table width="730px" class="FormTable_NWM">' +
	           "<tr><th style=\"height:30px;\"><#Date#></th>" +
	           "<th><#tm_reception#></th>" +
	           "<th><#tm_transmission#></th>" +
	           "<th><#Total#></th></tr>";

	for (var i = 0; i < daily_history.length; ++i) {
		var entry = daily_history[i];
		ymd = getYMD(entry[0]);
		rows++;

		htmldata += '<tr><td>' + ymdText(ymd[0], ymd[1], ymd[2]) + '</td>' +
		            '<td class="dl">' + rescale(entry[1]) + '</td>' +
		            '<td class="ul">' + rescale(entry[2]) + '</td>' +
		            '<td class="total">' + rescale(entry[1] + entry[2]) + '</td></tr>';

		if (entry[0] >= lastt) {
			lastd += entry[1];
			lastu += entry[2];
		}

		barDataDl.unshift(entry[1] / scaleFactors[scale]);
		barDataUl.unshift(entry[2] / scaleFactors[scale]);
		barLabels.unshift(months[ymd[1]] + ' ' + ymd[2]);
	}

	if(rows == 0)
		htmldata +='<tr><td class="hint-color" colspan="4"><#IPConnection_VSList_Norule#></td></tr>';

	document.getElementById('bwm-daily-grid').innerHTML = htmldata + '</table>';
	document.getElementById('last-dn').innerHTML = rescale(lastd);
	document.getElementById('last-up').innerHTML = rescale(lastu);
	document.getElementById('last-total').innerHTML = rescale(lastu + lastd);

	drawChart();
}

function drawChart(){
	if (barLabels.length == 0) return;
	if (barLabels.length > 45)
		border = 0;
	else
		border = 1;

	if (chartObj != undefined) chartObj.destroy();
	var ctx = document.getElementById("chart").getContext("2d");

	chartObj = new Chart(ctx, {
		type: 'bar',
		data: {
			labels: barLabels,
			datasets: [
				{
					data: barDataDl,
					label: "<#tm_reception#> (" + snames[scale] + ")",
					borderWidth: border,
					backgroundColor: "#4C8FC0",
					borderColor: "#000000"
				},
				{
					data: barDataUl,
					label: "<#tm_transmission#> (" + snames[scale] +")",
					borderWidth: border,
					backgroundColor: "#4CC08F",
					borderColor: "#000000"
				}
			]
		},
		options: {
			segmentShowStroke : false,
			segmentStrokeColor : "#000",
			animationEasing : "easeOutQuart",
			animationSteps : 100,
			animateScale : true,
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
						label: function (context) { return context.parsed.y.toLocaleString(ui_locale, { minimumFractionDigits: 2, maximumFractionDigits: 2 }) + " " + snames[scale]; },
					}
				},
				legend: {
					display: true,
					position: "top",
					labels: {color: "#CCC"}
				},
			},
			scales: {
				x: {
					grid: { display: false },
					ticks: {color: "#CCC" },
				},
				y: {
					grid: { color: "#282828" },
					ticks: {
						color: "#CCC",
						callback: function(value, index, values) {
							return value.toLocaleString(ui_locale);
						}
					}
				}
			}
		},
	});
}

</script>
</head>

<body onload="show_menu();init();" class="bg">

<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" action="apply.cgi" target="hidden_frame">
<input type="hidden" name="current_page" value="Main_TrafficMonitor_daily.asp">
<input type="hidden" name="next_page" value="Main_TrafficMonitor_daily.asp">
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
												<select id="page_select" class="input_option" style="width:120px" onchange="switchPage(this.options[this.selectedIndex].value)">
													<option value="1"><#menu4_2_1#></option>
													<option value="2"><#menu4_2_2#></option>
													<option value="3" selected><#menu4_2_3#></option>
													<option value="4">Monthly</option>
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
							<div style="background-color:#2f3e44;border-radius:10px;width:730px;"><canvas id="chart" style="cursor:crosshair;" height="140"></div>
						</td>
					</tr>
					<tr>
						<td>
							<div id="bwm-daily-grid"></div>
						</td>
					</tr>
					<tr>
						<td bgcolor="#4D595D">
							<table width="730"  border="1" align="left" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable" >
							<thead>
								<tr>
									<td colspan="2" id="TriggerList" style="text-align:left;"><#Last30days#> <span style="color:#FFF;background-color:transparent;" id='last-dates'></span></td>
								</tr>
							</thead>
							<tbody>
								<tr><th width="40%"><#tm_reception#></th><td id='last-dn'>-</td></tr>
								<tr><th width="40%"><#tm_transmission#></th><td id='last-up'>-</td></tr>
								<tr><th width="40%"><#Total#></th><td id='last-total'>-</td></tr>
							</tbody>
							</table>
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
