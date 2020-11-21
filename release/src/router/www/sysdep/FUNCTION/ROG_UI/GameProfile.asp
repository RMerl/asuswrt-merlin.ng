<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=Edge"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="icon" href="images/favicon.png">
<title><#Web_Title#> - <#Game_Profile#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="device-map/device-map.css">
<link rel="stylesheet" type="text/css" href="css/basic.css">
<link rel="stylesheet" type="text/css" href="css/gameprofile.css">
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="/js/httpApi.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/client_function.js"></script>
<script type="text/javascript" src="/validator.js"></script>
<script type="text/javascript" src="js/gameProfile.js"></script>

<script>
var wItem2 = new Array();

<% login_state_hook(); %>

var overlib_str0 = new Array();	//Viz add 2011.07 for record longer virtual srvr rule desc
var overlib_str = new Array();	//Viz add 2011.07 for record longer virtual srvr portrange value
var vts_rulelist_array = decodeURIComponent('<% nvram_char_to_ascii("","game_vts_rulelist"); %>').replace(/&#62/g, ">");
var nvram = httpApi.nvramGet(["vts_enable_x"]);
var gameList = new Object;

function initial(){
	show_menu();
	(nvram.vts_enable_x == '1') ? $('#PF_switch').prop('checked', true) :  $('#PF_switch').prop('checked', false);
	collectGameList();
	updatProfileOnline();
	if(vts_rulelist_array != ''){
		$('#listTable').show();
		genListTable();
	}
	else{
		genQuickAdd();
		$('#emptyTable').show();
	}
	
	genQuickAddInner();
	setTimeout("showDropdownClientList('setClientIP', 'ip', 'all', 'ClientList_Block_PC', 'pull_arrow', 'all');", 1000);
}

function collectGameList(){
	var _profile = gameProfile.profile;
	for(i=0; i<_profile.length; i++){
		if(gameList[_profile[i].id] == undefined){
			gameList[_profile[i].id] = {
				'id': _profile[i].id,
				'title': _profile[i].title,
				'class': _profile[i].class
			}
		}
	}
}

function updatProfileOnline(){
	$.ajax({
		url: 'http://nw-dlcdnet.asus.com/plugin/js/gameProfile.json',
		dataType: 'json', 
		error: function(xhr) {
			setTimeout("updatProfileOnline();", 5000);
		},
		success: function(response){
			gameProfile.profile = Object.keys(response).map(function(e){
				return response[e];
			});

			collectGameList();
			genQuickAdd();
			genQuickAddInner();
		}
	});
}

function genQuickAdd(){
	var code = '';
	var _list = Object.keys(gameList).map(function(e){
		return gameList[e];
	});

	for(i=0; i<_list.length; i++){
		code += '<div id="_'+ _list[i].id +'" class="game-profile game-p-'+ _list[i].class +'" onclick="addNewProfile(this.id);">';
		code += '<div class="game-p-t-container">';
		code += '<div class="game-p-bar"></div>';
		code += '<div class="game-p-title">'+ _list[i].title +'</div>';
		code += '</div>';
		code += '</div>';
	}

	$('#empty_qiuckAdd').html(code);
}

function genQuickAddInner(){
	var code = '';
	var _list = Object.keys(gameList).map(function(e){
		return gameList[e];
	});

	var _defaultObj = {
		'id': 'default_game',
		'title': 'Manual',
		'class': 'default'
	}

	_list.unshift(_defaultObj);
	for(i=0; i<_list.length; i++){
		code += '<div id="'+ _list[i].id +'" class="new-g-p-content" onclick="quickAddRule(this.id);">';
		code += '<div class="new-g-p-image game-p-'+ _list[i].class +'"></div>';
		code += '<div class="new-g-p-desc">'+ _list[i].title +'</div>';
		code += '</div>';
	}

	$('#inner_quickAdd').html(code);
}

function genListTable(){
	var vts_rulelist_row = vts_rulelist_array.split('<');
	var code = '';
	var _count = 0;
	code += '<div class="flexbox table-title">';
	code += '<div class="table-content1-width"><div><#AiProtection_filter_stream1#></div></div>';
	code += '<div class="table-content2-width"><div><#Clientlist_device#></div></div>';
	code += '<div class="table-content3-width"><div><#IPConnection_VServerProto_itemname#></div></div>';
	code += '<div class="table-content4-width"><div>Actions</div></div>';
	code += '</div>';

	for(i=1; i< vts_rulelist_row.length; i++){
		var vts_rulelist_col = vts_rulelist_row[i].split('>');
		var _platform = '';
		var _class = 'game-p-default';
		_count++;
		
		code += '<div class="flexbox flex-a-center table-per-content">';
		code += '<div class="flexbox flex-a-center table-content1-width">';
		// matching game
		for(j=0; j<gameProfile.profile.length; j++){
			var _target = gameProfile.profile[j];
			if(_target.port == vts_rulelist_col[1] 
			&& (_target.title == vts_rulelist_col[0] || vts_rulelist_col[0].indexOf(_target.title) != -1)){
				_platform = _target.platform;
				_class += ' game-p-' + _target.class;
			}
		}

		code += '<div class="table-content-image ' + _class + ' "></div>';
		code += '<div class="table-content1-container">';
		code += '<div class="table-content1-title">' + vts_rulelist_col[0] + '</div>';
		code += '<div class="flexbox flex-a-center table-content1-subcontainer">';
	
		// platform
		if(_platform != ''){
			code += '<div class="table-content1-platform">' + _platform + '</div>';
			code += '<div class="table-content1-divide"></div>';
		}
		
		// port
		var _port = vts_rulelist_col[1];
		if(vts_rulelist_col[1].length > 18){
			_port = vts_rulelist_col[1].slice(0, 14) + '...';
		}

		code += '<div title="'+ vts_rulelist_col[1] +'">' + _port + '</div>';
		code += '</div>';
		code += '</div>';
		code += '</div>';

		code += '<div class="table-content2-width">';
		var _clientName = '';
		for(j=0; j<clientList.length; j++){
			var _index = clientList[j];
			if(clientList[_index].ip == vts_rulelist_col[2]){
				_clientName = clientList[_index].name;
			}
		}

		code += '<div class="table-content1-title">' + _clientName + '</div>';
		code += '<div class="table-content1-subcontainer">' + vts_rulelist_col[2] + '</div>';
		code += '</div>';

		code += '<div class="table-content3-width">';
		code += '<div class="table-content1-title">' + vts_rulelist_col[4] + '</div>';
		code += '<div class="table-content1-subcontainer">' + vts_rulelist_col[3] + '</div>';
		code += '</div>';
		code += '<div class="table-content4-width">';
		code += '<div class="flexbox flex-a-center">';
		//code += '<div class="table-action-icon icon-pause"></div>';
		//code += '<div class="table-action-icon icon-edit"></div>';
		code += '<div class="table-action-icon icon-delete" onclick="deleteRule('+ i +')"></div>';
		code += '</div>';
		code += '</div>';
		code += '</div>';
	}

	$('#rule_num').html(_count);
	$('#list_content').html(code);
}

function deleteRule(flag){
	var vts_rulelist_row = vts_rulelist_array.split('<');
	var code = '';
	for(i=1; i<vts_rulelist_row.length; i++){
		if(i != flag){
			code += '<' + vts_rulelist_row[i];
		}
	}

	vts_rulelist_array = code;
	genListTable();
}

function applyRule(){
	document.form.game_vts_rulelist.value = vts_rulelist_array;	
	showLoading();
	document.form.submit();
}
/*------------ Mouse event of fake LAN IP select menu {-----------------*/
function setClientIP(num){
	document.getElementById('new_profile_localIP').value = num;
	hideClients_Block();
}

function pullLANIPList(obj){
	var element = document.getElementById('ClientList_Block_PC');
	var isMenuopen = element.offsetWidth > 0 || element.offsetHeight > 0;
	if(isMenuopen == 0){		
		element.style.display = 'block';		
		document.getElementById('new_profile_localIP').focus();	
	}
	else
		hideClients_Block();
}

function hideClients_Block(){
	document.getElementById('ClientList_Block_PC').style.display='none';
}
/*----------} Mouse event of fake LAN IP select menu-----------------*/

function validForm(){
	if(!Block_chars(document.form.vts_desc_x_0, ["<" ,">" ,"'" ,"%"])){
				return false;		
	}	
	if(!Block_chars(document.form.vts_port_x_0, ["<" ,">"])){
				return false;		
	}	

	if(document.form.vts_proto_x_0.value=="OTHER"){
		document.form.vts_lport_x_0.value = "";
		if (!check_multi_range(document.form.vts_port_x_0, 1, 255, false))
			return false;
	}

	if(!check_multi_range(document.form.vts_port_x_0, 1, 65535, true)){
		return false;
	}
	
	if(document.form.vts_lport_x_0.value.length > 0
			&& !validator.numberRange(document.form.vts_lport_x_0, 1, 65535)){
		return false;	
	}
	
	if(document.form.vts_ipaddr_x_0.value==""){
		alert("<#JS_fieldblank#>");
		document.form.vts_ipaddr_x_0.focus();
		document.form.vts_ipaddr_x_0.select();		
		return false;
	}
	if(document.form.vts_port_x_0.value==""){
		alert("<#JS_fieldblank#>");
		document.form.vts_port_x_0.focus();
		document.form.vts_port_x_0.select();		
		return false;
	}
	if(!validate_multi_range(document.form.vts_port_x_0, 1, 65535)
		|| !validator.validIPForm(document.form.vts_ipaddr_x_0, 0)){			
		return false;	
	}			
	
	return true;
}

function validate_multi_range(val, mini, maxi){
	var rangere=new RegExp("^([0-9]{1,5})\:([0-9]{1,5})$", "gi");
	if(rangere.test(val)){
		
		if(!validator.eachPort(document.form.vts_port_x_0, RegExp.$1, mini, maxi) || !validator.eachPort(document.form.vts_port_x_0, RegExp.$2, mini, maxi)){
				return false;								
		}else if(parseInt(RegExp.$1) >= parseInt(RegExp.$2)){
				alert("<#JS_validport#>");	
				return false;												
		}else				
			return true;	
	}else{
		if(!validate_single_range(val, mini, maxi)){	
					return false;											
				}
				return true;								
			}	
}
function validate_single_range(val, min, max) {
	for(j=0; j<val.length; j++){		//is_number
		if (val.charAt(j)<'0' || val.charAt(j)>'9'){			
			alert('<#JS_validrange#> ' + min + ' <#JS_validrange_to#> ' + max);
			return false;
		}
	}
	
	if(val < min || val > max) {		//is_in_range		
		alert('<#JS_validrange#> ' + min + ' <#JS_validrange_to#> ' + max);
		return false;
	}else	
		return true;
}	
var parse_port="";
function check_multi_range(obj, mini, maxi, allow_range){
	obj.value = document.form.vts_port_x_0.value.replace(/[-~]/gi,":");	// "~-" to ":"
	var PortSplit = obj.value.split(",");
	for(i=0;i<PortSplit.length;i++){
		PortSplit[i] = PortSplit[i].replace(/(^\s*)|(\s*$)/g, ""); 		// "\space" to ""
		PortSplit[i] = PortSplit[i].replace(/(^0*)/g, ""); 		// "^0" to ""	
		
		if(PortSplit[i] == "" ||PortSplit[i] == 0){
			alert("<#JS_ipblank1#>");
			obj.focus();
			obj.select();			
			return false;
		}
		if(allow_range)
			res = validate_multi_range(PortSplit[i], mini, maxi);
		else	res = validate_single_range(PortSplit[i], mini, maxi);
		if(!res){
			obj.focus();
			obj.select();
			return false;
		}						
		
		if(i ==PortSplit.length -1)
			parse_port = parse_port + PortSplit[i];
		else
			parse_port = parse_port + PortSplit[i] + ",";
			
	}
	document.form.vts_port_x_0.value = parse_port;
	parse_port ="";
	return true;	
}

function switchPortForward(obj){
	nvram.vts_enable_x = (obj.checked)?"1":"0";

	httpApi.nvramSet({
		"vts_enable_x": nvram.vts_enable_x,
		"action_mode": "apply",
		"action_wait": "5",
		"rc_service": "restart_firewall"
	});

	showLoading(5);
}

function addNewProfile(target){
	$('#emptyTable').hide();
	$('#listTable').hide();
	$('#addRuleField').show();
	$('.game-selected').removeClass('game-selected');
	$('#new_profile_name').val('');
	$('#new_profile_externalPort').val('');
	$('#new_profile_localPort').val('');
	$('#new_profile_localIP').val('');
	$('#new_profile_sourceIP').val('');
	// remove checked platform
	$('#platformPC').prop('checked', false);
	$('#platformXBOXONE').prop('checked', false);
	$('#platformXBOX360').prop('checked', false);
	$('#platformPS4').prop('checked', false);
	$('#platformPS3').prop('checked', false);
	$('#platformSTEAM').prop('checked', false);
	$('#platformSWITCH').prop('checked', false);
	$('#platformPC_field').hide();
	$('#platformXBOXONE_field').hide();
	$('#platformXBOX360_field').hide();
	$('#platformPS4_field').hide();
	$('#platformPS3_field').hide();
	$('#platformSTEAM_field').hide();
	$('#platformSWITCH_field').hide();
	if(target == undefined){
		$('#default_game').addClass('game-selected');
		$('#protocol_field').show();
		$('#externalPort_field').show();
		$('#localPort_field').show();
		$('#sourceIP_field').show();
	}
	else{
		var id = target.split('_')[1];
		$('#new_profile_name').prop('disabled', true);
		$('#protocol_field').hide();
		$('#externalPort_field').hide();
		$('#localPort_field').hide();
		$('#sourceIP_field').hide();
		$('#' + id).addClass('game-selected');
		var _name = '';
		for(i=0; i<gameProfile.profile.length; i++){
			var _platform = '';
			if(gameProfile.profile[i].id == id){
				_platform = gameProfile.profile[i].platform;
				$('#platform' + _platform).prop('checked', true);
				$('#platform' + _platform + '_field').show();
				_name = gameProfile.profile[i].title;
			}
		}
		
		$('#new_profile_name').val(_name);
	}
}

function cancelNewProfile(){
	$('#addRuleField').hide();
	if(vts_rulelist_array != ''){
		$('#listTable').show();
	}
	else{
		$('#emptyTable').show();
	}
}

function quickAddRule(id){
	$('.game-selected').removeClass('game-selected');
	$('#' + id).addClass('game-selected');
	
	// remove checked platform
	$('#platformPC').prop('checked', false);
	$('#platformXBOXONE').prop('checked', false);
	$('#platformXBOX360').prop('checked', false);
	$('#platformPS4').prop('checked', false);
	$('#platformPS3').prop('checked', false);
	$('#platformSTEAM').prop('checked', false);
	$('#platformSWITCH').prop('checked', false);
	$('#platformPC_field').hide();
	$('#platformXBOXONE_field').hide();
	$('#platformXBOX360_field').hide();
	$('#platformPS4_field').hide();
	$('#platformPS3_field').hide();
	$('#platformSTEAM_field').hide();
	$('#platformSWITCH_field').hide();
	if(id == 'default_game'){
		$('#new_profile_name').prop('disabled', false);
		$('#protocol_field').show();
		$('#externalPort_field').show();
		$('#localPort_field').show();
		$('#sourceIP_field').show();
	}
	else{
		$('#new_profile_name').prop('disabled', true);
		$('#protocol_field').hide();
		$('#externalPort_field').hide();
		$('#localPort_field').hide();
		$('#sourceIP_field').hide();
	}
	
	var _name = '';
	for(i=0; i<gameProfile.profile.length; i++){
		var _platform = '';
		if(gameProfile.profile[i].id == id){
			_platform = gameProfile.profile[i].platform;
			$('#platform' + _platform).prop('checked', true);
			$('#platform' + _platform + '_field').show();
			_name = gameProfile.profile[i].title;
		}
	}
	$('#new_profile_name').val(_name);
}

function newProfileOK(){
	// valid input
	var new_rule_num=0;
	var _platformArray = ['PC', 'XBOXONE', 'XBOX360', 'PS4', 'PS3', 'STEAM', 'SWITCH']
	var _manual = $('#protocol_field').is(':visible');
	if(!_manual){		// quick add
		var _platformCheck = false;
		for(i=0; i<_platformArray.length; i++){
			if($('#platform'+ _platformArray[i]).prop('checked')){
				_platformCheck = true;
				new_rule_num++;
			}
		}
		if(_platformCheck == false){
			alert('Please select at least one platform.');
			return false;
		}
	}
	else
		new_rule_num++;

	if(!Block_chars(document.getElementById("new_profile_name"), ["<" ,">" ,"%"])){
		return false;
	}
	if(!Block_chars(document.getElementById("new_profile_externalPort"), ["<" ,">"])){
		return false;
	}
	if(!Block_chars(document.getElementById("new_profile_localPort"), ["<" ,">"])){
		return false;
	}

	if(document.getElementById("new_profile_localIP").value == "") {
		alert("<#JS_fieldblank#>");
		document.getElementById("new_profile_localIP").focus();
		document.getElementById("new_profile_localIP").select();
		return false;
	}

	if(!validator.validIPForm(document.getElementById("new_profile_localIP"), 0)){
		return false;
	}

	if(parseInt(rule_num.innerHTML)+new_rule_num > 32){
		alert("<#JS_itemlimit1#> " + 32 + " <#JS_itemlimit2#>");
		return false;
	}

	$('#addRuleField').hide();
	$('#listTable').show();
	var _name = $('#new_profile_name').val();
	var _id = $('.game-selected').prop('id');
	if(_id == 'default_game'){
		vts_rulelist_array += '<' + _name;
		vts_rulelist_array += '>' + $('#new_profile_externalPort').val();
		vts_rulelist_array += '>' + $('#new_profile_localIP').val();
		vts_rulelist_array += '>' + $('#new_profile_localPort').val();	
		vts_rulelist_array += '>' + $('#new_profile_protocol').val();
		vts_rulelist_array += '>' + $('#new_profile_sourceIP').val();
		
	}
	else{
		for(i=0; i<gameProfile.profile.length; i++){
			var _target = gameProfile.profile[i];
			if(_target.id == _id && $('#platform' + _target.platform).prop('checked')){
				vts_rulelist_array += '<' + _name;
				//external port
				vts_rulelist_array += '>' + _target.port;
				vts_rulelist_array += '>' + $('#new_profile_localIP').val();
				// internal port
				vts_rulelist_array += '>'; 
				vts_rulelist_array += '>' + _target.protocol;
				//source IP
				vts_rulelist_array += '>';
			}
		}
	}

	genListTable();
}
</script>
</head>

<body onload="initial();" onunLoad="return unload_body();" class="bg">
<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="form" action="/start_apply.htm" target="hidden_frame" >
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="GameProfile.asp">
<input type="hidden" name="next_page" value="GameProfile.asp">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_wait" value="5">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="restart_firewall">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="game_vts_rulelist" value=''>
<input type="hidden" name="vts_enable_x" value='<% nvram_get("vts_enable_x"); %>'>

<table class="content" align="center" cellpadding="0" cellspacing="0" >
	<tr>
		<td width="17">&nbsp;</td>		
		<td valign="top" width="202">				
			<div  id="mainMenu"></div>	
			<div  id="subMenu"></div>		
		</td>						
    	<td valign="top">
			<div id="tabMenu" class="submenuBlock"></div>

			<div class="border-container">
				<div class="border-corner border-corner-top-left"></div>
				<div class="border-corner border-corner-bottom-left"></div>
				<div class="border-bar"></div>

				<!-- Title field -->
				<div class="flexbox title-container">
					<div class="title-symbol"></div>
					<div class="title-content">OPEN NAT</div>
				</div>

				<!-- Content field -->
				<div class="description-container"><#OpenNAT_desc#></div>
				<div class="world-map">
					<div class="map-connection-line"></div>
					<div class="location-indicator location-US3"></div>
					<div class="location-indicator location-US2"></div>
					<div class="location-indicator location-US1"></div>
					<div class="location-indicator location-BR"></div>
					<div class="location-indicator location-CL"></div>
					<div class="location-indicator location-EU1"></div>
					<div class="location-indicator location-EU2"></div>
					<div class="location-indicator location-RU"></div>
					<div class="location-indicator location-SA"></div>
					<div class="location-indicator location-CN"></div>
					<div class="location-indicator location-TW"></div>
					<div class="location-indicator location-AU"></div>
				</div>
				<!-- division line -->
				<div class="divide-line"></div>
				<!-- End division line -->

				<div class="flexbox flex-a-center control-f-container">
					<div class="control-description"><#IPConnection_VServerEnable_itemname#></div>
					<div class="switch-button-container">
						<label for="PF_switch" >
							<input type="checkbox" id="PF_switch" class="switch-button" onchange="switchPortForward(this);">
							<div class="switch-button-bg"></div>
							<div class="switch-button-circle"></div>
						</label>
					</div>
				</div>
			</div>

			<div id="emptyTable" class="border-container" style="display:none">
				<div class="border-corner border-corner-top-left"></div>
				<div class="border-corner border-corner-bottom-left"></div>
				<div class="border-bar"></div>

				<div class="flexbox title-container">
					<div class="title-symbol"></div>
					<div class="title-content"><#Game_Profile#></div>
				</div>

				<div class="description-container"><#Game_Profile_desc#></div>
				<div class="button-container button-container-left" onclick="addNewProfile();">
					<div class="button-icon icon-plus"></div>
					<div class="button-text"><#CTL_add#></div>
				</div>

				<div class="divide-line"></div>

				<div id='empty_qiuckAdd' class="flexbox flex-d-column flex-w-wrap game-profile-container"></div>
				<div class="divide-line"></div>
				<div class="right-desc">** <#AiProtection_title_Radar_desc2#></div>
			</div>

			<!-- New rule field -->
			<div id="addRuleField" class="border-container" style="display:none">
				<div class="border-corner border-corner-top-left"></div>
				<div class="border-corner border-corner-bottom-left"></div>
				<div class="border-bar"></div>

				<div class="flexbox flex-j-end cancel-button-padding" onclick="cancelNewProfile()">
					<div class="icon-button-container">
						<div class="icon-button icon-cancel"></div>
					</div>
				</div>

				<div class="flexbox flex-a-center new-g-p-t-field">
					<div class="new-g-p-title">New Game Profile</div>
				</div>

				<div>
					<div class="flexbox flex-a-center new-g-profile">
						<div class="new-g-p-step">1</div>
						<div class="new-g-p-s-title"><#Game_List#></div>	
					</div>
					<div id="inner_quickAdd" class="new-g-p-container"></div>
					<div class="divide-line new-p-divide"></div>
				</div>
				
				<div>
					<div class="flexbox flex-a-center new-g-profile">
						<div class="new-g-p-step">2</div>
						<div class="new-g-p-s-title"><#Game_Platform#></div>	
					</div>
					<div class="flexbox new-p-platform">
						<div id="platformPC_field" class="checkbox-container">
							<div>
								<input id="platformPC" type="checkbox">
								<label for="platformPC">
									<div></div>
								</label>
							</div>
							<div class="checkbox-desc">PC</div>
						</div>
						<div id="platformXBOXONE_field" class="checkbox-container">
							<div>
								<input id="platformXBOXONE" type="checkbox">
								<label for="platformXBOXONE">
									<div></div>
								</label>
							</div>
							<div class="checkbox-desc">XBOX ONE</div>
						</div>
						<div id="platformXBOX360_field" class="checkbox-container">
							<div>
								<input id="platformXBOX360" type="checkbox">
								<label for="platformXBOX360">
									<div></div>
								</label>
							</div>
							<div class="checkbox-desc">XBOX 360</div>
						</div>
						<div id="platformPS4_field" class="checkbox-container">
							<div>
								<input id="platformPS4" type="checkbox">
								<label for="platformPS4">
									<div></div>
								</label>
							</div>
							<div class="checkbox-desc">PS 4</div>
						</div>
						<div id="platformPS3_field" class="checkbox-container">
							<div>
								<input id="platformPS3" type="checkbox">
								<label for="platformPS3">
									<div></div>
								</label>
							</div>
							<div class="checkbox-desc">PS 3</div>
						</div>
						<div id="platformSTEAM_field" class="checkbox-container">
							<div>
								<input id="platformSTEAM" type="checkbox">
								<label for="platformSTEAM">
									<div></div>
								</label>
							</div>
							<div class="checkbox-desc">STEAM</div>
						</div>
						<div id="platformSWITCH_field" class="checkbox-container">
							<div>
								<input id="platformSWITCH" type="checkbox">
								<label for="platformSWITCH">
									<div></div>
								</label>
							</div>
							<div class="checkbox-desc">SWITCH</div>
						</div>
					</div>
					<div class="divide-line new-p-divide"></div>
				</div>

				<div>
					<div class="flexbox flex-a-center new-g-profile">
						<div class="new-g-p-step">3</div>
						<div class="new-g-p-s-title"><#Settings#></div>
					</div>

					<div class="new-p-platform">
						<div id="name_field" class="game-p-s-field">
							<div class="settings-filed-title"><#BM_UserList1#></div>
							<input id="new_profile_name" type="text" class="input-container" value="" maxlength="30" onkeypress="return validator.isString(this, event);" autocomplete="off" autocorrect="off" autocapitalize="off" >
						</div>

						<div id="protocol_field" class="game-p-s-field">
							<div class="settings-filed-title"><#IPConnection_VServerProto_itemname#></div>
							<div class="select-container">
								<select name="" id="new_profile_protocol">
									<option value="TCP">TCP</option>
									<option value="UDP">UDP</option>
									<option value="BOTH"><#option_both_direction#></option>
								</select>
								<div class="select-arrow">
									<div></div>
								</div>	
							</div>	
						</div>
						<div id="externalPort_field" class="game-p-s-field">
							<div class="settings-filed-title"><#IPConnection_VSList_External_Port#></div>
							<input id="new_profile_externalPort" type="text" class="input-container" value="" maxlength="60" onkeypress="return validator.isPortRange(this, event);" autocomplete="off" autocorrect="off" autocapitalize="off" >
						</div>

						<div id="localPort_field" class="game-p-s-field">
							<div class="settings-filed-title"><#IPConnection_VSList_Internal_Port#></div>
							<input id="new_profile_localPort" type="text" class="input-container" value="" maxlength="60" onkeypress="return validator.isNumber(this,event);" autocomplete="off" autocorrect="off" autocapitalize="off" >
							<div class="hint"><#feedback_optional#></div>
						</div>

						<div id="localIP_field" class="game-p-s-field" >
							<div class="settings-filed-title"><#IPConnection_VSList_Internal_IP#></div>
							<div style="position: relative">
								<input id="new_profile_localIP" type="text" class="input-container" value="" maxlength="15" onkeypress="return validator.isIPAddr(this, event);" autocomplete="off" autocorrect="off" autocapitalize="off">
								<div class="select-arrow" style="cursor:pointer;z-index: 999;" onclick="pullLANIPList(this);" >
									<div></div>
								</div>
								<!-- <div id="pull_arrow" style="display:none"></div> -->
								<div id="ClientList_Block_PC" class="clientlist_dropdown" style="margin-left:0;"></div>
							</div>
							
						</div>

						<div id="sourceIP_field" class="game-p-s-field">
							<div class="settings-filed-title"><#IPConnection_VSList_SourceTarget#></div>
							<input id="new_profile_sourceIP" type="text" class="input-container" value="" maxlength="15" onkeypress="return validator.isIPAddrPlusNetmask(this, event)" autocomplete="off" autocorrect="off" autocapitalize="off" >
							<div class="hint"><#feedback_optional#></div>
						</div>
					</div>
				</div>
				<div class="divide-line"></div>
				<div class="button-container button-container-center" onclick="newProfileOK();">
					<div class="button-icon button-icon-check"></div>
					<div class="button-text"><#CTL_ok#></div>
				</div>
			</div>
			<!-- End New rule field -->

			<!-- List Field -->
			<div id="listTable" class="border-container" style="display:none">
				<div class="border-corner border-corner-top-left"></div>
				<div class="border-corner border-corner-bottom-left"></div>
				<div class="border-bar"></div>

				<div class="flexbox title-container">
					<div class="title-symbol"></div>
					<div class="title-content"><#Game_Profile#></div>
				</div>

				<div class="flexbox flex-j-spaceB control-f-container">
					<div class="button-container " onclick="applyRule();">
						<div class="button-icon button-icon-check"></div>
						<div class="button-text"><#CTL_apply#></div>
					</div>
					<div class="button-container" onclick="addNewProfile();">
						<div class="button-icon icon-plus"></div>
						<div class="button-text"><#CTL_add#></div>
					</div>
				</div>

				<div class="flexbox flex-a-center flex-j-spaceB new-g-p-t-field ">
					<div class="new-g-p-title"><#Game_List#></div>
					<div class="new-g-p-rule"><span id="rule_num">2</span> Rules (<#List_limit#> 32)</div>
				</div>

				<div id="list_content"> 
					<div class="flexbox table-title">
						<div class="table-content1-width">
							<div><#AiProtection_filter_stream1#></div>
						</div>
						<div class="table-content2-width">
							<div><#Clientlist_device#></div>
						</div>
						<div class="table-content3-width">
							<div><#IPConnection_VServerProto_itemname#></div>
						</div>
						<div class="table-content4-width">
							<div>Actions</div>
						</div>
					</div>					
				</div>
			</div>	
			
			<!-- End List Field -->
			<!-- Edit field -->
			<!--div class="border-container" style="display:none">
				<div class="border-corner border-corner-top-left"></div>
				<div class="border-corner border-corner-bottom-left"></div>
				<div class="border-bar"></div>
	
				<div class="flexbox title-container">
					<div class="title-symbol"></div>
					<div class="title-content">GAME PROFILE</div>
				</div>
		
				<div class="flexbox flex-j-spaceB flex-a-center edit-a-container">
					<div class="button-container ">
						<div class="button-icon button-icon-check"></div>
						<div class="button-text">Apply</div>
					</div>
					<div class="cancel-container">
						<div class="icon-button icon-cancel"></div>
					</div>
				</div>

				<div class="flexbox flex-a-center new-g-p-t-field">
					<div class="edit-p-title icon-edit"></div>
					<div class="new-g-p-title">Edit Game Profile</div>
				</div>

				<div class="flexbox edit-content">
					<div class="edit-i-container">
						<div class="edit-image game-p-cod4"></div>
						<div>
							<div class="edit-i-title">Call of Duty 4</div>
							<div class="edit-i-subTitle">Modern Warfare</div>
							<div class="g-bar"></div>
						</div>
					</div>
					<div class="edit-divide"></div>
					<div class="edit-c-container">
						<div class="game-p-s-field">
							<div class="settings-filed-title">Name</div>
							<input type="text" class="input-container" value="Game Test">
							<div class="hint">*optional</div>
						</div>
							<div class="game-p-s-field">
							<div class="settings-filed-title">Protocol</div>
							<div class="select-container">
								<select name="" id="">
									<option value="">1</option>
									<option value="">2</option>
									<option value="">3</option>
								</select>
								<div class="select-arrow">
									<div></div>
								</div>	
							</div>	
						</div>
						<div class="game-p-s-field">
							<div class="settings-filed-title">External Port</div>
							<input type="text" class="input-container" value="Game Test">
							<div class="hint">*optional</div>
						</div>
							<div class="game-p-s-field">
							<div class="settings-filed-title">Internal Port</div>
							<input type="text" class="input-container" value="Game Test">
						</div>
							<div class="game-p-s-field">
							<div class="settings-filed-title">Internal IP</div>
							<input type="text" class="input-container" value="Game Test">
						</div>
							<div class="game-p-s-field">
							<div class="settings-filed-title">Souce IP</div>
							<input type="text" class="input-container" value="Game Test">
							<div class="hint">*optional</div>
						</div>
					</div>
				</div>

			</div-->
			<!-- End Edit field -->

		</td>
    	<td width="10" align="center" valign="top">&nbsp;</td>
	</tr>
</table>
<div id="footer"></div>
</form>
</body>
</html>
