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

<title><#Web_Title#> - <#menu5_3_3#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="/js/table/table.css">
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" language="JavaScript" src="/help.js"></script>
<script type="text/javascript" language="JavaScript" src="/validator.js"></script>
<script type="text/javascript" language="JavaScript" src="/js/table/table.js"></script>
<script type="text/javascript" language="JavaScript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/js/httpApi.js"></script>
<script>
var autofw_rulelist_array = [];
var wans_mode ='<% nvram_get("wans_mode"); %>';

function initial(){
	show_menu();
	// https://www.asus.com/support/FAQ/114110/
	httpApi.faqURL("114110", function(url){document.getElementById("faq").href=url;});
	well_known_apps();
	//parse nvram to array
	var parseNvramToArray = function() {
		var parseArray = [];
		var oriNvram = '<% nvram_char_to_ascii("","autofw_rulelist"); %>';
		var oriNvramRow = decodeURIComponent(oriNvram).split('<');
		for(var i = 0; i < oriNvramRow.length; i += 1) {
			if(oriNvramRow[i] != "") {
				var oriNvramCol = oriNvramRow[i].split('>');
				var eachRuleArray = new Array();
				for(var j = 0; j < oriNvramCol.length; j += 1) {
					eachRuleArray.push(oriNvramCol[j]);
				}
				parseArray.push(eachRuleArray);
			}
		}
		return parseArray;
	};

	autofw_rulelist_array = parseNvramToArray();
	showautofw_rulelist();

	//if(dualWAN_support && wans_mode == "lb")
	//	document.getElementById("lb_note").style.display = "";
}

function well_known_apps(){
	wItem = new Array(new Array("Quicktime 4 Client", "554", "TCP", "6970:32000", "UDP"),new Array("Real Audio", "7070", "TCP", "6970:7170", "UDP"));
	free_options(document.form.TriggerKnownApps);
	add_option(document.form.TriggerKnownApps, "<#Select_menu_default#>", "User Defined", 1);
	for (i = 0; i < wItem.length; i++){
		add_option(document.form.TriggerKnownApps, wItem[i][0], wItem[i][0], 0);
	}
}
function applyRule(){
	//parse array to nvram
	var tmp_value = "";
	for(var i = 0; i < autofw_rulelist_array.length; i += 1) {
		if(autofw_rulelist_array[i].length != 0) {
			tmp_value += "<";
			for(var j = 0; j < autofw_rulelist_array[i].length; j += 1) {
				tmp_value += autofw_rulelist_array[i][j];
				if( (j + 1) != autofw_rulelist_array[i].length)
					tmp_value += ">";
			}
		}
	}
	
	document.form.autofw_rulelist.value = tmp_value;

	showLoading();
	document.form.submit();
}

function done_validating(action){
	refreshpage();
}

function change_wizard(o){
	for(var i = 0; i < wItem.length; i++){
		if(wItem[i][0] != null){
			if(o.value == wItem[i][0]){
				var wellKnownApp = new Array();
				wellKnownApp.push(wItem[i][0]);
				wellKnownApp.push(wItem[i][1]);
				wellKnownApp.push(wItem[i][2]);
				wellKnownApp.push(wItem[i][3]);
				wellKnownApp.push(wItem[i][4]);
				var validDuplicateFlag = true;
				if(tableApi._attr.hasOwnProperty("ruleDuplicateValidation")) {
					var currentEditRuleArray = wellKnownApp;
					var filterCurrentEditRuleArray = autofw_rulelist_array;

					validDuplicateFlag = tableRuleDuplicateValidation[tableApi._attr.ruleDuplicateValidation](currentEditRuleArray, filterCurrentEditRuleArray);
					if(!validDuplicateFlag) {
						document.form.TriggerKnownApps.selectedIndex = 0;
						alert("<#JS_duplicate#>");
						return false;
					}
					autofw_rulelist_array.push(currentEditRuleArray);
					showautofw_rulelist();
				}			
				break;
			}
		}
	}
	document.form.TriggerKnownApps.selectedIndex = 0;
}

function showautofw_rulelist(){

	//set table Struct
	var tableStruct = {
		data: autofw_rulelist_array,
		container: "tableContainer",
		title: "<#IPConnection_TriggerList_groupitemdesc#>",
		capability: {
			add: true,
			del: true,
			clickEdit: true
		},
		header: [ 
			{
				"title" : "<#IPConnection_autofwDesc_itemname#>",
				"width" : "26%"
			},
			{
				"title" : "<#IPConnection_autofwOutPort_itemname#>",
				"width" : "22%"
			},
			{
				"title" : "<#IPConnection_VServerProto_itemname#>",
				"width" : "10%"
			},
			{
				"title" : "<#IPConnection_autofwInPort_itemname#>",
				"width" : "22%"
			},
			{
				"title" : "<#IPConnection_VServerProto_itemname#>",
				"width" : "10%"
			}
		],
		createPanel: {
			inputs : [
				{
					"editMode" : "text",
					"title" : "<#IPConnection_autofwDesc_itemname#>",
					"maxlength" : "18",
					"valueMust" : false,
					"validator" : "description"
				},
				{
					"editMode" : "text",
					"title" : "<#IPConnection_autofwOutPort_itemname#>",
					"maxlength" : "11",
					"validator" : "portRange"
				},
				{
					"editMode" : "select",
					"title" : "<#IPConnection_VServerProto_itemname#>",
					"option" : {"TCP" : "TCP", "UDP" : "UDP"}
				},
				{
					"editMode" : "text",
					"title" : "<#IPConnection_autofwInPort_itemname#>",
					"maxlength" : "11",
					"validator" : "portRange"
				},
				{
					"editMode" : "select",
					"title" : "<#IPConnection_VServerProto_itemname#>",
					"option" : {"TCP" : "TCP", "UDP" : "UDP"}
				},
			],
			maximum: 32
		},

		clickRawEditPanel: {
			inputs : [
				{
					"editMode" : "text",
					"maxlength" : "18",
					"valueMust" : false,
					"validator" : "description"
				},
				{
					"editMode" : "text",
					"maxlength" : "11",
					"validator" : "portRange"
				},
				{
					"editMode" : "select",
					"option" : {"TCP" : "TCP", "UDP" : "UDP"}
				},
				{
					"editMode" : "text",
					"maxlength" : "11",
					"validator" : "portRange"
				},
				{
					"editMode" : "select",
					"option" : {"TCP" : "TCP", "UDP" : "UDP"}
				}
			]
		},

		ruleDuplicateValidation : "triggerPort"
	}

	tableApi.genTableAPI(tableStruct);
}

function changeBgColor(obj, num){
	if(obj.checked)
 		document.getElementById("row" + num).style.background='#FF9';
	else
 		document.getElementById("row" + num).style.background='#FFF';
}

function trigger_validate_duplicate_noalert(o, v, l, off){
	for (var i=0; i<o.length; i++)
	{
		if (entry_cmp(o[i][0].toLowerCase(), v.toLowerCase(), l)==0){
			return false;
		}
	}
	return true;
}

function trigger_validate_duplicate(o, v, l, off){
	for(var i = 0; i < o.length; i++)
	{
		if(entry_cmp(o[i][1].toLowerCase(), v.toLowerCase(), l) == 0){
			alert("<#JS_duplicate#>");
			return false;
		}
	}
	return true;
}
</script>
</head>

<body onload=" initial();" onunLoad="return unload_body();">
<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="current_page" value="Advanced_PortTrigger_Content.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_wait" value="5">
<input type="hidden" name="action_script" value="restart_firewall">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="autofw_rulelist" value=''>
<input type="hidden" name="autofw_num_x_0" value="<% nvram_get("autofw_num_x"); %>" readonly="1" />

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
		<td valign="top" >
		
<table width="760px" border="0" cellpadding="4" cellspacing="0" class="FormTitle" id="FormTitle">
	<tbody>
	<tr>
		<td bgcolor="#4D595D" valign="top"  >
		<div>&nbsp;</div>
		<div class="formfonttitle"><#menu5_3#> - <#menu5_3_3#></div>
		<div style="margin:10px 0 10px 5px;" class="splitLine"></div>
		<div class="formfontdesc"><#IPConnection_porttrigger_sectiondesc#></div>
		<div class="formfontdesc" style="margin-top:-10px;">
			<a id="faq" href="" target="_blank" style="font-family:Lucida Console;text-decoration:underline;"><#menu5_3_3#>&nbspFAQ</a>
		</div>
		<div class="formfontdesc" id="lb_note" style="color:#FFCC00; display:none;"><#lb_note_portTrigger#></div>
	
		<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable">
					  <thead>
					  <tr>
						<td colspan="6"><#t2BC#></td>
					  </tr>
					  </thead>

          	<tr>
            	<th colspan="2"><#IPConnection_autofwEnable_itemname#></th>
            	<td colspan="4">
			  				<input type="radio" value="1" name="autofw_enable_x" class="content_input_fd" onClick="return change_common_radio(this, 'IPConnection', 'autofw_enable_x', '1')" <% nvram_match("autofw_enable_x", "1", "checked"); %>><#checkbox_Yes#>
			 			 		<input type="radio" value="0" name="autofw_enable_x" class="content_input_fd" onClick="return change_common_radio(this, 'IPConnection', 'autofw_enable_x', '0')" <% nvram_match("autofw_enable_x", "0", "checked"); %>><#checkbox_No#>
							</td>
		  			</tr>		  	
		  		
						<tr>
            	<th colspan="2"align="right" id="autofw_rulelist"><#IPConnection_TriggerList_widzarddesc#></th>
            	<td colspan="4">
            		<select name="TriggerKnownApps" class="input_option" onChange="change_wizard(this);">
            			<option value="User Defined"><#Select_menu_default#></option>
            		</select>
            	</td>
          	</tr>
          </table>
          
          <div id="tableContainer"></div>

			<div class="apply_gen">
				<input name="button" type="button" class="button_gen" onclick="applyRule();" value="<#CTL_apply#>"/>
			</div>
		
		
		</td>
	</tr>
</tbody>

</table>
</td>
</form>        </tr>
      </table>				
		<!--===================================Ending of Main Content===========================================-->		
	</td>
		
    <td width="10" align="center" valign="top">&nbsp;</td>
	</tr>
</table>

<div id="footer"></div>

</body>
</html>
