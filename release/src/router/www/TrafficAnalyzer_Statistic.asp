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
<title><#Web_Title#> - <#EZQoS#></title>
<link rel="stylesheet" type="text/css" href="index_style.css">
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="usp_style.css">
<link rel="stylesheet" type="text/css" href="datepicker.css">
<script type="text/javascript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/calendar/jquery-ui.js"></script>
<script type="text/javascript" src="/chart.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/client_function.js"></script>
<script type="text/javascript" src="/switcherplugin/jquery.iphone-switch.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/js/httpApi.js"></script>
<script language="JavaScript" type="text/javascript" src="/js/asus_eula.js"></script>
<style>
#holder {
    height: 330px;
    left: 50%;
    width:400px;
}

#flow_holder {
    height: 250px;
    width: 700px;
	margin-left:20px;
	padding-bottom:10px;
}

#top_client_table tr:hover{
	background-color:#FFF;
	opacity:0.4;
	color:#000;
}

#top_client_table tr{
	line-height:15px;

}
.clean_log {
	float: right;
	background-image: url("images/New_ui/delete.svg");
	height: 26px;
	width: 26px;
	background-repeat: no-repeat;
	margin-right: 10px;
	cursor: pointer;
}
.clean_log:hover {
	background-image: url("images/New_ui/delete_hover.svg");
}
</style>
<script>

var flow_obj;
var pie_obj;
window.onresize = function(){
	cal_panel_block("client_all_info_block");
}

function initial(){
	show_menu();
	register_event();
	load_time();

	if(document.form.bwdpi_db_enable.value == 1){
		document.getElementById('statistic_hint').innerHTML = "* <#Traffic_Analyzer_refresh_note#>";
	}
	else{
		document.getElementById('statistic_hint').innerHTML = "* <#Traffic_Analyzer_note#>";
	}

	get_every_client_data("all", "detail", "24", date_second, date_string);		//get clients and find top 5 clients' traffic last 24 hours
	setTimeout(function(){
		get_wan_data("all", "hour", "24", date_second, date_string);
	}, 1000);

	$('#traffic_unit').val(getTrafficUnit());

	if(!ASUS_EULA.status("tm"))
		ASUS_EULA.config(eula_confirm, cancel);
}

var date_string = "";
var date_second = "";
function load_time(){
	var time = new Date();
	var hour = "",date = "", month = "", year = "";
	date_second = parseInt(time.getTime()/1000);
	hour = time.getHours();
	date = time.getDate();
	month = time.getMonth() + 1;
	year = time.getFullYear();
	if(month < 10)
		month = "0" + month.toString();

	if(date < 10)
		date = "0" + date.toString();

	date_string = year + "/" + month + "/" + date + "/" + hour;
	$('#datepicker').val(month + "/" + date + "/" + year);
}

var top5_client_array = new Array();
var top5_app_array = new Array();
var total_clients_array = new Array();
var total_apps_array = new Array();
var total_apps_rx = 0
var total_apps_tx = 0

function get_client_used_apps_info(client_index, used_data_array, top5_info, type, type_detail){
	var total_traffic = 0;
	var total_apps_traffic = 0;
	var total_apps_traffic_temp = 0;
	var traffic_unit = "";
	var code = "";
	var period = $("#duration_option").val();
	var description = "";

	if(type == "router"){
		document.getElementById('top_client_title').innerHTML = "<#Client_Name#>:";
		if(period == "monthly"){
			description = "<#traffic_analysis_top5client_monthly#>";
		}
		else if(period == "weekly"){
			description = "<#traffic_analysis_top5client_weekly#>";
		}
		else if(period == "daily"){
			description = "<#traffic_analysis_top5client_daily#>";
		}
	}
	else{
		document.getElementById('top_client_title').innerHTML = "App:";
		if(period == "monthly"){
			description = "<#traffic_analysis_top5app_monthly#>";
		}
		else if(period == "weekly"){
			description = "<#traffic_analysis_top5app_weekly#>";
		}
		else if(period == "daily"){
			description = "<#traffic_analysis_top5app_daily#>";
		}
	}

	document.getElementById('info_block_title').innerHTML = description;
	if(top5_info == ""){
		if(type == "router")
			document.getElementById('top_client_name').innerHTML = "<#traffic_analysis_noclients#>";
		else
			document.getElementById('top_client_name').innerHTML = "<#traffic_analysis_noapps#>";

		document.getElementById('top_client_traffic').innerHTML = "<#traffic_analysis_notraffic#>";
	}
	else{
		if(type_detail == "detail"){
			if(type == "router"){
				document.getElementById('top_client_name').innerHTML = total_clients_array[client_index].name;
				if(document.getElementById('traffic_option').value == "both"){
					total_traffic = total_clients_array[client_index].rx + total_clients_array[client_index].tx;
				}
				else if(document.getElementById('traffic_option').value == "down"){
					total_traffic = total_clients_array[client_index].rx;
				}
				else{
					total_traffic = total_clients_array[client_index].tx;
				}
			}
			else{
				if(client_index == undefined){
					client_index = "0";
				}
				
				document.getElementById('top_client_name').innerHTML = total_apps_array[client_index].name;
				if(document.getElementById('traffic_option').value == "both"){
					total_traffic = total_apps_array[client_index].rx + total_apps_array[client_index].tx;
				}
				else if(document.getElementById('traffic_option').value == "down"){
					total_traffic = total_apps_array[client_index].rx;
				}
				else{
					total_traffic = top5_info[client_index].tx;
				}
			}
		}
		else{
			document.getElementById('top_client_name').innerHTML = total_clients_array[client_index].name;
			if(document.getElementById('traffic_option').value == "both"){
				total_traffic = total_clients_array[client_index].rx + total_clients_array[client_index].tx;
			}
			else if(document.getElementById('traffic_option').value == "down"){
				total_traffic = total_clients_array[client_index].rx;
			}
			else{
				total_traffic = total_clients_array[client_index].tx;
			}
		}

		traffic_unit = translate_traffic(total_traffic);
		document.getElementById('top_client_traffic').innerHTML = traffic_unit[0] + " " + traffic_unit[1];
		var percent = 0;
		var app_traffic = 0;
		for(i=0;i<5;i++){
			if(used_data_array[i] == undefined){		//if the length of apps less than 5
				break;
			}

			if(document.getElementById('traffic_option').value == "both"){
				app_traffic = used_data_array[i][1] + used_data_array[i][2];
			}
			else if(document.getElementById('traffic_option').value == "down"){
				app_traffic = used_data_array[i][2];
			}
			else{
				app_traffic = used_data_array[i][1];
			}

			percent =  parseInt((app_traffic/total_traffic)*100);
			if(percent < 1)
				percent = 1;

			traffic_temp = translate_traffic(app_traffic);
			code += "<tr>";
			if(type == "router")
				code += "<td style='padding:5px 0px 0px 20px;font-size:14px;width:100px;cursor:pointer' onClick=\"show_individual_info_block(\'"+used_data_array[i][0]+"\', \'"+ type +"\')\">" + used_data_array[i][0] + "</td>";
			else{
				if(clientList[used_data_array[i][0]] != undefined)
					code += "<td style='padding:5px 0px 0px 20px;font-size:14px;width:100px;cursor:pointer' onClick=\"show_individual_info_block(\'"+used_data_array[i][0]+"\', \'"+ type +"\')\">" + getClientCurrentName(used_data_array[i][0]) + "</td>";
				else
					code += "<td style='padding:5px 0px 0px 20px;font-size:14px;width:100px;cursor:pointer' onClick=\"show_individual_info_block(\'"+used_data_array[i][0]+"\', \'"+ type +"\')\">" + used_data_array[i][0] + "</td>";
			}
			code += "<td style='width:100px;padding-top:5px;'>";
			code += "<div style='width:" + percent + "%;height:15px;background-color:#FFFFFF;opacity:0.5;'></div>";
			code += "</td>";
			code += "<td style='padding-left:10px;padding-top:5px;'><div style='width:45px;font-size:14px;text-align:right;'>" + traffic_temp[0] + "</div></td>";
			code += "<td><div style='font-size:14px;padding-top:5px;'> " + traffic_temp[1] + "</div></td>";
			code += "</tr>";
		}

		if(used_data_array.length > 5){
			code += "<tr>";
			code += "<td style='padding:5px 0px 0px 20px;font-size:14px;width:100px;cursor:pointer' onclick=\"show_all_info(\'"+client_index+"\', \'"+ type +"\')\">More...</td>";
			code += "</tr>";
		}
	}

	document.getElementById('top_client_table').innerHTML = code;
	setHover_css();
}

function show_individual_info_block(client_index, type){
	var dura = 0;
	if(document.getElementById('duration_option').value == "monthly")
		dura = 31;
	else if(document.getElementById('duration_option').value == "weekly")
		dura = 7;
	else
		dura = 24;

	if(type == "router"){		//get app used by every client
		get_app_used_by_client_data_individual(client_index, "detail", dura , date_second, date_string);
	}
	else{
		get_client_used_app_data_individual(client_index, "detail", dura , date_second, date_string)
	}
}

function getTrafficUnit(){
	var value = 9;
	if(cookie.get('ASUS_Traffic_unit')){
		value = cookie.get('ASUS_Traffic_unit');
	}

	return value;
}

function translate_traffic(flow){
	var flow_unit = "Bytes";
	var unit = getTrafficUnit();

	if(unit == '1'){
		flow_unit = "MB";
		flow = flow/1024/1024;
	}
	else if(unit == '2'){
		flow_unit = "GB";
		flow = flow/1024/1024/1024;
	}
	else if( unit == '3'){
		flow_unit = "TB";
		flow = flow/1024/1024/1024/1024;
	}
	else{
		if(flow > 1024){
			flow_unit = "KB";
			flow = flow/1024;
			if(flow > 1024){
				flow_unit = "MB";
				flow = flow/1024;
				if(flow > 1024){
					flow_unit = "GB";
					flow = flow/1024;
				}
			}
		}
	}

	return [flow.toFixed(2), flow_unit];
}

/*generate clients(apps) list option and draw Pie Chart*/
function get_client_info(list_info, type){
	var code = "";
	var match_flag = 0;
	var temp_array = new Array();
	if(type == "router")
		code = "<option value='all' selected><#traffic_analysis_allclients#></option>";
	else
		code = "<option value='all' selected><#traffic_analysis_allapps#></option>";
	top5_client_array = [];
	top5_app_array = [];

	for(i=0;i<list_info.length;i++){
		if(type == "router"){
			for(j=0;j<clientList.length;j++){
				if(all_client_traffic[i][0] == clientList[j]){
					match_flag = 1;
					clientList[clientList[j]].totalTx = all_client_traffic[i][1];
					clientList[clientList[j]].totalRx = all_client_traffic[i][2];
					break;
				}
			}

			if(match_flag == 1){
				var clientName = getClientCurrentName(all_client_traffic[i][0]);
				code += "<option value=" + all_client_traffic[i][0] + ">" + clientName + "</option>";
				if(i<6){
					top5_client_array[i] = all_client_traffic[i][0];
					top5_client_array[all_client_traffic[i][0]] = {
					"mac":all_client_traffic[i][0],
					"name":clientName,
					"tx":all_client_traffic[i][1],
					"rx":all_client_traffic[i][2]};
				}
			}
			else{
				code += "<option value=" + all_client_traffic[i][0] + ">" + all_client_traffic[i][0] + "</option>";
				if(i<6){
					top5_client_array[i] = all_client_traffic[i][0];
					top5_client_array[all_client_traffic[i][0]] = {
					"mac":all_client_traffic[i][0],
					"name":all_client_traffic[i][0],
					"tx":all_client_traffic[i][1],
					"rx":all_client_traffic[i][2]};
				}
			}

			match_flag = 0;

			total_clients_array[i] = all_client_traffic[i][0];
			total_clients_array[all_client_traffic[i][0]] = {
				"mac":all_client_traffic[i][0],
				"name":all_client_traffic[i][0],
				"tx":all_client_traffic[i][1],
				"rx":all_client_traffic[i][2]
			};
		}
		else{
			code += "<option value=" + all_app_traffic[i][0].replace(/\s/g, '_') + ">" + all_app_traffic[i][0] + "</option>";
			if(i<6){
				top5_app_array[i] = all_app_traffic[i][0];
				top5_app_array[all_app_traffic[i][0]] = {
					"name":all_app_traffic[i][0],
					"tx":all_app_traffic[i][1],
					"rx":all_app_traffic[i][2]};
			}

			total_apps_array[i] = all_app_traffic[i][0];
			total_apps_array[all_app_traffic[i][0]] = {
				"mac":all_app_traffic[i][0],
				"name":all_app_traffic[i][0],
				"tx":all_app_traffic[i][1],
				"rx":all_app_traffic[i][2]
			};
		}
	}

	if(type == "router")
		draw_pie_chart(list_info, top5_client_array, type);		//list_info : all_client_traffic
	else
		draw_pie_chart(list_info, top5_app_array, type);	//list_info : all_app_traffic

	document.getElementById('client_option').innerHTML = code;
}

var router_traffic_array = new Array();
function get_wan_data(client, mode, dura, time, date_string){
	$.ajax({
		url: '/getWanTraffic.asp?client=' + client + '&mode=' + mode + '&dura=' + dura + '&date=' + time,
		dataType: 'script',
		error: function(xhr){
			setTimeout("get_wan_data(client, mode, dura, time, date_string);", 1000);
		},
		success: function(response){
			router_traffic_array = array_statistics;
			draw_flow(date_string, array_statistics);
		}
	});
}

var all_client_traffic = new Array();
function get_every_client_data(client, mode, dura, time, date_string){
	$.ajax({
		url: '/getAppTraffic.asp?client=' + client + '&mode=' + mode + '&dura=' + dura + '&date=' + time,
		dataType: 'script',
		error: function(xhr){
			//setTimeout("get_app_data(time);", 1000);
		},
		success: function(response){
			all_client_traffic = array_statistics;

			if(document.getElementById('traffic_option').value == "both"){
				all_client_traffic.sort(function(a,b){
					return (b[1] + b[2]) - (a[1] + a[2]);
				})
			}
			else if(document.getElementById('traffic_option').value == "down"){
				all_client_traffic.sort(function(a,b){
					return b[2] - a[2];
				});
			}
			else if(document.getElementById('traffic_option').value == "up"){
				all_client_traffic.sort(function(a,b){
					return 	b[1] - a[1];
				});
			}

			get_client_info(all_client_traffic, "router");
			setTimeout(function(){get_client_used_app_data(top5_client_array[0], "detail", dura , date_second, date_string)}, 3000);
		}
	});
}

var all_app_traffic = new Array();
function get_every_app_data(client, mode, dura, time, date_string){
	$.ajax({
		url: '/getWanTraffic.asp?client=' + client + '&mode=' + mode + '&dura=' + dura + '&date=' + time,
		dataType: 'script',
		error: function(xhr){
			//setTimeout("get_app_data(time);", 1000);
		},
		success: function(response){
			all_app_traffic = array_statistics;

			if(document.getElementById('traffic_option').value == "both"){
				all_app_traffic.sort(function(a,b){
					return (b[1] + b[2]) - (a[1] + a[2]);
				})
			}
			else if(document.getElementById('traffic_option').value == "down"){
				all_app_traffic.sort(function(a,b){
					return b[2] - a[2];
				});
			}
			else if(document.getElementById('traffic_option').value == "up"){
				all_app_traffic.sort(function(a,b){
					return 	b[1] - a[1];
				});
			}

			get_client_info(all_app_traffic, "app");
			setTimeout(function(){get_app_used_by_client_data(top5_app_array[0], "detail", dura , date_second, date_string)}, 3000);
		}
	});
}

function get_app_data(client, mode, dura, time, date_string){
	$.ajax({
		url: '/getAppTraffic.asp?client=' + client + '&mode=' + mode + '&dura=' + dura + '&date=' + time,
		dataType: 'script',
		error: function(xhr){
			//setTimeout("get_app_data(time);", 1000);
		},
		success: function(response){
			all_client_traffic = array_statistics;
			draw_flow(date_string, array_statistics);
		}
	});
}

var client_used_app_array = new Array();
function get_client_used_app_data(client, mode, dura, time, date_string, type, info_type){
	$.ajax({
		url: '/getWanTraffic.asp?client=' + client + '&mode=' + mode + '&dura=' + dura + '&date=' + time,
		dataType: 'script',
		error: function(xhr){
			//setTimeout("get_app_data(time);", 1000);

		},
		success: function(response){
			client_used_app_array = array_statistics;

			if(document.getElementById('traffic_option').value == "both"){
				client_used_app_array.sort(function(a,b){
					return (b[1] + b[2]) - (a[1] + a[2]);		//decrease
				});
			}
			else if(document.getElementById('traffic_option').value == "down"){
				client_used_app_array.sort(function(a,b){
					return b[2] - a[2];
				});
			}
			else{
				client_used_app_array.sort(function(a,b){
					return 	b[1] - a[1];
				});
			}


			get_client_used_apps_info(client, client_used_app_array, top5_client_array, "router");
			if(info_type == "client")
				show_detail_info(client, client_used_app_array, "router");
		}
	});
}

function show_detail_info(mac, used_data_array, type){
	var code = "";
	var traffic_temp = new Array();
	code += '<table style="width:95%;margin-left:20px;">';
	code += '<tr style="font-size:14px;">';
	if(type == "router"){
		if(clientList[mac] == undefined)
			code += '<th colspan="4"><#Client_Name#>: '+ mac +'</th>';
		else
			code += '<th colspan="4"><#Client_Name#>: '+ getClientCurrentName(mac); +'</th>';
	}
	else{
		code += '<th colspan="4">App: '+ mac +'</th>';
	}

	code += '</tr>';
	code += '<tr style="font-size:13px;">';
	if(type == "router"){
		code += "<th style='width:55%;text-align:left;'><#traffic_analysis_appname#></th>";
	}
	else{
		code += '<th style="width:55%;text-align:left;"><#Client_Name#></th>';
	}

	code += '<th style="width:15%;text-align:right;"><#option_upload#></th>';
	code += '<th style="width:15%;text-align:right;"><#option_download#></th>';
	code += '<th style="width:15%;text-align:right;"><#Total#></th>';
	code += '</tr>';
	for(i=0;i<used_data_array.length;i++){
		code += '<tr>';
		if(type == "router"){
			code += '<td style="text-align:left;">' + used_data_array[i][0] + '</td>';
		}
		else{
			code += '<td style="text-align:left;">' + getClientCurrentName(used_data_array[i][0]) + '</td>';
		}

		traffic_temp = translate_traffic(used_data_array[i][1]);
		code += '<td style="text-align:right;">' + traffic_temp[0] + ' ' + traffic_temp[1] + '</td>';
		traffic_temp = translate_traffic(used_data_array[i][2]);
		code += '<td style="text-align:right;">' + traffic_temp[0] + ' ' + traffic_temp[1] + '</td>';
		traffic_temp = translate_traffic(used_data_array[i][1] + used_data_array[i][2]);
		code += '<td style="text-align:right;">' + traffic_temp[0] + ' ' + traffic_temp[1] + '</td>';
		code += '</tr>';
	}

	code += '</table>';
	document.getElementById('detail_info_block').innerHTML = code;
}

function show_all_info(mac, type){
	var code = "";
	cal_panel_block("client_all_info_block");
	document.getElementById('client_all_info_block').style.display = "";
	var traffic_temp = new Array();
	code += '<div style="overflow-y:auto;height:93%">';
	code += '<table style="width:95%;margin-left:20px;">';
	if(type == "router"){
		code += "<tr style='font-size:14px;'>";
		if(clientList[mac] == undefined)
			code += "<th colspan='4'><#Client_Name#>: "+ mac +"</th>";
		else
			code += "<th colspan='4'><#Client_Name#>: "+ getClientCurrentName(mac) +"</th>";

		code += "</tr>";
		code += '<tr style="font-size:13px;">';
		code += "<th style='width:55%;text-align:left;'><#traffic_analysis_appname#></th>";
		code += '<th style="width:15%;text-align:right;"><#option_upload#></th>';
		code += '<th style="width:15%;text-align:right;"><#option_download#></th>';
		code += '<th style="width:15%;text-align:right;"><#Total#></th>';
		code += '</tr>';


		for(i=0;i<client_used_app_array.length;i++){
			code += '<tr>';
			code += '<td style="text-align:left;">' + client_used_app_array[i][0] + '</td>';
			traffic_temp = translate_traffic(client_used_app_array[i][1]);
			code += '<td style="text-align:right;">' + traffic_temp[0] + ' ' + traffic_temp[1] +'</td>';
			traffic_temp = translate_traffic(client_used_app_array[i][2]);
			code += '<td style="text-align:right;">' + traffic_temp[0] + ' ' + traffic_temp[1] +'</td>';
			traffic_temp = translate_traffic(client_used_app_array[i][1] + client_used_app_array[i][2]);
			code += '<td style="text-align:right;">' + traffic_temp[0] + ' ' + traffic_temp[1] +'</td>';
			code += '</tr>';
		}
	}
	else{
		code += "<tr style='font-size:14px;'>";
		code += "<th colspan='4'><#traffic_analysis_appname#> : "+ mac +"</th>";
		code += "</tr>";
		code += '<tr style="font-size:13px;">';
		code += "<th style='width:55%;text-align:left;'><#Client_Name#></th>";
		code += '<th style="width:15%;text-align:right;"><#option_upload#></th>';
		code += '<th style="width:15%;text-align:right;"><#option_download#></th>';
		code += '<th style="width:15%;text-align:right;"><#Total#></th>';
		code += '</tr>';


		for(i=0;i<app_used_by_client_array.length;i++){
			code += '<tr>';
			if(clientList[app_used_by_client_array[i][0]] == undefined)
				code += '<td style="text-align:left;">' + app_used_by_client_array[i][0] + '</td>';
			else
				code += '<td style="text-align:left;">' + clientList[app_used_by_client_array[i][0]].name + '</td>';


			traffic_temp = translate_traffic(app_used_by_client_array[i][1]);
			code += '<td style="text-align:right;">' + traffic_temp[0] + ' ' + traffic_temp[1] +'</td>';
			traffic_temp = translate_traffic(app_used_by_client_array[i][2]);
			code += '<td style="text-align:right;">' + traffic_temp[0] + ' ' + traffic_temp[1] +'</td>';
			traffic_temp = translate_traffic(app_used_by_client_array[i][1] + app_used_by_client_array[i][2]);
			code += '<td style="text-align:right;">' + traffic_temp[0] + ' ' + traffic_temp[1] +'</td>';
			code += '</tr>';
		}
	}

	code += '</table>';
	code += '</div>';
	code += '<div style="text-align:center;margin-top:10px">';
	code += '<input name="button" type="button" class="button_gen" onclick="hide_show_all_info()" value="Cancel">';
	code += '</div>';

	document.getElementById('client_all_info_block').innerHTML = code;
}

function hide_show_all_info(){
	document.getElementById('client_all_info_block').style.display = "none";
}


var app_used_by_client_array = new Array();
function get_app_used_by_client_data(client, mode, dura, time, date_string, type, info_type, type_detail){
	$.ajax({
		url: '/getAppTraffic.asp?client=' + client + '&mode=' + mode + '&dura=' + dura + '&date=' + time,
		dataType: 'script',
		error: function(xhr){
			//setTimeout("get_app_data(time);", 1000);

		},
		success: function(response){
			app_used_by_client_array = array_statistics;
			if(document.getElementById('traffic_option').value == "both"){
				app_used_by_client_array.sort(function(a,b){
					return (b[1] + b[2]) - (a[1] + a[2]);		//decrease
				});
			}
			else if(document.getElementById('traffic_option').value == "down"){
				app_used_by_client_array.sort(function(a,b){
					return b[2] - a[2];
				});
			}
			else{
				app_used_by_client_array.sort(function(a,b){
					return 	b[1] - a[1];
				});
			}

			get_client_used_apps_info(client, app_used_by_client_array, total_apps_array, "app" , "detail");

			if(info_type == "app")
				show_detail_info(client, app_used_by_client_array, "app");
		}
	});
}

function get_client_used_app_data_individual(client, mode, dura, time, date_string){
	$.ajax({
		url: '/getWanTraffic.asp?client=' + client + '&mode=' + mode + '&dura=' + dura + '&date=' + time,
		dataType: 'script',
		error: function(xhr){
			//setTimeout("get_app_data(time);", 1000);

		},
		success: function(response){
			client_used_app_array = array_statistics;
			if(document.getElementById('traffic_option').value == "both"){
				client_used_app_array.sort(function(a,b){
					return (b[1] + b[2]) - (a[1] + a[2]);		//decrease
				});
			}
			else if(document.getElementById('traffic_option').value == "down"){
				client_used_app_array.sort(function(a,b){
					return b[2] - a[2];
				});
			}
			else{
				client_used_app_array.sort(function(a,b){
					return 	b[1] - a[1];
				});
			}


			show_all_info(client, "router");
		}
	});

}

function get_app_used_by_client_data_individual(client, mode, dura, time, date_string){
	$.ajax({
		url: '/getAppTraffic.asp?client=' + client + '&mode=' + mode + '&dura=' + dura + '&date=' + time,
		dataType: 'script',
		error: function(xhr){

		},
		success: function(response){
			app_used_by_client_array = array_statistics;
			if(document.getElementById('traffic_option').value == "both"){
				app_used_by_client_array.sort(function(a,b){
					return (b[1] + b[2]) - (a[1] + a[2]);		//decrease
				});
			}
			else if(document.getElementById('traffic_option').value == "down"){
				app_used_by_client_array.sort(function(a,b){
					return b[2] - a[2];
				});
			}
			else{
				app_used_by_client_array.sort(function(a,b){
					return 	b[1] - a[1];
				});
			}

			show_all_info(client, "app");
		}
	});


}

function register_event(){
	var hour = "",date = "", month = "", year = "";
	var time = 0;
	var duration = 0;
	var mode = "";
	var traffic_chart = document.getElementById('traffic_chart').getContext('2d');
    flow_obj = new Chart(traffic_chart);
	var pie= document.getElementById("pie_chart").getContext("2d");
	pie_obj = new Chart(pie);
	$( "#datepicker" ).datepicker({
		onSelect: function(){
			time = $( "#datepicker" ).datepicker('getDate');
			date_second = parseInt(time.getTime()/1000);
			hour = time.getHours();
			date = time.getDate();
			month = time.getMonth() + 1;
			year = time.getFullYear();
			if(month < 10)
				month = "0" + month.toString();

			if(date < 10)
				date = "0" + date.toString();

			date_string = year + "/" + month + "/" + date + "/" + hour;
			date_second += 86400; // shift 24 hour to make traffic chart correct
			if(document.getElementById('duration_option').value == "monthly"){
				duration = 31;
				mode = "day";
			}
			else if(document.getElementById('duration_option').value == "weekly"){
				duration = 7;
				mode = "day";
			}
			else{
				duration = 24;
				mode = "hour";
			}

			get_every_client_data("all", "detail", duration, date_second, date_string);
			setTimeout(function(){
				get_wan_data("all", mode, duration, date_second, date_string);
			}, 1500);

			document.getElementById('router').className = "block_filter_pressed";
			document.getElementById('apps').className = "block_filter";
			document.getElementById('graphical_info_block').style.display = "";
			document.getElementById('detail_info_block').style.display = "none";
			document.getElementById('top5_info_block').style.backgroundColor = color[0];
		}
	 });
}

function convertTime(t){
	JS_timeObj.setTime(t*1000);
	JS_timeObj2 = JS_timeObj.toString();
	JS_timeObj2 = JS_timeObj2.substring(0,3) + ", " +
	              JS_timeObj2.substring(4,10) + "  " +
				  parent.checkTime(JS_timeObj.getHours()) + ":" +
				  parent.checkTime(JS_timeObj.getMinutes()) + ":" +
				  parent.checkTime(JS_timeObj.getSeconds()) + "  " +
				  JS_timeObj.getFullYear();

	return JS_timeObj2;
}

function genClientListOption(){
	if(clientList.length == 0){
		setTimeout("genClientListOption();", 500);
		return false;
	}

	document.getElementById("clientListOption").options.length = 1;
	for(var i=0; i<clientList.length; i++){
		var clientObj = clientList[clientList[i]];

		if(clientObj.isGateway || !clientObj.isOnline)
			continue;

		var newItem = new Option(clientObj.name, clientObj.mac);
		document.getElementById("clientListOption").options.add(newItem);
	}
}

function switch_content(obj){
	if(obj.className == "block_filter_pressed")
		return true;

	document.getElementById('router').className = "block_filter";
	document.getElementById('apps').className = "block_filter";
	obj.className = "block_filter_pressed";

	var duration = 0;
	var period = document.getElementById('duration_option').value;
	var description = "";
	if(period == "monthly"){
		duration = 31;
	}
	else if(period == "weekly"){
		duration = 7;
	}
	else{
		duration = 24;
	}

	if(obj.id == "router"){
		get_every_client_data("all", "detail", duration, date_second, date_string);
		if(period == "monthly"){
			description = "<#traffic_analysis_top5client_monthly#>";
		}
		else if(period == "weekly"){
			description = "<#traffic_analysis_top5client_weekly#>";
		}
		else if(period == "daily"){
			description = "<#traffic_analysis_top5client_daily#>";
		}
	}
	else{
		get_every_app_data("all", "detail", duration, date_second, date_string);

		if(period == "monthly"){
			description = "<#traffic_analysis_top5app_monthly#>";
		}
		else if(period == "weekly"){
			description = "<#traffic_analysis_top5app_weekly#>";
		}
		else if(period == "daily"){
			description = "<#traffic_analysis_top5app_daily#>";
		}
	}

	document.getElementById('info_block_title').innerHTML = description;
	document.getElementById('graphical_info_block').style.display = "";
	document.getElementById('detail_info_block').style.display = "none";
	document.getElementById('top5_info_block').style.backgroundColor = color[0];
}

function switch_date_type(obj){
	var duration = "";
	var mode = "";
	var info_date = "";
	var info_type = "";

	if(obj.value == "monthly"){
		mode = "day";
		duration = "31";
		info_date = "<#diskUtility_monthly#>";
	}
	else if(obj.value == "weekly"){
		mode = "day";
		duration = "7";
		info_date = "<#diskUtility_weekly#>";
	}
	else{		//Daily
		mode = "hour";
		duration = "24";
		info_date = "<#diskUtility_daily#>";
	}

	if(document.getElementById('router').className == "block_filter_pressed"){
		info_type = "Clients";
	}
	else{
		info_type = "Apps";
	}

	if(info_type == "Clients")
		get_every_client_data("all", "detail", duration, date_second, date_string);
	else
		get_every_app_data("all", "detail", duration, date_second, date_string);

	get_wan_data("all", mode, duration, date_second, date_string);
	document.getElementById('graphical_info_block').style.display = "block";
	document.getElementById('detail_info_block').style.display = "none";
	document.getElementById('top5_info_block').style.backgroundColor = color[0];
}

function change_traffic_direction(obj){
	var temp = new Array();
	draw_flow(date_string, router_traffic_array);

	if(document.getElementById('router').className == "block_filter_pressed"){
		if(document.getElementById('traffic_option').value == "both"){
			all_client_traffic.sort(function(a,b){ return (b[1] + b[2]) - (a[1] + a[2]);})
			client_used_app_array.sort(function(a,b){ return (b[1] + b[2]) - (a[1] + a[2]);})
		}
		else if(document.getElementById('traffic_option').value == "down"){
			all_client_traffic.sort(function(a,b){ return b[2] - a[2];});
			client_used_app_array.sort(function(a,b){ return b[2] - a[2];});
		}
		else if(document.getElementById('traffic_option').value == "up"){
			all_client_traffic.sort(function(a,b){ return b[1] - a[1];});
			client_used_app_array.sort(function(a,b){ return b[1] - a[1];});
		}
		get_client_info(all_client_traffic, "router");
	}
	else{
		if(document.getElementById('traffic_option').value == "both"){
			all_app_traffic.sort(function(a,b){	return (b[1] + b[2]) - (a[1] + a[2]);})
			app_used_by_client_array.sort(function(a,b){ return (b[1] + b[2]) - (a[1] + a[2]);})
		}
		else if(document.getElementById('traffic_option').value == "down"){
			all_app_traffic.sort(function(a,b){	return b[2] - a[2];});
			app_used_by_client_array.sort(function(a,b){ return b[2] - a[2];});
		}
		else if(document.getElementById('traffic_option').value == "up"){
			all_app_traffic.sort(function(a,b){	return 	b[1] - a[1];});
			app_used_by_client_array.sort(function(a,b){ return	b[1] - a[1];});
		}

		get_client_info(all_app_traffic, "app");
	}

	if(document.getElementById("client_option").value == "all"){
		if(document.getElementById('router').className == "block_filter_pressed")
			get_client_used_apps_info(top5_client_array[0], client_used_app_array, top5_client_array, "router");
		else
			get_client_used_apps_info(top5_app_array[0], app_used_by_client_array, top5_app_array, "app");
	}
	else{
		if(document.getElementById('router').className == "block_filter_pressed")
			get_client_used_apps_info(document.getElementById("client_option").value, client_used_app_array, top5_client_array, "router");
		else
			get_client_used_apps_info(document.getElementById("client_option").value, app_used_by_client_array, top5_app_array, "app");
	}

	document.getElementById('top5_info_block').style.backgroundColor = color[0];
}

function change_client(mac){
	var duration = "";
	var mode = "";
	var info_type = "";

	if(document.getElementById('duration_option').value == "monthly"){
		mode = "day";
		duration = "31";
	}
	else if(document.getElementById('duration_option').value == "weekly"){
		mode = "day";
		duration = "7";
	}
	else{
		mode = "hour";
		duration = "24";
	}

	if(document.getElementById('router').className == "block_filter_pressed"){
		info_type = "Clients";
	}
	else{
		info_type = "Apps";
	}

	if(info_type == "Clients"){
		get_wan_data(mac, mode, duration, date_second, date_string);
		if(mac != "all"){
			get_client_used_app_data(mac, "detail", duration, date_second, date_string, "router", "client");
			document.getElementById('graphical_info_block').style.display = "none";
			document.getElementById('detail_info_block').style.display = "";
		}
		else{
			document.getElementById('graphical_info_block').style.display = "";
			document.getElementById('detail_info_block').style.display = "none";
		}
	}
	else{
		get_app_data(mac, mode, duration, date_second, date_string);
		if(mac != "all"){
			mac = mac.replace(/\_/g," ");
			get_app_used_by_client_data(mac, "detail", duration , date_second, date_string, "router", "app", "1");
			document.getElementById('graphical_info_block').style.display = "none";
			document.getElementById('detail_info_block').style.display = "";
		}
		else{
			document.getElementById('graphical_info_block').style.display = "";
			document.getElementById('detail_info_block').style.display = "none";
		}
	}
}

function change_top5_clients(index, type){
	var duration = "";
	document.getElementById('top5_info_block').style.backgroundColor = color[index];
	if(document.getElementById('duration_option').value == "monthly"){
		duration = "31";
	}
	else if(document.getElementById('duration_option').value == "weekly"){
		duration = "7";
	}
	else{
		duration = "24";
	}

	if(type == "router")
		get_client_used_app_data(top5_client_array[index], "detail", duration, date_second, date_string);
	else
		get_app_used_by_client_data(top5_app_array[index], "detail", duration , date_second, date_string);

}

var pieOptions = {
	segmentShowStroke : false,
	segmentStrokeColor : "#000",
	animationEasing : "easeOutQuart",
	animationSteps : 100,
	animateScale : true
}
var pieData = new Array();
/*var pieData = [
	{
		unit : "MB",
		label: "Jieming",
		value: 20,
		color:"#878BB6",
		percent: 35
	},
];*/
//var color = ["#878BB6","#4ACAB4","#FF8153","#FFEA88","#FF5151","#DCB5FF"];
 // 紅#b3645b  橙#b98f53 黃#c6b36a 綠#849e75 藍#2b6692 紫#7c637a
var color = ["#B3645B","#B98F53","#C6B36A","#849E75","#2B6692","#7C637A"];
function draw_pie_chart(list_info, top5_info, type){
	var percent = 0;
	var percent_others = 100;
	var client_traffic = 0;
	var client_traffic_others = 0;
	var total_client_traffic = 0;
	var client_traffic_display = new Array();
	var pieData = [];
	var code = "";

	for(i=0;i<list_info.length;i++){
		if(document.getElementById('traffic_option').value == "both"){
			total_client_traffic += list_info[i][1] + list_info[i][2];
		}
		else if(document.getElementById('traffic_option').value == "down"){
			total_client_traffic += list_info[i][2];
		}
		else{
			total_client_traffic += list_info[i][1];
		}
	}

	if(top5_info == ""){
		pieData = [{
			unit: "",
			label: "No Data",
			value: 0,
			color:"#B3645B",
			percent: 100,
			id: "0"
		}];

		code = '<div style="width:110px;word-wrap:break-word;padding-left:5px;background-color:#B3645B;margin-right:-10px;border-top-left-radius:10px;border-bottom-left-radius:10px;"><#traffic_analysis_noclients#></div>';
	}
	else{
		for(i=0;i<top5_info.length && i<6;i++){
			if(document.getElementById('traffic_option').value == "both"){
				if(i<5){
					client_traffic = list_info[i][1] + list_info[i][2];
				}
				else{
					client_traffic_others += list_info[i][1] + list_info[i][2];
				}
			}
			else if(document.getElementById('traffic_option').value == "down"){
				if(i<5){
					client_traffic = list_info[i][2];
				}
				else{
					client_traffic_others += list_info[i][2];
				}
			}
			else{
				if(i<5){
					client_traffic = list_info[i][1];
				}
				else{
					client_traffic_others += list_info[i][1];
				}
			}

			if(i<5){
				percent = parseInt((client_traffic/total_client_traffic)*100);
				percent_others -= percent;
			}
			else{
				percent = percent_others;
			}

			if(percent < 1)
				percent = 1;

			client_traffic_display = translate_traffic(client_traffic);
			if(i==5){
				var temp = {
					label: "Others",
					percent: percent,
					value: client_traffic_display[0],
					unit: client_traffic_display[1],
					color: color[i],
					id: top5_info[i]
				};
			}
			else{
				var temp = {
					label: top5_info[top5_info[i]].name,
					percent: percent,
					value: client_traffic_display[0],
					unit: client_traffic_display[1],
					color: color[i],
					id: top5_info[i]
				};
			}

			pieData.push(temp);
			if(i == 0)
				code += '<div onclick=\'change_top5_clients(\"'+i+'\",\"'+type+'\");\' style="width:110px;word-wrap:break-word;padding-left:5px;background-color:'+color[i]+';margin-right:-10px;border-top-left-radius:10px;line-height:30px;cursor:pointer">'+top5_info[top5_info[i]].name+'</div>';
			else if(i == 4)
				code += '<div onclick=\'change_top5_clients(\"'+i+'\",\"'+type+'\");\' style="width:110px;word-wrap:break-word;padding-left:5px;background-color:'+color[i]+';margin-right:-10px;border-bottom-left-radius:10px;line-height:30px;cursor:pointer">'+top5_info[top5_info[i]].name+'</div>';
			else if(i != 5)
				code += '<div onclick=\'change_top5_clients(\"'+i+'\",\"'+type+'\");\' style="width:110px;word-wrap:break-word;padding-left:5px;background-color:'+color[i]+';margin-right:-10px;line-height:30px;cursor:pointer">'+top5_info[top5_info[i]].name+'</div>';
		}
	}

	document.getElementById('top5_client_banner').innerHTML = code;
	if(pie_flag != undefined)
		pie_flag.destroy();

	pie_obj.Pie(pieData, pieOptions);
}


var flowData = {
	/*labels : ["January","February","March","April","May","June","July","Augest","Sepmpter","Octuber", "November","December"],
	datasets : [{
		fillColor : "rgba(172,194,132,0.4)",
		strokeColor : "#ACC26D",
		pointColor : "#fff",
		pointStrokeColor : "#9DB86D",
		data : [203,156,2,1251,205,247,132,100,50,1,100,1050],
		unit : ["MB","GB","KB","MB","GB","KB","MB","GB","KB","MB","GB","KB"]
	}]*/
}
var router_total_traffic = 0;
function draw_flow(date, traffic){
	var month_translate = ["", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"];
	var date_array = date.split('/');		// year, month, date, hour
	var labels = new Array();
	var traffic_value = new Array();
	var traffic_value_displayed = new Array();
	var traffic_unit = new Array();
	var traffic_temp = new Array();
	var hour_temp = date_array[3], date_temp = date_array[2], month_temp = date_array[1], year_temp = date_array[0];
	router_total_traffic = 0;
	if(document.getElementById('duration_option').value == "daily"){
		for(i=0;i<24;i++){
			if(traffic[i] == undefined)
				traffic[i] = [0, 0];

			labels.unshift(hour_temp + " h");
			if(document.getElementById('traffic_option').value == 'both'){
				traffic_value.push(traffic[i][0] + traffic[i][1]);
				traffic_temp = translate_traffic(traffic[i][0] + traffic[i][1]);
			}
			else if(document.getElementById('traffic_option').value == 'down'){
				traffic_value.push(traffic[i][1]);
				traffic_temp = translate_traffic(traffic[i][1]);
			}
			else{
				traffic_value.push(traffic[i][0]);
				traffic_temp = translate_traffic(traffic[i][0]);
			}

			traffic_value_displayed.push(traffic_temp[0]);
			traffic_unit.push(traffic_temp[1]);

			hour_temp--;
			if(hour_temp < 0){
				hour_temp = 23;
				date_temp--;
				if(date_temp == 0){
					month_temp--;
					if(month_temp == 0){
						month_temp = 12;
						year_temp--;
					}

					if(month_temp == 1 || month_temp == 3 || month_temp == 5 || month_temp == 7 || month_temp == 8 || month_temp == 10 || month_temp == 12){
						date_temp = 31;
					}
					else if(month_temp == 2){
						if((year_temp % 4 == 0 && year_temp % 100 != 0) || (year_temp % 400 == 0)){
							date_temp = 29;
						}
						else{
							date_temp = 28;
						}
					}
					else{
						date_temp = 30;
					}
				}
			}
		}
	}
	else if(document.getElementById('duration_option').value == "weekly"){
		for(i=0;i<7;i++){
			if(traffic[i] == undefined)
				traffic[i] = [0, 0];

			labels.unshift(month_temp + "/" + date_temp);
			if(document.getElementById('traffic_option').value == 'both'){
				traffic_value.push(traffic[i][0] + traffic[i][1]);
				traffic_temp = translate_traffic(traffic[i][0] + traffic[i][1]);
			}
			else if(document.getElementById('traffic_option').value == 'down'){
				traffic_value.push(traffic[i][1]);
				traffic_temp = translate_traffic(traffic[i][1]);
			}
			else{
				traffic_value.push(traffic[i][0]);
				traffic_temp = translate_traffic(traffic[i][0]);
			}

			traffic_value_displayed.push(traffic_temp[0]);
			traffic_unit.push(traffic_temp[1]);

			date_temp--;
			if(date_temp == 0){
				month_temp--;
				if(month_temp == 0){
					month_temp = 12;
					year_temp--;
				}

				if(month_temp == 1 || month_temp == 3 || month_temp == 5 || month_temp == 7 || month_temp == 8 || month_temp == 10 || month_temp == 12){
						date_temp = 31;
				}
				else if(month_temp == 2){
					if((year_temp % 4 == 0 && year_temp % 100 != 0) || (year_temp % 400 == 0)){
						date_temp = 29;
					}
					else{
						date_temp = 28;
					}
				}
				else{
					date_temp = 30;
				}
			}

		}

	}
	else{		//monthly, can merge with weekly
		for(i=0;i<31;i++){
			if(traffic[i] == undefined)
				traffic[i] = [0, 0];

			labels.unshift(month_temp + "/" + date_temp);
			if(document.getElementById('traffic_option').value == 'both'){
				traffic_value.push(traffic[i][0] + traffic[i][1]);
				traffic_temp = translate_traffic(traffic[i][0] + traffic[i][1]);
			}
			else if(document.getElementById('traffic_option').value == 'down'){
				traffic_value.push(traffic[i][1]);
				traffic_temp = translate_traffic(traffic[i][1]);
			}
			else{
				traffic_value.push(traffic[i][0]);
				traffic_temp = translate_traffic(traffic[i][0]);
			}

			traffic_value_displayed.push(traffic_temp[0]);
			traffic_unit.push(traffic_temp[1]);

			date_temp--;
			if(date_temp == 0){
				month_temp--;
				if(month_temp == 0){
					month_temp = 12;
					year_temp--;
				}

				if(month_temp == 1 || month_temp == 3 || month_temp == 5 || month_temp == 7 || month_temp == 8 || month_temp == 10 || month_temp == 12){
						date_temp = 31;
				}
				else if(month_temp == 2){
					if((year_temp % 4 == 0 && year_temp % 100 != 0) || (year_temp % 400 == 0)){
						date_temp = 29;
					}
					else{
						date_temp = 28;
					}
				}
				else{
					date_temp = 30;
				}
			}
		}
	}

	flowData = {
		labels: labels,
		datasets:[{
			fillColor : "#729EAF",
			strokeColor : "#095877",
			pointColor : "#9FDEF7",
			pointStrokeColor : "#095877",
			data: traffic_value,
			displayValue : traffic_value_displayed,
			unit: traffic_unit
		}]
	};

	if(flow_flag != undefined)
		flow_flag.destroy();

	flow_obj.Line(flowData)

	for(i=0;i<router_traffic_array.length;i++){
		if(document.getElementById('traffic_option').value == 'both'){
				router_total_traffic += router_traffic_array[i][0] + router_traffic_array[i][1];
			}
			else if(document.getElementById('traffic_option').value == 'down'){
				router_total_traffic += router_traffic_array[i][1];
			}
			else{
				router_total_traffic += router_traffic_array[i][0];
		}
	}

	traffic_temp = translate_traffic(router_total_traffic);
	document.getElementById('total_traffic_field').innerHTML = traffic_temp[0] + " " + traffic_temp[1];

	if(document.getElementById('duration_option').value == "monthly"){
		document.getElementById('total_traffic_title').innerHTML = "<#Traffic_Analyzer_monthly#>";
	}
	else if(document.getElementById('duration_option').value == "weekly"){
		document.getElementById('total_traffic_title').innerHTML = "<#Traffic_Analyzer_weekly#>";
	}
	else{		//daily
		document.getElementById('total_traffic_title').innerHTML = "<#Traffic_Analyzer_daily#>";
	}

	document.getElementById('current_traffic_title').innerHTML = "<#Traffic_Analyzer_current#>";
	document.getElementById('current_traffic_percent_title').innerHTML = "<#Traffic_Analyzer_usedpercent#>";
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

	document.getElementById(obj).style.marginLeft = blockmarginLeft+"px";
}

function eula_confirm(){
	document.form.TM_EULA.value = 1;
	document.form.bwdpi_db_enable.value = 1;
	document.form.action_wait.value = "15";
	applyRule();
}

function cancel(){
	curState = 0;
	$('#iphone_switch').animate({backgroundPosition: -37}, "slow", function() {});
	document.form.action_script.value = "";
	document.form.action_wait.value = "5";
}
function switch_control(_status){
	if(_status) {
		if(reset_wan_to_fo.check_status()) {
			if(ASUS_EULA.check("tm")){
				document.form.bwdpi_db_enable.value = 1;
				applyRule();
			}
		}
		else
			cancel();
	}
	else {
		document.form.bwdpi_db_enable.value = 0;
		applyRule();
	}
}
function applyRule(){
	document.form.action_script.value = "restart_wrs;restart_firewall";

	if(reset_wan_to_fo.change_status)
		reset_wan_to_fo.change_wan_mode(document.form);

	document.form.submit();
}

function setHover_css(){
	$('#top_client_table tr').hover(
		function(){		//onMouseOver
			if(this.children[1].children[0])
				this.children[1].children[0].style.backgroundColor = "#000";
		},
		function(){		//onMouseOut
			if(this.children[1].children[0])
				this.children[1].children[0].style.backgroundColor = "#FFF";
		}
	);
}

function getClientCurrentName(_mac) {
	var clientName = _mac;
	var clientObj = clientList[_mac];
	if(clientObj) {
		clientName = (clientObj.nickName == "") ? clientObj.name : clientObj.nickName;
	}
	return clientName;
}
function updateTrafficAnalyzer() {
	initial();
}

function setUnit(unit){
	cookie.set('ASUS_Traffic_unit', unit);
	draw_flow(date_string, router_traffic_array);
	get_client_used_apps_info(top5_client_array[0], client_used_app_array, top5_client_array, "router");
	$('#current_traffic_field').html('');
	$('#current_traffic_percent_field').html('');
}
</script>
</head>
<body onload="initial();" onunload="unload_body();">
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>
<div id="agreement_panel" class="eula_panel_container"></div>
<div id="hiddenMask" class="popup_bg" style="z-index:999;">
	<table cellpadding="5" cellspacing="0" id="dr_sweet_advise" class="dr_sweet_advise" align="center">
	</table>
	<!--[if lte IE 11]><iframe class="hackiframe"></iframe><![endif]-->
</div>
<iframe name="hidden_frame" id="hidden_frame" width="0" height="0" frameborder="0"></iframe>
<div id="client_all_info_block" class="analysis_bg" style="width:730px;height:700px;position:absolute;z-index:100;margin-left:235px;border-radius:10px;display:none"></div>
<form method="post" name="form" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="current_page" value="/TrafficAnalyzer_Statistic.asp">
<input type="hidden" name="next_page" value="/TrafficAnalyzer_Statistic.asp">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="action_wait" value="5">
<input type="hidden" name="flag" value="">
<input type="hidden" name="TM_EULA" value="<% nvram_get("TM_EULA"); %>">
<input type="hidden" name="bwdpi_db_enable" value="<% nvram_get("bwdpi_db_enable"); %>">
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
									<div style="margin-top:-20px;">
										<table width="100%">
											<tr>
												<td class="formfonttitle" align="left">
													<div><#Traffic_Analyzer#> - <#Statistic#></div>
												</td>
												<td>
													<div>
														<table align="right">
															<tr>
																<td >
																	<div align="center" class="left" style="width:94px; float:left; cursor:pointer;" id="traffic_analysis_enable"></div>
																	<script type="text/javascript">
																		$('#traffic_analysis_enable').iphoneSwitch('<% nvram_get("bwdpi_db_enable"); %>',
																			function(){
																				switch_control(1);
																			},
																			function(){
																				switch_control(0);
																			}
																		);
																	</script>
																</td>
															</tr>
														</table>
													</div>
												</td>
											</tr>
										</table>
									</div>
									<div style="margin:0 0 10px 5px;" class="splitLine"></div>
									<div class="formfontdesc"><#Traffic_Analyzer_desc#></div>
									<div style="margin-left:10px;">
										<label style="font-size:16px;"><#Statistic_last_date#>:</label>
										<input class="input_12_table" id="datepicker" value="">
										<div id="statistic_hint" style="text-align:right;margin-top:-21px;padding-right:15px;color:#FC0;font-size:14px;">* <#Traffic_Analyzer_note#></div>
									</div>
									<div style="margin:10px 0 10px 4px;">
										<div style="float:left;">
											<table>
												<tr>
													<td style="width:400px">
														<div>
															<table>
																<tr>
																	<td>
																		<div style="font-size:16px;"><#Statistic_display_type#>:</div>
																	</td>
																	<td>
																		<div id="router" style="width:100px;text-align:center;font-size:14px;border-radius:5px" class="block_filter_pressed" onclick="switch_content(this);">Clients</div>
																	</td>
																	<td>
																		<div id="apps" style="width:100px;text-align:center;font-size:14px;border-radius:5px" class="block_filter" onclick="switch_content(this);"><#Apps#></div>
																	</td>
																	<!--td>
																		<div id="details" style="width:80px;text-align:center;font-size:14px;border-radius:5px" class="block_filter" onclick="switch_content(this);">Details</div>
																	</td-->
																</tr>
															</table>
														</div>
													</td>
													<td>
														<div>
															<table>
																<tr>
																	<td>
																		<div style="font-size:16px;"><#Statistic_show_type#>:</div>
																	</td>
																	<td>
																		<select class="input_option" id="traffic_option" onChange="change_traffic_direction(this);">
																			<option value="both" selected><#option_both_direction#></option>
																			<option value="down"><#option_download#></option>
																			<option value="up"><#option_upload#></option>
																		</select>
																	</td>
																	<td>
																		<select class="input_option" id="duration_option" onChange="switch_date_type(this);">
																			<option value="monthly"><#diskUtility_monthly#></option>
																			<option value="weekly"><#diskUtility_weekly#></option>
																			<option value="daily" selected><#diskUtility_daily#></option>
																		</select>
																	</td>
																	<td>
																		<select class="input_option" id="traffic_unit" onChange="setUnit(this.value)">
																			<option value="1">MB</option>
																			<option value="2">GB</option>
																			<option value="3">TB</option>
																			<option value="9"><#Auto#></option>
																		</select>
																	</td>
																</tr>
															</table>
														</div>
													</td>
												</tr>
											</table>
										</div>
										<div class="clean_log" onClick="httpApi.cleanLog('traffic_analyzer', updateTrafficAnalyzer);"></div>
										<div style="clear:both;"></div>
									</div>
									<div class="analysis_bg" style="border-radius:4px;width:100%">
										<div style="padding-top:5px;">
											<table style="width:99%;">
												<tr>
													<td style="width:25%;text-align:center;font-size:16px;" id="current_traffic_percent_title"></td>
													<td style="width:25%;text-align:center;font-size:16px;" id="current_traffic_title"></td>
													<td style="width:25%;text-align:center;font-size:16px;" id="total_traffic_title"></td>
													<td rowspan="2"; style="width:25%;text-align:center">
														<select class="input_option" style="background-color#1C2B32;" id="client_option" onChange="change_client(this.value);"></select>
													</td>
												</tr>
												<tr>
													<td style="width:25%;text-align:center;font-size:16px;color:#FC0" id="current_traffic_percent_field"></td>
													<td style="width:25%;text-align:center;font-size:16px;color:#FC0" id="current_traffic_field"></td>
													<td style="width:25%;text-align:center;font-size:16px;color:#FC0" id="total_traffic_field"></td>
												</tr>
											</table>
										</div>
										<canvas id="traffic_chart" width="700px" height="300" style="padding-left:15px;"></canvas>
									</div>


									<div id="graphical_info_block" style="margin-top:10px;margin-right:-20px;">
										<table style="width:100%">
											<tr>
												<td>
													<canvas id="pie_chart" width="300" height="300"></canvas>
												</td>
												<td style="vertical-align:top;width:110px;" id="top5_client_banner">
													<div onclick="change_top5_clients(0);" style="width:100px;word-wrap:break-word;padding-left:5px;background-color:#B3645B;margin-right:-10px;border-top-left-radius:10px;border-bottom-left-radius:10px;"><#traffic_analysis_noclients#></div>
												</td>
												<td>
													<div id="top5_info_block" style="width:310px;min-height:330px;;background-color:#B3645B;border-bottom-right-radius:10px;border-bottom-left-radius:10px;border-top-right-radius:10px;box-shadow: 3px 5px 5px #2E3537;">
														<table style="width:99%;padding-top:20px">
															<tr>
																<th style="font-size:16px;text-align:left;padding-left:10px;width:140px;color:#ADADAD" id="top_client_title"><#ParentalCtrl_username#>:</th>
																<td style="font-size:14px;" id="top_client_name"></td>
															</tr>
															<tr>
																<th style="font-size:16px;text-align:left;padding-left:10px;width:140px;color:#ADADAD"><#Traffic_Analyzer_usedtraffic#>:</th>
																<td style="font-size:14px;" id="top_client_traffic"></td>
															</tr>
															<tr>
																<th style="font-size:16px;text-align:left;padding-left:10px;width:140px;color:#ADADAD"><#Traffic_Analyzer_TopApps#> :</th>
																<td></td>
															</tr>
														</table>
														<table style="width:99%;border-collapse:collapse;" id="top_client_table"></table>
													</div>
												</td>
											</tr>
											<tr>
												<td colspan="3">
													<div style="font-size:16px;color:#FC0;text-align:center;" id="info_block_title"><#traffic_analysis_top5client_daily#></div>
												</td>
											</td>
										</table>
									</div>

									<div id="detail_info_block" class="analysis_bg" style="border-radius:4px;width:100%;min-height:350px;margin-top:10px;overflow-y:auto;height:370px;display:none"></div>
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
<div id="footer"></div>
</body>
</html>
