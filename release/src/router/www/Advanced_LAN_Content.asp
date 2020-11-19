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
<title><#Web_Title#> - <#menu5_2_1#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="other.css">
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/validator.js"></script>

<script><% wanlink(); %>

var origin_lan_ip = '<% nvram_get("lan_ipaddr"); %>';
var origin_lan_mask = '<% nvram_get("lan_netmask"); %>';

if(pptpd_support){	
	var pptpd_clients = '<% nvram_get("pptpd_clients"); %>';
	var pptpd_clients_subnet = pptpd_clients.split(".")[0]+"."
				+pptpd_clients.split(".")[1]+"."
				+pptpd_clients.split(".")[2]+".";
	var pptpd_clients_start_ip = parseInt(pptpd_clients.split(".")[3].split("-")[0]);
	var pptpd_clients_end_ip = parseInt(pptpd_clients.split("-")[1]);
}

if(tagged_based_vlan){
	var jsFile = document.createElement("script");
	jsFile.setAttribute("type","text/javascript");
	jsFile.setAttribute("src", "js/subnet_rule.js");
	document.getElementsByTagName("head")[0].appendChild(jsFile);
}

function initial(){
	show_menu();
	
	if(sw_mode == "1"){
		 document.getElementById("table_proto").style.display = "none";
		 document.getElementById("table_gateway").style.display = "none";
		 document.getElementById("table_dnsenable").style.display = "none";
		 document.getElementById("table_dns1").style.display = "none";
		 document.getElementById("table_dns2").style.display = "none";
		 /*  Not needed to show out. Viz 2012.04
		 if(pptpd_support){
		 	var chk_vpn = check_vpn();
			 if(chk_vpn){
		 		document.getElementById("VPN_conflict").style.display = "";	
			 }
		 }*/
	}
	else{
		display_lan_dns(<% nvram_get("lan_dnsenable_x"); %>);
		change_ip_setting('<% nvram_get("lan_proto"); %>');
	}

	if(tagged_based_vlan){
		var subnet_netmask = origin_lan_ip + '/' + netmask_to_bits(origin_lan_mask);

		parse_LanToLanRoute_to_object();
		get_LanToLanRoute(subnet_netmask);
	}

	if(redirect_dname_support)
		document.getElementById("redirect_dname_tr").style.display = "";
}

function applyRule(){
	if(validForm()){
		if(based_modelid == "MAP-AC1300" || based_modelid == "MAP-AC2200" || based_modelid == "VZW-AC1300" || based_modelid == "MAP-AC1750")
			alert("By applying new LAN settings, please reboot all Lyras connected to main Lyra manually.");

		if(tagged_based_vlan){
			var subnet_netmask = document.form.lan_ipaddr.value + '/' + netmask_to_bits(document.form.lan_netmask.value);

			update_LanToLanRoute_array(subnet_netmask)
			save_LanToLanRoute();
			document.form.subnet_rulelist_ext.disabled = false;
			document.form.subnet_rulelist_ext.value = subnet_rulelist_ext;
		}

		if(document.form.redirect_dname.value != "<% nvram_get("redirect_dname"); %>"){
			document.form.action_wait.value = "<% get_default_reboot_time(); %>";
			document.form.action_script.value = "reboot";
		}

		showLoading();
		document.form.submit();
	}
}

// test if WAN IP & Gateway & DNS IP is a valid IP
// DNS IP allows to input nothing
function valid_IP(obj_name, obj_flag){
		// A : 1.0.0.0~126.255.255.255
		// B : 127.0.0.0~127.255.255.255 (forbidden)
		// C : 128.0.0.0~255.255.255.254
		var A_class_start = inet_network("1.0.0.0");
		var A_class_end = inet_network("126.255.255.255");
		var B_class_start = inet_network("127.0.0.0");
		var B_class_end = inet_network("127.255.255.255");
		var C_class_start = inet_network("128.0.0.0");
		var C_class_end = inet_network("255.255.255.255");		
		var ip_obj = obj_name;
		var ip_num = inet_network(ip_obj.value);

		if(obj_flag == "DNS" && ip_num == -1){ //DNS allows to input nothing
			return true;
		}
		
		if(obj_flag == "GW" && ip_num == -1){ //GW allows to input nothing
			return true;
		}

		if(ip_num > A_class_start && ip_num < A_class_end){
		   obj_name.value = ipFilterZero(ip_obj.value);
			return true;
		}
		else if(ip_num > B_class_start && ip_num < B_class_end){
			alert(ip_obj.value+" <#JS_validip#>");
			ip_obj.focus();
			ip_obj.select();
			return false;
		}
		else if(ip_num > C_class_start && ip_num < C_class_end){
			obj_name.value = ipFilterZero(ip_obj.value);
			return true;
		}
		else{
			alert(ip_obj.value+" <#JS_validip#>");
			ip_obj.focus();
			ip_obj.select();
			return false;
		}
}

function validForm(){
	var alert_str = "";

	if(document.form.lan_hostname.value.length > 0)
		alert_str = validator.host_name(document.form.lan_hostname);
	else
		alert_str = "<#JS_fieldblank#>";
	if(alert_str != ""){
		showtext(document.getElementById("alert_hostname"), alert_str);
		document.getElementById("alert_hostname").style.display = "";
		document.form.lan_hostname.focus();
		document.form.lan_hostname.select();
		return false;
	}else{
		document.getElementById("alert_hostname").style.display = "none";
	}
 
	if(document.form.lan_domain.value.length > 0)
		alert_str = validator.domainName(document.form.lan_domain);
	if(alert_str != ""){
		showtext(document.getElementById("alert_domain"), alert_str);
		document.getElementById("alert_domain").style.display = "";
		document.form.lan_domain.focus();
		document.form.lan_domain.select();
		return false;
	}else{
		document.getElementById("alert_domain").style.display = "none";
	}

	if(sw_mode == 2 || sw_mode == 3  || sw_mode == 4){
		if(document.form.lan_dnsenable_x_radio[0].checked == 1)
			document.form.lan_dnsenable_x.value = 1;
		else
			document.form.lan_dnsenable_x.value = 0;
	
		if(document.form.lan_proto_radio[0].checked == 1){
			document.form.lan_proto.value = "dhcp";
			return true;
		}
		else{			
			document.form.lan_proto.value = "static";
			if(!valid_IP(document.form.lan_ipaddr, "")) return false;  //AP LAN IP 
			if(!valid_IP(document.form.lan_gateway, "GW"))return false;  //AP Gateway IP		

			if(document.form.lan_gateway.value == document.form.lan_ipaddr.value){
					alert("<#IPConnection_warning_WANIPEQUALGatewayIP#>");
					document.form.lan_gateway.focus();
					document.form.lan_gateway.select();					
					return false;
			}				

			return true;
		}	
	}else
		document.form.lan_proto.value = "static";		
	
	//router mode : WAN IP conflict with LAN ip subnet when wan is connected
	var wanip_obj = wanlink_ipaddr();
	//alert(wanip_obj +" , "+document.form.wan_netmask_x.value+" , "+document.form.wan_ipaddr_x.value);
	if(wanip_obj != "0.0.0.0"){				
		if(sw_mode == 1 && document.form.wan_ipaddr_x.value != "0.0.0.0" && document.form.wan_ipaddr_x.value != "" 
				&& document.form.wan_netmask_x.value != "0.0.0.0" && document.form.wan_netmask_x.value != ""){
			if(validator.matchSubnet2(document.form.wan_ipaddr_x.value, document.form.wan_netmask_x, document.form.lan_ipaddr.value, document.form.lan_netmask)){
						document.form.lan_ipaddr.focus();
						document.form.lan_ipaddr.select();
						alert("<#IPConnection_x_WAN_LAN_conflict#>");
						return false;
			}						
		}	
	}	
	
	var ip_obj = document.form.lan_ipaddr;
	var ip_num = inet_network(ip_obj.value);
	var ip_class = "";

	// test if netmask is valid.
	var netmask_obj = document.form.lan_netmask;
	var netmask_num = inet_network(netmask_obj.value);
	var netmask_reverse_num = ~netmask_num;
	var default_netmask = "";
	var wrong_netmask = 0;

	if(netmask_num < 0) wrong_netmask = 1;	

	if(ip_class == 'A')
		default_netmask = "255.0.0.0";
	else if(ip_class == 'B')
		default_netmask = "255.255.0.0";
	else
		default_netmask = "255.255.255.0";
	
	var test_num = netmask_reverse_num;
	while(test_num != 0){
		if((test_num+1)%2 == 0)
			test_num = (test_num+1)/2-1;
		else{
			wrong_netmask = 1;
			break;
		}
	}
	if(wrong_netmask == 1){
		alert(netmask_obj.value+" <#JS_validip#>");
		netmask_obj.value = default_netmask;
		netmask_obj.focus();
		netmask_obj.select();
		return false;
	}
	
	var subnet_head = getSubnet(ip_obj.value, netmask_obj.value, "head");
	var subnet_end = getSubnet(ip_obj.value, netmask_obj.value, "end");
	
	if(ip_num == subnet_head || ip_num == subnet_end){
		alert(ip_obj.value+" <#JS_validip#>");
		ip_obj.focus();
		ip_obj.select();
		return false;
	}
	
	// check IP changed or not
	if(sw_mode == "1" &&
	   (origin_lan_ip != document.form.lan_ipaddr.value ||
	    origin_lan_mask != document.form.lan_netmask.value)) {
		var pool_change = calculatorIPPoolRange();
		if(!pool_change)
			return false;
		else{
			document.form.lan_ipaddr_rt.value = document.form.lan_ipaddr.value;
			document.form.lan_netmask_rt.value = document.form.lan_netmask.value;			
		}
	}				

	return true;
}

function done_validating(action){
	refreshpage();
}


function calculatorIPPoolRange() {
	var gatewayIPArray = document.form.lan_ipaddr.value.split(".");
	var netMaskArray = document.form.lan_netmask.value.split(".");
	var ipPoolStartArray  = new Array();
	var ipPoolEndArray  = new Array();
	var ipPoolStart = "";
	var ipPoolEnd = "";

	ipPoolStartArray[0] = (gatewayIPArray[0] & 0xFF) & (netMaskArray[0] & 0xFF);
	ipPoolStartArray[1] = (gatewayIPArray[1] & 0xFF) & (netMaskArray[1] & 0xFF);
	ipPoolStartArray[2] = (gatewayIPArray[2] & 0xFF) & (netMaskArray[2] & 0xFF);
	ipPoolStartArray[3] = (gatewayIPArray[3] & 0xFF) & (netMaskArray[3] & 0xFF);
	ipPoolStartArray[3] += 1;

	ipPoolEndArray[0] = (gatewayIPArray[0] & 0xFF) | (~netMaskArray[0] & 0xFF);
	ipPoolEndArray[1] = (gatewayIPArray[1] & 0xFF) | (~netMaskArray[1] & 0xFF);
	ipPoolEndArray[2] = (gatewayIPArray[2] & 0xFF) | (~netMaskArray[2] & 0xFF);
	ipPoolEndArray[3] = (gatewayIPArray[3] & 0xFF) | (~netMaskArray[3] & 0xFF);
	ipPoolEndArray[3] -= 1;

	ipPoolStart = ipPoolStartArray[0] + "." + ipPoolStartArray[1] + "." + ipPoolStartArray[2] + "." + ipPoolStartArray[3];
	if(inet_network(ipPoolStart) <= inet_network(document.form.lan_ipaddr.value)) {
		ipPoolStart = ipPoolStartArray[0] + "." + ipPoolStartArray[1] + "." + ipPoolStartArray[2] + "." + (parseInt(ipPoolStartArray[3]) + 1);
	}
	ipPoolEnd = ipPoolEndArray[0] + "." + ipPoolEndArray[1] + "." + ipPoolEndArray[2] + "." + ipPoolEndArray[3];

	if((document.form.dhcp_start.value != ipPoolStart) || (document.form.dhcp_end.value != ipPoolEnd)){
			if(confirm("<#JS_DHCP1#>")){
				document.form.dhcp_start.value = ipPoolStart;
				document.form.dhcp_end.value = ipPoolEnd;
			}else{
				return false;
			}
	}

	return true;
}

function display_lan_dns(flag){
	inputCtrl(document.form.lan_dns1_x, conv_flag(flag));
	inputCtrl(document.form.lan_dns2_x, conv_flag(flag));
}

function change_ip_setting(flag){
	if(flag == "dhcp"){
		inputCtrl(document.form.lan_dnsenable_x_radio[0], 1);
		inputCtrl(document.form.lan_dnsenable_x_radio[1], 1);
		flag = 1; 
	}
	else if(flag == "static"){
		document.form.lan_dnsenable_x_radio[1].checked = 1;
		inputCtrl(document.form.lan_dnsenable_x_radio[0], 0);
		inputCtrl(document.form.lan_dnsenable_x_radio[1], 0);
		flag = 0; 
		display_lan_dns(flag);
	}

	inputCtrl(document.form.lan_ipaddr, conv_flag(flag));
	inputCtrl(document.form.lan_netmask, conv_flag(flag));
	inputCtrl(document.form.lan_gateway, conv_flag(flag));
}

function conv_flag(_flag){
	if(_flag == 0)
		_flag = 1;
	else
		_flag = 0;		
	return _flag;
}

function check_vpn(){		//true: lAN ip & VPN client ip conflict
		var lan_ip_subnet = origin_lan_ip.split(".")[0]+"."+origin_lan_ip.split(".")[1]+"."+origin_lan_ip.split(".")[2]+".";
		var lan_ip_end = parseInt(origin_lan_ip.split(".")[3]);
		if(lan_ip_subnet == pptpd_clients_subnet && lan_ip_end >= pptpd_clients_start_ip && lan_ip_end <= pptpd_clients_end_ip){
				return true;
		}else{
				return false;	
		}		
}
</script>
</head>

<body onload="initial();" onunLoad="return unload_body();" class="bg">
<div id="TopBanner"></div>
<div id="hiddenMask" class="popup_bg" style="z-index:10000;">
	<table cellpadding="5" cellspacing="0" id="dr_sweet_advise" class="dr_sweet_advise" align="center">
		<tr>
		<td>
			<div class="drword" id="drword" style="height:110px;"><#Main_alert_proceeding_desc4#> <#Main_alert_proceeding_desc1#>...
				<br/>
				<br/>
	    </div>
		  <div class="drImg"><img src="images/alertImg.png"></div>
			<div style="height:70px;"></div>
		</td>
		</tr>
	</table>
<!--[if lte IE 6.5]><iframe class="hackiframe"></iframe><![endif]-->
</div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="Advanced_LAN_Content.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="apply_new">
<input type="hidden" name="action_script" value="restart_net_and_phy">
<input type="hidden" name="action_wait" value="35">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="wan_ipaddr_x" value="<% nvram_get("wan0_ipaddr"); %>">
<input type="hidden" name="wan_netmask_x" value="<% nvram_get("wan0_netmask"); %>" >
<input type="hidden" name="wan_proto" value="<% nvram_get("wan_proto"); %>">
<input type="hidden" name="lan_proto" value="<% nvram_get("lan_proto"); %>">
<input type="hidden" name="lan_dnsenable_x" value="<% nvram_get("lan_dnsenable_x"); %>">
<input type="hidden" name="lan_ipaddr_rt" value="<% nvram_get("lan_ipaddr_rt"); %>">
<input type="hidden" name="lan_netmask_rt" value="<% nvram_get("lan_netmask_rt"); %>">
<input type="hidden" name="subnet_rulelist_ext" value='<% nvram_get("subnet_rulelist_ext"); %>' disabled>

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
		<td align="left" valign="top">
  <table width="760px" border="0" cellpadding="5" cellspacing="0" class="FormTitle" id="FormTitle">
	<tbody>
	<tr>
		  <td bgcolor="#4D595D" valign="top">
		  <div>&nbsp;</div>
		  <div class="formfonttitle"><#menu5_2#> - <#menu5_2_1#></div>
		  <div style="margin:10px 0 10px 5px;" class="splitLine"></div>
      <div class="formfontdesc"><#LANHostConfig_display1_sectiondesc#></div>
      <div id="VPN_conflict" class="formfontdesc" style="color:#FFCC00;display:none;"><#LANHostConfig_display1_sectiondesc2#></div>
		  
		  <table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
		  
		  <tr>
			<th>
			  <a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,13);"><#LANHostConfig_x_DDNSHostNames_itemname#></a>
			</th>
			<td>
			  <input type="text" maxlength="32" class="input_32_table" name="lan_hostname" value="<% nvram_get("lan_hostname"); %>" onkeypress="return validator.isString(this, event)" autocorrect="off" autocapitalize="off"><br/>
			  <span id="alert_hostname" style="color:#FC0;"></span>
			</td>
		  </tr>

		  <tr>
			<th>
			  <a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,2);"><#LANHostConfig_DomainName_itemname#></a>
			</th>
			<td>
			  <input type="text" maxlength="32" class="input_32_table" name="lan_domain" value="<% nvram_get("lan_domain"); %>" onkeypress="return validator.isString(this, event)" autocorrect="off" autocapitalize="off"><br/>
			  <span id="alert_domain" style="color:#FC0;"></span>
			</td>
		  </tr>

			<tr id="table_proto">
			<th width="30%"><#LANHostConfig_x_LAN_DHCP_itemname#></th>
			<td>
				<input type="radio" name="lan_proto_radio" class="input" onclick="change_ip_setting('dhcp')" value="dhcp" <% nvram_match("lan_proto", "dhcp", "checked"); %>><#checkbox_Yes#>
				<input type="radio" name="lan_proto_radio" class="input" onclick="change_ip_setting('static')" value="static" <% nvram_match("lan_proto", "static", "checked"); %>><#checkbox_No#>
			</td>
			</tr>
                 <tr>
			<th width="30%">
			  <a class="hintstyle" href="javascript:void(0);" onClick="openHint(4,1);"><#IPConnection_ExternalIPAddress_itemname#></a>
			</th>			
			<td>
			  <input type="text" maxlength="15" class="input_15_table" id="lan_ipaddr" name="lan_ipaddr" value="<% nvram_get("lan_ipaddr"); %>" onKeyPress="return validator.isIPAddr(this, event);" autocorrect="off" autocapitalize="off">
			</td>
		  </tr>
		  
		  <tr>
			<th>
			  <a class="hintstyle"  href="javascript:void(0);" onClick="openHint(4,2);"><#IPConnection_x_ExternalSubnetMask_itemname#></a>
			</th>
			<td>
				<input type="text" maxlength="15" class="input_15_table" name="lan_netmask" value="<% nvram_get("lan_netmask"); %>" onkeypress="return validator.isIPAddr(this, event);" autocorrect="off" autocapitalize="off">
			  <input type="hidden" name="dhcp_start" value="<% nvram_get("dhcp_start"); %>">
			  <input type="hidden" name="dhcp_end" value="<% nvram_get("dhcp_end"); %>">
			</td>
		  </tr>

			<tr id="table_gateway">
			<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,3);"><#IPConnection_x_ExternalGateway_itemname#></a></th>
			<td>
				<input type="text" name="lan_gateway" maxlength="15" class="input_15_table" value="<% nvram_get("lan_gateway"); %>" onKeyPress="return validator.isIPAddr(this, event);" autocorrect="off" autocapitalize="off">
			</td>
			</tr>

			<tr id="table_dnsenable">
      <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,12);"><#IPConnection_x_DNSServerEnable_itemname#></a></th>
			<td>
				<input type="radio" name="lan_dnsenable_x_radio" value="1" onclick="display_lan_dns(1)" <% nvram_match("lan_dnsenable_x", "1", "checked"); %> /><#checkbox_Yes#>
			  <input type="radio" name="lan_dnsenable_x_radio" value="0" onclick="display_lan_dns(0)" <% nvram_match("lan_dnsenable_x", "0", "checked"); %> /><#checkbox_No#>
			</td>
      </tr>          		
          		
      <tr id="table_dns1">
      <th>
				<a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,13);"><#IPConnection_x_DNSServer1_itemname#></a>
			</th>
      <td>
				<input type="text" maxlength="15" class="input_15_table" name="lan_dns1_x" value="<% nvram_get("lan_dns1_x"); %>" onkeypress="return validator.isIPAddr(this, event)" autocorrect="off" autocapitalize="off">
			</td>
      </tr>

      <tr id="table_dns2">
      <th>
				<a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,14);"><#IPConnection_x_DNSServer2_itemname#></a>
			</th>
      <td>
				<input type="text" maxlength="15" class="input_15_table" name="lan_dns2_x" value="<% nvram_get("lan_dns2_x"); %>" onkeypress="return validator.isIPAddr(this, event)" autocorrect="off" autocapitalize="off" >
			</td>
      </tr>  
      <tr id="redirect_dname_tr" style="display:none;">
      <th>Redirect DNS</th>
      <td>
				<input type="radio" name="redirect_dname" value="1" <% nvram_match("redirect_dname", "1", "checked"); %>><#checkbox_Yes#>
				<input type="radio" name="redirect_dname" value="0" <% nvram_match("redirect_dname", "0", "checked"); %>><#checkbox_No#>
			</td>
      </tr>

		</table>	
		

		<div class="apply_gen">
			<input class="button_gen" onclick="applyRule()" type="button" value="<#CTL_apply#>"/>
		</div>

	
		
	  </td>
	</tr>

	</tbody>	
  </table>		
					
		</td>
	</form>					
				</tr>
			</table>				
			<!--===================================End of Main Content===========================================-->
</td>

    <td width="10" align="center" valign="top">&nbsp;</td>
	</tr>
</table>

<div id="footer"></div>
</body>
</html>
