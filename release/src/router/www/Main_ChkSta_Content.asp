<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="X-UA-Compatible" content="IE=Edge">
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title><#Network_Tools#> - <#NetworkTools_ChkSta#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="/device-map/device-map.css">
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script language="JavaScript" type="text/javascript" src="/help.js"></script>
<script language="JavaScript" type="text/javascript" src="/validator.js"></script>
<script language="JavaScript" type="text/javascript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/switcherplugin/jquery.iphone-switch.js"></script>
<script language="JavaScript" type="text/javascript" src="/client_function.js"></script>
<script>
var node_info = [<% show_info_between_nodes(); %>];
var chksta_mac = '<% nvram_get("chksta_mac"); %>';
var chksta_band = '<% nvram_get("chksta_band"); %>';
function initial(){
	show_menu();	
	setTimeout("showDropdownClientList('setClientIP', 'mac', 'all', 'ClientList_Block_PC', 'pull_arrow', 'all');", 1000);
	$("#sta_table").hide();
	if(chksta_mac != ""){
		document.form.chksta_mac.value = chksta_mac;
		document.form.chksta_band.value = chksta_band;
	}
}

var cancelChkSTA = "";
function chkSTAStatus(){
	$.ajax({
		url: '/ajax_chk_sta.asp',
		dataType: 'script',	
		error: function(xhr) {
			setTimeout("chkSTAStatus();", 1000);
		},
		success: function(response){	
			cancelChkSTA = setTimeout(function(){
				chkSTAStatus();
				renderSTAStatus();
			}, 2000);			
		}
	});	
}

// >IP>MAC>STA_NUM>BAND>RSSI>TX_RATE>RX_RATE
function renderSTAStatus(){
	var code = "";
	var band = (diag_band == '1') ? '5 GHz' : '2.4 GHz';
	var diag_array = new Array();
	var count = 1;
	var cap = (diag_cap == "") ? ["", "0", "0", "0"] : (diag_cap.split('<'))[1].split('>');

	diag_array[0] = new Array();
	diag_array[0].push(cap[1]);
	diag_array[0].push(cap[2]);
	diag_array[0].push(cap[3]);

	if(diag_re1 != ""){
		var re1 = diag_re1.split('<')[1].split('>');
		count = 2;
		diag_array[1] = new Array();
		diag_array[1].push(re1[1]);
		diag_array[1].push(re1[2]);
		diag_array[1].push(re1[3]);
	}

	if(diag_re2 != ""){
		var re2 = diag_re2.split('<')[1].split('>');
		count = 3;
		diag_array[2] = new Array();
		diag_array[2].push(re2[1]);
		diag_array[2].push(re2[2]);
		diag_array[2].push(re2[3]);
	}

	var target = TARGET;
	if(clientList[target] != undefined){
		target = clientList[target].name + " ( " + clientList[target].mac + " )";
	}

	code += '<tr><th><#MAC_Address#></th><td colspan="3">'+ target +'</td></tr>';
	code += '<tr><th><#Interface#></th><td colspan="3">'+ band +'</td></tr>';
	code += '<tr><th style="width:250px;"></th><th>RSSI</th><th>Tx rate (Mbps)</th><th>Rx rate (Mbps)</th></tr>';
	for(i=0; i<count; i++){
		code += '<tr><th>'+ get_cfg_clientlist[i].model_name + ' ('+ get_cfg_clientlist[i].mac +')'+'</th><td>'+ diag_array[i][0] +'</td><td>'+ diag_array[i][1] +'</td><td>'+ diag_array[i][2] +'</td></tr>';
	}

	$("#sta_table").html(code);	
	var code = '';
	var node_array = [];

	/* CAP to RE1/RE2 */
	if(nodes_info.length >= 1 && get_cfg_clientlist[0]){
		code += '<tr><th style="width:250px"><#MAC_Address#></th><td colspan="3">'+ get_cfg_clientlist[0].model_name + '('+ get_cfg_clientlist[0].mac +')'+'</td></tr>';
		code += '<tr><th></th><th>RSSI</th><th>Tx rate (Mbps)</th><th>Rx rate (Mbps)</th></tr>';

		for(i=0; i<nodes_info[0].length; i++){
			if(i == 0 || get_cfg_clientlist[i] == undefined){
				continue;
			}

			node_array = nodes_info[0][i].split(">");
			if(node_array == "")
				node_array = ["0", "0", "0"];

			code += '<tr><th>'+ get_cfg_clientlist[i].model_name + ' ('+ get_cfg_clientlist[i].mac +')'+'</th><td>'+ node_array[0] +'</td><td>'+ node_array[1] +'</td><td>'+ node_array[2] +'</td></tr>';
		}		
	}

	$("#cap_table").html(code);

	/* RE1 to CAP/RE2 */
	code = '';
	node_array = [];
	if(nodes_info.length >= 2 && get_cfg_clientlist[1]){
		code += '<tr><th style="width:250px"><#MAC_Address#></th><td colspan="3">'+ get_cfg_clientlist[1].model_name + '('+ get_cfg_clientlist[1].mac +')'+'</td></tr>';
		code += '<tr><th></th><th>RSSI</th><th>Tx rate (Mbps)</th><th>Rx rate (Mbps)</th></tr>';

		for(i=0; i<nodes_info[1].length; i++){
			if(i == 1 || get_cfg_clientlist[i] == undefined){
				continue;
			}

			node_array = nodes_info[1][i].split(">");
			if(node_array == "")
				node_array = ["0", "0", "0"];
	
			code += '<tr><th>'+ get_cfg_clientlist[i].model_name + ' ('+ get_cfg_clientlist[i].mac +')'+'</th><td>'+ node_array[0] +'</td><td>'+ node_array[1] +'</td><td>'+ node_array[2] +'</td></tr>';
		}
	}

	$("#re1_table").html(code);

	/* RE2 to CAP/RE1 */
	code = '';
	node_array = [];
	if(nodes_info.length >= 3 && get_cfg_clientlist[2]){
		code += '<tr><th style="width:250px"><#MAC_Address#></th><td colspan="3">'+ get_cfg_clientlist[2].model_name + '('+ get_cfg_clientlist[2].mac +')'+'</td></tr>';
		code += '<tr><th></th><th>RSSI</th><th>Tx rate (Mbps)</th><th>Rx rate (Mbps)</th></tr>';

		for(i=0; i<nodes_info[2].length; i++){
			if(i == 2 || get_cfg_clientlist[i] == undefined){
				continue;
			}

			node_array = nodes_info[2][i].split(">");
			if(node_array == "")
				node_array = ["0", "0", "0"];

			code += '<tr><th>'+ get_cfg_clientlist[i].model_name + ' ('+ get_cfg_clientlist[i].mac +')'+'</th><td>'+ node_array[0] +'</td><td>'+ node_array[1] +'</td><td>'+ node_array[2] +'</td></tr>';
		}
	}

	$("#re2_table").html(code);
}

function check_macaddr(obj,flag){ //control hint of input mac address
	if(flag == 1){
		var childsel=document.createElement("div");
		childsel.setAttribute("id","check_mac");
		childsel.style.color="#FFCC00";
		obj.parentNode.appendChild(childsel);
		document.getElementById("check_mac").innerHTML="<#LANHostConfig_ManualDHCPMacaddr_itemdesc#>";		
		document.getElementById("check_mac").style.display = "";
		return false;
	}else if(flag ==2){
		var childsel=document.createElement("div");
		childsel.setAttribute("id","check_mac");
		childsel.style.color="#FFCC00";
		obj.parentNode.appendChild(childsel);
		document.getElementById("check_mac").innerHTML="<#IPConnection_x_illegal_mac#>";
		document.getElementById("check_mac").style.display = "";
		return false;		
	}else{	
		document.getElementById("check_mac") ? document.getElementById("check_mac").style.display="none" : true;
		return true;
	}	
}

function setClientIP(_macaddr){
	document.form.chksta_mac.value = _macaddr;
	var target = clientList[_macaddr];
	if(target.isWL != 0){
		if(target.isWL == "1"){
			document.form.chksta_band.value = 0;
		}
		else{
			document.form.chksta_band.value = 1;
		}
	}

	hideClients_Block();
}

function hideClients_Block(){
	document.getElementById("pull_arrow").src = "/images/arrow-down.gif";
	document.getElementById('ClientList_Block_PC').style.display='none';
}

function pullLANIPList(obj){
	var element = document.getElementById('ClientList_Block_PC');
	var isMenuopen = element.offsetWidth > 0 || element.offsetHeight > 0;
	if(isMenuopen == 0){		
		obj.src = "/images/arrow-top.gif"
		element.style.display = 'block';		
		document.form.chksta_mac.focus();		
	}
	else
		hideClients_Block();
}

function applyRule(){
	document.form.submit();
}

var TARGET = "";
function setSTA(){
	if(document.form.chksta_mac.value == ""){
		alert("<#JS_fieldblank#>");
		document.form.chksta_mac.focus();
		document.form.chksta_mac.select();	
		return false;
	}

	if(!check_macaddr(document.form.chksta_mac, check_hwaddr_flag(document.form.chksta_mac))){
		document.form.chksta_mac.focus();
		document.form.chksta_mac.select();	
		return false;	
	}
	
	clearTimeout(cancelChkSTA);
	$("#sta_table").hide();
	$("#detect_btn").hide();
	$("#detecting_msg").show();
	setTimeout(function(){
		$("#detecting_msg").hide();
		$("#detect_btn").show();
		$("#sta_table").show();
	}, 5000);

	TARGET = document.form.chksta_mac.value;
	applyRule();
	triggerChkSTA();
	chkSTAStatus();
}

function triggerChkSTA(){
	$.ajax({
		url: '/ajax_trigger_chk_sta.asp',
		dataType: 'script',	
		error: function(xhr) {
			setTimeout("triggerChkSTA();", 1000);
		},
		success: function(response){	
			setTimeout("triggerChkSTA();", 5000);		
		}
	});	
}
</script>
</head>
<body onload="initial();">
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>
<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="POST" name="form" action="/apply.cgi" target="hidden_frame">
<input type="hidden" name="current_page" value="Main_ChkSta_Content.asp">
<input type="hidden" name="next_page" value="Main_ChkSta_Content.asp">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="saveNvram">
<input type="hidden" name="action_wait" value="1">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
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
									<div class="formfonttitle"><#Network_Tools#> - <#NetworkTools_ChkSta#></div>
									<div style="margin:10px 0 10px 5px;" class="splitLine"></div>
									<div class="formfontdesc"><#NetworkTools_ChkSta_Desc#></div>
									<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable_table" style="margin-top:8px;">
										<thead>
											<tr>
												<td colspan="3"><#NetworkTools_target#></td>
											</tr>
										</thead>
						
										<tr>
									  		<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,10);"><#PPPConnection_x_MacAddressForISP_itemname#></a></th>
							        		<th><#Interface#></th>
							        		<th><#NetworkTools_Diagnose_btn#></th>
										</tr>			  
									  	<tr>
										  	<!-- client info -->	
											<td width="80%">
												<input type="text" class="input_20_table" maxlength="17" name="chksta_mac" onClick="hideClients_Block();" autocorrect="off" autocapitalize="off" style="margin-left:-12px;width:255px;" onKeyPress="return validator.isHWAddr(this,event)" placeholder="ex: <% nvram_get("lan_hwaddr"); %>">
												<img id="pull_arrow" height="14px;" src="/images/arrow-down.gif" style="position:absolute;*margin-left:-3px;*margin-top:1px;" onclick="pullLANIPList(this);" title="<#select_device_name#>">
												<div id="ClientList_Block_PC" class="clientlist_dropdown"></div>	
						            		</td>
						            		<td>
						            			<select class="input_option" name="chksta_band">
						            				<option value="0">2.4 GHz</option>
						            				<option value="1">5 GHz</option>
						            			</select>
						            		</td>
						            		<td width="20%">
												<div class="apply_gen" style="margin-top:0;background: #475A5F;"> 
													<input id="detect_btn" type="button" class="button_gen" onClick="setSTA();" value="<#CTL_Detect#>">
													<div id="detecting_msg" style="color: #FC0;margin: 6px 30px;display:none"><#QKSet_detect_sanglass#>...</div>
												</div>
						            		</td>
										</tr>	 			  
									</table>
									<table id="sta_table" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable_table" style="margin:8px 0;"></table> 
									<table id="cap_table" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable_table" style="margin:8px 0;"></table>		
									<table id="re1_table" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable_table" style="margin:8px 0;"></table>		
									<table id="re2_table" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable_table" style="margin:8px 0;"></table>		
								</td>
							</tr>
						</table>
					</td>
				</tr>
			</table>
			<!--===================================Ending of Main Content===========================================-->
		</td>
		<td width="10" align="center" valign="top"></td>
	</tr>
</table>
</form>

<form method="post" name="wolform" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="Main_WOL_Content.asp">
<input type="hidden" name="next_page" value="Main_WOL_Content.asp">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_wait" value="3">
<input type="hidden" name="action_script" value="restart_time">
<input type="hidden" name="wollist" value="<% nvram_get("wollist"); %>">
</form>

<div id="footer"></div>
</body>
</html>
