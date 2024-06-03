<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title><#Web_Title#> - VLAN Switch</title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="other.css">
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/detect.js"></script>
<script type="text/javascript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/js/httpApi.js"></script>
<style>
	li {
		font-family: monospace;
	}
	.role {
		color:#A4B7C3;
		margin-left: 10px;
	}
	.model {
		font-size:20px;
		font-weight:bold;
		margin-top:-5px;
	}
	.status_note {
		color:#A4B7C3;
		margin-top:18px;
	}
	.status_note_status_idx_null {
		color:#A4B7C3;
		margin-top:39px;
	}
	.status_note_white {
		color:#A4B7C3;
		margin-top:2px;
		font-size:12px;
	}
	.status_note_white_status_idx_null {
		color:#A4B7C3;
		margin-top:26px;
		font-size:12px;
	}

	.ul-align{
		margin-left:-25px;
	}

	.center-align{
		text-align:center;
	}

	.FormTable thead td{
		text-align: center;
	}

	select {
		width: 90%;
	}

	.port_status_table{
		text-align: center;
		width: 99%;
		margin: 0 auto;
	}
.port_status_bg{
	display: flex;
	align-items: flex-start;
	height: auto;
	position: relative;
	margin-top: 15px;
}
.port_status_bg:before{
	content: "";
	position: absolute;
	left: 0%;
	bottom: -3px;
	height: 1px;
	width: 95%;
	border-bottom: 1px solid #3E4E59;
}
.port_status_bg > div{
	display: flex;
	flex-wrap: wrap;
	height: auto;
	position: relative;
}
.port_status_bg .label_W_bg{
	width: auto;
	max-width: 92px;
	margin-right: 6px;
}
.port_status_bg .label_W_bg:before{
	position: absolute;
	content: "";
	right: -4px;
	top: -4%;
	height: 100%;
	border-left: 1px solid #3E4E59;
}
.port_status_bg .label_L_bg:before{
	position: absolute;
	content: "";
	top: -4%;
	height: 100%;
	border-right: 1px solid #3E4E59;
	left: -6px;
}
.port_status_bg .label_W_bg > div, .port_status_bg .label_L_bg > div{
	display: flex;
	width: 46px;
	height: auto;
	flex-direction: column;
	align-items: center;
	margin-bottom: 8px;
}
.port_status_bg .label_L_bg{
	margin-left: 3px;
	flex-grow: 1;
	flex-shrink: 0;
	flex-basis: 60%;
}
.port_status_bg .port_icon{
	background-image: url(/images/New_ui/amesh/port_status.svg);
	background-size: 100px;
	width: 30px;
	height: 26px;
	display: flex;
	justify-content: center;
	align-items: center;
	border-radius: 2px;
	position: relative;
}
.port_status_bg .port_icon.conn{
	background-position: 0px 0px;
}
.port_status_bg .port_icon.warn{
	background-position: 0px -43px;
}
.port_status_bg .port_icon.unplug{
	background-position: 0px -86px;
}
</style>
<script>
var VLAN_Profile_select = [];
var VLAN_port_status = {};

var str_port_binding_note_tmp = "";
var str_port_binding_note = stringSafeGet("<#VLAN_port_binding_note#>");
var str_modify = stringSafeGet(" <#web_redirect_suggestion_modify#>");

var rate_map = [
		{value:'10',text:'10 Mbps'},
		{value:'100',text:'100 Mbps'},
		{value:'1000',text:'1 Gbps'},
		{value:'2500',text:'2.5 Gbps'},
		{value:'5000',text:'5 Gbps'	},
		{value:'10000',text:'10 Gbps'}
];

httpApi.get_VLAN_port_status = function(mac) {
	var retData = {};

	var capability_map = [
		{type:'WAN', bit:0},
		{type:'LAN', bit:1},
		{type:'GAME', bit:2},
		{type:'PLC', bit:3},
		{type:'WAN2', bit:4},
		{type:'WAN3', bit:5},
		{type:'SFPP', bit:6},
		{type:'USB', bit:7},
		{type:'MOBILE', bit:8},
		{type:'WANLAN', bit:9},
		{type:'MOCA', bit:10},
		{type:'POE', bit:11},
		{type:'IPTV_BRIDGE', bit:26},
		{type:'IPTV_VOIP', bit:27},
		{type:'IPTV_STB', bit:28},
		{type:'DUALWAN_SECONDARY_WAN', bit:29},
		{type:'DUALWAN_PRIMARY_WAN', bit:30}
	];

	var rate_map_USB = [
		{value:'480', text:'USB2.0'},
		{value:'5000', text:'USB3.0'},
		{value:'10000',	text:'USB3.1'},
		{value:'20000', text:'USB3.2'}
	];

	function get_port_status_handler(response) {
		var response_temp = JSON.parse(JSON.stringify(response));
		var port_info_temp = {};
		if (response_temp["port_info"] != undefined) {
			if (response_temp["port_info"][mac] != undefined) {
				port_info_temp = {"WAN": [], "LAN": [],	"USB": []};
				var port_info = response_temp["port_info"][mac];
				$.each(port_info, function(index, data) {
					var label = index.substr(0, 1);
					var label_idx = index.substr(1, 1);
					data["label"] = label;
					data["label_priority"] = ((label == "W") ? 1 : ((label == "L") ? 2 : 3));
					data["label_idx"] = label_idx;
					data["label_port_name"] = (function() {
						if (data.cap_support.WAN) {
							if (label_idx == "0")
								return "WAN";
							else
								return "WAN " + label_idx;
						}
						else if (data.cap_support.LAN) {
							return "LAN " + label_idx;
						}
						else if (data.cap_support.USB) {
							return "USB";
						}
					})();

					data["ui_display_text"] = "";
					if (data.ui_display != undefined && data.ui_display != "") {
						data["ui_display_text"] = data.ui_display;
					}
					data["phy_port_id"] = data.phy_port_id;
					data["ext_port_id"] = data.ext_port_id;

					var link_rate = isNaN(parseInt(data.link_rate)) ? 0 : parseInt(data.link_rate);
					var max_rate = isNaN(parseInt(data.max_rate)) ? 0 : parseInt(data.max_rate);
					data["link_rate_text"] = (data.is_on == "1") ? "0 Mbps" : "";

					var link_rate_data = rate_map.filter(function(item, index, array) {
						return (item.value == data.link_rate);
					})[0];

					if(link_rate_data != undefined){
						data["link_rate_text"] = link_rate_data.text;
					}

					if (data.cap_support.USB) {
						data["link_rate_text"] = ((data.is_on == "1") ? (link_rate + " Mbps") : "");
						var max_rate_data = rate_map_USB.filter(function(item, index, array) {
							return (item.value == max_rate);
						})[0];
					} else {
						var max_rate_data = rate_map.filter(function(item, index, array) {
							return (item.value == max_rate);
						})[0];
					}

					data["max_rate_text"] = "0 Mbps";
					if (max_rate_data != undefined) {
						data["max_rate_text"] = max_rate_data.text;
						data["special_port_name"] = "";
						if (data["cap_support"]["GAME"] == true) {
							data["special_port_name"] = `<#Port_Gaming#>`;
						}
						else {
							if (data.cap_support.USB) {
								data["special_port_name"] = (data.is_on == "1") ? "USB Modem" : max_rate_data.text;
							}
							else {
								var max_rate_value = parseInt(max_rate_data.value);
								data["special_port_name"] = max_rate_data.text.replace(" Gbps", "");
								if (max_rate == 10000) {
									if (data["cap_support"]["SFPP"] == true)
										data["special_port_name"] = data["special_port_name"] + "G SFP+";
									else
										data["special_port_name"] = data["special_port_name"] + "G baseT";
								} else
									data["special_port_name"] = data["special_port_name"] + "G";
								
							}
						}
					}

					data["link_rate_status"] = 1; //normal
					if(!(data.cap_support.USB)){
						if(data.is_on == "1" && link_rate < 1000)
							data["link_rate_status"] = 0;//abnormal
					}

					var sort_key = "";
					if (data.cap_support.DUALWAN_PRIMARY_WAN || data.cap_support.DUALWAN_SECONDARY_WAN) {
						port_info_temp["WAN"].push(data);
						sort_key = "WAN";
					}
					else if (data.cap_support.USB) {
						port_info_temp['USB'].push(data);
						sort_key = 'USB';
					}
					else {
						port_info_temp["LAN"].push(data);
						sort_key = "LAN";
					}

					port_info_temp[sort_key].sort(function(a, b) {
						//first compare label priority, W>L>U
						var a_label_priority = parseInt(a.label_priority);
						var b_label_priority = parseInt(b.label_priority);
						var label_priority = ((a_label_priority == b_label_priority) ? 0 : ((a_label_priority > b_label_priority) ? 1 : -1));
						if (label_priority != 0) {
							return label_priority;
						} else { //second compare label idx
							var a_label_idx = parseInt(a.label_idx);
							var b_label_idx = parseInt(b.label_idx);
							return ((a_label_idx == b_label_idx) ? 0 : ((a_label_idx > b_label_idx) ? 1 : -1));
						}
					});
				});
			}
		}

		return response;
	}

	var set_cap_support = function(_port_info){
		$.each(_port_info, function(index, data){
			var cap = data.cap;
			if(data["cap_support"] == undefined)
				data["cap_support"] = {};
			$.each(capability_map, function(index, capability_item){
				data["cap_support"][capability_item.type] = ((parseInt(cap) & (1 << parseInt(capability_item.bit))) > 0) ? true : false;
			});
		});
	};

	$.ajax({
		url: "/get_port_status.cgi?node_mac=" + mac,
		dataType: 'json',
		async: false,
		error: function() {},
		success: function(response) {

			if (response["port_info"] != undefined) {
				if(mac == "all"){
					$.each(response["port_info"], function(node_mac, node_port_info){
						set_cap_support(response["port_info"][node_mac]);
					});
				}
				else{
					if(response["port_info"][mac] != undefined){
						set_cap_support(response["port_info"][mac]);
					}
				}

			}

			retData = response;
		}
	});

	return get_port_status_handler(retData);
}

var convertRulelistToJson = function(attrArray, rulelist) {
	var rulelist_json = [];

	var each_rule = rulelist.split("<");
	var convertAtoJ = function(rule_array) {
		var rule_json = {}
		$.each(attrArray, function(index, value) {
			rule_json[value] = rule_array[index];
		});
		return rule_json;
	}

	$.each(each_rule, function(index, value) {
		if (value != "") {
			var one_rule_array = value.split(">");
			var one_rule_json = convertAtoJ(one_rule_array);
			if (!one_rule_json.error) rulelist_json.push(one_rule_json);
		}
	});

	return rulelist_json;
}

var add_back_missing_apg_dut_list = function(dut_list) {
	$.each(meshList, function(idx, meshNodeMac) {
		if (dut_list.indexOf(meshNodeMac) == -1) dut_list += "<" + meshNodeMac + ">>"
	})
	return dut_list;
}

apgRuleTable = [
	"mac",
	"wifiband",
	"lanport"
]

vlanRuleTable = [
	"vlan_idx",
	"vid",
	"port_isolation"
];

sdnRuleTable = [
	"idx",
	"sdn_name",
	"sdn_enable",
	"vlan_idx",
	"subnet_idx",
	"apg_idx",
	"vpnc_idx",
	"vpns_idx",
	"dns_filter_idx",
	"urlf_idx",
	"nwf_idx",
	"cp_idx",
	"gre_idx",
	"firewall_idx",
	"kill_switch",
	"access_host_service",
	"wan_idx",
	"pppoe-relay"
];

var orig_wans_dualwan = decodeURIComponent(httpApi.nvramCharToAscii(["wans_dualwan"]).wans_dualwan);
var orig_wans_lanport = decodeURIComponent(httpApi.nvramCharToAscii(["wans_lanport"]).wans_lanport);
var orig_aggregation = httpApi.nvramGet(["bond_wan","wanports_bond","lacp_enabled"], true);
var original_stb_port = httpApi.nvramGet(["switch_stb_x","iptv_stb_port","iptv_voip_port","iptv_bridge_port"], true);

var sdn_rl = decodeURIComponent(httpApi.nvramCharToAscii(["sdn_rl"]).sdn_rl)
var vlan_rl = decodeURIComponent(httpApi.nvramCharToAscii(["vlan_rl"]).vlan_rl)
var sdn_rl_json = convertRulelistToJson(sdnRuleTable, sdn_rl);
var vlan_rl_json = convertRulelistToJson(vlanRuleTable, vlan_rl);
var vlan_rl_vid_array = [];
var apg_dutList = {};
var meshNodelist = httpApi.hookGet("get_cfg_clientlist", true);
var meshList = [];

$.each(meshNodelist, function(idx, meshNode) {
	if (httpApi.aimesh_get_node_lan_port(meshNode).length == 0) return true;

	var oneNode = {};
	oneNode.macAddress = meshNode.mac;
	oneNode.model = meshNode.model_name;
	oneNode.productid = meshNode.product_id;
	oneNode.location = meshNode.alias;
	oneNode.connection = meshNode.online;
	oneNode.capability = meshNode.capability;
	oneNode.config = meshNode.config;
    var capability_value = (oneNode.capability["4"]=="")?0:oneNode.capability["4"];
	oneNode.port = [];

	if (httpApi.get_VLAN_port_status(oneNode.macAddress).node_info != undefined &&
		!(capability_value & 4194304) ) { //capability bit(22)

		var oneNode_port_status = httpApi.get_VLAN_port_status(oneNode.macAddress).port_info[oneNode.macAddress];
		for (var key in oneNode_port_status) {
			if (key.indexOf("L") == -1) continue;

			var oneLanPort = {};
			var portIndex = 0;
			portIndex = key.charAt(key.length - 1);
			oneLanPort.portLabel = key;
			oneLanPort.status = oneNode_port_status[key].is_on;
			var link_rate_tmp = rate_map.filter(function(item, index, array) {
				return (item.value == oneNode_port_status[key].link_rate);
			})[0];
			link_rate_tmp = (link_rate_tmp == undefined)? rate_map[0]:link_rate_tmp;	//<#Status_Unplugged#>
			oneLanPort.speed = link_rate_tmp.text;
			var max_rate_tmp = rate_map.filter(function(item, index, array) {
				return (item.value == oneNode_port_status[key].max_rate);
			})[0];
			max_rate_tmp = (max_rate_tmp == undefined)? rate_map[0]:max_rate_tmp;	//<#Status_Unplugged#>
			var ui_display_tmp = "";
			if(oneNode_port_status[key].ui_display != undefined && oneNode_port_status[key].ui_display != ""){
				ui_display_tmp = oneNode_port_status[key].ui_display;
			}
			oneLanPort.ui_display = ui_display_tmp;
			oneLanPort.phy_port_id = oneNode_port_status[key].phy_port_id;
			oneLanPort.ext_port_id = oneNode_port_status[key].ext_port_id;
			oneLanPort.max_rate = max_rate_tmp.text;
			oneLanPort.detail = oneNode_port_status[key];
			oneLanPort.profile = "0";
			oneLanPort.mode = "";
			oneLanPort.wans_lanport = "";
			oneLanPort.wan_bonding = "";
			oneLanPort.link_aggregation = "";
			oneLanPort.iptv = "";

			//oneNode.port.push(oneLanPort);
			oneNode.port[portIndex - 1] = oneLanPort;
		}

		VLAN_port_status[meshNode.mac] = oneNode;
		meshList.push(meshNode.mac);

	}
})

//Collect VLAN_Profile_select from sdn_rl
$.each(sdn_rl_json, function(idx, _sdn_rl) {
	var oneRule = {};

	var vlanArrayIdx = -1;
	$.each(vlan_rl_json, function(idx, _vlan_rl) {
		if (_sdn_rl.vlan_idx == _vlan_rl.vlan_idx) vlanArrayIdx = idx;
	})
	if (vlanArrayIdx < 0) return true;

	// oneRule.name = _sdn_rl.sdn_name;
	oneRule.name = decodeURIComponent(httpApi.nvramCharToAscii(["apg" + _sdn_rl.apg_idx + "_ssid"])["apg" + _sdn_rl.apg_idx + "_ssid"]);
	oneRule.vid = vlan_rl_json[vlanArrayIdx].vid;
	oneRule.iso = vlan_rl_json[vlanArrayIdx].port_isolation;
	oneRule.apgIdx = _sdn_rl.apg_idx;

	var apg_dut_list = decodeURIComponent(httpApi.nvramCharToAscii([
		"apg" + _sdn_rl.apg_idx + "_dut_list"
	])["apg" + _sdn_rl.apg_idx + "_dut_list"])

	apg_dut_list = add_back_missing_apg_dut_list(apg_dut_list);

	apg_dutList["apg" + _sdn_rl.apg_idx + "_dut_list"] = convertRulelistToJson(apgRuleTable, apg_dut_list)

	VLAN_Profile_select.push(oneRule);
})

//Collect VLAN_Profile_select from vlan_rl
$.each(vlan_rl_json, function(idx, _vlan_rl) {
	var match_count=0;
	for(var y=0;y<VLAN_Profile_select.length;y++){
		if(VLAN_Profile_select[y].vid == _vlan_rl.vid)
			match_count++;
	}
	if(match_count==0){
		var oneRule = {};
		oneRule.name = _vlan_rl.vid;
		oneRule.vid = _vlan_rl.vid;
		oneRule.iso = _vlan_rl.port_isolation;
		oneRule.apgIdx = "";

		VLAN_Profile_select.push(oneRule);
	}
})

for (var key in apg_dutList) {
	var dutList = apg_dutList[key];
	var apgIdx = key.replace("apg", "").replace("_dut_list", "");
	var vlanId = "";
	var vlanIdx = "";
	$.each(sdn_rl_json, function(idx, sdn) {
		if (sdn.apg_idx == apgIdx) {
			vlanId = sdn.vlan_idx;
			return false;
		}
	})
	$.each(vlan_rl_json, function(idx, vlan) {
		if (vlan.vlan_idx == vlanId) {
			vlanIdx = vlan.vid;
			return false;
		}
	})
	$.each(dutList, function(idx, node) {
		if (node.lanport != "") {
			var lanPortArray = node.lanport.split(",");
			for (var i = 0; i < lanPortArray.length; i++) {
				if(VLAN_port_status[node.mac].port.length > 0){
					if(isNaN(lanPortArray[i])){
						var port_idx_p = findKeyByPortLabel(VLAN_port_status[node.mac].port, lanPortArray[i]);
						VLAN_port_status[node.mac].port[port_idx_p].profile = vlanIdx;
					}
					else{	//for old version
						VLAN_port_status[node.mac].port[parseInt(lanPortArray[i]) - 1].profile = vlanIdx;
					}
				}
			}
		}
	})

	for (var key in apg_dutList) {
		var dutList = apg_dutList[key];
		var lanport_array = [];
		$.each(dutList, function(key, node) {
			lanport_array = node.lanport.split(",");
			for (var a = 0; a < lanport_array.length; a++) {
				if (lanport_array[a] != "" && VLAN_port_status[node.mac].port.length > 0) {
					if(isNaN(lanport_array[a])){
						var port_idx_m = findKeyByPortLabel(VLAN_port_status[node.mac].port, lanport_array[a]);
						VLAN_port_status[node.mac].port[port_idx_m].mode = "Access";
					}
					else{	//for old version
						VLAN_port_status[node.mac].port[parseInt(lanport_array[a]) - 1].mode = "Access";
					}
				}
			}
		})
	}
}

$.each(vlan_rl_json, function(idx, vlan) {
	vlan_rl_vid_array.push(vlan.vid);
});

var vlan_trunklist_tmp = decodeURIComponent(httpApi.nvramCharToAscii(["vlan_trunklist"]).vlan_trunklist);
var vlan_trunklist_array = vlan_trunklist_tmp.split("<");
var vlan_trunklist_port_array = [];

if (vlan_trunklist_array.length > 1) {
	for (var b = 1; b < vlan_trunklist_array.length; b++) {
		vlan_trunklist_port_array = vlan_trunklist_array[b].split(">");
		for (var c = 0; c < vlan_trunklist_port_array.length; c++) {
			if (c > 0) {
				vlan_rl_vid_array_tmp = vlan_rl_vid_array;
				vlan_trunklist_port = vlan_trunklist_port_array[c].split("#")[0];
				vlan_trunklist_port_vid = vlan_trunklist_port_array[c].split("#")[1];

				if(isNaN(parseInt(vlan_trunklist_port))){
					var port_idx_v = findKeyByPortLabel(VLAN_port_status[vlan_trunklist_port_array[0]].port, vlan_trunklist_port);
					if(VLAN_port_status[vlan_trunklist_port_array[0]].port[port_idx_v]){
						VLAN_port_status[vlan_trunklist_port_array[0]].port[port_idx_v].mode = "Trunk";
						VLAN_port_status[vlan_trunklist_port_array[0]].port[port_idx_v].profile = vlan_trunklist_port_vid;
					}
					else{
						console.log(vlan_trunklist_port_array[0]+" L: Not exist.");
					}
				}
				else{
					if(VLAN_port_status[vlan_trunklist_port_array[0]].port[parseInt(vlan_trunklist_port) - 1]){
						VLAN_port_status[vlan_trunklist_port_array[0]].port[parseInt(vlan_trunklist_port) - 1].mode = "Trunk";
						VLAN_port_status[vlan_trunklist_port_array[0]].port[parseInt(vlan_trunklist_port) - 1].profile = vlan_trunklist_port_vid;
					}
					else{
						console.log(vlan_trunklist_port_array[0]+": Not exist.");
					}
				}
			}
		}
	}
}

function findKeyByPortLabel(data, targetLabel) {
	for (const key in data) {
		if (data.hasOwnProperty(key) && data[key].portLabel === targetLabel) {
			return key;
		}
	}
	return null; // Return null if the portLabel is not found
}

var vlan_rl_vid_array_tmp = [];
var reportedNodeNumber = 0;

var get_VLAN_Status;

function initial() {
	show_menu();
	var vlan_switch_array = { "VLAN" : ["VLAN", "Advanced_VLAN_Switch_Content.asp"], "Profile" : ["Profile", "Advanced_VLAN_Profile_Content.asp"]};
	$('#divSwitchMenu').html(gen_switch_menu(vlan_switch_array, "VLAN"));

	get_VLAN_Status = update_VLAN_ports();
	gen_VLAN_port_table(get_VLAN_Status);

	setInterval(check_get_port_status_update, 3000);
}

function check_get_port_status_update(){
	var nodeInfo = [];

	$.ajax({
		url: "/get_port_status.cgi?node_mac=all",
		dataType: 'json',
		success: function(response){
			for(var node in response.node_info){nodeInfo.push(node)}
			if(reportedNodeNumber == 0){
				reportedNodeNumber = nodeInfo.length;
			}
			else if(nodeInfo.length != reportedNodeNumber){
				top.location.href = top.location.href;
			}
		}
	})
}

function arrayRemove(arr, value) {
	return arr.filter(function(ele) {
		return ele != value;
	});
}

function update_VLAN_ports() { //ajax update VLAN ports	

	return VLAN_port_status; //no update temporarily
}

function gen_VLAN_port_table(port_profile) {
	var mesh_mac = Object.keys(port_profile);

	var role_str, model_str, macaddr_str, loc_val, loc_str, specific_location, connected_flag = "";
	var port_length = 0;

	if (mesh_mac.length == 0) {
		$('#tableCAP').hide();
	}
	else {

		$('#tableCAP').empty().show();

		for (var i = 0; i < mesh_mac.length; i++) { //[for loop] mesh system start

			role_str = (i == 0) ? "<#Device_type_02_RT#>" : "<#AiMesh_Node#>";
			model_str = port_profile[mesh_mac[i]].model;
			macaddr_str = port_profile[mesh_mac[i]].macAddress;
			loc_val = port_profile[mesh_mac[i]].location;
			specific_location = aimesh_location_arr.filter(function(item, index, _array){
				return (item.value == loc_val);
			})[0];
			if(specific_location != undefined){
				loc_str = specific_location.text;
			}
			else{
				loc_str = loc_val;
			}
			connected_flag = port_profile[mesh_mac[i]].connection;
			port_length = port_profile[mesh_mac[i]].port.length;


			if (i > 0) { //Node
				var $div_system_node = $("<div>").attr("id", "tableNode_" + mesh_mac[i]).appendTo($("#tableCAP"));
				var $target_div = $div_system_node;
				var table_id = "tableNode_" + mesh_mac[i] + "_id";
			} else { //CAP
				var $target_div = $("#tableCAP");
				var table_id = "tableCAP_id";
			}

			var $system_model_bg = $("<div>")
				.addClass("formfontdesc model")
				.html(model_str)
				.appendTo($target_div)
				.hide();

			var $system_role_bg = $("<span>")
				.addClass("formfontdesc role")
				.html(role_str)
				.appendTo($system_model_bg)
				.hide();

			//table head
			var $port_table_bg = $("<table>").addClass("FormTable").appendTo($target_div);

			$port_table_bg.attr("id", table_id)
				.attr('border', '1')
				.attr('cellpadding', '4')
				.attr('cellspacing', '0')
				.css('width', '100%');

			var $port_table_thead = $("<thead>").appendTo($port_table_bg);
			var $port_table_thead_tr = $("<tr>").appendTo($port_table_thead);
			var $port_table_thead2 = $("<td>")
				.attr('colSpan', '6')
				.appendTo($port_table_thead_tr)
				.css({
					"text-align": "left",
					"padding-left": "10px"
				})

			if (i > 0) {
				$port_table_bg.css({
				    "margin-top": "-30px"
				})
				$port_table_thead2.html("<#AiMesh_Node#>");
			} else {
				$port_table_thead2.html("<#AiMesh_Router#>");
			}

			//table content : status
			var $port_table_tr_status = $("<tr>").appendTo($port_table_bg);
			var $port_table_th_status = $("<th>").appendTo($port_table_tr_status);
			$port_table_th_status.css({"width":"20%"});
			$port_table_th_status.html(`<ul class='ul-align'><li>${model_str}</li><li>${macaddr_str}</li><li>${loc_str}</li></ul>`);

			//table content : mode
			var $port_table_tr_mode = $("<tr>").appendTo($port_table_bg);
			var $port_table_th_mode = $("<th>").appendTo($port_table_tr_mode);
			$port_table_th_mode.css({"width":"20%"});
			$port_table_th_mode.html("<#DSL_Mode#>");

			//table content : profile
			var $port_table_tr_profile = $("<tr>").appendTo($port_table_bg);
			var $port_table_th_profile = $("<th>").appendTo($port_table_tr_profile);
			$port_table_th_profile.css({"width":"20%"});
			$port_table_th_profile.html("SDN (VLAN) Profile"); /* Untranslated */			

			//Note: modify array
			var wan_aggregation_array = [];
			var link_aggregation_array = [];
			var iptv_array = [];
			var wan_array = [];
			var ext_switch_array = [];

			var wan_aggregation_ui_display = [];
			var link_aggregation_ui_display = [];
			var iptv_ui_display = [];
			var wan_ui_display = [];
			var ext_switch_ui_display = [];	

			if (port_length > 0) { //With port_info //with empty 0 item

				var col_count=0;
				// Convert the JSON object into an array of key-value pairs (entries)
				const port_info = Object.entries(port_profile[mesh_mac[i]].port);
				port_info.forEach(([key, value]) => {
					col_count++;
					var icon_lanport_idx = "";
					var lanport_idx = port_profile[mesh_mac[i]].port[key].portLabel.replace("L", "");
					if(col_count<=4){
						
						var $port_table_td_status = $("<td>").appendTo($port_table_tr_status);
						$port_table_td_status.css("width", "100px");
						var $port_table_td_profile = $("<td>").appendTo($port_table_tr_profile);
						var $port_table_td_mode = $("<td>").appendTo($port_table_tr_mode);
						var $port_status_title, $port_status_icon, $port_status_idx, $port_status_note;
						var aggressive_tag = 0;
						$port_status_title = $("<div>").appendTo($port_table_td_status);
						$port_status_title.css({
							"width": "30px",
							"border-bottom": "0px #000 solid !important",
							"margin": "0 auto"
						})
							.addClass("port_status_bg")
							.addClass("port_status_bg_vlan")
						var port_rate_txt = "";
						if (port_profile[mesh_mac[i]].port[key].speed) {
							port_rate_txt = port_profile[mesh_mac[i]].port[key].speed;
						}

						var port_ui_display_txt = "";
						var port_max_rate_txt = "";
						if (port_profile[mesh_mac[i]].port[key].ui_display) {
							port_ui_display_txt = port_profile[mesh_mac[i]].port[key].ui_display;
							icon_lanport_idx = "";
						}
						else if (port_profile[mesh_mac[i]].port[key].max_rate) {
							port_max_rate_txt = port_profile[mesh_mac[i]].port[key].max_rate;
							icon_lanport_idx = lanport_idx;
						}
						switch (port_profile[mesh_mac[i]].port[key].status) {
							case '0': //unplug
						
									$port_status_icon = $("<div>").appendTo($port_status_title);
									$port_status_icon.addClass("port_icon unplug");
									$("<div>").html((port_ui_display_txt=="")? port_max_rate_txt:port_ui_display_txt).appendTo($port_table_td_status);
									break;
							case '1': //connected
									$port_status_title.attr("title", port_rate_txt);
									$port_status_icon = $("<div>").appendTo($port_status_title);
									$port_status_icon.addClass("port_icon conn");
									$("<div>").html((port_ui_display_txt=="")? port_max_rate_txt:port_ui_display_txt).appendTo($port_table_td_status);
									break;
							case '2': //warning
									$port_status_title.attr("title", port_rate_txt);
									$port_status_icon = $("<div>").appendTo($port_status_title);
									$port_status_icon.addClass("port_icon warn");
									$("<div>").html((port_ui_display_txt=="")? port_max_rate_txt:port_ui_display_txt).appendTo($port_table_td_status);
									break;
							default: //unplug
									$port_status_icon = $("<div>").appendTo($port_status_title);
									$port_status_icon.addClass("port_icon unplug");
									break;
						}
						$port_status_idx = $("<div>").appendTo($port_status_icon);
						//$port_status_idx.html(j + 1)
						$port_status_idx.html(icon_lanport_idx).addClass("status_idx");
						//if( i == 0 && orig_wans_dualwan.indexOf("lan") >= 0 && orig_wans_lanport == (j+1) ) //for CAP only
						if( i == 0 && orig_wans_dualwan.indexOf("lan") >= 0 && orig_wans_lanport == (lanport_idx) ) //for CAP only
						{
							port_profile[mesh_mac[i]].port[key].wans_lanport = '1';
							aggressive_tag = 1;
							$port_status_note = $("<div>").appendTo($port_status_icon);
							if(top.webWrapper){
								if($port_status_idx.html() != "")
									$port_status_note.html("Aggregation").addClass("status_note_white");
								else
									$port_status_note.html("Aggregation").addClass("status_note_white_status_idx_null");
							}
							else{
								if($port_status_idx.html() != "")
									$port_status_note.html("Aggregation").addClass("status_note");
								else
									$port_status_note.html("Aggregation").addClass("status_note_status_idx_null");
							}
						}
						if( i == 0 && key == 3 && orig_aggregation.bond_wan == 1 ){ //CAP only because have no info for RE, general LAN4
							port_profile[mesh_mac[i]].port[key].wan_bonding = '1';
						}

						var select_node_capability = httpApi.aimesh_get_node_capability(port_profile[mesh_mac[i]]);
						var lacp = manage_get_lacp(port_profile[mesh_mac[i]], select_node_capability);
						if((i == 0 && orig_aggregation.lacp_enabled == 1) || (lacp.support && lacp.value == "1") ) { //lacp for CAP || for RE
							const bonding_port_settings = get_bonding_ports(port_profile[mesh_mac[i]].productid);
							for(var b = 0; b < bonding_port_settings.length; b++){
                                if(lanport_idx == bonding_port_settings[b].val){
                                  port_profile[mesh_mac[i]].port[key].link_aggregation = '1';
                                }
							}
						}
						if( i == 0 && original_stb_port.switch_stb_x != 0){ //with iptv settings
							if(original_stb_port.iptv_stb_port.indexOf(lanport_idx) > 0 || original_stb_port.iptv_voip_port.indexOf(lanport_idx) > 0 || original_stb_port.iptv_bridge_port.indexOf(lanport_idx) > 0){
								port_profile[mesh_mac[i]].port[key].iptv = '1';
							}
						}

						if (port_profile[mesh_mac[i]].port[key].wan_bonding == '1' || port_profile[mesh_mac[i]].port[key].link_aggregation == '1' ||
						port_profile[mesh_mac[i]].port[key].iptv == '1') {
							aggressive_tag = 1;
							$port_status_note = $("<div>").appendTo($port_status_icon);
							if(top.webWrapper){
								if($port_status_idx.html() != "")
									$port_status_note.html("Aggregation").addClass("status_note_white");
								else
									$port_status_note.html("Aggregation").addClass("status_note_white_status_idx_null");
							}
							else{
								if($port_status_idx.html() != "")
									$port_status_note.html("Aggregation").addClass("status_note");
								else
									$port_status_note.html("Aggregation").addClass("status_note_status_idx_null");
							}
							if (port_profile[mesh_mac[i]].port[key].wan_bonding == '1') {
								wan_aggregation_array.push(lanport_idx);
								if(port_ui_display_txt!="")
									wan_aggregation_ui_display.push(port_ui_display_txt);
							}
							if (port_profile[mesh_mac[i]].port[key].link_aggregation == '1') {
								link_aggregation_array.push(lanport_idx);
								if(port_ui_display_txt!="")
									link_aggregation_ui_display.push(port_ui_display_txt);
							}
							if (port_profile[mesh_mac[i]].port[key].iptv == '1') {
								iptv_array.push(lanport_idx);
								if(port_ui_display_txt!="")
									iptv_ui_display.push(port_ui_display_txt);
							}
						}

						// 1. (cap with bit PHY_PORT_CAP_WAN)
						//console.log("cap_WAN: "+port_profile[mesh_mac[i]].port[key].detail.cap_support.WAN);
						if(port_profile[mesh_mac[i]].port[key].detail.cap_support.WAN){
							aggressive_tag = 2;
							wan_array.push(lanport_idx);
							if(port_ui_display_txt!="")
								wan_ui_display.push(port_ui_display_txt);
						}

						// 2. (cap with bit PHY_PORT_CAP_LAN  && with bit PHY_PORT_CAP_WANLAN) && ext_port_id != -1 
						if(port_profile[mesh_mac[i]].port[key].detail.cap_support.LAN &&
							port_profile[mesh_mac[i]].port[key].detail.cap_support.WANLAN &&
							port_profile[mesh_mac[i]].port[key].ext_port_id != -1){
							aggressive_tag = 3;
							ext_switch_array.push(lanport_idx);
							if(port_ui_display_txt!="")
								ext_switch_ui_display.push(port_ui_display_txt);
						}

						$port_table_td_mode.empty();
						$port_table_td_mode.append(insert_vlan_mode_selector(macaddr_str, lanport_idx, port_profile[mesh_mac[i]].port[key].mode, aggressive_tag));
						$port_table_td_profile.empty();
						$port_table_td_profile.append(insert_vlan_profile_selector(macaddr_str, lanport_idx, port_profile[mesh_mac[i]].port[key].profile, aggressive_tag));
					}

				});	//forEach

				const col2_count = [0];	//row 0 already done.
				if(port_info.length > 4){

					var $port_table_tr_status = [];
					var $port_table_tr_mode = [];
					var $port_table_tr_profile = [];
					// Determine the row for 4 ports
					var rowCount = Math.ceil((port_info.length) / 4);
					for(var r=1; r<rowCount; r++){

					//table content : status
					$port_table_tr_status[r] = $("<tr>").appendTo($port_table_bg);
					var $port_table_th_status = $("<th>").appendTo($port_table_tr_status[r]);
					$port_table_th_status.css({"width":"20%"});
					$port_table_th_status.html("<ul class='ul-align'><li>" + macaddr_str + "</li></ul>");	//<li>" + loc_str + "</li>
					//table content : mode
					$port_table_tr_mode[r] = $("<tr>").appendTo($port_table_bg);
					var $port_table_th_mode = $("<th>").appendTo($port_table_tr_mode[r]);
					$port_table_th_mode.css({"width":"20%"});
					$port_table_th_mode.html("<#DSL_Mode#>");
					//table content : profile
					$port_table_tr_profile[r] = $("<tr>").appendTo($port_table_bg);
					var $port_table_th_profile = $("<th>").appendTo($port_table_tr_profile[r]);
					$port_table_th_profile.css({"width":"20%"});
					$port_table_th_profile.html("SDN (VLAN) Profile");

					col2_count[r]=0;
					port_info.forEach(([key, value]) => {
						col2_count[r]++;
						var icon_lanport_idx = "";
						var lanport_idx = port_profile[mesh_mac[i]].port[key].portLabel.replace("L", "");

						// Determine the group based on colX_count
						var row_colStart = Math.floor((col2_count[r] - 1) / 4) * 4 + 1;
						var row_colEnd = row_colStart + 3;
						
						if(row_colStart > 4*r && col2_count[r] >= row_colStart && col2_count[r] <= row_colEnd && row_colStart <= 4*(r+1)){
							
							var $port_table_td_status = $("<td>").appendTo($port_table_tr_status[r]);
							$port_table_td_status.css("width", "100px");
							var $port_table_td_mode = $("<td>").appendTo($port_table_tr_mode[r]);
							var $port_table_td_profile = $("<td>").appendTo($port_table_tr_profile[r]);
							var $port_status_title, $port_status_icon, $port_status_idx, $port_status_note;
							var aggressive_tag = 0;
							$port_status_title = $("<div>").appendTo($port_table_td_status);
							$port_status_title.css({
								"width": "30px",
								"border-bottom": "0px #000 solid !important",
								"margin": "0 auto"
							})
								.addClass("port_status_bg")
								.addClass("port_status_bg_vlan")
							var port_rate_txt = "";
							if (port_profile[mesh_mac[i]].port[key].speed) {
								port_rate_txt = port_profile[mesh_mac[i]].port[key].speed;
							}

							var port_ui_display_txt = "";
							var port_max_rate_txt = "";
							if (port_profile[mesh_mac[i]].port[key].ui_display) {
								port_ui_display_txt = port_profile[mesh_mac[i]].port[key].ui_display;
								icon_lanport_idx = "";
							}
							else if (port_profile[mesh_mac[i]].port[key].max_rate) {
								port_max_rate_txt = port_profile[mesh_mac[i]].port[key].max_rate;
								icon_lanport_idx = lanport_idx;
							}
							switch (port_profile[mesh_mac[i]].port[key].status) {
								case '0': //unplug

										$port_status_icon = $("<div>").appendTo($port_status_title);
										$port_status_icon.addClass("port_icon unplug");
										$("<div>").html((port_ui_display_txt=="")? port_max_rate_txt:port_ui_display_txt).appendTo($port_table_td_status);
										break;
								case '1': //connected
										$port_status_title.attr("title", port_rate_txt);
										$port_status_icon = $("<div>").appendTo($port_status_title);
										$port_status_icon.addClass("port_icon conn");
										$("<div>").html((port_ui_display_txt=="")? port_max_rate_txt:port_ui_display_txt).appendTo($port_table_td_status);
										break;
								case '2': //warning
										$port_status_title.attr("title", port_rate_txt);
										$port_status_icon = $("<div>").appendTo($port_status_title);
										$port_status_icon.addClass("port_icon warn");
										$("<div>").html((port_ui_display_txt=="")? port_max_rate_txt:port_ui_display_txt).appendTo($port_table_td_status);
										break;
								default: //unplug
										$port_status_icon = $("<div>").appendTo($port_status_title);
										$port_status_icon.addClass("port_icon unplug");
										break;
							}	
							$port_status_idx = $("<div>").appendTo($port_status_icon);
							$port_status_idx.html(icon_lanport_idx).addClass("status_idx");


							var select_node_capability = httpApi.aimesh_get_node_capability(port_profile[mesh_mac[i]]);
							var lacp = manage_get_lacp(port_profile[mesh_mac[i]], select_node_capability);
							if( (i == 0 && orig_aggregation.lacp_enabled == 1) || (lacp.support && lacp.value == "1") ) { //lacp for CAP || for RE
								const bonding_port_settings = get_bonding_ports(port_profile[mesh_mac[i]].productid);
								for(var c = 0; c < bonding_port_settings.length; c++){
									if(lanport_idx == bonding_port_settings[c].val){
										port_profile[mesh_mac[i]].port[key].link_aggregation = '1';
									}
								}
							}

							if( i == 0 && original_stb_port.switch_stb_x != 0){ //with iptv settings
								if(original_stb_port.iptv_stb_port.indexOf(lanport_idx) > 0 || original_stb_port.iptv_voip_port.indexOf(lanport_idx) > 0 || original_stb_port.iptv_bridge_port.indexOf(lanport_idx) > 0){
									port_profile[mesh_mac[i]].port[key].iptv = '1';
								}
							}

							if (port_profile[mesh_mac[i]].port[key].wan_bonding == '1' || port_profile[mesh_mac[i]].port[key].link_aggregation == '1' ||
							port_profile[mesh_mac[i]].port[key].iptv == '1') {
								aggressive_tag = 1;
								$port_status_note = $("<div>").appendTo($port_status_icon);
								if(top.webWrapper){
									if($port_status_idx.html() != "")
										$port_status_note.html("Aggregation").addClass("status_note_white");
									else
										$port_status_note.html("Aggregation").addClass("status_note_white_status_idx_null");
								}
								else{
									if($port_status_idx.html() != "")
										$port_status_note.html("Aggregation").addClass("status_note");
									else
										$port_status_note.html("Aggregation").addClass("status_note_status_idx_null");
								}
								if (port_profile[mesh_mac[i]].port[key].wan_bonding == '1') {
									wan_aggregation_array.push(lanport_idx);
									if(port_ui_display_txt!="")
										wan_aggregation_ui_display.push(port_ui_display_txt);
								}
								if (port_profile[mesh_mac[i]].port[key].link_aggregation == '1') {
									link_aggregation_array.push(lanport_idx);
									if(port_ui_display_txt!="")
										link_aggregation_ui_display.push(port_ui_display_txt);
								}
								if (port_profile[mesh_mac[i]].port[key].iptv == '1') {
									iptv_array.push(lanport_idx);
									if(port_ui_display_txt!="")
										iptv_ui_display.push(port_ui_display_txt);
								}
							}

							// 1. cap with bit PHY_PORT_CAP_WAN
							//console.log("cap_WAN: "+port_profile[mesh_mac[i]].port[key].detail.cap_support.WAN);
							if(port_profile[mesh_mac[i]].port[key].detail.cap_support.WAN){
								aggressive_tag = 2;
								wan_array.push(lanport_idx);
								if(port_ui_display_txt!="")
									wan_ui_display.push(port_ui_display_txt);
							}

							// 2. (cap with bit PHY_PORT_CAP_LAN  && with bit PHY_PORT_CAP_WANLAN) && ext_port_id != -1
							if(port_profile[mesh_mac[i]].port[key].detail.cap_support.LAN &&
								port_profile[mesh_mac[i]].port[key].detail.cap_support.WANLAN &&
								port_profile[mesh_mac[i]].port[key].ext_port_id != -1){
								aggressive_tag = 3;
								ext_switch_array.push(lanport_idx);
								if(port_ui_display_txt!="")
									ext_switch_ui_display.push(port_ui_display_txt);
							}

							$port_table_td_mode.empty();
							$port_table_td_mode.append(insert_vlan_mode_selector(macaddr_str, lanport_idx, port_profile[mesh_mac[i]].port[key].mode, aggressive_tag));
							$port_table_td_profile.empty();
							$port_table_td_profile.append(insert_vlan_profile_selector(macaddr_str, lanport_idx, port_profile[mesh_mac[i]].port[key].profile, aggressive_tag));

						}

					});	//forEach

					}// for r
				}

			} 
			else{ //without port_profile

				if (connected_flag == "0") {
					$("<div>").html("<#Disconnected#>")
						.css("color", "#FFCC00")
						.css('margin-top', '30px')
						.appendTo($port_table_tr_status);
				}
				else{
					$("<img>").attr("src", "images/InternetScan.gif")
						.css('width', '30px')
						.css('height', '30px')
						.css('margin-top', '30px')
						.appendTo($port_table_tr_status);

					setTimeout("refreshpage()", 30000);
				}
			}

			//Note for LAN port
			var note_wans_lanport = "";
			var note_wan_aggregation = "";
			var note_link_aggregation = "";
			var note_iptv = "";

			var note_WAN_port = "";
			var note_ext_switch = "";

			if( i == 0 && orig_wans_dualwan.indexOf("lan") >= 0 && orig_wans_lanport != "")	//for CAP only
			{
				note_wans_lanport += "<b>LAN ";
				note_wans_lanport += orig_wans_lanport;
				note_wans_lanport += "</b>: ";
				str_port_binding_note_tmp = str_port_binding_note.replace("%@", "<#dualwan#>");
				note_wans_lanport += str_port_binding_note_tmp;
				note_wans_lanport += str_modify.replace('<a>', '<a id="modify_wans">');
				$("<div>").html(note_wans_lanport).appendTo($target_div).css('text-align','left');
				$("#modify_wans").attr("href", "Advanced_WANPort_Content.asp")
					.css("color", "#FFCC00")
					.css("cursor", "pointer")
					.css("text-decoration", "underline");
			}

			if( i == 0 && orig_aggregation.bond_wan == 1 ){	//CAP only because have no info for RE
				if (wan_aggregation_array.length) {
					for (var b = 0; b < wan_aggregation_array.length; b++) {
						if (b > 0)
							note_wan_aggregation += " / ";

						if(link_aggregation_ui_display[a]){
							note_link_aggregation += "<b>";
							note_link_aggregation += link_aggregation_ui_display[a];
						}
						else{
							note_link_aggregation += "<b>LAN ";
							note_link_aggregation += link_aggregation_array[a];
						}
						note_wan_aggregation += "</b>";
					}
					note_wan_aggregation += ": ";
					str_port_binding_note_tmp = str_port_binding_note.replace("%@", "<#WANAggregation#>");
					note_wan_aggregation += str_port_binding_note_tmp;
					note_wan_aggregation += str_modify.replace('<a>', '<a id="modify_wan_bonding">');
					$("<div>").html(note_wan_aggregation).appendTo($target_div).css('text-align','left');
					$("#modify_wan_bonding").attr("href", "Advanced_WAN_Content.asp")
						.css("color", "#FFCC00")
						.css("cursor", "pointer")
						.css("text-decoration", "underline");
				}
			}	


			var select_node_capability_for_note = httpApi.aimesh_get_node_capability(port_profile[mesh_mac[i]]);
			var lacp_for_note = manage_get_lacp(port_profile[mesh_mac[i]], select_node_capability_for_note);
			if ( (i == 0 && orig_aggregation.lacp_enabled ==1) || (lacp_for_note.support && lacp_for_note.value == "1") ) { //lacp for CAP || for RE
				if(link_aggregation_array.length) {
					for (var a = 0; a < link_aggregation_array.length; a++) {
						if (a > 0)
							note_link_aggregation += " / ";

						if(link_aggregation_ui_display[a]){
							note_link_aggregation += "<b>";
							note_link_aggregation += link_aggregation_ui_display[a];
						}
						else{
							note_link_aggregation += "<b>LAN ";
							note_link_aggregation += link_aggregation_array[a];
						}

						note_link_aggregation += "</b>";
					}
					note_link_aggregation += ": ";
					str_port_binding_note_tmp = str_port_binding_note.replace("%@", "<#NAT_lacp#>");
					note_link_aggregation += str_port_binding_note_tmp;
					$("<div>").html(note_link_aggregation).appendTo($target_div).css('text-align','left');
				}
			}
			
			if ( i == 0 && iptv_array.length) {
				for (var c = 0; c < iptv_array.length; c++) {
					if (c > 0)
						note_iptv += " / ";

					if(iptv_ui_display[c]){
						note_iptv += "<b>";
						note_iptv += iptv_ui_display[c];
					}
					else{
						note_iptv += "<b>LAN ";
						note_iptv += iptv_array[c];
					}
					note_iptv += "</b>";
				}
				note_iptv += ": ";
				str_port_binding_note_tmp = str_port_binding_note.replace("%@", "<#menu_dsl_iptv#>");
				note_iptv += str_port_binding_note_tmp;
				note_iptv += str_modify.replace('<a>', '<a id="modify_iptv">');
				$("<div>").html(note_iptv).appendTo($target_div).css('text-align','left');
				$("#modify_iptv").attr("href", "Advanced_IPTV_Content.asp")
					.css("color", "#FFCC00")
					.css("cursor", "pointer")
					.css("text-decoration", "underline");
			}

			if ( wan_array.length) {
				for (var w = 0; w < wan_array.length; w++) {
					if (w > 0)
						note_WAN_port += " / ";

					if(wan_ui_display[w]){
						note_WAN_port += "<b>";
						note_WAN_port += wan_ui_display[w];
					}
					else{
						note_WAN_port += "<b>LAN ";
						note_WAN_port += wan_array[w];
					}
					note_WAN_port += "</b>";
				}
				note_WAN_port += ": ";
				str_port_binding_note_tmp = str_port_binding_note.replace("%@", "<#Ethernet_wan#>");
				note_WAN_port += str_port_binding_note_tmp;
				$("<div>").html(note_WAN_port).appendTo($target_div).css('text-align','left');
			}

			if ( ext_switch_array.length) {
				for (var e = 0; e < ext_switch_array.length; e++) {
					if (e > 0)
						note_ext_switch += " / ";

					if(ext_switch_ui_display[e]){
						note_ext_switch += "<b>";
						note_ext_switch += ext_switch_ui_display[e];
					}
					else{
						note_ext_switch += "<b>LAN ";
						note_ext_switch += ext_switch_array[e];
					}
					note_ext_switch += "</b>";
				}
				note_ext_switch += ": ";
				str_port_binding_note_tmp = str_port_binding_note.replace("%@", "external switch port");	/* Untranslated */
				note_ext_switch += str_port_binding_note_tmp;
				$("<div>").html(note_ext_switch).appendTo($target_div).css('text-align','left');
			}

			$("<br><br>").appendTo($target_div);

		} //[for loop] mesh system end

	}
}

function insert_vlan_mode_selector(mac, idx, flag, aggregation) {

	var mac_str = mac.replace(/\:/g, '');
	var $insert_selector = $("<select>").attr({
			"name": "switch_mode_" + mac_str + "_" + idx,
			"id": "switch_mode_" + mac_str + "_" + idx
		})
		.addClass("input_option");

	var VLAN_mode_select = {
		"All": "<#All#>(<#Setting_factorydefault_value#>)",
		"Access": "Access",
		"Trunk": "Trunk"
	}

	for (var key in VLAN_mode_select) {

		if (key == flag) {
			matched = true;
		} else {
			matched = false;
		}
		$insert_selector.append($("<option></option>")
			.attr("value", key)
			.text(VLAN_mode_select[key])
			.attr("selected", matched));
	}


	if (aggregation) {
		var $default_selector = $("<select>").attr({
			"name": "switch_mode_" + mac_str + "_" + idx,
			"id": "switch_mode_" + mac_str + "_" + idx
		})
		.addClass("input_option")
		.addClass("input_max_length");

		$default_selector.attr("disabled", true);
		$default_selector.append(
			$("<option></option>").attr("value", "0") // defined 0 for default
			.text("<#All#>(<#Setting_factorydefault_value#>)")
		);

		return $default_selector;
	} 
	else {
		$insert_selector.change(function() {
			update_vlan_profile_selector(mac_str, idx, flag, $insert_selector);
		});
		return $insert_selector;
	}

}

function update_vlan_profile_selector(mac_str, idx, flag, obj) {

	$("#switch_vlan_" + mac_str + "_" + idx).empty();

	if(obj.val()=="All"){

		$("#switch_vlan_" + mac_str + "_" + idx).attr("disabled", true);
		$("#switch_vlan_" + mac_str + "_" + idx).append(
			$("<option></option>").attr("value", "0") // defined 0 for default
			.text("<#Setting_factorydefault_value#>")	// Untranslated
		);

	}
	else if(obj.val()=="Access"){
		if(VLAN_Profile_select.length > 0){	//with vlan_rl 
			$("#switch_vlan_" + mac_str + "_" + idx).attr("disabled", false);
			$.each(VLAN_Profile_select, function(i, VLAN_Profile_select) {
				text = VLAN_Profile_select.name;

				if (VLAN_Profile_select.vid == flag) { //suppose flag is vid binded for Access/Trunk mode
					matched = true;
				} 
				else {
					matched = false;
				}

				if(VLAN_Profile_select.apgIdx != ""){	//except pure VLAN profile
					$("#switch_vlan_" + mac_str + "_" + idx).append(
						$("<option></option>").attr("value", VLAN_Profile_select.vid)
							.text(text)
							.attr("selected", matched)
					);
				}
			});
		}
		else{		//without vlan_rl 
			$("#switch_vlan_" + mac_str + "_" + idx).attr("disabled", true);
			$("#switch_vlan_" + mac_str + "_" + idx).append(
				$("<option></option>").attr("value", "0") // defined 0 for default
									  .text("<#Setting_factorydefault_value#>")	// Untranslated
			);
		}
	}
	else{	//Trunk mode

		$("#switch_vlan_" + mac_str + "_" + idx).attr("disabled", false);
		$("#switch_vlan_" + mac_str + "_" + idx).append(
			$("<option></option>").attr("value", "all") // defined 0 for default
			.text("Allow all tagging(<#Setting_factorydefault_value#>)")	// Untranslated
		);
		$.each(VLAN_Profile_select, function(i, VLAN_Profile_select) {

			if(VLAN_Profile_select.apgIdx != ""){	//except pure VLAN profile
				text = VLAN_Profile_select.name;
			}
			else{
				text = "("+VLAN_Profile_select.vid+")";	
			}

			if (VLAN_Profile_select.vid == flag) { //suppose flag is vid binded for Access/Trunk mode
				matched = true;
			} 
			else {
				matched = false;
			}
			$("#switch_vlan_" + mac_str + "_" + idx).append(
				$("<option></option>").attr("value", VLAN_Profile_select.vid)
				.text(text)
				.attr("selected", matched)
			);
		});
	}
}

function insert_vlan_profile_selector(mac, idx, flag, aggregation) {
	
	var vlan_profile_length = VLAN_Profile_select.length;
	var mac_str = mac.replace(/\:/g, '');
	var $insert_selector = $("<select>").attr({
			"name": "switch_vlan_" + mac_str + "_" + idx,
			"id": "switch_vlan_" + mac_str + "_" + idx
		})
		.addClass("input_option")
		.addClass("input_max_length");

	if (aggregation) {
		var $default_selector = $("<select>").attr({
			"name": "switch_vlan_" + mac_str + "_" + idx,
			"id": "switch_vlan_" + mac_str + "_" + idx
		})
		.addClass("input_option")
		.addClass("input_max_length");

		$default_selector.attr("disabled", true);
		$default_selector.append(
			$("<option></option>").attr("value", "0") // defined 0 for default
			.text("<#Setting_factorydefault_value#>")
		);

		return $default_selector;
	} 
	else {

		if($("#switch_mode_" + mac_str + "_" + idx).val()=="All"){

			$insert_selector.attr("disabled", true);
			$insert_selector.append(
				$("<option></option>").attr("value", "0") // defined 0 for default
				.text("<#Setting_factorydefault_value#>")	// Untranslated
			);

		}
		else if($("#switch_mode_" + mac_str + "_" + idx).val()=="Access"){
			
			$insert_selector.attr("disabled", false);
			$.each(VLAN_Profile_select, function(i, VLAN_Profile_select) {
				text = VLAN_Profile_select.name;

				if (VLAN_Profile_select.vid == flag) { //suppose flag is vid binded for Access/Trunk mode
					matched = true;
				} 
				else {
					matched = false;
				}

				if(VLAN_Profile_select.apgIdx != ""){	//except pure VLAN profile
					$insert_selector.append(
						$("<option></option>").attr("value", VLAN_Profile_select.vid)
							.text(text)
							.attr("selected", matched)
					);
				}
			});

		}
		else{	//Trunk mode

			$insert_selector.attr("disabled", false);
			$insert_selector.append(
				$("<option></option>").attr("value", "all") // defined 0 for default
				.text("Allow all tagging(<#Setting_factorydefault_value#>)")	// Untranslated
			);
			$.each(VLAN_Profile_select, function(i, VLAN_Profile_select) {

				if(VLAN_Profile_select.apgIdx != ""){	//except pure VLAN profile
					text = VLAN_Profile_select.name;
				}
				else{
					text = "("+VLAN_Profile_select.vid+")";	
				}

				if (VLAN_Profile_select.vid == flag) { //suppose flag is vid binded for Access/Trunk mode
					matched = true;
				} 
				else {
					matched = false;
				}
				$insert_selector.append(
					$("<option></option>").attr("value", VLAN_Profile_select.vid)
					.text(text)
					.attr("selected", matched)
				);
			});
		}
		return $insert_selector;
	}
}

function applyRule() {
	var each_port_assigned = collect_assigned_from_table_array();
	if(each_port_assigned){
		if(!confirm(stringSafeGet("<#VLAN_eachport_assign#>"))){
			return;
		}
	}	

	collect_Access_mode_from_table_array();
	var vlanSwitchPost = getApgDutListPostData(apg_dutList);
	var vlanTrunkListPost = getvlanTrunkListPostData();
	vlanSwitchPost.vlan_trunklist = vlanTrunkListPost;

	vlanSwitchPost.action_mode = "apply";
	vlanSwitchPost.rc_service = "restart_net_and_phy";

	var reboot_time = httpApi.hookGet("get_default_reboot_time");
	if(isWLclient()){
		showLoading(reboot_time);
		setTimeout(function(){
			showWlHintContainer();
		}, reboot_time*1000);
		check_isAlive_and_redirect({"page": "Advanced_VLAN_Switch_Content.asp", "time": reboot_time});
	}
	httpApi.nvramSet(vlanSwitchPost, function() {
		if(isWLclient()) return;
		showLoading(reboot_time);
		check_isAlive_and_redirect({"page": "Advanced_VLAN_Switch_Content.asp", "time": reboot_time});
	});
}

function collect_assigned_from_table_array(){	//check if all ports were assigned
	var mac_id_str_cap = Object.keys(VLAN_port_status)[0].replace(/\:/g, '');
	var port_length = VLAN_port_status[Object.keys(VLAN_port_status)[0]].port.length;
	for (var x = 1; x < VLAN_port_status[Object.keys(VLAN_port_status)[0]].port.length + 1; x++) {	//each port
		if ($("#switch_mode_" + mac_id_str_cap + "_" + x).val() != "All") {
			port_length -= 1;
		}
	}

	return (port_length==0)? true:false;
}

var mac_id_str = "";
function collect_Access_mode_from_table_array() {
	clean_apgX_dut_list_lanport(apg_dutList);
	for (var y = 0; y < Object.keys(VLAN_port_status).length; y++) { //how many macAddr
		mac_id_str = Object.keys(VLAN_port_status)[y].replace(/\:/g, '');

		const port_info = Object.entries(VLAN_port_status[Object.keys(VLAN_port_status)[y]].port);
		port_info.forEach(([key, value]) => {		//each port
			var lanport_idx = VLAN_port_status[Object.keys(VLAN_port_status)[y]].port[key].portLabel.replace("L", "");
			if ($("#switch_mode_" + mac_id_str + "_" + lanport_idx).val() == "Access") {
				for (var z = 0; z < VLAN_Profile_select.length; z++) {
					if (VLAN_Profile_select[z].vid == $("#switch_vlan_" + mac_id_str + "_" + lanport_idx).val()) {
						var apgX = VLAN_Profile_select[z].apgIdx;
						for (var i = 0; i < apg_dutList['apg' + apgX + '_dut_list'].length; i++) {
							if (apg_dutList['apg' + apgX + '_dut_list'][i].mac == Object.keys(VLAN_port_status)[y]) {
								if (apg_dutList['apg' + apgX + '_dut_list'][i].lanport == "") {
									apg_dutList['apg' + apgX + '_dut_list'][i].lanport = VLAN_port_status[Object.keys(VLAN_port_status)[y]].port[key].portLabel;
								}
								else{
									apg_dutList['apg' + apgX + '_dut_list'][i].lanport = apg_dutList['apg' + apgX + '_dut_list'][i].lanport + "," + VLAN_port_status[Object.keys(VLAN_port_status)[y]].port[key].portLabel;
								}
							}
						}
					}
				}
			}
		});
	}
}

var apgPostData = {};

function getApgDutListPostData(apgJson) {
	for (var apg in apgJson) {
		var apgFlatArray = [];

		$.each(apgJson[apg], function(idx, dutList) {
			var apgDutList = [];
			for (var attr in dutList) {
				apgDutList.push(dutList[attr])
			}

			apgFlatArray.push(apgDutList.join(">"))
		})

		apgPostData[apg] = "<" + apgFlatArray.join("<");
	}

	return apgPostData;
}

function clean_apgX_dut_list_lanport(apg_dutList) {
	$.each(apg_dutList, function(idx, mac_array) {
		for (var x = 0; x < mac_array.length; x++) {
			mac_array[x].lanport = ""; //clean apgX_dut_list lanport
		}
	})
}

function getvlanTrunkListPostData() {
	var TrunkListPostData = "";
	for (var y = 0; y < Object.keys(VLAN_port_status).length; y++) {	//by MAC
		mac_id_str = Object.keys(VLAN_port_status)[y].replace(/\:/g, '');
		var TrunkListByLanport = "";
		var TrunkListbyMac = "";
		const port_info = Object.entries(VLAN_port_status[Object.keys(VLAN_port_status)[y]].port);
		port_info.forEach(([key, value]) => {		//each port
			var lanport_idx = VLAN_port_status[Object.keys(VLAN_port_status)[y]].port[key].portLabel.replace("L", "");
			if ($("#switch_mode_" + mac_id_str + "_" + lanport_idx).val() == "Trunk" && $("#switch_vlan_" + mac_id_str + "_" + lanport_idx).val() != "all") {
				for (var z = 0; z < VLAN_Profile_select.length; z++) {
					TrunkListByLanport = getTrunkListVID($("#switch_vlan_" + mac_id_str + "_" + lanport_idx).val());
				}
				TrunkListbyMac += ">" + VLAN_port_status[Object.keys(VLAN_port_status)[y]].port[key].portLabel + "#" + TrunkListByLanport;
			}
			else if ($("#switch_mode_" + mac_id_str + "_" + lanport_idx).val() == "Trunk" && $("#switch_vlan_" + mac_id_str + "_" + lanport_idx).val() == "all") {
				TrunkListbyMac += ">" + VLAN_port_status[Object.keys(VLAN_port_status)[y]].port[key].portLabel + "#all";
			}
		});
		
		if (TrunkListbyMac != "") {
			TrunkListPostData += "<" + Object.keys(VLAN_port_status)[y] + TrunkListbyMac;
		}
	}

	return TrunkListPostData;
}
function getTrunkListVID(flag) {	//suppose flag(=flag[0]) is the only one vid binded for trunk mode
	var TrunkListVIDbyLanport = "";

	$.each(VLAN_Profile_select, function(i, Profile) {
		if (Profile.vid == flag) {
			TrunkListVIDbyLanport += (TrunkListVIDbyLanport == "") ? Profile.vid : "," + Profile.vid;
		}
	});

	return TrunkListVIDbyLanport;
}

function getvlanBlkListPostData() {
	var blkListPostData = "";
	for (var y = 0; y < Object.keys(VLAN_port_status).length; y++) {
		mac_id_str = Object.keys(VLAN_port_status)[y].replace(/\:/g, '');
		//Object.keys(VLAN_port_status)[y]
		var blkListByLanport = "";
		var blkListbyMac = "";
		for (var x = 1; x < VLAN_port_status[Object.keys(VLAN_port_status)[y]].port.length + 1; x++) {
			if ($("#switch_mode_" + mac_id_str + "_" + x).val() == "Trunk") {
				for (var z = 0; z < VLAN_Profile_select.length; z++) {
					blkListByLanport = getBlkListVID($("#switch_vlan_" + mac_id_str + "_" + x).val());
				}
				blkListbyMac += ">" + x + "#" + blkListByLanport;
			}
		}
		if (blkListbyMac != "") {
			blkListPostData += "<" + Object.keys(VLAN_port_status)[y] + blkListbyMac;
		}
	}

	return blkListPostData;
}

function getBlkListVID(flag) {
	var blkListVIDbyLanport = "";

	$.each(VLAN_Profile_select, function(i, VLAN_Profile_select) {
		if (VLAN_Profile_select.vid != flag) {
			blkListVIDbyLanport += (blkListVIDbyLanport == "") ? VLAN_Profile_select.vid : "," + VLAN_Profile_select.vid;
		}
	});

	return blkListVIDbyLanport;
}

function get_showLoading_status(_rc_service){
	var result = {"time": 20, "disconnect": false};//restart_net_and_phy will disconnect and logout
	if(_rc_service != undefined){
		if(_rc_service.indexOf("restart_net_and_phy;") >= 0){
			result.time = httpApi.hookGet("get_default_reboot_time");
			result.disconnect = true;
		}
		else if(_rc_service.indexOf("restart_chilli;restart_uam_srv;") >= 0){
			result.time = 30;
		}
	}
	return result;
}

function check_isAlive_and_redirect(_parm){
	var page = "";
	var time = 30;
	var interval_time = 2;
	if(_parm != undefined){
		if(_parm.page != undefined && _parm.page != "") page = _parm.page;
		if(_parm.time != undefined && _parm.time != "" && !isNaN(_parm.time)) time = parseInt(_parm.time);
	}
	var lan_ipaddr = httpApi.nvramGet(["lan_ipaddr"]).lan_ipaddr;
	setTimeout(function(){
		var interval_isAlive = setInterval(function(){
			httpApi.isAlive("", lan_ipaddr, function(){ clearInterval(interval_isAlive); top.location.href = "/" + page + "";});
		}, 1000*interval_time);
	}, 1000*(time - interval_time));
}

function manage_get_lacp(_node_info, _node_capability){
	var result = {"support": false, "value": 0};
	var lacp = _node_capability.lacp;
	if(lacp)
		result.support = true;

	if("config" in _node_info) {
		if("link_aggregation" in _node_info.config) {
			if("lacp_enabled" in _node_info.config.link_aggregation) {
				result.value = parseInt(_node_info.config.link_aggregation.lacp_enabled);
			}
		}
	}

	if(isNaN(result.value))
		result.value = 0;

	return result;
}
</script>
</head>

<body onload="initial();" class="bg">
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" id="form" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="Advanced_VLAN_Switch_Content.asp">
<input type="hidden" name="next_page" value="Advanced_VLAN_Switch_Content.asp">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="apply_new">
<input type="hidden" name="action_script" value="restart_net">
<input type="hidden" name="action_wait" value="10">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<!--input type="hidden" name="vlan_rulelist" value="<% nvram_char_to_ascii("","vlan_rulelist"); %>"-->
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
								<td bgcolor="#4D595D" valign="top" style="padding: 0px;">
									<div>&nbsp;</div>
									<div class="formfonttitle">VLAN</div>
									
									<div style="margin:10px 0 10px 5px;" class="splitLine"></div>
									<div class="formfontdesc"><#VALN_Switch_desc#></div>
									<div id="divSwitchMenu" style="margin-top:-40px;float:right;"></div>
									
									<div id="divVLANTable" style="width:100%;margin-top:25px;">
										<div id="tableCAP" class="port_status_table"></div>
									</div>
									
									<div class="apply_gen">
										<input class="button_gen" onclick="applyRule()" type="button" value="<#CTL_apply#>"/>
									</div>
								</td>
							</tr>
						</tbody>	
						</table>					
					</td>	
				</tr>
			</table>				
			<!--===================================End of Main Content===========================================-->
		</td>
		<td width="10" align="center" valign="top">&nbsp;</td>
	</tr>
</table>
</form>	
<div id="footer"></div>
</body>
</html>

