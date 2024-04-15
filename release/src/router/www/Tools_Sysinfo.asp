<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=Edge"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title><#Web_Title#> - System Information</title>
<link rel="stylesheet" type="text/css" href="index_style.css">
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="/css/networkMap.css">
<script language="JavaScript" type="text/javascript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/js/chart.min.js"></script>
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script language="JavaScript" type="text/javascript" src="/help.js"></script>
<script language="JavaScript" type="text/javascript" src="/tmhist.js"></script>
<script language="JavaScript" type="text/javascript" src="/tmmenu.js"></script>
<script language="JavaScript" type="text/javascript" src="/client_function.js"></script>
<script type="text/javascript" src="/js/table/table.js"></script>
<script>

var odmpid = "<% nvram_get("odmpid");%>";

overlib_str_tmp = "";
overlib.isOut = true;


var pieColor = ["rgba(0, 84, 159, 1)",
                "rgba(0, 172, 223, 1)",
                "rgba(85, 208, 255, 1)"]

var memchartPie;
var swapchartPie;
var cputempGraph;
var cpudata = [];
var wifi24data = [];
var wifi51data = [];
var wifi52data = [];
var wifi6data = [];


function draw_mem_charts(){

/* Memory */
	if (memchartPie != undefined) {
		memchartPie.update();
		return;
	}
	var memchart = document.getElementById("memchartId").getContext("2d");
	var memdata = [mem_stats_arr[8], mem_stats_arr[9] - mem_stats_arr[1], mem_stats_arr[1]];

	memchartPie = new Chart(memchart, {
		type: 'doughnut',
		data: {
			labels: ["Used", "Reclaimable", "Free"],
			datasets: [{
				label: "Memory",
				data: memdata,
				backgroundColor: pieColor,
	                        hoverBackgroundColor: pieColor,
	                        borderColor: "#444",
		                borderWidth: "1"
			}],
		},
		options: {
			responsive: false,
			animation: false,
			segmentShowStroke : false,
			segmentStrokeColor : "#000",
			legend: {
				display: true,
				position: 'right',
				labels: {fontColor: '#FFF'}
			},
			tooltips: {
				callbacks: {
					label: function (tooltipItem, data) {
						var label = data.labels[tooltipItem.index];
						var value = data.datasets[tooltipItem.datasetIndex].data[tooltipItem.index];
						return label + ": " + parseFloat(value).toFixed(2) + " MB";
					}
				}
			}
		}
	});

/* SWAP */
	if (swapchartPie != undefined) {
		swapchartPie.update();
		return;
	}
	var swapchart = document.getElementById("swapchartId").getContext("2d");
	var swapdata = [mem_stats_arr[4], mem_stats_arr[5]-mem_stats_arr[4]];

	if (mem_stats_arr[5] > 0) {
		swapchartPie = new Chart(swapchart, {
			type: 'doughnut',
			data: {
				labels: ["Used", "Free"],
				datasets: [{
					label: "Swap",
					data: swapdata,
					backgroundColor: pieColor,
					hoverBackgroundColor: pieColor,
					borderColor: "#444",
					borderWidth: "1"
				}],
			},
			options: {
				responsive: false,
				animation: false,
				segmentShowStroke : false,
				segmentStrokeColor : "#000",
				legend: {
					display: true,
					position: 'right',
					labels: {fontColor: '#FFF'}
				},
				tooltips: {
					callbacks: {
						label: function (tooltipItem, data) {
							var label = data.labels[tooltipItem.index];
							var value = data.datasets[tooltipItem.datasetIndex].data[tooltipItem.index];
							return label + ": " + parseInt(value) + " MB";
						}
					}
				}
			}
		});
	}

/* NVRAM */

	nvram_total = <% sysinfo("nvram.total"); %>;
	used_percentage = Math.round((mem_stats_arr[6]/nvram_total)*100);
	$("#nvram_bar").css("width", used_percentage +"%");
	$("#nvram_label").html("<span>NVRAM Usage :</span> " + mem_stats_arr[6] + " / " + nvram_total + " bytes");
	if (used_percentage > 90)
		$("#nvram_bar").css("background-color", "orange");
	else
		$("#nvram_bar").css("background-color", "#00ACDF");

/* JFFS */

	jffs_total = parseFloat("<% sysinfo("jffs.total"); %>");
	jffs_free = parseFloat(mem_stats_arr[7]);

	if (jffs_total < 0) {
		$("#jffs_div").hide();
		$("#jffs_label").html("<span>JFFS partition not mounted</span>");
	} else {
		jffs_used = jffs_total - jffs_free;

		used_percentage = Math.round((jffs_used) / jffs_total * 100);
		$("#jffs_bar").css("width", used_percentage +"%");
		$("#jffs_label").html("<span>JFFS Usage: </span>" + jffs_used.toFixed(2) + " / " + jffs_total.toFixed(2) + " MB");
		if (used_percentage > 90)
			$("#jffs_bar").css("background-color", "orange");
		else
			$("#jffs_bar").css("background-color", "#00ACDF");
	}
}


function draw_temps_charts(){
	if (cpudata.length > 20)
		cpudata.shift();
	if (wifi24data.length > 20)
		wifi24data.shift();
	if (wifi51data.length > 20)
		wifi51data.shift();
	if (wifi52data.length > 20)
		wifi52data.shift();
	if (wifi6data.length > 20)
		wifi6data.shift();

        if (cputempGraph != undefined) {
                cputempGraph.update();
                return;
        }
        var cpuchart = document.getElementById("tempchartId").getContext("2d");
	var datasets = [];

/* CPU */
	datasets.push({
		label: "CPU",
		data: cpudata,
		backgroundColor: "rgba(0, 128, 191, 0.3)",
		borderColor: "rgba(0, 128, 191, 1)",
		borderWidth: "2",
		pointStyle: "line",
		lineTension: "0"
	});

/* 2.4 GHz */
	if (typeof wifi24data[0] === "number" && wifi24data[0] > 0) {
		datasets.push({
			label: "2.4 GHz",
			data: wifi24data,
			backgroundColor: "rgba(200, 200, 0, 0.3)",
			borderColor: "rgba(200, 200, 0, 1)",
			borderWidth: "2",
			pointStyle: "line",
			lineTension: "0",
		});
	}

/* 5 GHz */
	if (typeof wifi51data[0] === "number" && wifi51data[0] > 0) {
		datasets.push({
			label: "5 GHz",
			data: wifi51data,
			backgroundColor: "rgba(0, 200, 200, 0.3)",
			borderColor: "rgba(0, 200, 200,  1)",
			borderWidth: "2",
			pointStyle: "line",
			lineTension: "0",
		});
	}
/* 5 GHz-2 */
	if (typeof wifi52data[0] === "number" && wifi52data[0] > 0) {
		datasets.push({
			label: "5 GHz-2",
			data: wifi52data,
			backgroundColor: "rgba(200, 0, 200, 0.3)",
			borderColor: "rgba(200, 0, 200, 1)",
			borderWidth: "2",
			pointStyle: "line",
			lineTension: "0",
		});
	}

/* 6 GHz */
	if (typeof wifi6data[0] === "number" && wifi6data[0] > 0) {
		datasets.push({
			label: "6 GHz",
			data: wifi6data,
			backgroundColor: "rgba(128, 191, 0, 0.3)",
			borderColor: "rgba(128, 191, 0, 1)",
			borderWidth: "2",
			pointStyle: "line",
			lineTension: "0",
		});
	}

	cputempGraph = new Chart(cpuchart, {
		type: "line",
		data: {datasets: datasets},
		options: {
			responsive: false,
			animation: false,
			segmentShowStroke : false,
			segmentStrokeColor : "#000",
			legend: {
				display: true,
				position: "right",
				labels: {fontColor: "#CCC"}
			},
			tooltips: {
				displayColors: false,
				bodySpacing: 6,
				callbacks: {
					title: function (context) {return "";},
					label: function (tooltipItem, data) {
						var label = data.datasets[tooltipItem.datasetIndex].label;
						var value = data.datasets[tooltipItem.datasetIndex].data[tooltipItem.index];
						return label + " - " + value + "°C";
					}
				}
			},
			scales: {
				xAxes: [{
					labels: [0,3,6,9,12,15,18,21,24,27,30,33,36,39,42,45,48,51,54,57],
					ticks: {
						fontColor: "#CCC",
						beginAtZero: true,
						display: true,
					}
				}],
				yAxes: [{
					ticks: {
						fontColor: "#CCC",
						callback: function(value, index, ticks) {return value + "°C";}
					}
				}],
			}
		}
	});
}


function initial(){
	show_menu();

	if (wl_info.band5g_2_support) {
		document.getElementById("wifi51_clients_th").innerHTML = "Wireless Clients (5 GHz-1)";
		document.getElementById("wifi5_2_clients_tr").style.display = "";
	}
	if (wl_info.band6g_support) {
		document.getElementById("wifi6_clients_tr").style.display = "";
	}

	if (band5g_support) {
		document.getElementById("wifi5_clients_tr").style.display = "";
	}

	showbootTime();

	if ((odmpid != "") && (odmpid.toUpperCase() != based_modelid.toUpperCase())) {
		document.getElementById("model_id").innerHTML = "<span>" + odmpid + "</span>";
		document.getElementById("model_id").innerHTML += " (base model: <span>" + based_modelid + "</span>)";
	}
	else
		document.getElementById("model_id").innerHTML = productid;

	var rc_caps = "<% nvram_get("rc_support"); %>";
	var rc_caps_arr = rc_caps.split(' ').sort();
	rc_caps = rc_caps_arr.toString().replace(/,/g, " ");
	document.getElementById("rc_td").innerHTML = rc_caps;

	hwaccel_state();
	update_temperatures();
	updateClientList();
	update_sysinfo();
	show_wifi_version();
}

function update_temperatures(){
	$.ajax({
		url: '/ajax_coretmp.asp',
		dataType: 'script',
		error: function(xhr){
			update_temperatures();
		},
		success: function(response){
			if (based_modelid === 'GT-AXE16000') {
				curr_coreTmp_24_raw = curr_coreTmp_wl3_raw;
				curr_coreTmp_5_raw = curr_coreTmp_wl0_raw;
				curr_coreTmp_52_raw = curr_coreTmp_wl1_raw;
				curr_coreTmp_6_raw = curr_coreTmp_wl2_raw;
			} else {
				curr_coreTmp_24_raw = curr_coreTmp_wl0_raw;
				if (band5g_support)
					curr_coreTmp_5_raw = curr_coreTmp_wl1_raw;
				if (wl_info.band5g_2_support)
					curr_coreTmp_52_raw = curr_coreTmp_wl2_raw;
				else if (wl_info.band6g_support)
					curr_coreTmp_6_raw = curr_coreTmp_wl2_raw;
			}

			code = "<span>2.4 GHz:</span> " + curr_coreTmp_24_raw;
			wifi24data.push(parseInt(curr_coreTmp_24_raw.replace("&deg;C", "")));

			if (wl_info.band5g_2_support) {
				code += "&nbsp;&nbsp;-&nbsp;&nbsp;<span>5 GHz-1: </span>" + curr_coreTmp_5_raw;;
				code += "&nbsp;&nbsp;-&nbsp;&nbsp;<span>5 GHz-2: </span>" + curr_coreTmp_52_raw;
				wifi51data.push(parseInt(curr_coreTmp_5_raw.replace("&deg;C", "")));
				wifi52data.push(parseInt(curr_coreTmp_52_raw.replace("&deg;C", "")));
			} else if (band5g_support) {
				code += "&nbsp;&nbsp;-&nbsp;&nbsp;<span>5 GHz: </span>" + curr_coreTmp_5_raw;
				wifi51data.push(parseInt(curr_coreTmp_5_raw.replace("&deg;C", "")));
			}

			if (wl_info.band6g_support) {
				code += "&nbsp;&nbsp;-&nbsp;&nbsp;<span>6 GHz: </span>" + curr_coreTmp_6_raw;
				wifi6data.push(parseInt(curr_coreTmp_6_raw.replace("&deg;C", "")));
			}

			if (curr_cpuTemp != "") {
				code +="&nbsp;&nbsp;-&nbsp;&nbsp;<span>CPU: </span>" + parseInt(curr_cpuTemp) +"&deg;C";
				cpudata.push(parseInt(curr_cpuTemp));
			}
			document.getElementById("temp_td").innerHTML = code;
			draw_temps_charts();
			setTimeout("update_temperatures();", 3000);
		}
	});
}


function hwaccel_state(){
	var qos_enable = '<% nvram_get("qos_enable"); %>';
	var qos_type = '<% nvram_get("qos_type"); %>';

	if (hnd_support) {
		var machine_name = "<% get_machine_name(); %>";
		if (machine_name.search("aarch64") != -1)
			code = "<span>Runner:</span> ";
		else
			code = "<span>Archer:</span> ";

		var state = "<% sysinfo("hwaccel.runner"); %>";

		code += state;

		code += "&nbsp;&nbsp;-&nbsp;&nbsp;<span>Flow Cache:</span> ";
		state = "<% sysinfo("hwaccel.fc"); %>";

		code += state;
	}

	document.getElementById("hwaccel").innerHTML = code;
}


function showbootTime(){
        Days = Math.floor(boottime / (60*60*24));        
        Hours = Math.floor((boottime / 3600) % 24);
        Minutes = Math.floor(boottime % 3600 / 60);
        Seconds = Math.floor(boottime % 60);
        
        document.getElementById("boot_days").innerHTML = Days;
        document.getElementById("boot_hours").innerHTML = Hours;
        document.getElementById("boot_minutes").innerHTML = Minutes;
        document.getElementById("boot_seconds").innerHTML = Seconds;
        boottime += 1;
        setTimeout("showbootTime()", 1000);
}


function show_connstate(){
	document.getElementById("conn_td").innerHTML = conn_stats_arr[0] + " / <% sysinfo("conn.max"); %>&nbsp;&nbsp;-&nbsp;&nbsp;" + conn_stats_arr[1] + " active";

	if (based_modelid === 'GT-AXE16000') {
		wlc_24_arr = wlc_3_arr;
		wlc_51_arr = wlc_0_arr;
		wlc_52_arr = wlc_1_arr;
		wlc_6_arr = wlc_2_arr;
	} else {
		wlc_24_arr = wlc_0_arr;
		if (band5g_support)
			wlc_51_arr = wlc_1_arr;
		if (wl_info.band5g_2_support)
			wlc_52_arr = wlc_2_arr;
		else if (wl_info.band6g_support)
			wlc_6_arr = wlc_2_arr;
	}

	document.getElementById("wlc_24_td").innerHTML = "<span>Associated: </span>" + wlc_24_arr[0] + "&nbsp;&nbsp;-&nbsp;&nbsp;" +
	                                                 "<span>Authorized: </span>" + wlc_24_arr[1] + "&nbsp;&nbsp;-&nbsp;&nbsp;" +
	                                                 "<span>Authenticated: </span>" + wlc_24_arr[2];

	if (band5g_support) {
		document.getElementById("wlc_51_td").innerHTML = "<span>Associated: </span>" + wlc_51_arr[0] + "&nbsp;&nbsp;-&nbsp;&nbsp;" +
		                                                 "<span>Authorized: </span>" + wlc_51_arr[1] + "&nbsp;&nbsp;-&nbsp;&nbsp;" +
		                                                 "<span>Authenticated: </span>" + wlc_51_arr[2];
	}

	if (wl_info.band5g_2_support) {
		document.getElementById("wlc_52_td").innerHTML = "<span>Associated: </span>" + wlc_52_arr[0] + "&nbsp;&nbsp;-&nbsp;&nbsp;" +
		                                                 "<span>Authorized: </span>" + wlc_52_arr[1] + "&nbsp;&nbsp;-&nbsp;&nbsp;" +
		                                                 "<span>Authenticated: </span>" + wlc_52_arr[2];
	}

	if (wl_info.band6g_support) {
		document.getElementById("wlc_6_td").innerHTML = "<span>Associated: </span>" + wlc_6_arr[0] + "&nbsp;&nbsp;-&nbsp;&nbsp;" +
		                                                "<span>Authorized: </span>" + wlc_6_arr[1] + "&nbsp;&nbsp;-&nbsp;&nbsp;" +
		                                                "<span>Authenticated: </span>" + wlc_6_arr[2];
	}
}


function show_memcpu(){
	document.getElementById("cpu_stats_td").innerHTML = cpu_stats_arr[0] + ", " + cpu_stats_arr[1] + ", " + cpu_stats_arr[2];
	document.getElementById("mem_total_div").innerHTML = mem_stats_arr[0] + " MB";
	document.getElementById("mem_used_div").innerHTML = mem_stats_arr[8] + " MB";
	document.getElementById("mem_available_div").innerHTML = mem_stats_arr[9] + " MB";
	document.getElementById("mem_free_div").innerHTML = mem_stats_arr[1] + " MB";
	document.getElementById("mem_buffer_div").innerHTML = mem_stats_arr[2] + " MB";
	document.getElementById("mem_cache_div").innerHTML = mem_stats_arr[3] + " MB";
	if (parseInt(mem_stats_arr[5]) == 0) {
		document.getElementById("mem_swap_total_div").innerHTML = "<span>No swap configured</span>";
		document.getElementById("swap_div").style.display="none";
	} else {
		document.getElementById("mem_swap_total_div").innerHTML = mem_stats_arr[5] + " MB";
		document.getElementById("mem_swap_used_div").innerHTML = mem_stats_arr[4] + " MB";
		document.getElementById("swap_div").style.display="flex";
	}

	draw_mem_charts();
}


function updateClientList(e){
	$.ajax({
		url: '/update_clients.asp',
		dataType: 'script',
		error: function(xhr) {
			setTimeout("updateClientList();", 1000);
		},
		success: function(response){
			setTimeout("updateClientList();", 3000);
		}
	});
}

function update_sysinfo(e){
	$.ajax({
		url: '/ajax_sysinfo.asp',
		dataType: 'script',
		error: function(xhr) {
			setTimeout("update_sysinfo();", 1000);
		},
		success: function(response){
			show_memcpu();
			show_connstate();
			setTimeout("update_sysinfo();", 3000);
		}
	});
}

function show_wifi_version() {
	var buf = "<td>";

	buf += "<% sysinfo("driver_version.0"); %>";
	if (band5g_support)
		buf += "<br><% sysinfo("driver_version.1"); %>";
	if (wl_info.band5g_2_support || wl_info.band6g_support)
		buf += "<br><% sysinfo("driver_version.2"); %>";
	if (based_modelid === 'GT-AXE16000')
		buf += "<br><% sysinfo("driver_version.3"); %>";
	buf += "</td>";

	document.getElementById("wifi_version_td").innerHTML = buf;
}

</script>
</head>

<body onload="initial();" onunLoad="return unload_body();" class="bg">
<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="current_page" value="Tools_Sysinfo.asp">
<input type="hidden" name="next_page" value="Tools_Sysinfo.asp">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="action_wait" value="5">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="SystemCmd" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="ct_tcp_timeout" value="<% nvram_get("ct_tcp_timeout"); %>">
<input type="hidden" name="ct_udp_timeout" value="<% nvram_get("ct_udp_timeout"); %>">


<table class="content" align="center" cellpadding="0" cellspacing="0">
  <tr>
    <td width="17">&nbsp;</td>
    <td valign="top" width="202">
      <div id="mainMenu"></div>
      <div id="subMenu"></div></td>
    <td valign="top">
        <div id="tabMenu" class="submenuBlock"></div>

      <!--===================================Beginning of Main Content===========================================-->
      <table width="98%" border="0" align="left" cellpadding="0" cellspacing="0">
        <tr>
          <td valign="top">
            <table width="760px" border="0" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTitle" id="FormTitle">
                <tbody>
                <tr bgcolor="#4D595D">
                        <td valign="top">
                        <div>&nbsp;</div>
                        <div class="formfonttitle">Tools - System Information</div>
                        <div style="margin:10px 0 10px 5px;" class="splitLine"></div>

				<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
					<thead>
						<tr>
							<td colspan="2">Router</td>
						</tr>
					</thead>
					<tr>
						<th>Model</th>
							<td id="model_id"><% nvram_get("productid"); %></td>
					</tr>
					<tr>
						<th>Firmware Build</th>
						<td><% nvram_get("buildinfo"); %></td>
					</tr>
					<tr>
						<th>Bootloader</th>
						<td><% sysinfo("cfe_version"); %></td>
					</tr>
					<tr>
						<th>Wireless Driver Version</th>
						<td id="wifi_version_td"></td>
					</tr>
					<tr>
						<th>Features</th>
						<td id="rc_td"></td>
					</tr>
					<tr>
						<th><#General_x_SystemUpTime_itemname#></th>
						<td><span style="color: #FFF;" id="boot_days"></span> <span><#Day#></span> <span style="color: #FFF;" id="boot_hours"></span> <span><#Hour#></span> <span style="color: #FFF;" id="boot_minutes"></span> <span><#Minute#></span> <span style="color: #FFF;" id="boot_seconds"></span> <span><#Second#></span></td>
					</tr>

				</table>
				<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
					<thead>
						<tr>
							<td colspan="2">CPU</td>
						</tr>
					</thead>
					<tr>
						<th>CPU Model</th>
						<td><% sysinfo("cpu.model"); %>	@ <% sysinfo("cpu.freq"); %> MHz</td>
					</tr>
					<tr>
						<th>CPU Load Average (1, 5, 15 mins)</th>
						<td id="cpu_stats_td"></td>
					</tr>

				</table>

				<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
					<thead>
						<tr>
							<td width="50%">Memory</td>
							<td width="50%">Swap</td>
						</tr>
					</thead>
					<tr>
						<td><canvas id="memchartId" height="150"></canvas></td>
						<td><canvas id="swapchartId" height="150"></canvas></td>
					</tr>
					<tr>
						<td>
							<div style="display: flex;">
								<div class="hint-color" style="width:20%;"> Total :</div>
								<div style="width:76%;padding-left: 10px;" id="mem_total_div"></div>
							</div>
							<div style="display: flex;">
								<div class="hint-color" style="width:20%;">Used</div>
								<div style="width:76%;padding-left: 10px;" id="mem_used_div"></div>
							</div>

							<div style="display: flex;">
								<div class="hint-color" style="width:20%;">Available :</div>
								<div style="width:76%;padding-left: 10px;" id="mem_available_div"></div>
							</div>
							<div style="display: flex;">
								<div class="hint-color" style="width:20%;">Free :</div>
								<div style="width:76%;padding-left: 10px;" id="mem_free_div"></div>
							</div>
							<div style="display: flex;">
								<div class="hint-color" style="width:20%;">Buffers :</div>
								<div style="width:76%;padding-left: 10px;" id="mem_buffer_div"></div>
							</div>
							<div style="display: flex;">
								<div class="hint-color" style="width:20%;">Cache :</div>
								<div style="width:76%;padding-left: 10px;" id="mem_cache_div"></div>
							</div>
						</td>

						<td style="vertical-align:top;">
							<div style="display: flex;">
								<div class="hint-color" style="width:20%;"<th>Total Swap :</div>
								<div style="width:76%; padding-left: 10px;" id="mem_swap_total_div"></div>
							</div>
							<div id="swap_div" style="display: flex;">
								<div class="hint-color" style="width:20%;"<th>Used Swap :</div>
								<div style="width:76%; padding-left: 10px;" id="mem_swap_used_div"></div>
							</div>
						</td>

					</tr>
				</table>

				<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
					<thead>
						<tr>
							<td colspan="2">Internal Storage</td>
						</tr>
					</thead>
					<tr>
						<td width="50%" style="padding: 10px;">
						        <div class="bar-container" style="width:60%;">
							        <div id="nvram_bar" class="core-color-container"></div>
							</div>
							<div style="padding-top:5px;" id="nvram_label"><span>NVRAM Usage: </span></div>
						</td>
						<td width="50%" style="padding: 10px;">
							<div id="jffs_div" class="bar-container" style="width:60%;">
								<div id="jffs_bar" class="core-color-container"></div>
							</div>
							<div style="padding-top:5px;" id="jffs_label"><span>JFFS Usage: </span></div>
						</td>
					</tr>
				</table>

				<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
					<thead>
						<tr>
							<td colspan="2">Temperatures</td>
						</tr>
					</thead>
					<tr>
						<td colspan="2"><canvas style="background-color:#2f3e44;border-radius:10px;"id="tempchartId" height="200" width="700"></canvas></td>
					</tr>
					<tr>
						<th>Temperatures</th>
						<td id="temp_td"></td>
					</tr>

				</table>

				<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
					<thead>
						<tr>
							<td colspan="2">Network</td>
						</tr>
					</thead>
					<tr>
						<th>HW acceleration</th>
						<td id="hwaccel"></td>
					</tr>
					<tr>
						<th>Connections</th>
						<td id="conn_td"></td>
					</tr>
					<tr>
						<th>Wireless Clients (2.4 GHz)</th>
						<td id="wlc_24_td"></td>
					</tr>
					<tr id="wifi5_clients_tr" style="display:none;">
						<th id="wifi51_clients_th">Wireless Clients (5 GHz)</th>
						<td id="wlc_51_td"></td>
					</tr>
					<tr id="wifi5_2_clients_tr" style="display:none;">
						<th>Wireless Clients (5 GHz-2)</th>
						<td id="wlc_52_td"></td>
					</tr>
					<tr id="wifi6_clients_tr" style="display:none;">
						<th>Wireless Clients (6 GHz)</th>
						<td id="wlc_6_td"></td>
					</tr>
				</table>
				</td>
			</tr>
	        </tbody>
            </table>
            </form>
            </td>

       </tr>
      </table>
      <!--===================================Ending of Main Content===========================================-->
    </td>
    <td width="10" align="center" valign="top">&nbsp;</td>
  </tr>
</table>
<div id="footer"></div>
</body>
</html>

