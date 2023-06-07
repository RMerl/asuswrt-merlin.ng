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
<title><#Web_Title#> - <#smart_connect_rule#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="/device-map/device-map.css">
<script type="text/javascript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/validator.js"></script>
<script type="text/javascript" src="/js/httpApi.js"></script>
<script type="text/javascript" src="/client_function.js"></script>
<style>
.FormTable th{
	width:20%;
}

.FormTable td span{
	color:#FFFFFF;
}

</style>
<script>

<% wl_get_parameter(); %>

let nvram = {},
	variable = {};

let bsd_steering_policy_array = [],
	bsd_sta_select_policy_array = [],
	bsd_if_select_policy_array = [],
	bsd_if_qualify_policy_array = [];

let bsd_steering_policy_binary = [],
	bsd_sta_select_policy_binary = [],
	bsd_if_qualify_policy_binary = [];

let smart_connect_array = [],
	wl_ifnames = [];
	// hook = ['wl_cap_2g', 'wl_cap_5g', 'wl_cap_5g_2'];

hook = httpApi.hookGet('wl_cap_2g', true);

(function(){
	let array = ['bsd_bounce_detect', 'smart_connect_x', 'smart_connect_selif_x', 'wl_ifnames'];

	for(let i=0; i<wl_info.wl_if_total; i++){
		let temp = [
			'wl'+ i +'_bsd_steering_policy',
			'wl'+ i +'_bsd_sta_select_policy',
			'wl'+ i +'_bsd_if_select_policy',
			'wl'+ i +'_bsd_if_qualify_policy',
			'wl'+ i +'_txchain',
		];

		array.push(...temp);
	}	
	
	nvram = httpApi.nvramGet(array);
	Object.assign(variable, nvram);
	wl_ifnames = nvram['wl_ifnames'].split(' ');

	/* recorrect wlreless interface name */
	[wl_ifnames[0], wl_ifnames[1], wl_ifnames[2], wl_ifnames[3]] = [wl_ifnames[3], wl_ifnames[0], wl_ifnames[1], wl_ifnames[2]];
	for(let i=0; i< wl_info.wl_if_total; i++){
		let shift = i;
		
		if(based_modelid == 'GT-AXE16000'){		// wireless介面順序變換調整
			shift = (i + 3) % 4;		// wireless unit ['1', '2', '3', '0']			
		}
		
		variable['wl'+ i +'_bsd_steering_policy'] = nvram['wl'+ shift +'_bsd_steering_policy'];
		variable['wl'+ i +'_bsd_sta_select_policy'] = nvram['wl'+ shift +'_bsd_sta_select_policy'];
		variable['wl'+ i +'_bsd_if_select_policy'] = nvram['wl'+ shift +'_bsd_if_select_policy'];
		variable['wl'+ i +'_bsd_if_qualify_policy'] = nvram['wl'+ shift +'_bsd_if_qualify_policy'];
		variable['wl'+ i +'_txchain'] = nvram['wl'+ shift +'_txchain'];

		bsd_steering_policy_array[i] = variable['wl'+ i +'_bsd_steering_policy'].split(' ');
		bsd_sta_select_policy_array[i] = variable['wl'+ i +'_bsd_sta_select_policy'].split(' ');
		bsd_if_select_policy_array[i] = variable['wl'+ i +'_bsd_if_select_policy'].split(' ');
		bsd_if_qualify_policy_array[i] = variable['wl'+ i +'_bsd_if_qualify_policy'].split(' ');

		bsd_steering_policy_binary[i] = parseInt(bsd_steering_policy_array[i][6], 16).toString(2);
		bsd_sta_select_policy_binary[i] = parseInt(bsd_sta_select_policy_array[i][10], 16).toString(2);
		bsd_if_qualify_policy_binary[i] = parseInt(bsd_if_qualify_policy_array[i][1], 16).toString(2);
	}

	/* handle smart connect  */
	smart_connect_array = [...wl_ifnames];
	let smart_connect_mode = parseInt(nvram['smart_connect_selif_x']).toString(2);
	while(smart_connect_mode.length < 4){
		smart_connect_mode = '0' + smart_connect_mode;
	}

	let mode_reverse = smart_connect_mode.split('').reverse();
	for(let i=0; i<mode_reverse.length; i++){
		if(mode_reverse[i] === '0'){
			smart_connect_array[i] = '';
		}
	}
})();

let spatialStreams = (function(){
	let array = [];
	for(let i=0; i<wl_info.wl_if_total; i++){
		let count = 0;
		let string = variable['wl'+ i +'_txchain'];
		string = parseInt(string).toString(2).split('');
		string.forEach(element => {
			(element === '1') ? count++ : '';
		});
		array.push(count);
	}

	return array;
})();

let phyRate = (function(){
	let array = [];
	if(based_modelid === 'GT-AXE16000'){
		array = [1148, 4804, 4804, 4804];
	}



	return array;
})();

function initial(){
	show_menu();
	genBand();
	genSteerLoadBalance();
	genSteerBWUtilization();		// Steering Trigger BW Utilization
	genStreerRSSI();
	genSteerPhyRateLess();
	genSteerPhyRateGreat();
	genSteerCapability();

	genStaRSSI();
	genStaPhyRateLess();
	genStaPhyRateGreat();
	genStaCapability();

	genIfSelectBand();
	genIfQualityBWUtilization();
	genIfQualityaCapability();

	genBounceDetect();
}

function genBand(){
	let code = '<th style="width:12%"><#Interface#></th>';
	let wl_array = ['2.4 GHz'];
	if(wl_info.band5g_2_support){
		wl_array.push('5 GHz-1', '5 GHz-2');
	}
	else{
		wl_array.push('5 GHz');
	}

	if(wl_info.band6g_support){
		wl_array.push('6 GHz');
	}

	for(let i=0; i<wl_array.length; i++){
		code += '<td align="center" style="width:22%">'+ wl_array[i] +'</td>';
	}

	document.querySelector('#band_field').innerHTML = code;
}

function genSteerLoadBalance(){
	let code = '<th><#enable_Load_Balance#></th>';
	for(let i=0; i<wl_info.wl_if_total; i++){
		let _array = bsd_steering_policy_binary[i].split('');
		let flag = false;
		if(_array[_array.length-7] && _array[_array.length-7] == '1'){
			flag = true;
		}

		code += '<td align="center">';
		if(smart_connect_array[i] === ''){
			code += '- -';
			continue;
		}
		
		if(flag){
			code += '<input type="radio" value="1" id="wl'+ i +'_bsd_steering_balance_yes" name="wl'+ i +'_bsd_steering_balance" onchange="loadBalanceChange('+ i +', this)" checked><#checkbox_Yes#>';
			code += '<input type="radio" value="0" id="wl'+ i +'_bsd_steering_balance_no" name="wl'+ i +'_bsd_steering_balance" onchange="loadBalanceChange('+ i +', this)"><#checkbox_No#>';
		}
		else{
			code += '<input type="radio" value="1" id="wl'+ i +'_bsd_steering_balance_yes" name="wl'+ i +'_bsd_steering_balance" onchange="loadBalanceChange('+ i +', this)"><#checkbox_Yes#>';
			code += '<input type="radio" value="0" id="wl'+ i +'_bsd_steering_balance_no" name="wl'+ i +'_bsd_steering_balance" onchange="loadBalanceChange('+ i +', this)" checked><#checkbox_No#>';
		}
		
		code += '</td>';
	}

	document.querySelector('#steering_load_balance_field').innerHTML = code;	
}

function genSteerBWUtilization(){
	let code = '<th><#smart_connect_Bandwidth#></th>';
	for(let i=0; i< wl_info.wl_if_total; i++){
		if(smart_connect_array[i] === ''){
			code += '<td align="center">- -</td>';
			continue;
		}

		code += '<td align="center">';
		code += '<div style="display:flex;">';
		code += '<input type="range" min="0" max="100" value="'+ bsd_steering_policy_array[i][0] +'" step="1"  id="wl'+ i +'_bsd_steering_bandutil_t"  oninput="bandwidthUtiliChange(this)" style="width:80px;">';
		code += '<span style="width:50px;">'+ bsd_steering_policy_array[i][0] +' %</span>';
		code += '</div>';
		code += '</td>';
	}

	document.querySelector('#steering_bw_field').innerHTML = code;
}

function genStreerRSSI(){
	let code = '<th>RSSI</th>';
	for(let i=0; i<wl_info.wl_if_total; i++){

		if(smart_connect_array[i] === ''){
			code += '<td align="center">- -</td>';
			continue;
		}

		code += '<td align="center">';
		code += '<select class="input_option" id="wl'+ i +'_bsd_steering_rssi_s" name="wl'+ i +'_bsd_steering_rssi_s">';
		code += '<option value="0" class="content_input_fd"><#option_less#></option>';
		code += '<option value="1" class="content_input_fd"><#option_greater#></option>';
		code += '</select>';
		code += '<input type="text" onkeypress="return validator.isNegativeNumber(this,event)" value="'+ bsd_steering_policy_array[i][3] +'" class="input_3_table" id="wl'+ i +'_bsd_steering_rssi" name="wl'+ i +'_bsd_steering_rssi" maxlength="4"> dBm';
		code += '</td>';
	}

	document.querySelector('#steering_rssi_field').innerHTML = code;

	for(let i=0; i<wl_info.wl_if_total; i++){
		if(smart_connect_array[i] === ''){
			continue;
		}

		let _array = bsd_steering_policy_binary[i].split('');
		document.querySelector('#wl'+ i +'_bsd_steering_rssi_s').value = _array[_array.length-2];
	}
}

function genSteerPhyRateLess(){
	let code = '<th><#PHY_Rate_Less#></th>';
	for(let i=0; i<wl_info.wl_if_total; i++){
		if(smart_connect_array[i] === ''){
			code += '<td align="center">- -</td>';
			continue;
		}

		code += '<td align="center">';
		code += '<div style="display:flex;">';
		code += '<input type="range" min="0" max="'+ phyRate[i] +'" value="'+ bsd_steering_policy_array[i][4] +'" step="1"  id="wl'+ i +'_bsd_steering_phy_l_t"  oninput="phyRateChange(this)" style="width:60px;">';
		code += '<span style="width:70px;">'+ bsd_steering_policy_array[i][4] +' Mbps</span> ';
		code += '</div>';
		code += '</td>';
	}

	document.querySelector('#steering_phy_rate_less_field').innerHTML = code;
}

function genSteerPhyRateGreat(){
	let code = '<th><#PHY_Rate_Greater#></th>';
	for(let i=0; i<wl_info.wl_if_total; i++){
		if(smart_connect_array[i] === ''){
			code += '<td align="center">- -</td>';
			continue;
		}

		code += '<td align="center">';
		code += '<div style="display:flex;">';
		code += '<input type="range" min="0" max="'+ phyRate[i] +'" value="'+ bsd_steering_policy_array[i][5] +'" step="1"  id="wl'+ i +'_bsd_steering_phy_g_t"  oninput="phyRateChange(this)" style="width:60px;">';
		code += '<span style="width:70px;">'+ bsd_steering_policy_array[i][5] +' Mbps</span>';
		code += '</div>';
		code += '</td>';
	}

	document.querySelector('#steering_phy_rate_great_field').innerHTML = code;
}

function genSteerCapability(){
	let code = '<th><#Capability#></th>';
	for(let i=0; i<wl_info.wl_if_total; i++){
		if(smart_connect_array[i] === ''){
			code += '<td align="center">- -</td>';
			continue;
		}

		code += '<td align="center">';
		code += '<select class="input_option" id="wl'+ i +'_bsd_steering_vht_s" name="wl'+ i +'_bsd_steering_vht_s">';
		code += '<option value="0" class="content_input_fd"><#All#></option>';
		code += '<option value="1" class="content_input_fd">802.11ac/ax only</option>';
		code += '<option value="2" class="content_input_fd">802.11a/b/g/n</option>';
		code += '<option value="3" class="content_input_fd">802.11ax only</option>';
		code += '<option value="4" class="content_input_fd">802.11a/b/g/n/ac</option>';
		code += '</select>';
		code += '</td>';
	}

	document.querySelector('#steering_capability_field').innerHTML = code;

	for(let i=0; i<wl_info.wl_if_total; i++){
		if(smart_connect_array[i] === ''){
			continue;
		}

		let _array = bsd_steering_policy_binary[i].split('');
		if(_array[_array.length-12] && _array[_array.length-12] == '1'){
			document.querySelector('#wl'+ i +'_bsd_steering_vht_s').value = '3';
		}
		else if(_array[_array.length-11] && _array[_array.length-11] == '1'){
			document.querySelector('#wl'+ i +'_bsd_steering_vht_s').value = '4';
		}
		else if(_array[_array.length-4] && _array[_array.length-4] == '1'){
			document.querySelector('#wl'+ i +'_bsd_steering_vht_s').value = '1';
		}
		else if(_array[_array.length-3] && _array[_array.length-3] == '1'){
			document.querySelector('#wl'+ i +'_bsd_steering_vht_s').value = '2';
		}
	}
}

function genStaRSSI(){
	let code = '<th>RSSI</th>';
	for(let i=0; i<wl_info.wl_if_total; i++){
		if(smart_connect_array[i] === ''){
			code += '<td align="center">- -</td>';
			continue;
		}

		code += '<td  style="width:22%" align="center">';
		code += '<select class="input_option" id="wl'+ i +'_bsd_sta_select_policy_rssi_s" name="wl'+ i +'_bsd_sta_select_policy_rssi_s">';
		code += '<option value="0" class="content_input_fd"><#option_less#></option>';
		code += '<option value="1" class="content_input_fd"><#option_greater#></option>';
		code += '</select>';
		code += '<input type="text" onkeypress="return validator.isNegativeNumber(this,event)" value="'+ bsd_sta_select_policy_array[i][1] +'" class="input_3_table" id="wl'+ i +'_bsd_sta_select_policy_rssi" name="wl'+ i +'_bsd_sta_select_policy_rssi" maxlength="4">dBm';
		code += '</td>';
	}

	document.querySelector('#sta_rssi_field').innerHTML = code;

	for(let i=0; i<wl_info.wl_if_total; i++){
		if(smart_connect_array[i] === ''){
			continue;
		}

		let _array = bsd_sta_select_policy_binary[i].split('');
		document.querySelector('#wl'+ i +'_bsd_sta_select_policy_rssi_s').value = _array[_array.length-2];
	}
}

function genStaPhyRateLess(){
	let code = '<th style="width:12%"><#PHY_Rate_Less#></th>';
	for(let i=0; i<wl_info.wl_if_total; i++){
		if(smart_connect_array[i] === ''){
			code += '<td align="center">- -</td>';
			continue;
		}

		code += '<td style="width:22%" align="center">';
		code += '<div style="display:flex;">';
		code += '<input type="range" min="0" max="'+ phyRate[i] +'" value="'+ bsd_sta_select_policy_array[i][2] +'" step="1"  id="wl'+ i +'_bsd_sta_select_policy_phy_l_t"  oninput="phyRateChange(this)" style="width:60px;">';
		code += '<span style="width:70px;">'+ bsd_sta_select_policy_array[i][2] +' Mbps</span>';
		code += '</div>';
		code += '</td>';
	}

	document.querySelector('#sta_phy_rate_less_field').innerHTML = code;
}

function genStaPhyRateGreat(){
	let code = '<th style="width:12%"><#PHY_Rate_Greater#></th>';
	for(let i=0; i<wl_info.wl_if_total; i++){
		if(smart_connect_array[i] === ''){
			code += '<td align="center">- -</td>';
			continue;
		}

		code += '<td style="width:22%" align="center">';
		code += '<div style="display:flex;">';
		code += '<input type="range" min="0" max="'+ phyRate[i] +'" value="'+ bsd_sta_select_policy_array[i][3] +'" step="1"  id="wl'+ i +'_bsd_sta_select_policy_phy_g_t"  oninput="phyRateChange(this)" style="width:60px;">';
		code += '<span style="width:70px;">'+ bsd_sta_select_policy_array[i][3] +' Mbps</span>';
		code += '</div>';
		code += '</td>';
	}

	document.querySelector('#sta_phy_rate_great_field').innerHTML = code;
}

function genStaCapability(){
	let code = '<th style="width:12%"><#Capability#></th>';
	for(let i=0; i<wl_info.wl_if_total; i++){
		if(smart_connect_array[i] === ''){
			code += '<td align="center">- -</td>';
			continue;
		}

		code += '<td style="width:22%" align="center">';
		code += '<select class="input_option" id="wl'+ i +'_bsd_sta_select_policy_vht_s" name="wl'+ i +'_bsd_sta_select_policy_vht_s">';
		code += '<option value="0" class="content_input_fd"><#All#></option>';
		code += '<option value="1" class="content_input_fd">802.11ac/ax only</option>';
		code += '<option value="2" class="content_input_fd">802.11a/b/g/n</option>';
		code += '<option value="3" class="content_input_fd">802.11ax only</option>';
		code += '<option value="4" class="content_input_fd">802.11a/b/g/n/ac</option>';
		code += '</select>';
		code += '</td>';
	}

	document.querySelector('#sta_capability_field').innerHTML = code;

	for(let i=0; i<wl_info.wl_if_total; i++){
		if(smart_connect_array[i] === ''){
			continue;
		}

		let _array = bsd_sta_select_policy_binary[i].split('');
		if(_array[_array.length-16] && _array[_array.length-16] == '1'){
			document.querySelector('#wl'+ i +'_bsd_sta_select_policy_vht_s').value = '3';
		}
		else if(_array[_array.length-15] && _array[_array.length-15] == '1'){
			document.querySelector('#wl'+ i +'_bsd_sta_select_policy_vht_s').value = '4';
		}
		else if(_array[_array.length-4] && _array[_array.length-4] == '1'){
			document.querySelector('#wl'+ i +'_bsd_sta_select_policy_vht_s').value = '1';
		}
		else if(_array[_array.length-3] && _array[_array.length-3] == '1'){
			document.querySelector('#wl'+ i +'_bsd_sta_select_policy_vht_s').value = '2';
		}
	}
}

function genIfQualityBWUtilization(){
	let code = '<th><#smart_connect_Bandwidth#></th>';
	for(let i=0; i< wl_info.wl_if_total; i++){
		if(smart_connect_array[i] === ''){
			code += '<td align="center">- -</td>';
			continue;
		}

		code += '<td align="center">';
		code += '<div style="display:flex;">';
		code += '<input type="range" min="0" max="100" value="'+ bsd_if_qualify_policy_array[i][0] +'" step="1"  id="wl'+ i +'_bsd_if_qualify_policy_t"  oninput="bandwidthUtiliChange(this)" style="width:80px;">';
		code += '<span style="width:50px;">'+ bsd_if_qualify_policy_array[i][0] +' %</span>';
		code += '</div>';
		code += '</td>';
	}

	document.querySelector('#if_quality_bw_field').innerHTML = code;
}

function genIfQualityaCapability(){
	let code = '<th style="width:12%"><#Capability#></th>';
	for(let i=0; i<wl_info.wl_if_total; i++){
		if(smart_connect_array[i] === ''){
			code += '<td align="center">- -</td>';
			continue;
		}

		code += '<td style="width:22%" align="center">';
		code += '<select class="input_option" id="wl'+ i +'_bsd_if_qualify_policy_vht_s" name="wl'+ i +'_bsd_if_qualify_policy_vht_s" onchange="ifCapabilityChange('+ i +', this)">';
		code += '<option value="0" class="content_input_fd"><#All#></option>';
		code += '<option value="1" class="content_input_fd">802.11ac/ax only</option>';
		code += '<option value="2" class="content_input_fd">802.11a/b/g/n</option>';
		code += '<option value="3" class="content_input_fd">802.11ax only</option>';
		code += '<option value="4" class="content_input_fd">802.11a/b/g/n/ac</option>';
		code += '</select>';
		code += '</td>';
	}

	document.querySelector('#if_quality_capability_field').innerHTML = code;

	for(let i=0; i<wl_info.wl_if_total; i++){
		if(smart_connect_array[i] === ''){
			continue;
		}

		let _array = bsd_if_qualify_policy_binary[i].split('');
		if(_array[_array.length-12] && _array[_array.length-12] == '1'){
			document.querySelector('#wl'+ i +'_bsd_if_qualify_policy_vht_s').value = '3';
		}
		else if(_array[_array.length-11] && _array[_array.length-11] == '1'){
			document.querySelector('#wl'+ i +'_bsd_if_qualify_policy_vht_s').value = '4';
		}
		else if(_array[_array.length-3] && _array[_array.length-3] == '1'){
			document.querySelector('#wl'+ i +'_bsd_if_qualify_policy_vht_s').value = '1';
		}
		else if(_array[_array.length-2] && _array[_array.length-2] == '1'){
			document.querySelector('#wl'+ i +'_bsd_if_qualify_policy_vht_s').value = '2';
		}
	}
}

let if_name_array = [];
for(let i=0; i<smart_connect_array.length; i++){
	let temp = ['2.4 GHz', '5 GHz-1', '5 GHz-2', '6 GHz'];
	if(smart_connect_array[i] !== ''){
		if_name_array[i] = [smart_connect_array[i], temp[i]];
	}
}
function genIfSelectBand(){
	let code = '<th style="width:12%"><#Interface_target#></th>';
	for(let i=0; i< wl_info.wl_if_total; i++){
		if(smart_connect_array[i] === ''){
			code += '<td style="width:22%;" align="center">- -</td>';
			continue;
		}

		let _array = [...if_name_array],
			flag_none = '';
		_array.splice(i, 1);

		code += '<td style="width:22%;" align="center">';
		code += '<div>';			
		for(let j=1; j<if_name_array.length; j++){
			code += '<div style="margin: 3px 0;">' + j +': ';
			code += '<select class="input_option" id="wl'+ i +'_bsd_if_select_policy_'+ j +'" name="wl'+ i +'_bsd_if_select_policy_'+ j +'" onChange="targetBandChange('+ i +', '+ j +')">';
			for(let k=0; k<_array.length; k++){
				let flag = '';
				if(bsd_if_select_policy_array[i][j-1] === _array[k][0]){
					flag = 'selected';
				}
				else if(bsd_if_select_policy_array[i][j-1] === undefined){
					flag_none = 'selected';
				}

				code += '<option value="'+ _array[k][0] +'" class="content_input_fd" '+ flag +'>'+ _array[k][1] +'</option>';
			}


			if(j !== 1){
		 		code += '<option value="none" class="content_input_fd" '+ flag_none +'>none</option>';
		 	}

			code += '</select>';
			code += '</div>';
		}

		code += '</div>';
		code += '</td>';
	}

	document.querySelector('#if_select_band_field').innerHTML = code;
}

function genBounceDetect(){
	let bounce_detect_array = variable['bsd_bounce_detect'].split(' ');
	let code = '<thead><tr><td colspan="2"><#smart_connect_BD#></td></tr></thead>';
	
	/* Windows Time */
	code += '<tr>';
	code += '<th width="20%"><#smart_connect_STA_window#></th>';
	code += '<td>';
	code += '<input type="text" onkeypress="return validator.isNumber(this,event)" value="'+ bounce_detect_array[0] +'" class="input_6_table" id="windows_time_sec" name="windows_time_sec" maxlength="4" autocorrect="off" autocapitalize="off"> <#Second#>';
	code += '</td>';
	code += '</tr>';

	/* Counts */
	code += '<tr>';
	code += '<th><#smart_connect_STA_counts#></th>';
	code += '<td>';
	code += '<input type="text" onkeypress="return validator.isNumber(this,event)" value="'+ bounce_detect_array[1] +'" class="input_6_table" id="bsd_counts" name="bsd_counts" maxlength="4" autocorrect="off" autocapitalize="off">';
	code += '</td>';
	code += '</tr>';

	/* Dwell Time */
	code += '<tr>';
	code += '<th><#smart_connect_STA_dwell#></th>';
	code += '<td>';
	code += '<input type="text" onkeypress="return validator.isNumber(this,event)" value="'+ bounce_detect_array[2] +'" class="input_6_table" id="dwell_time_sec" name="dwell_time_sec" maxlength="4" autocorrect="off" autocapitalize="off"> <#Second#>';
	code += '</td>';
	code += '</tr>';

	document.querySelector('#bounce_detect_field').innerHTML = code;
}

function phyRateChange(obj){
	var target = obj.nextElementSibling;
	var obj_value = obj.value;
	target.innerHTML = (obj_value == '0') ? 'Disable' : obj_value + ' Mbps'; 		
}

function bandwidthUtiliChange(obj){
	var target = obj.nextElementSibling;
	var obj_value = obj.value;
	target.innerHTML = obj_value + ' %';
}

function targetBandChange(band, sequence){
	let selected_value = document.querySelector('#wl'+ band +'_bsd_if_select_policy_'+ sequence).value;
	let sequence_ori = [...bsd_if_select_policy_array[band]];
	let	sequence_changed = new Array(sequence_ori.length).fill('');
	sequence_changed[sequence-1] = selected_value;
	let _array = [...if_name_array];
	_array.splice(band, 1);
	for(let i=0; i<=sequence_ori.length; i++){
		if(sequence_ori[i] !== 'none' && sequence_ori[i] === selected_value){
			sequence_ori[i] = (i === 0) ? '' : 'none'	
			break;
		}
	}

	for(let i=0; i<=sequence_ori.length; i++){
		if(sequence_ori[i] !== '' && sequence_changed[i] === ''){
			sequence_changed[i] = sequence_ori[i];
		}
	}

	for(let i=0; i<sequence_changed.length; i++){
		if(sequence_changed[i] === ''){
			for(let j=0; j<_array.length; j++){
				let _value = _array[j][0];
				if(sequence_changed.indexOf(_value) === -1){
					sequence_changed[i] = _value;
					break;
				}
			}
		}
	}

	for(let i=0; i<sequence_changed.length; i++){
		if(sequence_changed[i] === undefined){
			continue;
		}
		
		let index = i + 1;
		document.querySelector('#wl'+ band +'_bsd_if_select_policy_'+ index).value = sequence_changed[i];		
	}
	
	bsd_if_select_policy_array[band] = [...sequence_changed];
}

function loadBalanceChange(band, obj){
	let enable = (obj.value === '1') ? true : false;
	if(enable){
		document.querySelector('#wl'+ band +'_bsd_steering_bandutil_t').disabled = true;
		document.querySelector('#wl'+ band +'_bsd_steering_rssi_s').disabled = true;
		document.querySelector('#wl'+ band +'_bsd_steering_rssi').disabled = true;
		document.querySelector('#wl'+ band +'_bsd_steering_phy_l_t').disabled = true;
		document.querySelector('#wl'+ band +'_bsd_steering_phy_g_t').disabled = true;
		document.querySelector('#wl'+ band +'_bsd_steering_vht_s').disabled = true;
	}
	else{
		document.querySelector('#wl'+ band +'_bsd_steering_bandutil_t').disabled = false;
		document.querySelector('#wl'+ band +'_bsd_steering_rssi_s').disabled = false;
		document.querySelector('#wl'+ band +'_bsd_steering_rssi').disabled = false;
		document.querySelector('#wl'+ band +'_bsd_steering_phy_l_t').disabled = false;
		document.querySelector('#wl'+ band +'_bsd_steering_phy_g_t').disabled = false;
		document.querySelector('#wl'+ band +'_bsd_steering_vht_s').disabled = false;
	}	
}

function ifCapabilityChange(band, obj){
	let cap_value = obj.value;
	if(cap_value !== '0'){
		document.querySelector('#wl'+ band +'_bsd_steering_vht_s').value = cap_value;
		document.querySelector('#wl'+ band +'_bsd_steering_vht_s').disabled = true;
		document.querySelector('#wl'+ band +'_bsd_sta_select_policy_vht_s').value = cap_value;
		document.querySelector('#wl'+ band +'_bsd_sta_select_policy_vht_s').disabled = true;
	}
	else{
		document.querySelector('#wl'+ band +'_bsd_steering_vht_s').disabled = false;
		document.querySelector('#wl'+ band +'_bsd_sta_select_policy_vht_s').disabled = false;
	}
}

function applyRule(){
	for(let i=0; i<wl_info.wl_if_total; i++){
		if(smart_connect_array[i] === ''){
			continue;
		}

		if(based_modelid == 'GT-AXE16000'){		// wireless介面順序變換調整
			shift = (i + 3) % 4;		// wireless unit ['1', '2', '3', '0']			
		}
		/*
			Steering policy
			['Bandwidth Utilization', 'X', 'X', 'RSSI value', 'PHY rate less', 'PHY rate great', 'binary string']
		*/
		let bsd_steering_policy = variable['wl'+ i +'_bsd_steering_policy'].split(' ');
		let load_balance_enable = document.querySelectorAll('[name=wl'+ i +'_bsd_steering_balance]')[0].checked;
		if(!load_balance_enable){
			bsd_steering_policy[0] = document.querySelector('#wl'+ i +'_bsd_steering_bandutil_t').value;
			bsd_steering_policy[3] = document.querySelector('#wl'+ i +'_bsd_steering_rssi').value;
			bsd_steering_policy[4] = document.querySelector('#wl'+ i +'_bsd_steering_phy_l_t').value;
			bsd_steering_policy[5] = document.querySelector('#wl'+ i +'_bsd_steering_phy_g_t').value;
		}
		
		bsd_steering_policy[6] = (function(){
			let binary = parseInt(bsd_steering_policy[6], 16).toString(2);
			while(binary.length<32){
				binary = '0' + binary;
			}

			let binary_array = binary.split('');
			let length = binary_array.length;
			
			if(load_balance_enable){
				binary_array[length-7] = '1';
			}
			else{
				binary_array[length-2] = document.querySelector('#wl'+ i +'_bsd_steering_rssi_s').value;	// RSSI
				let cap_value = document.querySelector('#wl'+ i +'_bsd_steering_vht_s').value;
				if(cap_value === '3'){
					binary_array[length-12] = '1';
				}
				else if(cap_value === '4'){
					binary_array[length-11] = '1';
				}
				else if(cap_value === '1'){
					binary_array[length-4] = '1';
				}
				else if(cap_value === '2'){
					binary_array[length-3] = '1';
				}
				else if(cap_value === '0'){
					binary_array[length-12] = '0';
					binary_array[length-11] = '0';
					binary_array[length-4] = '0';
					binary_array[length-3] = '0';
				}
			}

			binary = binary_array.join('');
			return '0x' + parseInt(binary, 2).toString(16);
		})();

		variable['wl'+ shift +'_bsd_steering_policy'] = bsd_steering_policy.join(' ');

		/*
			STA Selection Policy
			['X', 'RSSI value', 'PHY rate less', 'PHY rate great', 'X', 'X', 'X', 'X', 'X', 'X', 'binary string']
		*/
		let bsd_sta_select_policy = variable['wl'+ i +'_bsd_sta_select_policy'].split(' ');
		bsd_sta_select_policy[1] = document.querySelector('#wl'+ i +'_bsd_sta_select_policy_rssi').value;
		bsd_sta_select_policy[2] = document.querySelector('#wl'+ i +'_bsd_sta_select_policy_phy_l_t').value;
		bsd_sta_select_policy[3] = document.querySelector('#wl'+ i +'_bsd_sta_select_policy_phy_g_t').value;
		bsd_sta_select_policy[10] = (function(){
			let binary = parseInt(bsd_sta_select_policy[10], 16).toString(2);
			while(binary.length<32){
				binary = '0' + binary;
			}

			let binary_array = binary.split('');
			let length = binary_array.length;

			binary_array[length-2] = document.querySelector('#wl'+ i +'_bsd_sta_select_policy_rssi_s').value;	// RSSI
			let cap_value = document.querySelector('#wl'+ i +'_bsd_sta_select_policy_vht_s').value;
			if(cap_value === '3'){
				binary_array[length-16] = '1';
			}
			else if(cap_value === '4'){
				binary_array[length-15] = '1';
			}
			else if(cap_value === '1'){
				binary_array[length-4] = '1';
			}
			else if(cap_value === '2'){
				binary_array[length-3] = '1';
			}
			else if(cap_value === '0'){
				binary_array[length-16] = '0';
				binary_array[length-15] = '0';
				binary_array[length-4] = '0';
				binary_array[length-3] = '0';
			}

			binary = binary_array.join('');
			return '0x' + parseInt(binary, 2).toString(16);
		})();

		variable['wl'+ shift +'_bsd_sta_select_policy'] = bsd_sta_select_policy.join(' ');
		
		/*
			Interface Quality
			['Bandwidth Utilization', 'binary', 'X']
		*/
		let bsd_if_qualify_policy = variable['wl'+ i +'_bsd_if_qualify_policy'].split(' ');
		bsd_if_qualify_policy[0] =  document.querySelector('#wl'+ i +'_bsd_if_qualify_policy_t').value;
		bsd_if_qualify_policy[1] =  (function(){
			let binary = parseInt(bsd_if_qualify_policy[1], 16).toString(2);
			while(binary.length<32){
				binary = '0' + binary;
			}

			let binary_array = binary.split('');
			let length = binary_array.length;

			let cap_value = document.querySelector('#wl'+ i +'_bsd_if_qualify_policy_vht_s').value;
			if(cap_value === '3'){				
				binary_array[length-12] = '1';
			}
			else if(cap_value === '4'){
				binary_array[length-11] = '1';
			}
			else if(cap_value === '1'){
				binary_array[length-3] = '1';
			}
			else if(cap_value === '2'){
				binary_array[length-2] = '1';
			}
			else if(cap_value === '0'){
				binary_array[length-11] = '0';
				binary_array[length-10] = '0';
				binary_array[length-3] = '0';
				binary_array[length-2] = '0';
			}
			
			binary = binary_array.join('');
			return '0x' + parseInt(binary, 2).toString(16);
		})();

		variable['wl'+ shift +'_bsd_if_qualify_policy'] = bsd_if_qualify_policy.join(' ');

		let if_array = [];
		if(smart_connect_array[i] !== ''){
			for(let j=1; j<wl_ifnames.length; j++){
				if(document.querySelector('#wl'+ i +'_bsd_if_select_policy_'+ j) === null){
					continue;
				}

				let string = document.querySelector('#wl'+ i +'_bsd_if_select_policy_'+ j).value;
				if(string === 'none'){
					continue;
				}

				if_array.push(string);
			}
		}
		
		variable['wl'+ shift +'_bsd_if_select_policy'] = if_array.join(' ');

		/*
			Bounce Detect 
			[Window Time, Counts, Dwell Time]
		*/
		let bounce_array = [document.querySelector('#windows_time_sec').value,
							document.querySelector('#bsd_counts').value,
							document.querySelector('#dwell_time_sec').value];

		variable['bsd_bounce_detect'] = bounce_array.join(' ');
	}

	variable['action_mode'] = 'apply';
	variable['rc_service'] = 'restart_wireless';
	httpApi.nvramSet(variable, function(){
		showLoading(10);
		setTimeout(function(){
			location.reload();
		}, 10000);
		
	});
}

function restoreRule(){
	let array = ['bsd_bounce_detect'];
	let restoreData = [];
	for(let i=0; i<wl_info.wl_if_total; i++){
		let temp = [
			'wl'+ i +'_bsd_steering_policy',
			'wl'+ i +'_bsd_sta_select_policy',
			'wl'+ i +'_bsd_if_select_policy',
			'wl'+ i +'_bsd_if_qualify_policy',
		];

		array.push(...temp);
	}	
	
	restoreData = httpApi.nvramDefaultGet(array);
	restoreData['action_mode'] = 'apply';
	restoreData['rc_service'] = 'restart_wireless';
	httpApi.nvramSet(restoreData, function(){
		showLoading(10);
		setTimeout(function(){
			location.reload();
		}, 10000);
		
	});
}

</script>
</head>

<body onload="initial();" onunLoad="return unload_body();" class="bg">
<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="Advanced_Smart_Connect.asp">
<input type="hidden" name="next_page" value="Advanced_Smart_Connect.asp">
<input type="hidden" name="action_mode" value="apply_new">
<input type="hidden" name="action_script" value="restart_wireless">
<input type="hidden" name="action_wait" value="3">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<table class="content" align="center" cellpadding="0" cellspacing="0">
  <tr>
	<td width="17">&nbsp;</td>
	
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
	<td align="left" valign="top" >
	  <table width="760px" border="0" cellpadding="4" cellspacing="0" class="FormTitle" id="FormTitle">
		<tbody>
		<tr>
		  	<td bgcolor="#4D595D" valign="top">
			<div>&nbsp;</div>
			<div class="formfonttitle"><#menu5_1#> - <#smart_connect_rule#></div>
			<div style="margin:10px 0 10px 5px;" class="splitLine"></div>
			<div class="formfontdesc"><#smart_connect_hint#></div>
			<div style="text-align:right;margin-top:-36px;padding-bottom:3px;"><input type="button" class="button_gen" value="<#View_List#>" onClick="pop_clientlist_listview(true)"></div>
			<div id="bsd_steering_div">
				<table cellspacing="0" cellpadding="4" bordercolor="#6b8fa3" border="1" align="center" width="100%" class="FormTable" style="margin-top:10px">
					<thead>
						<tr>
							<td colspan="5"><#smart_connect_Steering#></td>
						</tr>
					</thead>
					<tr id="band_field"></tr>
				
					<tr id="steering_load_balance_field"></tr>

					<tr id="steering_bw_field"></tr>

					<tr id="steering_rssi_field"></tr>

					<tr id="steering_phy_rate_less_field"></tr>

					<tr id="steering_phy_rate_great_field"></tr>

					<tr id="steering_capability_field"></tr>
				</table>

				<table cellspacing="0" cellpadding="4" bordercolor="#6b8fa3" border="1" align="center" width="100%" class="FormTable" style="margin-top:10px">
					<thead>
						<tr>
							<td colspan="5"><#smart_connect_STA#></td>
						</tr>
					</thead>
					<tr id="sta_rssi_field"></tr>
					
					<tr id="sta_phy_rate_less_field"></tr>
		
					<tr id="sta_phy_rate_great_field"></tr>

					<tr id="sta_capability_field"></tr>				
				</table>
				<table cellspacing="0" cellpadding="4" bordercolor="#6b8fa3" border="1" align="center" width="100%" class="FormTable" style="margin-top:10px">
					<thead>
						<tr>
							<td colspan="5"><#smart_connect_ISQP#></td>
						</tr>
					</thead>
					<tr id="if_select_band_field"></tr>

					<tr id="if_quality_bw_field"></tr>
			
					<tr id="if_quality_capability_field"></tr>
				</table>

				<table id="bounce_detect_field" cellspacing="0" cellpadding="4" bordercolor="#6b8fa3" border="1" align="center" width="100%" class="FormTable" style="margin-top:10px"></table>

				<div class="apply_gen">
					<input type="button" id="restoreButton" class="button_gen" value="<#Setting_factorydefault_value#>" onclick="restoreRule();">
					<input type="button" id="applyButton" class="button_gen" value="<#CTL_apply#>" onclick="applyRule();">
				</div>
			</div>
		  	
		  	</td>
		</tr>
		</tbody>
		
	  </table>
	</td>
</form>
</tr>
</table>
<!--===================================Ending of Main Content===========================================-->

	</td>
	<td width="10" align="center" valign="top"></td>
  </tr>
</table>

<div id="footer"></div>
</body>
</html>
