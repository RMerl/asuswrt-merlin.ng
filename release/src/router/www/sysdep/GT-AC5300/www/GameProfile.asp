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
<title><#Web_Title#> - <#Game_Profile#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="device-map/device-map.css">
<script language="JavaScript" type="text/javascript" src="js/jquery.js"></script>
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/help.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script language="JavaScript" type="text/javascript" src="/client_function.js"></script>
<script language="JavaScript" type="text/javascript" src="/validator.js"></script>
<script language="JavaScript" type="text/javascript" src="js/gameProfile.js"></script>
<script>
var wItem2 = new Array();

<% login_state_hook(); %>

var overlib_str0 = new Array();	//Viz add 2011.07 for record longer virtual srvr rule desc
var overlib_str = new Array();	//Viz add 2011.07 for record longer virtual srvr portrange value
var vts_rulelist_array = "<% nvram_char_to_ascii("","game_vts_rulelist"); %>";

function initial(){
	show_menu();
	loadGameOptions();
	setTimeout("showDropdownClientList('setClientIP', 'ip', 'all', 'ClientList_Block', 'pull_arrow', 'online');", 1000);	
	showvts_rulelist();
	addOnlineHelp(document.getElementById("faq"), ["ASUSWRT", "port", "forwarding"]);
	update_game_profile();
}
function extractValue(row, col){
	var obj = document.getElementById('vts_rulelist_table').rows[row].cells[col];
	if(obj.innerHTML.lastIndexOf("...")<0)
		return obj.innerHTML;
	else
		return obj.title;
}
function applyRule(){
	/*f(parent.usb_support){
		if(!validator.numberRange(document.form.vts_ftpport, 1, 65535)){
			return false;	
		}	
	}	*/
	
	var rule_num = document.getElementById('vts_rulelist_table').rows.length;
	var item_num = document.getElementById('vts_rulelist_table').rows[0].cells.length;
	var item_idx = [0,2,3,4,5,1];
	var tmp_value = "";

	for(i=0; i<rule_num; i++){
		tmp_value += "<"		
		for(j=0; j<Math.min(item_num-1, item_idx.length); j++){
			tmp_value += extractValue(i, item_idx[j]);
			if(j != item_num-2)	
				tmp_value += ">";
		}
	}

	if(tmp_value == "<"+"<#IPConnection_VSList_Norule#>" || tmp_value == "<")
		tmp_value = "";	

	document.form.game_vts_rulelist.value = tmp_value;
	
	showLoading();
	document.form.submit();
}

function loadGameOptions(){
	for(i=0;i<gameProfile.profile.length;i++){
		wItem2[i] = new Array();
		var tmp = gameProfile.profile[i];
		wItem2[i].push(tmp.title);
		wItem2[i].push(tmp.port);
		wItem2[i].push(tmp.protocol);
	}

	free_options(document.form.KnownGames);
	add_option(document.form.KnownGames, "<#Select_menu_default#>", 0, 1);
	for(var i = 1; i < wItem2.length; i++)
		add_option(document.form.KnownGames, wItem2[i][0], i, 0);
}

function change_wizard(o, id){
	if(id == "KnownGames"){
		document.form.vts_lport_x_0.value = "";
		for(var i = 0; i < wItem2.length; ++i){
			if(wItem2[i][0] != null && o.value == i){
				if(wItem2[i][2] == "TCP")
					document.form.vts_proto_x_0.options[0].selected = 1;
				else if(wItem2[i][2] == "UDP")
					document.form.vts_proto_x_0.options[1].selected = 1;
				else if(wItem2[i][2] == "BOTH")
					document.form.vts_proto_x_0.options[2].selected = 1;
				else if(wItem2[i][2] == "OTHER")
					document.form.vts_proto_x_0.options[3].selected = 1;
				
				document.form.vts_ipaddr_x_0.value = login_ip_str();
				document.form.vts_port_x_0.value = wItem2[i][1];
				document.form.vts_desc_x_0.value = wItem2[i][0];
				
				break;
			}
		}
	}
}

/*------------ Mouse event of fake LAN IP select menu {-----------------*/
function setClientIP(num){
	document.form.vts_ipaddr_x_0.value = num;
	hideClients_Block();
}

function pullLANIPList(obj){
	var element = document.getElementById('ClientList_Block');
	var isMenuopen = element.offsetWidth > 0 || element.offsetHeight > 0;
	if(isMenuopen == 0){		
		obj.src = "/images/arrow-top.gif"
		element.style.display = 'block';		
		document.form.vts_ipaddr_x_0.focus();		
	}
	else
		hideClients_Block();
}

function hideClients_Block(){
	document.getElementById("pull_arrow").src = "/images/arrow-down.gif";
	document.getElementById('ClientList_Block').style.display='none';
	validator.validIPForm(document.form.vts_ipaddr_x_0, 0);
}
/*----------} Mouse event of fake LAN IP select menu-----------------*/

function addRow(obj, head){
	if(head == 1)
		vts_rulelist_array += "<"
	else
		vts_rulelist_array += ">"
			
	vts_rulelist_array += obj.value;
	obj.value = "";
}

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

function addRow_Group(upper){
	if(validForm()){
		if('<% nvram_get("vts_enable_x"); %>' != "1")
			document.form.vts_enable_x[0].checked = true;
		
		var rule_num = document.getElementById('vts_rulelist_table').rows.length;
		var item_num = document.getElementById('vts_rulelist_table').rows[0].cells.length;	
		if(rule_num >= upper){
				alert("<#JS_itemlimit1#> " + upper + " <#JS_itemlimit2#>");
				return false;
		}	
		
//Viz check same rule  //match(out port+out_proto) is not accepted
	if(item_num >=2){
		for(i=0; i<rule_num; i++){
				if(entry_cmp(document.getElementById('vts_rulelist_table').rows[i].cells[4].innerHTML.toLowerCase(), document.form.vts_proto_x_0.value.toLowerCase(), 3)==0 
				|| document.form.vts_proto_x_0.value == 'BOTH'
				|| document.getElementById('vts_rulelist_table').rows[i].cells[4].innerHTML == 'BOTH'){
						
						if(overlib_str[i]){
							if(document.form.vts_port_x_0.value == overlib_str[i]){
									alert("<#JS_duplicate#>");
									document.form.vts_port_x_0.value =="";
									document.form.vts_port_x_0.focus();
									document.form.vts_port_x_0.select();							
									return false;
							}
						}else{
							if(document.form.vts_port_x_0.value == document.getElementById('vts_rulelist_table').rows[i].cells[1].innerHTML){
									alert("<#JS_duplicate#>");
									document.form.vts_port_x_0.value =="";
									document.form.vts_port_x_0.focus();
									document.form.vts_port_x_0.select();							
									return false;
							}
						}	
				}	
			}				
		}
			
		addRow(document.form.vts_desc_x_0 ,1);
		addRow(document.form.vts_port_x_0, 0);
		addRow(document.form.vts_ipaddr_x_0, 0);
		addRow(document.form.vts_lport_x_0, 0);
		addRow(document.form.vts_proto_x_0, 0);
		addRow(document.form.vts_target_x_0, 0);		
		document.form.vts_proto_x_0.value="TCP";
		showvts_rulelist();
		return true;
	}
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


function edit_Row(r){ 	
	var i=r.parentNode.parentNode.rowIndex;
  	
	document.form.vts_desc_x_0.value = document.getElementById('vts_rulelist_table').rows[i].cells[0].innerHTML;
	document.form.vts_port_x_0.value = document.getElementById('vts_rulelist_table').rows[i].cells[1].innerHTML; 
	document.form.vts_ipaddr_x_0.value = document.getElementById('vts_rulelist_table').rows[i].cells[2].innerHTML; 
	document.form.vts_lport_x_0.value = document.getElementById('vts_rulelist_table').rows[i].cells[3].innerHTML;
	document.form.vts_proto_x_0.value = document.getElementById('vts_rulelist_table').rows[i].cells[4].innerHTML;
	
  del_Row(r);	
}

function del_Row(r){
  var i=r.parentNode.parentNode.rowIndex;
  document.getElementById('vts_rulelist_table').deleteRow(i);
  
  var vts_rulelist_value = "";
	for(k=0; k<document.getElementById('vts_rulelist_table').rows.length; k++){
		for(j=0; j<document.getElementById('vts_rulelist_table').rows[k].cells.length-1; j++){
			if(j == 0)	
				vts_rulelist_value += "<";
			else
				vts_rulelist_value += ">";
				
			if(document.getElementById('vts_rulelist_table').rows[k].cells[j].innerHTML.lastIndexOf("...")<0){
				vts_rulelist_value += document.getElementById('vts_rulelist_table').rows[k].cells[j].innerHTML;
			}else{
				vts_rulelist_value += document.getElementById('vts_rulelist_table').rows[k].cells[j].title;
			}			
		}
	}
	
	vts_rulelist_array = vts_rulelist_value;
	if(vts_rulelist_array == "")
		showvts_rulelist();
}
var overlib_desc = new Array();	//Viz add 2011.07 for record longer virtual srvr rule desc
var overlib_port = new Array();	//Viz add 2011.07 for record longer virtual srvr portrange value
var overlib_src = new Array();	//Viz add 2011.07 for record longer virtual srvr portrange value

function showvts_rulelist(){
	var vts_rulelist_row = decodeURIComponent(vts_rulelist_array).split('<');
	var code = "";

	code +='<table width="100%" cellspacing="0" cellpadding="4" align="center" class="list_table" id="vts_rulelist_table">';
	if(vts_rulelist_row.length == 1)
		code +='<tr><td style="color:#FFCC00;" colspan="6"><#IPConnection_VSList_Norule#></td></tr>';
	else{
		for(var i = 1; i < vts_rulelist_row.length; i++){
			overlib_desc[i] ="";
			overlib_port[i] ="";
			overlib_src[i] ="";
			code +='<tr id="row'+i+'">';
			var vts_rulelist_col = vts_rulelist_row[i].split('>');
			var desc = vts_rulelist_col[0];
			var port_range = vts_rulelist_col[1];
			var dest_ip = vts_rulelist_col[2];
			var dest_port = vts_rulelist_col[3];
			var protocol = vts_rulelist_col[4];
			var source_target = (vts_rulelist_col.length < 6) ? "" : vts_rulelist_col[5];

			if(desc.length >23){
				overlib_desc[i] += desc;
				desc = desc.substring(0, 21)+"...";
				code +='<td style="width:143px" title="'+overlib_desc[i]+'">'+ desc +'</td>';
			}else{
				code +='<td style="width:143px">'+ desc +'</td>';
			}

			if(source_target.length >13){
				overlib_src[i] += source_target;
				source_target = source_target.substring(0, 11)+"...";
				code +='<td style="width:143px" title='+overlib_src[i]+'>'+ source_target +'</td>';
			}else{
				code +='<td style="width:143px">'+ source_target +'</td>';
			}

			if(port_range.length >13){
				overlib_port[i] += port_range;
				port_range = port_range.substring(0, 11)+"...";
				code +='<td style="width:102px" title='+overlib_port[i]+'>'+ port_range +'</td>';
			}else{
				code +='<td style="width:102px">'+ port_range +'</td>';
			}

			code +='<td style="width:131px">'+ dest_ip +'</td>';
			code +='<td style="width:67px">'+ dest_port +'</td>';
			code +='<td style="width:70px">'+ protocol +'</td>';
			code +='<td style="width:42px"><!--input class="edit_btn" onclick="edit_Row(this);" value=""/-->';
			code +='<input class="remove_btn" onclick="del_Row(this);" value=""/></td></tr>';
		}
	}
  code +='</table>';
	document.getElementById("vts_rulelist_Block").innerHTML = code;	     
}

function changeBgColor(obj, num){
	if(obj.checked)
 		document.getElementById("row" + num).style.background='#FF9';
	else
 		document.getElementById("row" + num).style.background='#FFF';
}

function update_game_profile() {
  $.ajax({
    url: 'https://nw-dlcdnet.asus.com/plugin/js/gameProfile.js',
    dataType: 'script',	
    error: function(xhr) {
		setTimeout("update_game_profile();", 2000);
    },
    success: function(response){
    	loadGameOptions();
    }
  });
}

</script>
</head>

<body onload="initial();" onunLoad="return unload_body();">
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

<table class="content" align="center" cellpadding="0" cellspacing="0" >
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
		<td valign="top" >
		
<table width="760px" border="0" cellpadding="4" cellspacing="0" class="FormTitle" id="FormTitle">
	<tbody>
	<tr>
		<td bgcolor="#4D595D" valign="top"  >
		<div>&nbsp;</div>
		<div class="formfonttitle"><#Game_Profile#></div>
		<div style="margin-left:5px;margin-top:10px;margin-bottom:10px"><img src="/images/New_ui/export/line_export.png"></div>
		<div>
			<table width="700px" style="margin-left:25px;">
				<tr>
					<td>
						<div id="guest_image" style="background: url(images/ROG_GameProfile.png);width: 261px;height: 148px;"></div>
					</td>
					<td>&nbsp;&nbsp;</td>
					<td style="font-size: 14px;">
						<div class="formfontdesc" style="font-size:20px;"><#Game_Profile_desc1#></div>
						<div class="formfontdesc"><#Game_Profile_desc2#></div>

					</td>
				</tr>
			</table>
		</div>		
		<div>
			<div style="width:644px;height:175px;background:url('images/New_ui/game_logos.png');margin: 0 auto"></div>
			<div style="margin: 10px 10px 10px 20px;font-size:13px;"><#Game_Profile_desc3#></div>
			<div style="margin: 10px 10px 10px 20px;text-align: right"><#Game_Profile_desc4#></div>
		</div>	
		<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable">
					  <thead>
					  <tr>
						<td colspan="4"><#t2BC#></td>
					  </tr>
					  </thead>

          	<tr>
            	<th><#IPConnection_VServerEnable_itemname#><input type="hidden" name="vts_num_x_0" value="<% nvram_get("vts_num_x"); %>" readonly="1" /></th>
            	<td>
								<input type="radio" value="1" name="vts_enable_x" class="content_input_fd" onclick="return change_common_radio(this, 'IPConnection', 'vts_enable_x', '1')" <% nvram_match("vts_enable_x", "1", "checked"); %>><#checkbox_Yes#>
								<input type="radio" value="0" name="vts_enable_x" class="content_input_fd" onclick="return change_common_radio(this, 'IPConnection', 'vts_enable_x', '0')" <% nvram_match("vts_enable_x", "0", "checked"); %>><#checkbox_No#>
            	</td>
		</tr>
		  
		<tr>
			<th><#IPConnection_VSList_gameitemdesc#></th>
			<td id="VSGameList">
				<select name="KnownGames" id="KnownGames" class="input_option" onchange="change_wizard(this, 'KnownGames');"></select>
			</td>
		</tr>
      	</table>			
      	
		<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable_table">
			<thead>
			<tr>
              	<td colspan="7"><#IPConnection_VSList_title#>&nbsp;(<#List_limit#>&nbsp;32)</td>
            </tr>
 		  	</thead>
 		  	
          		<tr>
			<th><#BM_UserList1#></th>
            		<th><a class="hintstyle" href="javascript:void(0);" onClick=""><#IPConnection_VSList_SourceTarget#></a></th>								
            		<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,24);"><#FirewallConfig_LanWanSrcPort_itemname#></a></th>
            		<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,25);"><#IPConnection_VServerIP_itemname#></a></th>
            		<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,26);"><#IPConnection_VServerLPort_itemname#></a></th>
            		<th><#IPConnection_VServerProto_itemname#></th>
			<th><#list_add_delete#></th>
          		</tr>  
          		        
          		<tr>
  				<td style="width:150px">
  					<input type="text" maxlength="30" class="input_15_table" name="vts_desc_x_0" onKeyPress="return validator.isString(this, event)" autocorrect="off" autocapitalize="off"/>
  				</td>
  				<td style="width:148px">
  					<input type="text" maxlength="30" class="input_15_table" name="vts_target_x_0" onKeyPress="return validator.isString(this, event)" autocorrect="off" autocapitalize="off"/>
  				</td>  				
        			<td style="width:106px">
					<input type="text" maxlength="" class="input_12_table" name="vts_port_x_0" onkeypress="return validator.isPortRange(this, event)" autocorrect="off" autocapitalize="off"/>
				</td>
				<td style="width:157px">
					<input type="text" maxlength="15" class="input_12_table" name="vts_ipaddr_x_0" align="left" onkeypress="return validator.isIPAddr(this, event)" style="float:left;"/ autocomplete="off" onClick="hideClients_Block();" autocorrect="off" autocapitalize="off">
					<img id="pull_arrow" height="14px;" src="images/arrow-down.gif" align="right" onclick="pullLANIPList(this);" title="<#select_IP#>">
					<div id="ClientList_Block" class="clientlist_dropdown" style="margin-left:2px;margin-top:25px;"></div>
				</td>
				<td style="width:70px">
					<input type="text" maxlength="5"  class="input_6_table" name="vts_lport_x_0" onKeyPress="return validator.isNumber(this,event);" autocorrect="off" autocapitalize="off"/>
				</td>
				<td style="width:73px">
					<select name="vts_proto_x_0" class="input_option">
						<option value="TCP">TCP</option>
						<option value="UDP">UDP</option>
						<option value="BOTH">BOTH</option>
						<option value="OTHER">OTHER</option>
					</select>
				</td>
				<td style="width:45px">
					<input type="button" class="add_btn" onClick="addRow_Group(32);" name="vts_rulelist2" value="">
				</td>
				</tr>
				</table>		
				
				<div id="vts_rulelist_Block"></div>
				
				<div class="apply_gen">
					<input name="button" type="button" class="button_gen" onclick="applyRule();" value="<#CTL_apply#>"/>
				</div>

		
		</td>
		</tr>
	</tbody>	
	
	</table>
</td>
         
        </tr>
      </table>				
		<!--===================================Ending of Main Content===========================================-->		
	</td>
		
    <td width="10" align="center" valign="top">&nbsp;</td>
	</tr>
</table>

<div id="footer"></div>
</form>
<script>

</script>
</body>
</html>
