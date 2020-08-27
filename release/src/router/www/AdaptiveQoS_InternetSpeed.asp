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
<title><#Web_Title#> - <#InternetSpeed#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" language="JavaScript" src="/help.js"></script>
<script type="text/javascript" language="JavaScript" src="/validator.js"></script>
<script type="text/javaScript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/js/httpApi.js"></script>
<style type="text/css">
.speedtest_logo{
	width: 185px;
	height: 40px;
	background: url(images/speedtest/speedtest.svg) no-repeat center;
	float: left;
}

.ookla_info{
	font-size: 12px;
	color: #7D96A7;
	margin-top: 5px;
}

.speedTest_div{
	background: #2C373D;
	border: 1px solid #364752;
	box-shadow: 0 2px 4px 0 rgba(0,0,0,0.20) , 0 1px 4px 0 rgba(60,60,60,0.30);
	border-radius: 5px;
	width: 750px;
}

.speedTest_result_div{
	height: 60px;
	background: #33434B;
	border-radius: 30px 0 0 30px;
	margin-left: 20px;
}

.date_div{
	width: 96px;
	height: 22px;
	border-bottom: solid #303D43 6px;
	font-size: 14px;
	text-align: right;
	color: #CDEBFF;
	float: left;
}

.download_upload_text{
	font-size: 12px;
	color: #A4B7C3;
	letter-spacing: 0.6px;
	line-height: 14px;
	margin-left:20px;
}

.speedTest_btn_normal{
	width:134px;
	height:124px;
	background-image:url('images/speedtest/btn_go_normal.svg');
	margin-left: 88px;
	text-align: center;
	line-height: 124px;
	font-size: 35px;
	color:#FFFFFF;
	cursor: pointer;
}

.speedTest_btn_press{
	width:134px;
	height:124px;
	background-image:url('images/speedtest/btn_go_press.svg');
	margin-left: 88px;
	text-align: center;
	line-height: 124px;
	font-size: 18px;
	color:#00FCFF;
}

.speedTest_btn_disable{
	width:134px;
	height:124px;
	background-image:url('images/speedtest/btn_go_disable.svg');
	margin-left: 88px;
	text-align: center;
	line-height: 124px;
	font-size: 35px;
	color:#FFFFFF;
}

.ping_result_sub_div{
	float: left;
	margin-left: 25px;
}

.speedTest_result_text{
	vertical-align: top;
	color: #A4B7C3;
}

.speed_level_ultra{
	width: 245px;
	height: 50px;
	background-image:url('images/speedtest/btn_speed_ultra.svg');
	cursor: pointer;
}

.speed_level_super{
	width: 245px;
	height: 50px;
	background-image:url('images/speedtest/btn_speed_super.svg');
	cursor: pointer;
}

.speed_level_great{
	width: 245px;
	height: 50px;
	background-image:url('images/speedtest/btn_speed_great.svg');
	cursor: pointer;
}

.speed_level_fast{
	width: 245px;
	height: 50px;
	background-image:url('images/speedtest/btn_speed_fast.svg');
	cursor: pointer;
}

.speed_level_good{
	width: 245px;
	height: 50px;
	background-image:url('images/speedtest/btn_speed_good.svg');
	cursor: pointer;
}

.speed_level_normal{
	width: 245px;
	height: 50px;
	background-image:url('images/speedtest/btn_speed_default.svg');
	cursor: pointer;
}

.speed_icon_white{
	margin-top: 8px;
	margin-left: 8px;
}

.speed_icon_ultra_white{
	float: left;
	width: 30px;
	height: 30px;
	background: url(images/speedtest/speed_ultra_white.svg) no-repeat center;
}

.speed_icon_super_white{
	float: left;
	width: 30px;
	height: 30px;
	background: url(images/speedtest/speed_super_white.svg) no-repeat center;
}

.speed_icon_great_white{
	float: left;
	width: 30px;
	height: 30px;
	background: url(images/speedtest/speed_great_white.svg) no-repeat center;
}

.speed_icon_fast_white{
	float: left;
	width: 30px;
	height: 30px;
	background: url(images/speedtest/speed_fast_white.svg) no-repeat center;
}

.speed_icon_good_white{
	float: left;
	width: 30px;
	height: 30px;
	background: url(images/speedtest/speed_good_white.svg) no-repeat center;
}

.speed_icon_normal_s{
	float: left;
	margin-top: 14px;
	margin-left: 8px;
	width: 18px;
	height: 18px;
	background: url(images/speedtest/speed_normal.svg) no-repeat center;
}

.speed_icon_normal_l{
	float: left;
	margin-top: 23px;
	margin-left: 30px;
	width: 24px;
	height: 24px;
	background: url(images/speedtest/speed_normal.svg) no-repeat center;
}

.speed_result_level_icon{
	float: left;
	width: 30px;
	height: 30px;
	margin-left: 30px;
	margin-top: 18px;
}

.speed_icon_ultra{
	background: url(images/speedtest/speed_ultra.svg) no-repeat center;
}

.speed_icon_super{
	background: url(images/speedtest/speed_super.svg) no-repeat center;
}

.speed_icon_great{
	background: url(images/speedtest/speed_great.svg) no-repeat center;
}

.speed_icon_fast{
	background: url(images/speedtest/speed_fast.svg) no-repeat center;
}

.speed_icon_good{
	background: url(images/speedtest/speed_good.svg) no-repeat center;
}

.speed_icon_normal{
	background: url(images/speedtest/speed_normal.svg) no-repeat center;
}

.speed_level_title{
	float: left; 
	margin-left: 27px;
	margin-top: 13px;
	font-size: 12px;
}

.speed_level_title_default{
	float: left; 
	margin-left: 20px;
	margin-top: 13px;
	font-size: 12px;
	width:170px;
}

.speed_level_text{
	float: left; 
	width: 80px;
	margin-left: 10px; 
	margin-top: 13px; 
	font-size: 16px;
}

.speed_level_more{
	float: left;
	margin-top: 19px;
	margin-left: 8px;
	width: 30px;
	height: 10px;
	background: url(images/speedtest/more.svg) no-repeat center;
}

.history_icon{
	position: absolute;
	left: 5px;
	bottom: 8px;
	width: 30px;
	height: 30px;
	background: url(images/speedtest/speedtest_history.svg) no-repeat center;
}

.history_delete{
	width: 32px;
	height: 32px;
	background: url(images/speedtest/speedtest_delete.svg) no-repeat center;
	background-size: 50%;
	position: relative;
	top: calc(50% - 16px);
	left: 15px;
	float: left;
	cursor: pointer;
}

.history_delete:hover{
	background: rgba(37,151,255,0.10) url(images/speedtest/speedtest_delete_enable.svg) no-repeat center;
	background-size: 50%;
	border-radius: 50%;
}

.left_arrow{
	width: 32px;
	height: 32px;
	background: url(images/speedtest/speedtest_arrow_left.svg) no-repeat center;
	background-size: 50%;
	position: relative;
	top: calc(50% - 16px);
	right: 10px;
	float: right;
	cursor: pointer;
}

.left_arrow:hover{
	border-radius: 50%;
	background: rgba(255,255,255,0.04) url(images/speedtest/speedtest_arrow_left_white.svg) no-repeat center;
	background-size: 50%;
}

.right_arrow{
	width: 32px;
	height: 32px;
	background: url(images/speedtest/speedtest_arrow_right.svg) no-repeat center;
	background-size: 50%;
	position: relative;
	top: calc(50% - 16px);
	float: right;
	margin-left: 10px;
	margin-right: 15px;
	cursor: pointer;
}

.right_arrow:hover{
	border-radius: 50%;
	background: rgba(255,255,255,0.04) url(images/speedtest/speedtest_arrow_right_white.svg) no-repeat center;
	background-size: 50%;
}

.history_item_range{
	height: 24px;
	position: relative;
	top: calc(50% - 12px);
	right: 19px;
	float: right;
	font-size: 14px;
	line-height: 24px;
}

.history_select{
	width: 12px;
	height: 12px;
	border: rgba(255,255,255,0.3) solid 1px;
	position: relative;
	top: calc(50% - 6px);
	left: 24px;
	cursor:pointer;
}

.history_select_checked{
	background: #2597FF url(images/speedtest/checked_24px.svg) no-repeat center;
	background-size: 90%;
}

.history_select:hover{
	border: 1px solid #FFFFFF;
}

.history_row{
	width: 750px;
	height: 48px; 
	border-bottom: #4C5B65 solid 1px;
}

.history_title{
	color:#A4B7C3;
	margin-top: -10px;
}

.no_test_result{
	position: relative;
	height: 48px;
	top: 15px;
	left: calc(50% - 80px);
	font-size: 18px;
	color: #7F91A4;
}

.history_timestamp_div{
	width: 115px;
	margin-left: 65px;
	margin-top: -9px;
	float: left;
}

.history_downup_div{
	width: 135px;
	float: left;
	text-align: center;
}

.history_downup_font{
	color: #7ECAFF;
}

.history_ping_div{
	width: 100px;
	float: left;
	text-align: center;
}

.detail_speed_level{
	width: 800px;
	height: 525px;
	background-color: #2E3B43;
	border-radius: 5px;
	position: fixed; 
	right: calc(50% - 400px);
	top: calc(50% - 260px);
	z-index: 10000;
	display: none;
	box-shadow: 0 2px 4px 0 rgba(0,0,0,0.20) , 0 1px 4px 0 rgba(60,60,60,0.30);	
}

.detail_title{
	margin-top: 50px;
	font-size: 24px;
	line-height: 120%;
	border-bottom: #4C5B65 solid 1px;
}

.detail_last_test_div{
	height: 130px;
	border-bottom: #4C5B65 solid 1px;
}

.speedtest_result_div_small{
	width: 50%;
	float:left;
}


.speed_level_range{
	width: 100%;
	height: 75px;
	line-height: 75px;
	text-align: center;
}

.speed_level_rainbow_bar{
	background: #dc00b9; /* Old browsers */
	background: -moz-linear-gradient(top,  #dc00b9 0%, #2e00d4 20%, #0051b1 40%, #3f93fb 60%, #31ffaf 80%, #2c3f4a 100%, #2c3f4a 100%); /* FF3.6-15 */
	background: -webkit-linear-gradient(top,  #dc00b9 0%,#2e00d4 20%,#0051b1 40%,#3f93fb 60%,#31ffaf 80%,#2c3f4a 100%,#2c3f4a 100%); /* Chrome10-25,Safari5.1-6 */
	background: linear-gradient(to bottom,  #dc00b9 0%,#2e00d4 20%,#0051b1 40%,#3f93fb 60%,#31ffaf 80%,#2c3f4a 100%,#2c3f4a 100%); /* W3C, IE10+, FF16+, Chrome26+, Opera12+, Safari7+ */
	filter: progid:DXImageTransform.Microsoft.gradient( startColorstr='#dc00b9', endColorstr='#2c3f4a',GradientType=0 ); /* IE6-9 */
}

.speed_level_close{
	display: inline-block;
	height: 26px; 
	width: 26px;
	top: calc(50% - 13px);
	left: 92%;
	position: relative;
}

.close_icon{
	background: url(images/speedtest/close.svg) no-repeat center;
	background-size: 50%;
	background-color: #2E3B43;
	border-radius: 50%;
}

.close_icon:hover{
	background: url(images/speedtest/close_hover.svg) no-repeat center;
	background-size: 50%;
	background-color: #2E3B43;
}

.detail_level_desc_div{
	height: 236px;
	border-bottom: #4C5B65 solid 1px;
}

.speed_level_text2{
	font-size: 24px;
	margin-top: 25px;
	float: left;
}

.level_desc_title{
	font-size: 14px;
	font-weight: bold;
}

.speed_level_desc_div{
	width: 90%;
	height: 75px;
	border-bottom: #4C5B65 solid 1px;
	float: left;
}

.speed_level_desc_icon{
	width: 10%;
	height: 18px;
	float: left;
	margin-top: 3px;
	background-size: contain;
}

.speed_level_desc_level{
	width: 14%;
	height: 64px;
	line-height: 110%;
	margin-top: 3px;
	float: left;
	overflow-wrap: break-word;
}

.speed_level_desc_detail{
	width: 86%;
	float: left;
}
.speed_level_desc_title{
	margin: 3px 0 3px 3px;
	line-height: 110%;
}

.speed_level_desc_font{
	color: #A4B7C3; 
	line-height: 120%;
	margin-left: 3px;
}

.speed_level_mask{
	position:fixed;
	margin: auto;
	top: 0;
	left: 0;
	width:100%;
	z-index:9999;
	background: rgba(19,24,32,0.80);
	filter:alpha(opacity=94);  /*IE5、IE5.5、IE6、IE7*/
	opacity: .94;  /*Opera9.0+、Firefox1.5+、Safari、Chrome*/
	background-repeat: repeat;
	display:none;
	overflow:hidden;
}

.alert_notification{
	width: 500px;
	height: 200px;
	background-color: #4C5B65;
	opacity: 0.95;
	border-radius: 4px;
	position: fixed;
	right: calc(50% - 250px);
	top: calc(50% - 100px);
	z-index: 10001;
	box-shadow: 0 8px 10px 1px rgba(0,0,0,0.20) , 0 1px 4px 0 rgba(60,60,60,0.30);
	display: none;
}

.notice_icon{
	width: 20px;
	height: 20px;
	background: url(images/speedtest/alert_bell.svg) no-repeat center;
	background-size: contain;
	float: left;
}

.alert_header{
	height: 20px;
	font-size: 20px;
	color: #FFFFFF;
	letter-spacing: 0.27px;
	float: left;
	margin-left: 10px;
}

.alert_text_noImg{
	width: 80%;
	margin-left: 60px;
	clear: both;
	font-size: 14px;
	color: #C9D1D7;
	float: left;
}

.alert_text_withImg{
	width: 70%;
	margin-left: 60px;
	margin-right: 20px;
	clear: both;
	font-size: 14px;
	color: #C9D1D7;
	float: left;
}

.alert_notification_close{
	height: 26px;
	width: 26px;
	top: 10px;
	left: 92%;
	position: relative;
}

.wan_disconn{
	width: 45px;
	height: 45px;
	margin-top: 10px;
	background: url(images/speedtest/wan_connect_trouble.svg) no-repeat center;
	background-size: contain;
	float: left;
}

.speedTest_btn_normal_20{
	font-size: 20px;
}

.speedTest_btn_normal_25{
	font-size: 25px;
}
</style>

<script>
var speedTest_result = [];
var speedTest_history = [];
var speedTest_servers = [];
var speedTest_history_str = "";
var delete_array = [];
var page_id = 0;
var range_start_num = 0;
var range_end_num = 0;
var max_page_id = 0;
var ookla_state = httpApi.nvramGet(["ookla_state"], true).ookla_state;
var test_start_time = httpApi.nvramGet(["ookla_start_time"], true).ookla_state;
var get_result_time = 0;
var test_timeout = 60000;
var go_btn_type = "go";
var ookla_states = {
		"IDLE": "0",
		"RUN": "1", 
		"ERR_DISCON": "2",
		"ERR_TIMEOUT": "3",
		"ERR_TERMINATE": "4",
		"ERR_UNKNOWN": "5"
};
var preferred_lang = httpApi.nvramGet(["preferred_lang"], true).preferred_lang;

if(rog_support){
	$("<link>")
	.attr({ rel: "stylesheet",
	type: "text/css",
	href: "css/internetSpeed_customize.css"
	})
	.appendTo("head");
}

function customize_speedTestBtn(){
	if($("#speedTest_btn").hasClass("speedTest_btn_normal")){
		if(preferred_lang == "UK" || preferred_lang == "SV")
			$("#speedTest_btn").addClass("speedTest_btn_normal_20");
		else if(preferred_lang == "RU" || preferred_lang == "NL" || preferred_lang == "RO")
			$("#speedTest_btn").addClass("speedTest_btn_normal_25");
	}
}

function customize_result_title(){
	if(preferred_lang == "TH"){
		var ping_result_sub_div = document.querySelectorAll('.ping_result_sub_div');
		for(var i = 0; i < ping_result_sub_div.length; i++){
			ping_result_sub_div[i].style.marginLeft = "15px";
		}
	}
}

function init(){
	show_menu();
	customize_speedTestBtn();
	customize_result_title();
	get_speedTest_history();
	show_history_results();

	if(ookla_state == "1"){
		setTimeout('continue_show_result();', 100);
	}
	else if(speedTest_history.length > 1)
		show_latest_record();

	check_wan_status();
}

function continue_show_result(){
	if(wanConnectStatus){
		reset_speed_level_btn(); 
		change_speedTestBtn("testing");
		get_speedTest_result();
	}
	else
		set_ookla_state(ookla_states.ERR_TERMINATE);
}

function check_wan_status(){
	if(go_btn_type != "testing"){
		if(!wanConnectStatus && go_btn_type != "disable"){
			change_speedTestBtn("disable");
			$("#notification_str").show();
			set_ookla_state(ookla_states.ERR_DISCON);
		}
		else if(wanConnectStatus && go_btn_type != "go"){
			change_speedTestBtn("go");
			$("#notification_str").hide();
		}

		setTimeout("check_wan_status();", 1000);
	}
}

function convert_to_localtime(gmt_time){
	var milliseconds = new Date(gmt_time).getTime();
	var timezone_offset = parseInt(timezone.slice(1,3))*60*60*1000;
	var target_time_ms = 0;
	var convert_time = { "date": "", "time": "", "ampm": "" };

	if(timezone.slice(0, 1) == "+"){
		target_time_ms = milliseconds + timezone_offset;
	}
	else if(timezone.slice(0, 1) == "-"){
		target_time_ms = milliseconds - timezone_offset;
	}

	var target_time  = new Date(target_time_ms);
	var year = target_time.getUTCFullYear();
	var month = target_time.getUTCMonth() + 1;
	var date = target_time.getUTCDate();
	var hours = target_time.getUTCHours();
	var target_hours = (hours < 10) ? '0' + hours : hours;
	var minutes = target_time.getUTCMinutes();
	var target_min = (minutes < 10) ? '0' + minutes : minutes;


	var target_date = year + "/" + month + '/' + date;
	var target_time = target_hours + ":" + target_min;

	convert_time.date = target_date;
	convert_time.time = target_time;
	return convert_time;
}

function get_speedTest_servers(){
	speedTest_servers = httpApi.hookGet("ookla_speedtest_get_servers", true);
	if(speedTest_servers.length < 2)
		setTimeout("get_speedTest_servers();", 1000);
}

function change_speedTestBtn(type){
	if(type == "go"){
		$("#speedTest_btn").removeClass().addClass("speedTest_btn_normal");
		$("#speedTest_btn").text("<#InternetSpeed_Go#>");
		$("#speedTest_btn").css("pointer-events", "auto");
		$("#speed_level_div").css("pointer-events", "auto");
		go_btn_type = "go";
	}
	else if(type == "testing"){
		$("#speedTest_btn").removeClass().addClass("speedTest_btn_press");
		$("#speedTest_btn").text("<#InternetSpeed_Testing#>");
		$("#speedTest_btn").css("pointer-events", "none");
		$("#speed_level_div").css("pointer-events", "none");
		go_btn_type = "testing";
	}
	else if(type == "disable"){
		$("#speedTest_btn").removeClass().addClass("speedTest_btn_disable");
		$("#speedTest_btn").css("pointer-events", "none");
		go_btn_type = "disable";
	}
	customize_speedTestBtn();
}

function convert_speedTestHistory_to_str(){
	var tmp_str = "";
	speedTest_history_str = "";

	for(var i = 0; i < speedTest_history.length -1 ; i++){
		tmp_str = JSON.stringify(speedTest_history[i]);
		speedTest_history_str += tmp_str + '\n';
	}
}

function reset_speed_level_btn(){
	$("#speed_level_div").removeClass().addClass("speed_level_normal");	
	$("#speed_level_title").removeClass().addClass("speed_level_title_default");
	$("#speed_level_text").text("");
	$("#speed_level_text").removeClass();
	$("#speed_level_icon").removeClass();

}

function do_speedTest_exe(exe_type, server_id){
	var start = new Date();
	var type = "";
	var id = "";

	test_start_time =  start.getTime();
	set_ookla_start_time(test_start_time);
	if(typeof exe_type !== "undefined")
		type = exe_type;

	if(typeof server_id !== "undefined")
		id = server_id;

	$.ajax({
		url: "/ookla_speedtest_exe.cgi",
		type: "POST",
		data: {
			"type": type,
			"id": id
		},
		error: function(){
			console.log("do_speedTest_exe error");
		},		
		success: function() {
			if(type == "")
				setTimeout("get_speedTest_result();", 1000);
			else if(type == "list")
				setTimeout("get_speedTest_servers();", 1000);
		}
	});
}

function reset_speedTest_div(){
	$("#download_test_result").text("--");
	$("#upload_test_result").text("--");
	$("#ping_result_latency").text("--");
	$("#ping_result_jitter").text("--");
	$("#packet_loss_result").text("--");
	$("#date_div").text("");
	$("#time_div").text("");
	reset_speed_level_btn();
}

function start_speedTest(){
	reset_speedTest_div();
	change_speedTestBtn("testing");
	do_speedTest_exe();
}


function save_speedTest_history(){
	convert_speedTestHistory_to_str();
	$.ajax({
		url: "/ookla_speedtest_write_history.cgi",
		type: "POST",
		data: {
			"speedTest_history": speedTest_history_str
		},
		success: function( response ) {
			setTimeout("get_speedTest_history();", 1000);
		}
	})
}

function get_speedTest_result(){
	speedTest_result = httpApi.hookGet("ookla_speedtest_get_result", true);
	var result_time = {};
	var now = new Date();
	var get_next = true;

	get_result_time = now.getTime();
	if(get_result_time - test_start_time >= test_timeout){
		error_status = 1;
		error_handling(ookla_states.ERR_TIMEOUT);
		return;
	}

	if(speedTest_result.length < 2){
		setTimeout("get_speedTest_result();", 100);
	}
	else{
		if(typeof speedTest_result[speedTest_result.length - 2].error != "undefined"){
			error_handling(ookla_states.ERR_TERMINATE);
			return;
		}

		for(var i = 0; i < speedTest_result.length - 1; i++){
			if(speedTest_result[i].type == "testStart"){
				result_time = convert_to_localtime(speedTest_result[i].timestamp);
				$("#date_div").text(result_time.date);
				$("#time_div").text(result_time.time);
			}
			else if(speedTest_result[i].type == "download"){
				var download_result = convert_Bps_to_Mbps(speedTest_result[i].download.bandwidth);
				$("#download_test_result").text(download_result);
			}
			else if(speedTest_result[i].type == "upload"){
				var upload_result = convert_Bps_to_Mbps(speedTest_result[i].upload.bandwidth);
				$("#upload_test_result").text(upload_result);
			}
			else if(speedTest_result[i].type == "result"){
				get_next = false;
				change_speedTestBtn("go");
				check_wan_status();
				result_time = convert_to_localtime(speedTest_result[i].timestamp);
				$("#date_div").text(result_time.date);
				$("#time_div").text(result_time.time);
				$("#ping_result_latency").text(speedTest_result[i].ping.latency.toFixed(2));
				$("#ping_result_jitter").text(speedTest_result[i].ping.jitter.toFixed(2));
				$("#download_test_result2").text(convert_Bps_to_Mbps(speedTest_result[i].download.bandwidth));
				$("#upload_test_result2").text(convert_Bps_to_Mbps(speedTest_result[i].upload.bandwidth));
				if(typeof speedTest_result[i].packetLoss != "undefined")
					$("#packet_loss_result").text(speedTest_result[i].packetLoss.toFixed(2));
				else
					$("#packet_loss_result").text("0.00");
				change_speed_level(convert_Bps_to_Mbps(speedTest_result[i].download.bandwidth));

				$("#speedtest_result_timestamp").text(result_time.date+" "+result_time.time);

				if(speedTest_history.length == 101){
					speedTest_history.splice(99, 1);
				}
				speedTest_history.splice(0, 0, speedTest_result[i]);
				update_history_inform();
				show_history_results();
				save_speedTest_history();
			}
		}

		if(get_next)
			setTimeout("get_speedTest_result();", 100);
	}
}

function error_handling(error_status){//2: wan disconnected 3: timeout  4: error  5:unknown
	var error_msg = "<#InternetSpeed_Failure_Hint#>"
	set_ookla_state(error_status);
	reset_speedTest_div();
	change_speedTestBtn("go");
	check_wan_status();
	show_alert_message(error_msg);
}

function convert_Bps_to_Mbps(bytes){
	var Mbps;

	Mbps = (bytes * 8) / (1000 * 1000);
	return Mbps.toFixed(2);
}

function get_speedTest_history(){
	speedTest_history = httpApi.hookGet("ookla_speedtest_get_history", true);
	if(speedTest_history.length == 0)
		speedTest_history = [{}];

	update_history_inform();
}

function change_speed_level(download_speed){
	var speed = parseInt(download_speed);

	$("#speed_level_title").removeClass().addClass("speed_level_title");
	$("#speed_level_text").removeClass().addClass("speed_level_text");

	if(speed < 6){
		$("#speed_level_text").text("<#InternetSpeed_Normal#>");
		$("#speed_level_icon").removeClass().addClass("speed_icon_normal_s");
		$("#speed_level_div").removeClass().addClass("speed_level_normal");
		$("#speed_level_text2").text("<#InternetSpeed_Normal#>");
		$("#speed_level_icon2").removeClass().addClass("speed_icon_normal_l");
		$("#level_desc_title").text("<#InternetSpeed_Normal_title#>");
		$("#level_desc").text("<#InternetSpeed_Normal_desc#>");
	}
	else if(speed >= 6 && speed <= 50){
		$("#speed_level_text").text("<#InternetSpeed_Good#>");
		$("#speed_level_icon").removeClass().addClass("speed_icon_white speed_icon_good_white");
		$("#speed_level_div").removeClass().addClass("speed_level_good");
		$("#speed_level_text2").text("<#InternetSpeed_Good#>");
		$("#speed_level_icon2").removeClass().addClass("speed_result_level_icon speed_icon_good");
		$("#level_desc_title").text("<#InternetSpeed_Good_title#>");
		$("#level_desc").text("<#InternetSpeed_Good_desc#>");
	}
	else if(speed >= 51 && speed <= 100){
		$("#speed_level_text").text("<#InternetSpeed_Great#>");
		$("#speed_level_icon").removeClass().addClass("speed_icon_white speed_icon_great_white");
		$("#speed_level_div").removeClass().addClass("speed_level_great");
		$("#speed_level_text2").text("<#InternetSpeed_Great#>");
		$("#speed_level_icon2").removeClass().addClass("speed_result_level_icon speed_icon_great");
		$("#level_desc_title").text("<#InternetSpeed_Great_title#>");
		$("#level_desc").text("<#InternetSpeed_Great_detail#>");
	}
	else if(speed >= 101 && speed <= 250){
		$("#speed_level_text").text("<#InternetSpeed_Fast#>");
		$("#speed_level_icon").removeClass().addClass("speed_icon_white speed_icon_fast_white");
		$("#speed_level_div").removeClass().addClass("speed_level_fast");
		$("#speed_level_text2").text("<#InternetSpeed_Fast#>");
		$("#speed_level_icon2").removeClass().addClass("speed_result_level_icon speed_icon_fast");
		$("#level_desc_title").text("<#InternetSpeed_Fast_title#>");
		$("#level_desc").text("<#InternetSpeed_Fast_detail#>");
	}
	else if(speed >= 251 && speed <= 900){
		$("#speed_level_text").text("<#InternetSpeed_Super#>");
		$("#speed_level_icon").removeClass().addClass("speed_icon_white speed_icon_super_white");
		$("#speed_level_div").removeClass().addClass("speed_level_super");
		$("#speed_level_text2").text("<#InternetSpeed_Super#>");
		$("#speed_level_icon2").removeClass().addClass("speed_result_level_icon speed_icon_super");
		$("#level_desc_title").text("<#InternetSpeed_Super_title#>");
		$("#level_desc").text("<#InternetSpeed_Super_detail#>");
	}
	else if(speed >= 901){
		$("#speed_level_text").text("<#InternetSpeed_Ultra#>");
		$("#speed_level_icon").removeClass().addClass("speed_icon_white speed_icon_ultra_white");
		$("#speed_level_div").removeClass().addClass("speed_level_ultra");
		$("#speed_level_text2").text("<#InternetSpeed_Ultra#>");
		$("#speed_level_icon2").removeClass().addClass("speed_result_level_icon speed_icon_ultra");
		$("#level_desc_title").text("<#InternetSpeed_Ultar_title#>");
		$("#level_desc").text("<#InternetSpeed_Ultra_detail#>");
	}
}

function show_history_results(){
	var i = 0;

	$("#speedtest_records").empty();
	$("#speedtest_records").css("height", "auto");
	if(speedTest_history.length < 2){
		$("<div>")
			.attr({"id": "no_test_result_row"})
			.addClass("no_test_result")
			.html("No Test Results.")
			.appendTo($("#speedtest_records"));
	}
	else{
		for(i = (range_start_num - 1) ; i < range_end_num; i++){
			add_speedTest_history(speedTest_history[i], "last");
		}
	}

	if($("#speedtest_records").height() < 270)
		$("#speedtest_records").css("height", "270px");
	else
		$("#speedtest_records").css("height", "auto");
}

function add_speedTest_history(history_obj, type){//type: 'last', 'first'
	var record_time = convert_to_localtime(history_obj.timestamp);
	var parentObj = $("#speedtest_records");
	var recordObj = $("<div>").addClass("history_row");

	if($("#speedtest_records").find("#no_test_result_row")[0] != undefined)
		$("#no_test_result_row").remove();

	(type == "last")? recordObj.appendTo(parentObj): recordObj.prependTo(parentObj);
	var selectObj = 
			$("<div>")
			.attr({"id": history_obj.result.id})
			.addClass("history_select")
			.appendTo(recordObj);

	selectObj.click(
		function(){
			if(selectObj.hasClass("history_select_checked")){
				for(var i = 0; i < delete_array.length; i++){
					if(delete_array[i] == this.id){
						delete_array.splice(i, 1);
					}
				}
			}
			else{
				delete_array.push(this.id);
			}
			selectObj.toggleClass("history_select_checked");
		});

	var timestampObj = 
			$("<div>")
			.addClass("history_timestamp_div")
			.appendTo(recordObj);

			$("<div>")
			.html(record_time.date)
			.appendTo(timestampObj);

			$("<div>")
			.html(record_time.time)
			.appendTo(timestampObj);

	var downloadObj =
			$("<div>")
			.addClass("history_downup_div history_downup_font")
			.html(convert_Bps_to_Mbps(history_obj.download.bandwidth))
			.appendTo(recordObj);

	var uploadObj =
			$("<div>")
			.addClass("history_downup_div history_downup_font")
			.html(convert_Bps_to_Mbps(history_obj.upload.bandwidth))
			.appendTo(recordObj);

	var latencyObj =
			$("<div>")
			.addClass("history_ping_div")
			.html(history_obj.ping.latency.toFixed(2))
			.appendTo(recordObj);

	var jitterObj =
			$("<div>")
			.addClass("history_ping_div")
			.html(history_obj.ping.jitter.toFixed(2))
			.appendTo(recordObj);

	var packetloss = 0;
	if(typeof history_obj.packetLoss != "undefined")
		packetloss = history_obj.packetLoss.toFixed(2);

	var packetLossObj = 
			$("<div>")
			.addClass("history_ping_div")
			.html(packetloss)
			.appendTo(recordObj);
}

function delete_history(type){
	var save_history = 0;
	var history_tmp = [];

	if(type == "all"){
		speedTest_history.length = 0;
		speedTest_history = [{}];
		save_history = 1;
	}
	else{
		if(delete_array.length == 0)
			return;

		for(var i = 0; i < delete_array.length; i++){
			for(var j = 0; j < speedTest_history.length - 1; j++){
				if(delete_array[i] == speedTest_history[j].result.id.toString()){
					speedTest_history.splice(j, 1);
					save_history = 1;
					break;
				}
			}
		}
	}

	delete_array.length = 0;
	if($("#history_select_all").hasClass("history_select_checked"))
		$("#history_select_all").removeClass("history_select_checked");

	if(save_history)
		save_speedTest_history();

	update_history_inform();
	show_history_results();

}

function select_all(){
	if($("#history_select_all").hasClass("history_select_checked") == false){
		$("#history_select_all").addClass("history_select_checked");
		for(var i = range_start_num - 1; i < range_end_num; i++){
			var selectObj = $("#"+speedTest_history[i].result.id);
			if(selectObj.hasClass("history_select_checked") == false){
				selectObj.addClass("history_select_checked");
				delete_array.push(speedTest_history[i].result.id.toString());
			}
		}
	}
	else{
		$("#history_select_all").removeClass("history_select_checked");
		for(var i = range_start_num - 1; i < range_end_num; i++){
			var selectObj = $("#"+speedTest_history[i].result.id);
			if(selectObj.hasClass("history_select_checked")){
				selectObj.removeClass("history_select_checked");
				for(var j = 0; j < delete_array.length; j++){
					if(delete_array[j] == speedTest_history[i].result.id.toString()){
						delete_array.splice(j, 1);
						break;
					}
				}
			}
		}
	}
}

function update_history_inform(){
	max_page_id = (speedTest_history.length <= 11)? 0 : Math.ceil((speedTest_history.length - 1)/10) - 1;
	range_start_num = page_id*10 + 1;
	range_end_num = Math.min((range_start_num + 9), (speedTest_history.length - 1));

	if(speedTest_history.length > 1){
		var range_str = range_start_num + '-' + range_end_num;
		$("#item_range_str").text(range_str);
		$("#history_item_range").css("display", "block");
	}
	else{
		$("#history_item_range").css("display", "none");
	}
}

function next_history_page(){
	if(page_id < max_page_id){
		page_id++;
		update_history_inform();
		$("#speedtest_records").empty();
		show_history_results();
		if($("#history_select_all").hasClass("history_select_checked"))
			$("#history_select_all").removeClass("history_select_checked");
	}
}

function previous_history_page(){
	if(page_id > 0){
		page_id--;
		update_history_inform();
		$("#speedtest_records").empty();
		show_history_results();
		if($("#history_select_all").hasClass("history_select_checked"))
			$("#history_select_all").removeClass("history_select_checked");
	}
}

function show_speed_level_detail(){
	$("#hiddenMask").css("height", $("#FormTitle").height());
	$("#hiddenMask").fadeIn(300);
	$("#detail_speed_level").fadeIn(300);
}

function close_speed_level_detail(){
	$("#detail_speed_level").fadeOut(300);
	$("#hiddenMask").fadeOut(300);
}

function show_latest_record(){
	var latest_record = speedTest_history[0];
	var result_time = convert_to_localtime(latest_record.timestamp);
	var download_result = convert_Bps_to_Mbps(latest_record.download.bandwidth);
	var upload_result = convert_Bps_to_Mbps(latest_record.upload.bandwidth);	

	$("#date_div").text(result_time.date);
	$("#time_div").text(result_time.time);
	$("#download_test_result").text(download_result);
	$("#download_test_result2").text(download_result);
	$("#upload_test_result").text(upload_result);
	$("#upload_test_result2").text(upload_result);
	$("#ping_result_latency").text(latest_record.ping.latency.toFixed(2));
	$("#ping_result_jitter").text(latest_record.ping.jitter.toFixed(2));
	if(typeof latest_record.packetLoss != "undefined")
		$("#packet_loss_result").text(latest_record.packetLoss.toFixed(2));
	else
		$("#packet_loss_result").text("0.00");
	change_speed_level(convert_Bps_to_Mbps(latest_record.download.bandwidth));
}

function set_ookla_state(state){
	$.ajax({
		url: "/set_ookla_speedtest_state.cgi",
		type: "POST",
		data: {
			"ookla_state": state
		},
		error: function(){
		},
		success: function() {
		}
	});
}

function show_alert_message(msg){
	if (typeof msg == "undefined") {
		var tmp_str = "<#InternetSpeed_WAN_Discon_Hint#>";
		$("#alert_str").removeClass().addClass("alert_text_withImg");
		$("#alert_str").html(tmp_str);
		$("#alert_img").removeClass().addClass("wan_disconn");
	}
	else{
		$("#alert_str").removeClass().addClass("alert_text_noImg");
		$("#alert_str").html(msg);
		$("#alert_img").removeClass();
	}

	$("#hiddenMask").css("height", $("#FormTitle").height());
	$("#hiddenMask").fadeIn(300);
	$("#alert_div").fadeIn(300);
}

function hide_alert_message(){
	$("#alert_div").fadeOut(300);
	$("#hiddenMask").fadeOut(300);
}

function set_ookla_start_time(time){
	$.ajax({
		url: "/set_ookla_speedtest_start_time.cgi",
		type: "POST",
		data: {
			"ookla_start_time": time
		},
		error: function(){
		},
		success: function(){
		}
	});
}
</script>
</head>

<body onload="init();" onunLoad="return unload_body();" class="bg">
<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>
<div id="hiddenMask" class="speed_level_mask"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="current_page" value="AdaptiveQoS_InternetSpeed.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">

<table class="content" align="center" cellpadding="0" cellspacing="0">
	<tr>
		<td width="17">&nbsp;</td>
		
		<td valign="top" width="202">
		<div  id="mainMenu"></div>
		<div  id="subMenu"></div>
		</td>
		
	<td valign="top">
		<div id="tabMenu" class="submenuBlock"></div>
		<!--===================================Beginning of Main Content===========================================-->
<table width="98%" border="0" align="left" cellpadding="0" cellspacing="0">
	<tr>
		<td align="left" valign="top" >

		<table width="760px" border="0" cellpadding="5" cellspacing="0" class="FormTitle" id="FormTitle">
			<tr>
				<td bgcolor="#4D595D" valign="top"  >
					<div>&nbsp;</div>
					<div class="formfonttitle"><#InternetSpeed#></div>
					<div style="margin:10px 0 10px 5px;" class="splitLine"></div>
					<div class="formfontdesc">
						<div style="width: 70%; float: left; margin-right: 15px; margin-bottom: 15px;"><#InternetSpeed_desc#></a>
						</div>
						<div class="speedtest_logo"></div>
						<div id="notification_str" style="clear: both; color: rgb(255, 204, 0); display: none;">
							<div><#InternetSpeed_Pause#></div>
							<div onclick="show_alert_message();" style="text-decoration:underline; cursor: pointer;"><#InternetSpeed_More_Info#></div>
						</div>
					</div>
					<div style="clear: both;">
						<div class="speedTest_div">
							<div style="height: 50px; margin-top: 15px;">
								<div id="date_div" class="date_div"></div>
								<div id="time_div" style="font-size: 30px; color: #CDEBFF; margin-top: 5px; margin-left: 5px; float: left;"></div>
							</div>
							<div style="height: 130px;">
								<div id="speedTest_btn" class="speedTest_btn_normal" style="float:left; margin-top: -5px;" onclick ="start_speedTest();"><#InternetSpeed_Go#></div>
								<div style="width:225px; height:124px; float:left; margin-left: 35px; margin-top: 20px;">
									<div style="height: 15px;">
										<div style="background-image: url(images/speedtest/download.svg); background-repeat: no-repeat; background-position: left; background-size: 11px 11px;">
											<span class="download_upload_text"><#InternetSpeed_Download#></span>
										</div>
									</div>
									<div id="download_test_result" style="height: 72px; font-size: 54px; color: #FFFFFF; line-height: 72px; margin-left: 20px;">--</div>
									<div style="height: 15px; margin-left: 22px; color:#B2BBC1"><#InternetSpeed_Mbps#></div>
								</div>
								<div style="width: 225px; height:124px; float:left;  margin-top: 20px;">
									<div style="height: 15px;">
										<div style="background-image: url(images/speedtest/upload.svg); background-repeat: no-repeat; background-position: left; background-size: 11px 11px;">
											<span class="download_upload_text"><#InternetSpeed_Upload#></span>
										</div>
									</div>
									<div id="upload_test_result" style="height: 72px; font-size: 54px; color: #FFFFFF; line-height: 72px; margin-left: 20px;">--</div>
									<div style="height: 15px; margin-left: 22px; color:#B2BBC1"><#InternetSpeed_Mbps#></div> 
								</div>
							</div>
							<div align="center" class="speedTest_result_div">
								<div id="speed_level_div" class="speed_level_normal" style="margin-left: 20px; margin-top: -8px; float: left;" onclick="show_speed_level_detail();">
									<div id="speed_level_title" class="speed_level_title_default"><#InternetSpeed_Level#></div>
									<div id="speed_level_text"></div>
									<div id="speed_level_icon"></div>
									<div class="speed_level_more"></div>
								</div>
								<div id="ping_result_div" style="height: 40px;  width: 430px; margin-left: 30px; margin-top: 6px; float: left; font-size: 12px;">
									<div style="float: left;">
										<span class="speedTest_result_text"><#InternetSpeed_Ping#></span><span id="ping_result_latency" style="margin-left: 10px; font-size: 24px;">--</span><span style="margin-left: 5px;"><#InternetSpeed_ms#></span>
									</div>
									<div class="ping_result_sub_div">
										<span class="speedTest_result_text"><#InternetSpeed_Jitter#></span><span id="ping_result_jitter" style="margin-left: 10px; font-size: 24px;">--</span><span style="margin-left: 5px;"><#InternetSpeed_ms#></span>
									</div>
									<div class="ping_result_sub_div">
										<span class="speedTest_result_text"><#InternetSpeed_Loss#></span><span id="packet_loss_result" style="margin-left: 10px; font-size: 24px;">--</span><span style="margin-left: 5px;">%</span>
									</div>
								</div>
							</div>
						</div>
						<div class="ookla_info"><#InternetSpeed_ookla_mark#></div>
						<div style="width: 750px; height: 72px; font-size: 20px; position:relative;"><div class="history_icon"></div><div style="position:absolute; bottom:12px; left:45px;"><#InternetSpeed_History#></div></div>
						<div  id="speedTest_history_div"  class="speedTest_div">
							<div class="history_row">
								<div class="history_delete" onclick="delete_history();"></div>
								<div class="right_arrow" onclick="next_history_page();"></div>
								<div class="left_arrow" onclick="previous_history_page();"></div>
								<div id="history_item_range" class="history_item_range"><span id="item_range_str"></span>&nbsp;<#InternetSpeed_Items#></div>
							</div>
							<div class="history_row">
								<div id="history_select_all" class="history_select" onclick="select_all();"></div>
								<div class="history_timestamp_div history_title" style="margin-top: 0px;"><#diskUtility_time#></div>
								<div class="history_downup_div history_title" style="color:#A4B7C3">
									<div style="background-image: url(images/speedtest/download.svg); background-repeat: no-repeat; background-position: left; background-size: 11px 11px;"><#InternetSpeed_Download#></div>
									<div>(<#InternetSpeed_Mbps#>)</div>
								</div>
								<div class="history_downup_div history_title">
									<div style="background-image: url(images/speedtest/upload.svg); background-repeat: no-repeat; background-position: left; background-size: 11px 11px;"><#InternetSpeed_Upload#></div>
									<div>(<#InternetSpeed_Mbps#>)</div>
								</div>
								<div class="history_ping_div history_title">
									<div><#InternetSpeed_Ping#></div>
									<div>(<#InternetSpeed_ms#>)</div>
								</div>
								<div class="history_ping_div history_title">
									<div><#InternetSpeed_Jitter#></div>
									<div>(<#InternetSpeed_ms#>)</div>
								</div>
								<div class="history_ping_div history_title">
									<div><#InternetSpeed_Loss#></div>
									<div>(%)</div>
								</div>
							</div>
							<div id="speedtest_records">
							</div>
						</div>
						<div id="detail_speed_level" class="detail_speed_level">
							<div style="width: 35%; height: 525px;  margin-left: 24px; float:left;">
								<div class="detail_title"><#InternetSpeed_LastTest#></div>
								<div class="detail_last_test_div">
									<div id="speedtest_result_timestamp" style="height: 36px; font-size: 18px; margin-top: 15px;"></div>
									<div class="speedtest_result_div_small">
										<div style="height: 15px;">
											<div style="background-image: url(images/speedtest/download.svg); background-repeat: no-repeat; background-position: left; background-size: 11px 11px;">
												<span class="download_upload_text"><#InternetSpeed_Download#></span>
											</div>
										</div>
										<div id="download_test_result2" style="height: 48px; font-size: 36px; color: #FFFFFF; line-height: 48px; margin-left: 20px;">--</div>
										<div style="height: 15px; margin-left: 22px; color:#B2BBC1"><#InternetSpeed_Mbps#></div>
									</div>
									<div class="speedtest_result_div_small">
										<div style="height: 15px;">
											<div style="background-image: url(images/speedtest/upload.svg); background-repeat: no-repeat; background-position: left; background-size: 11px 11px;">
												<span class="download_upload_text"><#InternetSpeed_Upload#></span>
											</div>
										</div>
										<div id="upload_test_result2" style="height: 48px; font-size: 36px; color: #FFFFFF; line-height: 48px; margin-left: 20px;">--</div>
										<div style="height: 15px; margin-left: 22px; color:#B2BBC1"><#InternetSpeed_Mbps#></div> 
									</div>
								</div>
								<div class="detail_level_desc_div">
									<div style="color: #A4B7C3; font-size: 12px; margin-top: 20px;"><#InternetSpeed_downloadSpeedRating#></div>
									<div style="height: 70px;">
										<div id="speed_level_text2" class="speed_level_text2"></div>
										<div id="speed_level_icon2"></div>
									</div>
									<div>
										<div id="level_desc_title" class="level_desc_title"></div>
										<div id="level_desc" style="font-size: 12px;"></div>
									</div>
								</div>
							</div>
							<div style="width: 62%; height: 525px; background-color: #171616; border-radius: 0 5px 5px 0; float:left;">
								<div style="height: 45px; width: 100%;"><div class="close_icon speed_level_close" onclick="close_speed_level_detail();"></div></div>
								<div style="width: 97%; height: 445px;">
									<div style="width: 15%; height: 450px; float: left;">
										<div style="width: 90%; float: left;">
											<div class="speed_level_range">901+</div>
											<div class="speed_level_range">251 - 900</div>
											<div class="speed_level_range">101 - 250</div>
											<div class="speed_level_range">51 - 100</div>
											<div class="speed_level_range">6 - 50</div>
											<div class="speed_level_range">0 - 5</div>
										</div>
										<div class="speed_level_rainbow_bar" style="width: 4px; height: 445px; border-radius: 5px; margin-left: 90%;"></div>
										<div style="text-align: right;"><#InternetSpeed_Mbps#></div>
									</div>
									<div style="width: 85%;  height: 450px; float: left;">
										<div style="height: 65px;">
											<div class="speed_level_desc_icon speed_icon_ultra"></div>
											<div class="speed_level_desc_div" style="border-top: #4C5B65 solid 1px;">
												<div class="speed_level_desc_level"><#InternetSpeed_Ultra#></div>
												<div class="speed_level_desc_detail">
													<div class="speed_level_desc_title"><#InternetSpeed_Ultar_title#></div>
													<div class="speed_level_desc_font"><#InternetSpeed_Ultra_detail#></div>
												</div>
											</div>
										</div>
										<div style="height: 65px;">
											<div class="speed_level_desc_icon speed_icon_super"></div>
											<div class="speed_level_desc_div">
												<div class="speed_level_desc_level"><#InternetSpeed_Super#></div>
												<div class="speed_level_desc_detail">
													<div class="speed_level_desc_title"><#InternetSpeed_Super_title#></div>
													<div class="speed_level_desc_font"><#InternetSpeed_Super_detail#></div>
												</div>
											</div>
										</div>
										<div style="height: 65px;">
											<div class="speed_level_desc_icon speed_icon_fast"></div>
											<div class="speed_level_desc_div">
												<div class="speed_level_desc_level"><#InternetSpeed_Fast#></div>
												<div class="speed_level_desc_detail">
													<div class="speed_level_desc_title"><#InternetSpeed_Fast_title#></div>
													<div class="speed_level_desc_font"><#InternetSpeed_Fast_detail#></div>
												</div>
											</div>
										</div>
										<div style="height: 65px;">
											<div class="speed_level_desc_icon speed_icon_great"></div>
											<div class="speed_level_desc_div">
												<div class="speed_level_desc_level"><#InternetSpeed_Great#></div>
												<div class="speed_level_desc_detail">
													<div class="speed_level_desc_title"><#InternetSpeed_Great_title#></div>
													<div class="speed_level_desc_font"><#InternetSpeed_Great_detail#></div>
												</div>
											</div>
										</div>
										<div style="height: 65px;">
											<div class="speed_level_desc_icon speed_icon_good"></div>
											<div class="speed_level_desc_div">
												<div class="speed_level_desc_level"><#InternetSpeed_Good#></div>
												<div class="speed_level_desc_detail">
													<div class="speed_level_desc_title"><#InternetSpeed_Good_title#></div>
													<div class="speed_level_desc_font"><#InternetSpeed_Good_desc#></div>
												</div>
											</div>
										</div>
										<div style="height: 65px;">
											<div class="speed_level_desc_icon speed_icon_normal"></div>
											<div class="speed_level_desc_div">
												<div class="speed_level_desc_level"><#InternetSpeed_Normal#></div>
												<div class="speed_level_desc_detail">
													<div class="speed_level_desc_title"><#InternetSpeed_Normal_title#></div>
													<div class="speed_level_desc_font"><#InternetSpeed_Normal_desc#></div>
												</div>
											</div>
										</div>
									</div>
								</div>
							</div>
						</div>
						<div id="alert_div" class="alert_notification">
							<div>
								<div class="close_icon alert_notification_close" onclick='hide_alert_message();'></div>
								<div style="margin-top: 25px; margin-left: 30px;">
									<div class="notice_icon"></div>
									<div class="alert_header"><#InternetSpeed_Notice#></div>
								</div>
								<div>
								<div id="alert_str" class="alert_notification_text"></div>
									<div id="alert_img" class=""></div>
								</div>
							</div>
							<div></div>
						</div>
					</div>
				</td>
			</tr>
		</table>
	  </td>
</form>
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
