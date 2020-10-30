<!DOCTYPE html>
<html>
<head>
	<meta charset="utf-8">
	<meta http-equiv="X-UA-Compatible" content="IE=11;IE=Edge">
	<meta http-equiv="Pragma" CONTENT="no-cache">
	<meta http-equiv="Expires" CONTENT="-1">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<link rel="shortcut icon" href="images/favicon.png">
	<title><#menu1#> - <#Status_Str#></title>
	<link rel="stylesheet" href="../NM_style.css" type="text/css">
	<link rel="stylesheet" href="../form_style.css" type="text/css">
	<link rel="stylesheet" href="../css/networkMap.css" type="text/css">
	<script src="../js/jquery.js" type="text/javascript"></script>
	<script src="../js/httpApi.js" type="text/javascript"></script>
	<script src="../state.js" type="text/javascript"></script>
	<script src="../js/device.js" type="text/javascript"></script>
	<script src="../calendar/jquery-ui.js"  type="text/javascript"></script>
<script>

/*Initialize array*/
var cpu_info_old = new Array();
var core_num = '<%cpu_core_num();%>';
var array_size = 46;
var cpu_usage_array = new Array();
var ram_usage_array = new Array();

var last_rx = 0;
var last_tx = 0;
var current_rx = 0;
var current_tx = 0;
var qos_enable = "<% nvram_get("qos_enable"); %>";
var qos_ibw = "<% nvram_get("qos_ibw"); %>";
var qos_obw = "<% nvram_get("qos_obw"); %>";
if (qos_enable > 0 && qos_ibw > 0 && qos_obw > 0) {
	max_rx = qos_ibw * 1024 / 8;
	max_tx = qos_obw * 1024 / 8;
} else {
	var max_rx = 100 * 1024;
	var max_tx = 100 * 1024;
}


var color_table = ["#c6dafc", "#7baaf7", "#4285f4", "#3367d6"];
var led_table = ["<#btn_disable#>", "<#Priority_Level_3#>", "<#Medium#>", "<#High#>"];
$(document).ready(function(){
	if(system.INTELplatform){
		register_event();
		var ledLv = httpApi.nvramGet(["bc_ledLv"]).bc_ledLv;
		translated_value = Math.round(100*(ledLv/3));
		setTimeout(function(){
			document.getElementById('slider').children[0].style.width = translated_value + "%";
			document.getElementById('slider').children[1].style.left = translated_value + "%";
		}, 10);
		$("#color_pad").html(led_table[ledLv]);
		$("#slider .ui-slider-range").css("background-color", color_table[ledLv]);
		$("#slider .ui-slider-handle").css("border-color", color_table[ledLv]);
		$('#led_field').show();
	}

	getVariable();
	genElement();

	initiailzeParameter();
	genCPUElement();
	genRAMElement();
	genNETelement();
	get_ethernet_ports();
	detect_CPU_RAM();
	update_traffic();
	if(isSupport("ledg")){
		$("#light_effect_tab").show();
	}  
});

var nvram = new Object();
var variable = new Object();
function getVariable(){
	var _array = new Array('sw_mode', 'lan_ipaddr_t', 'lan_ipaddr', 'secret_code', 'serial_no');
	var _element = new Array();
	if(system.band2gSupport){
		_element = ['wl0_hwaddr'];
		_array.push.apply(_array, _element);
	}

	if(system.band5gSupport){
		_element = ['wl1_hwaddr'];
		_array.push.apply(_array, _element);
	}

	if(system.band5g2Support || system.band6gSupport){
		_element = ['wl2_hwaddr'];
		_array.push.apply(_array, _element);
	}

	if(system.band60gSupport){
		_element = ['wl3_hwaddr'];
		_array.push.apply(_array, _element);
	}

	if(system.yadnsSupport){
		_element = ['yadns_enable_x', 'yadns_mode'];
		_array.push.apply(_array, _element);
	}
	
	nvram = httpApi.nvramGet(_array);
	nvram['lan_hwaddr'] = (httpApi.hookGet('get_lan_hwaddr')) ? httpApi.hookGet('get_lan_hwaddr') : '';
	if(system.yadnsSupport){
		//nvram['yadns_clients'] = (httpApi.hookGet('yadns_clients')) ? httpApi.hookGet('yadns_clients') : '';
		nvram['yadns_clients'] = [ <% yadns_clients(); %> ];
	}
 	variable = Object.assign(variable, nvram);
}

function genElement(){
	var code = '';
	if(system.INTELplatform){
		code += '<div class="info-title"><#LED_Brightness#></div>';
		code += '<div class="display-flex flex-a-center led-container">';
		code += '<div id="slider"></div>';
		code += '<div id="color_pad"></div>';
		code += '</div>';

		$('#led_field').html(code);
		code = '';	// initialize
	}

	// LAN IP
	var _lanIP = (variable.lan_ipaddr_t == '') ? variable.lan_ipaddr : variable.lan_ipaddr_t;
	code += '<div class="info-block"><div class="info-title"><#LAN_IP#></div><div class="info-content">'+ _lanIP +'</div></div>';

	// PIN CODE
	code += '<div class="info-block"><div class="info-title"><#PIN_code#></div><div class="info-content">'+ variable.secret_code +'</div></div>';

	// Serial Number
	if(variable.serial_no != ''){
		code += '<div class="info-block"><div class="info-title"><#Serial_Number#></div><div class="info-content">'+ variable.serial_no +'</div></div>';
	}

	// MAC Address
	code += '<div class="info-block"><div class="info-title">LAN <#MAC_Address#></div><div class="info-content">'+ variable.lan_hwaddr +'</div></div>';
	if(system.band2gSupport){
		code += '<div class="info-block"><div class="info-title">2.4 GHz <#MAC_Address#></div><div class="info-content">'+ variable.wl0_hwaddr +'</div></div>';
	}

	if(system.triBandSupport){
		if(system.band6gSupport){
			code += '<div class="info-block"><div class="info-title">5 GHz <#MAC_Address#></div><div class="info-content">'+ variable.wl1_hwaddr +'</div></div>';
			code += '<div class="info-block"><div class="info-title">6 GHz <#MAC_Address#></div><div class="info-content">'+ variable.wl2_hwaddr +'</div></div>';
		}
		else{
			code += '<div class="info-block"><div class="info-title">5 GHz-1 <#MAC_Address#></div><div class="info-content">'+ variable.wl1_hwaddr +'</div></div>';
			code += '<div class="info-block"><div class="info-title">5 GHz-2 <#MAC_Address#></div><div class="info-content">'+ variable.wl2_hwaddr +'</div></div>';
		}	
	}
	else{
		code += '<div class="info-block"><div class="info-title">5 GHz <#MAC_Address#></div><div class="info-content">'+ variable.wl1_hwaddr +'</div></div>';
	}

	if(system.band60gSupport){
		code += '<div class="info-block"><div class="info-title">LAN <#MAC_Address#></div><div class="info-content">'+ variable.wl3_hwaddr +'</div></div>';
	}

	$('#hw_information_field').html(code);

	//YADEX DNS
	if(system.yadnsSupport &&　parent.sw_mode == 1){
		code = '';
		var yadns_enable = variable.yadns_enable_x;
		var yadns_mode = variable.yadns_mode;
		var yadns_clients = variable.yadns_clients;
		var mode = (yadns_enable != '0') ? yadns_mode : 4;
		var modeDesc = ['<#YandexDNS_mode0#>', '<#YandexDNS_mode1#>', '<#YandexDNS_mode2#>', '', '<#btn_Disabled#>'];
		code += '<div class="division-block"><#YandexDNS#></div>';
		code += '<div class="info-block"><div class="info-content">' + modeDesc[mode] +'</div></div>';
		for(var i=0; i<3; i++){
			if(yadns_enable != 0 && i != mode && yadns_clients[i]){
				code += '<div class="info-block">';
				code += '<div class="info-title">'+ modeDesc[i] +'</div>';
				code += '<div class="info-content"><#Full_Clients#> '+ yadns_clients[i] +'</div>';
				code += '</div>';
			}
		}

		$('#yadns_field').html(code);
		if(!system.yadnsHideQIS || yadns_enable != 0){
			$('#yadns_field').show();
		}
	}
}

function register_event(){
	$(function() {
		$( "#slider" ).slider({
			orientation: "horizontal",
			range: "min",
			min: 1,
			max: 4,
			value: 4,
			slide:function(event, ui){
				$("#color_pad").html(led_table[ui.value-1]);
				$("#slider .ui-slider-range").css("background-color", color_table[ui.value-1]);
				$("#slider .ui-slider-handle").css("border-color", color_table[ui.value-1]);
			},
			stop:function(event, ui){
				set_led(ui.value);	  
			}
		}); 
	});
}

function set_led(value){
	var obj = {
		"action_mode": "apply",
		"rc_service": "reset_led",
	}

	obj.bc_ledLv = value - 1;
	httpApi.nvramSet(obj);
}

function initiailzeParameter(){
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

	for(i=0;i<array_size;i++){
		ram_usage_array[i] = 101;
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

		$("#rx_quantification").html(adjust_unit(diff_rx));
		$("#tx_quantification").html(adjust_unit(diff_tx));

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
				var render_CPU = function(cpu_info_new){
					var pt = "";
					var percentage = total_diff = usage_diff = 0;
					var length = Object.keys(cpu_info_new).length;

					for(i=0;i<length;i++){
						pt = "";
						total_diff = (cpu_info_old[i].total == 0)? 0 : (cpu_info_new["cpu"+i].total - cpu_info_old[i].total);
						usage_diff = (cpu_info_old[i].usage == 0)? 0 : (cpu_info_new["cpu"+i].usage - cpu_info_old[i].usage);

						percentage = (total_diff == 0) ? 0 : parseInt(100*usage_diff/total_diff);
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

				var render_RAM = function(memory){
					var pt = "";
					var used_percentage = total_MB = free_MB = used_MB = 0;
					total_MB = Math.round(memory.total/1024);
					free_MB = Math.round(memory.free/1024);
					used_MB = Math.round(memory.used/1024);
					
					$("#ram_total_info").html(total_MB + " MB");
					$("#ram_free_info").html(free_MB + " MB");
					$("#ram_used_info").html(used_MB + " MB");

					used_percentage = Math.round((memory.used/memory.total)*100);
					$("#ram_bar").css("width", used_percentage + "%");
					$("#ram_quantification").html(used_percentage + "%");
					ram_usage_array.push(100 - used_percentage);
					ram_usage_array.splice(0,1);
					for(i=0;i<array_size;i++){
						pt += i*6 +","+ ram_usage_array[i] + " ";
					}

//					document.getElementById('ram_graph').setAttribute('points', pt);
				}		

				render_CPU(cpuInfo);
				render_RAM(memInfo);
				setTimeout("detect_CPU_RAM();", 2000);
			}
		});
	}
}

function genCPUElement(){
	var code = '<div class="division-block"><#Status_CPU#></div>';
	code += '<div>';
	for(i=0;i<core_num;i++){
		code += '<div class="display-flex flex-a-center info-block">';
		code += '<div class="bar-container">';
		code += '<div id="cpu'+ i +'_bar" class="core-color-container core-color-'+ i +'"></div>';
		code += '</div>';
		code += '<div class="bar-text-width bar-text">Core '+ parseInt(i+1) +'</div>';
		code += '<div id="cpu'+ i +'_quantification" class="bar-text-width bar-text-percent"></div>';
		code += '</div>';
	}

	code += '</div>';

	code += '<svg class="svg-block" width="100%" height="100px">';
	code += '<g>';
	code += '<line stroke-width="1" stroke-opacity="1" stroke="rgb(255,255,255)" x1="0" y1="0%" x2="100%" y2="0%" />';
	code += '<line stroke-width="1" stroke-opacity="0.2" stroke="rgb(255,255,255)" x1="0" y1="25%" x2="100%" y2="25%" />';
	code += '<line stroke-width="1" stroke-opacity="0.2" stroke="rgb(255,255,255)" x1="0" y1="50%" x2="100%" y2="50%" />';
	code += '<line stroke-width="1" stroke-opacity="0.2" stroke="rgb(255,255,255)" x1="0" y1="75%" x2="100%" y2="75%" />';
	code += '<line stroke-width="1" stroke-opacity="1" stroke="rgb(255,255,255)" x1="0" y1="100%" x2="100%" y2="100%" />';
	code += '</g>';
	code += '<g>';
	code += '<text font-family="Verdana" fill="#FFFFFF" font-size="8" x="0" y="98%">0%</text>';
	code += '<text font-family="Verdana" fill="#FFFFFF" font-size="8" x="0" y="55%">50%</text>';
	code += '<text font-family="Verdana" fill="#FFFFFF" font-size="8" x="0" y="11%">100%</text>';	
	code += '</g>';
	code += '<line stroke-width="1" stroke-opacity="1"   stroke="rgb(0,0,121)"   x1="0"   y1="0%" x2="0"   y2="100%" />';
	code += '<line stroke-width="1" stroke-opacity="0.3" stroke="rgb(40,255,40)" x1="11%"  y1="0%" x2="11%"  y2="100%" />';
	code += '<line stroke-width="1" stroke-opacity="0.3" stroke="rgb(40,255,40)" x1="22%"  y1="0%" x2="22%"  y2="100%" />';
	code += '<line stroke-width="1" stroke-opacity="0.3" stroke="rgb(40,255,40)" x1="33%"  y1="0%" x2="33%"  y2="100%" />';
	code += '<line stroke-width="1" stroke-opacity="0.3" stroke="rgb(40,255,40)" x1="44%" y1="0%" x2="44%" y2="100%" />';
	code += '<line stroke-width="1" stroke-opacity="0.3" stroke="rgb(40,255,40)" x1="55%" y1="0%" x2="55%" y2="100%" />';
	code += '<line stroke-width="1" stroke-opacity="0.3" stroke="rgb(40,255,40)" x1="66%" y1="0%" x2="66%" y2="100%" />';
	code += '<line stroke-width="1" stroke-opacity="0.3" stroke="rgb(40,255,40)" x1="77%" y1="0%" x2="77%" y2="100%" />';
	code += '<line stroke-width="1" stroke-opacity="0.3" stroke="rgb(40,255,40)" x1="88%" y1="0%" x2="88%" y2="100%" />';
	code += '<line stroke-width="1" stroke-opacity="1"   stroke="rgb(0,0,121)"   x1="100%" y1="0%" x2="100%" y2="100%" />';
	for(i=0;i<core_num;i++){
		code += '<polyline id="cpu'+ i +'_graph" class="svg-line core-fill-color-'+ i +'" points=""></polyline>';
	}

	code += '</svg>';
	$('#cpu_field').html(code);
}

function genRAMElement(){
	var code = '<div class="division-block"><#Status_RAM#></div>';
	code += '<div>';
	code += '<div>';
	code += '<div class="display-flex flex-a-center ram-content">';
	code += '<div class="ram-text-width"><#Status_Used#></div>';
	code += '<div class="ram-text-width"><#Status_Free#></div>';
	code += '<div class="ram-text-width"><#Status_Total#></div>';
	code += '</div>';	
	code += '<div class="display-flex flex-a-center ram-content">';
	code += '<div id="ram_used_info" class="ram-text-width"></div>';
	code += '<div id="ram_free_info" class="ram-text-width"></div>';
	code += '<div id="ram_total_info" class="ram-text-width"></div>';
	code += '</div>';
	code += '</div>';
	code += '<div class="display-flex flex-a-center info-block">';
	code += '<div class="bar-container">';
	code += '<div id="ram_bar" class="core-color-container ram-color"></div>';
	code += '</div>';
	code += '<div class="bar-text-width bar-text"></div>';
	code += '<div id="ram_quantification" class="bar-text-width bar-text-percent"></div>';
	code += '</div>';
	code += '</div>';
/*
	code += '<svg class="svg-block" width="100%" height="100px">';
	code += '<g>';
	code += '<line stroke-width="1" stroke-opacity="1" stroke="rgb(255,255,255)" x1="0" y1="0%" x2="100%" y2="0%" />';
	code += '<line stroke-width="1" stroke-opacity="0.2" stroke="rgb(255,255,255)" x1="0" y1="25%" x2="100%" y2="25%" />';
	code += '<line stroke-width="1" stroke-opacity="0.2" stroke="rgb(255,255,255)" x1="0" y1="50%" x2="100%" y2="50%" />';
	code += '<line stroke-width="1" stroke-opacity="0.2" stroke="rgb(255,255,255)" x1="0" y1="75%" x2="100%" y2="75%" />';
	code += '<line stroke-width="1" stroke-opacity="1" stroke="rgb(255,255,255)" x1="0" y1="100%" x2="100%" y2="100%" />';
	code += '</g>';
	code += '<g>';
	code += '<text font-family="Verdana" fill="#FFFFFF" font-size="8" x="0" y="98%">0%</text>';
	code += '<text font-family="Verdana" fill="#FFFFFF" font-size="8" x="0" y="55%">50%</text>';
	code += '<text font-family="Verdana" fill="#FFFFFF" font-size="8" x="0" y="11%">100%</text>';	
	code += '</g>';
	code += '<line stroke-width="1" stroke-opacity="1" stroke="rgb(0,0,121)" x1="0" y1="0%" x2="0" y2="100%" />';
	code += '<line stroke-width="1" stroke-opacity="0.3" stroke="rgb(40,255,40)" x1="11%" y1="0%" x2="11%" y2="100%" />';
	code += '<line stroke-width="1" stroke-opacity="0.3" stroke="rgb(40,255,40)" x1="22%" y1="0%" x2="22%" y2="100%" />';
	code += '<line stroke-width="1" stroke-opacity="0.3" stroke="rgb(40,255,40)" x1="33%" y1="0%" x2="33%" y2="100%" />';
	code += '<line stroke-width="1" stroke-opacity="0.3" stroke="rgb(40,255,40)" x1="44%" y1="0%" x2="44%" y2="100%" />';
	code += '<line stroke-width="1" stroke-opacity="0.3" stroke="rgb(40,255,40)" x1="55%" y1="0%" x2="55%" y2="100%" />';
	code += '<line stroke-width="1" stroke-opacity="0.3" stroke="rgb(40,255,40)" x1="66%" y1="0%" x2="66%" y2="100%" />';
	code += '<line stroke-width="1" stroke-opacity="0.3" stroke="rgb(40,255,40)" x1="77%" y1="0%" x2="77%" y2="100%" />';
	code += '<line stroke-width="1" stroke-opacity="0.3" stroke="rgb(40,255,40)" x1="88%" y1="0%" x2="88%" y2="100%" />';
	code += '<line stroke-width="1" stroke-opacity="1" stroke="rgb(0,0,121)"   x1="100%" y1="0%" x2="100%" y2="100%" />';
	code += '<polyline id="ram_graph" class="svg-line ram-fill-color" points="">';
	code += '</svg>';
*/
	$('#ram_field').html(code);
}

function genNETelement() {
	var code = '<div class="division-block">Internet Traffic</div>';
	code += '<div>';
	code += '<div>';
	code += '<div class="display-flex flex-a-center info-block">';
	code += '<div class="bar-container">';
	code += '<div id="rx_bar" class="core-color-container rx-color"></div>';
	code += '</div>';
	code += '<div class="bar-text bar-text-net">DL</div>';
	code += '<div id="rx_quantification" class="bar-text-percent bar-text-percent-net">-- KB/s</div>';
	code += '</div>';

	code += '<div class="display-flex flex-a-center info-block">';
	code += '<div class="bar-container">';
	code += '<div id="tx_bar" class="core-color-container tx-color"></div>';
	code += '</div>';
	code += '<div class="bar-text bar-text-net">UL</div>';
	code += '<div id="tx_quantification" class="bar-text-percent bar-text-percent-net">-- KB/s</div>';
	code += '</div>';

	$('#net_field').html(code);
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
				var speedMapping = {
					'M': '100 Mbps',
					'G': '1 Gbps',
					'Q': '2.5 Gbps',
					'F': '5 Gbps',
					'T': '10 Gbps',
					'X': '<#Status_Unplugged#>'
				};	

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

			if(!Object.keys(wanLanStatus).length){
				$('#phy_ports').hide();
				return;
			}

			wanLanStatus = parseStrToArray(wanLanStatus);
			var code = '<div class="division-block"><#Status_Ethernet_Ports#></div>';
			code += '<div>';
			code += '<div class="display-flex flex-a-center table-header">';
			code += '<div class="port-block-width table-content"><#Status_Ports#></div>';
			code += '<div class="port-block-width table-content"><#Status_Str#></div>';
			code += '</div>';
			for(var i=0; i<wanLanStatus.length; i++){
				code += '<div class="display-flex flex-a-center table-body">';
				code += '<div class="port-block-width table-content table-content-first">'+ wanLanStatus[i][0] +'</div>';
				code += '<div class="port-block-width table-content">'+ wanLanStatus[i][1] +'</div>';
				code += '</div>';	
			}
			
			code += '</div>';
			$('#phy_ports').html(code);
			setTimeout("get_ethernet_ports();", 3000);
		}
	});
}

function switchTab(id){
	var obj = {
		'wireless_tab': 'router.asp',
		'status_tab': 'router_status.asp',
		'light_effect_tab': 'router_light_effect.asp'
	}
	var path = window.location.pathname.split('/').pop();
	var targetPath = obj[id];
	if(targetPath == path){return false;}

	location.href = targetPath;
}
</script>
</head>
<body>
<div class="main-block">
	<div class="display-flex flex-a-center">
		<div id="wireless_tab" class="tab-block" onclick="switchTab(this.id)"><#menu5_1#></div>
		<div id="status_tab" class="tab-block tab-click" onclick="switchTab(this.id)"><#Status_Str#></div>
		<div id="light_effect_tab" class="tab-block"style="display:none;" onclick="switchTab(this.id)">Aura RGB</div><!-- untranslated -->
	</div>
	<div id="net_field" class="unit-block"></div>
	<div id="cpu_field" class="unit-block"></div>
	<div id="ram_field" class="unit-block"></div>
	<div id="phy_ports" class="unit-block"></div>
	<div id="led_field" class="unit-block" style="display:none"></div>
	<div id='hw_information_field' class="unit-block"></div>
	<div id="yadns_field" class="unit-block" style="display:none"></div>
</div>
</body>
</html>
