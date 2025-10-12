<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<meta http-equiv="X-UA-Compatible" content="IE=Edge" />
<title><#Web_Title#> - <#traffic_monitor#> : <#menu4_2_1#></title>
<link rel="stylesheet" type="text/css" href="index_style.css">
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<script language="JavaScript" type="text/javascript" src="/js/jquery.js"></script>
<script language="JavaScript" type="text/javascript" src="/js/httpApi.js"></script>
<script language="JavaScript" type="text/javascript" src="client_function.js"></script>
<script language="JavaScript" type="text/javascript" src="/js/chart.min.js"></script>
<script language="JavaScript" type="text/javascript" src="help.js"></script>
<script language="JavaScript" type="text/javascript" src="state.js"></script>
<script language="JavaScript" type="text/javascript" src="general.js"></script>
<script language="JavaScript" type="text/javascript" src="popup.js"></script>
<script language="JavaScript" type="text/javascript" src="/js/trafmon.js"></script>

<style>
.chartCanvas {
	cursor: crosshair;
	background-color: #2f3e44;
	border-radius: 10px;
	width: 100% !important;
}
</style>

<script type='text/javascript'>

var speed_data = {};
var last_speed_data = {};
var chartObj = {};
var refresh_toggle = 1;

const updateFrequency = 1000;
const maxSamples = 60;
const showBitrate = 1;

const labelsColor = "#CCC";
const gridColor = "#282828";
const ticksColor = "#CCC";
const rxBorderColor = "rgba(76, 143, 192, 1)";
const rxBackgroundColor = "rgba(76, 143, 192, 0.3)";
const txBorderColor = "rgba(76, 192, 143, 1)";
const txBackgroundColor = "rgba(76, 192, 143, 0.3)";

// disable auto log out
AUTOLOGOUT_MAX_MINUTE = 0;


function init(){
	if(bwdpi_support){
		document.getElementById('content_title').innerHTML = "<#traffic_monitor#>";
	}
	update_traffic();
}


function toggle_refresh(){
	if (refresh_toggle == 1) {
		refresh_toggle = 0;
		document.getElementById('refresh_button').value = "Resume";
	} else {
		refresh_toggle = 1;
		document.getElementById('refresh_button').value = "Pause";
	}
}

function init_data_object(){
	for (var ifname in netdev) {

/* Speed data */
		if (!speed_data[ifname]) {
			speed_data[ifname] = {};
			speed_data[ifname].rx = [];
			speed_data[ifname].tx = [];
			speed_data[ifname].max_rx = 0;
			speed_data[ifname].max_tx = 0;
			speed_data[ifname].friendly = get_friendly_ifname(ifname);

			last_speed_data[ifname] = {};
			last_speed_data[ifname].rx = 0;
			last_speed_data[ifname].tx = 0;
		}

/* Canvas */
		var htmldata = '<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">';
		htmldata += '<thead><tr><td>' + speed_data[ifname].friendly + '</td></tr></thead>';
		htmldata += '<tr>';
		htmldata += '<td style="padding:14px;" width="100%"><div style="height: 250px;"><canvas class="chartCanvas" id="' + ifname + '_Chart"></canvas></div>';
		htmldata += '<div style="padding-top: 14px; display: flex; justify-content: space-between;">';
		htmldata += '<div class="hint-color">Current In: <span style="color: white;" id="' + ifname + '_RX_current"></span></div><div class="hint-color">Max In: <span style="color: white;" id="'+ ifname + '_RX_max"></span></div>';
		htmldata += '<div class="hint-color">Current Out: <span style="color: white;" id="' + ifname + '_TX_current"></span></div><div class="hint-color">Max Out: <span style="color: white;"id="'+ ifname + '_TX_max"></span></div>';
		htmldata += '</div></td>'
		htmldata += '</tr></table>';

		if (ifname == "INTERNET")	// Always insert at the top
			document.getElementById("graph_content").insertAdjacentHTML("afterbegin", htmldata);
		else
			document.getElementById("graph_content").insertAdjacentHTML("beforeend", htmldata);

/* Chart objects */
		chartObj[ifname] = {};
	}
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
			 if (Object.keys(speed_data).length === 0) {
				init_data_object();
			}

			for (var ifname in netdev) {
				var diff_rx = 0;
				var diff_tx = 0;

				var current_rx = netdev[ifname].rx;
				var current_tx = netdev[ifname].tx;

				if(last_speed_data[ifname].rx != 0){
					if((current_rx - last_speed_data[ifname].rx) < 0){
						diff_rx = 1;
					}
					else{
						diff_rx = (current_rx - last_speed_data[ifname].rx) / (updateFrequency/1000);
					}
				}

				if(last_speed_data[ifname].tx != 0){
					if((current_tx - last_speed_data[ifname].tx) < 0){
						diff_tx = 1;
					}
					else{
						diff_tx = (current_tx - last_speed_data[ifname].tx) / (updateFrequency/1000);
					}
				}

				last_speed_data[ifname].rx = current_rx;
				last_speed_data[ifname].tx = current_tx;

				speed_data[ifname].rx.push(diff_rx);
				speed_data[ifname].tx.push(diff_tx);

				if(diff_rx > speed_data[ifname].max_rx){
					speed_data[ifname].max_rx = diff_rx;
				}
				if(diff_tx > speed_data[ifname].max_tx){
					speed_data[ifname].max_tx = diff_tx;
				}

				if(speed_data[ifname].rx.length > maxSamples){
					speed_data[ifname].rx.shift();
				}
				if(speed_data[ifname].tx.length > maxSamples){
					speed_data[ifname].tx.shift();
				}
				if(refresh_toggle == 1) {
					drawGraph(ifname);
					document.getElementById(ifname + "_RX_current").innerHTML = rescale_data_rate(diff_rx, (showBitrate ? 2 : 1));
					document.getElementById(ifname + "_TX_current").innerHTML = rescale_data_rate(diff_tx, (showBitrate ? 2 : 1));
					document.getElementById(ifname + "_RX_max").innerHTML = rescale_data_rate(speed_data[ifname].max_rx, (showBitrate ? 2 : 1));
					document.getElementById(ifname + "_TX_max").innerHTML = rescale_data_rate(speed_data[ifname].max_tx, (showBitrate ? 2 : 1));
				}
			}
			setTimeout("update_traffic();", updateFrequency);
		}
	});
}


function drawGraph(ifname){
	var displayed_data_rx = speed_data[ifname].rx;
	var displayed_data_tx = speed_data[ifname].tx;

	if (chartObj[ifname].obj != undefined) {
		chartObj[ifname].obj.update();
		return;
	}

	var ctx = document.getElementById(ifname + '_Chart').getContext("2d");

	chartObj[ifname].obj = new Chart(ctx, {
		type: "line",
		data: {
			labels: Array.from({length: maxSamples}, (v, i) => i),
			datasets: [
				{
					label: speed_data[ifname].friendly + " In",
					data: displayed_data_rx,
					backgroundColor: rxBackgroundColor,
					borderColor: rxBorderColor,
					borderWidth: "2",
					pointRadius: "0",
					fill: { target: "origin"}
				},
				{
					label: speed_data[ifname].friendly + " Out",
					data: displayed_data_tx,
					backgroundColor: txBackgroundColor,
					borderColor: txBorderColor,
					borderWidth: "2",
					pointRadius: "0",
					fill: { target: "origin"}
				}
			]
		},
		options: {
			responsive: true,
			maintainAspectRatio: false,
			animation: false,
			segmentShowStroke: false,
			interaction: {
				mode: 'index',
				intersect: false
			},
			plugins: {
				tooltip: {
					position: 'nearest',
					intersect: false,
					mode: 'index',
					displayColors: false,
					bodySpacing: 6,
					callbacks: {
						title: function (context) {return "";},
						label: function (context) {
							var label = context.dataset.label || '';
							var value = context.parsed.y;
							return label + " - " + rescale_data_rate(value, (showBitrate ? 2 : 1));
						}
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
					display: true,
					type: 'linear',
					min: 0,
					max: maxSamples,
					grid: { color: gridColor },
					ticks: {
						display: false,
						stepSize: 10 / (updateFrequency/1000),
					}
				},
				y: {
					grace: "5%",
					min: 0,
					grid: { color: gridColor },
					ticks: {
						color: ticksColor,
						callback: function(value, index, ticks) {return rescale_data_rate(value, (showBitrate ? 2 : 1));}
					}
				},
			}
		}
	});
}


</script>
</head>

<body onload="show_menu();init();" class="bg">
<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" action="apply.cgi" target="hidden_frame">
<input type="hidden" name="current_page" value="Main_TrafficMonitor_realtime.asp">
<input type="hidden" name="next_page" value="Main_TrafficMonitor_realtime.asp">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="action_wait" value="">
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
						<table width="740px" border="0" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="TMTable">
							<tr>
								<td>
									<table width="100%">
										<tr>
											<td  class="formfonttitle" align="left">
												<div id="content_title" style="margin-top:5px;"><#Menu_TrafficManager#> - <#traffic_monitor#></div>
											</td>
											<td>
												<div align="right">
													<select id="page_select" onchange="tm_switchPage(this.options[this.selectedIndex].value, '1')" class="input_option">
														<option value="1" selected><#menu4_2_1#></option>
														<option value="2"><#menu4_2_2#></option>
														<option value="3"><#menu4_2_3#></option>
														<option value="4">Monthly</option>
														<option value="5">Settings</option>
													</select>
												</div>
											</td></tr></table>
								</td>
							</tr>
							<tr>
								<td height="5"><div class="splitLine"></div></td>
							</tr>
							<tr>
								<td><div class="apply_gen"><input id="refresh_button" type="button" onClick="toggle_refresh();" value="Pause" class="button_gen"></div></td>
							</tr>
							<tr>
								<td id="graph_content"></td>
							</tr>
						</table>
					</td>
				</tr>
				</tbody>
			</table>
			</td>
		</tr>
		</table>
	</td>
	</tr>
</table>

<div id="footer"></div>
</body>
</html>
