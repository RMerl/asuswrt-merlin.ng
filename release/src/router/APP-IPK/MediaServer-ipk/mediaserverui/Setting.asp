<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Cache-Control" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>Cloud Sync Setting</title>
<link rel="stylesheet" type="text/css" href="index_style.css">
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="routercss.css">
<link rel="stylesheet" type="text/css" href="ext/css/ext-all.css">
<!--<link href="multiLanguageCss/english.css" rel="stylesheet" type="text/css" id="languageCss" />
<link href="multiLanguageCss/english_1.css" rel="stylesheet" type="text/css" id="languageCss_1" />-->
<script type="text/javascript" src="/jquery.js"></script>
<script>
var generalSetting_data_tmp;
var cloud_rulelist_array = '';
var base_dir="";
var $j = jQuery.noConflict();
var multi_INT = 0;
var del_type = 0;
var modify_rowIndex = 0;
var url = "ms_apply.cgi";
var action_mode = "initial";
var type = "General";
var cloud_type_tmp = 0;
var cloud_status_tmp = '<img src="images/statusIcon/Finished.gif">';
url += "?action_mode=" + action_mode + "&download_type=" +type+ "&t=" +Math.random();
$j.ajax({url: url,
		async: false,
		success: function(data){initial_multi_INT_status(data);},
		error: function(XMLHttpRequest, textStatus, errorThrown){
		  if(XMLHttpRequest.status==598){
			if(XMLHttpRequest.responseText!=null){
				self.location = "http://"+ location.host + XMLHttpRequest.responseText +"/mediaserverui/mediaserver.asp";
			}
			else{
				self.location = "http://"+ location.host +"/Main_Login.asp";}
			}}
		});
$j.ajax({url: "cloud_print.cgi",
		async: false,
		success: function(data){cloud_rulelist_array = data;}
		});

function initial_multi_INT_status(data){
	var array = new Array();
	eval("array="+data);
	base_dir = array[8];
	generalSetting_data_tmp = data;
	var lang = array[14];
	if(lang == "EN")
	multi_INT = 0;
	else if(lang == "TW")
	multi_INT = 1;
	else if(lang == "CN")
	multi_INT = 2;
	else if(lang == "RU")
	multi_INT = 3;
	else if(lang == "FR")
	multi_INT = 4;
	else if(lang == "DE")
	multi_INT = 5;
	else if(lang == "BR")
	multi_INT = 6;
	else if(lang == "CZ")
	multi_INT = 7;
	else if(lang == "DA")
	multi_INT = 8;
	else if(lang == "FI")
	multi_INT = 9;
	else if(lang == "MS")
	multi_INT = 10;
	else if(lang == "NO")
	multi_INT = 11;
	else if(lang == "PL")
	multi_INT = 12;
	else if(lang == "SV")
	multi_INT = 13;
	else if(lang == "TH")
	multi_INT = 14;
	else if(lang == "TR")
	multi_INT = 15;
	else
	multi_INT = 0;
	}
</script>
<script type="text/javascript" src="multiLanguage_all.js"></script>
<script type="text/javascript" src="multiLanguage_setting.js"></script>
<script language="JavaScript" type="text/javascript" src="state.js"></script>
<script language="JavaScript" type="text/javascript" src="general.js"></script>
<script language="JavaScript" type="text/javascript" src="popup.js"></script>
<script type="text/javascript" src="/ext/ext-base.js"></script>
<script type="text/javascript" src="/ext/ext-all.js"></script>
<!--<script type="text/javascript" language="JavaScript" src="/help.js"></script>-->
<script type="text/javascript" language="JavaScript" src="detect.js"></script>
<script type="text/javascript" src="/plugin/jquery.iphone-switch.js"></script>
<script type="text/javascript" src="Setting.js"></script>
<style type="text/css">
a:focus{outline:none;}
.panel{
	width:450px;
	position:absolute;
	margin-top:-8%;
	margin-left:35%;
	z-index:200;
	display:none;
}
.floder_panel{
	background-color:#999;
	border:2px outset #CCC;
	font-size:15px;
	font-family:Verdana, Geneva, sans-serif;
	color:#333333;
	width:450px;
	position:absolute;
	margin-top:-8%;
	margin-left:35%;
	z-index:400;
	display:none;
}
</style>
<script>

function initial(){
	show_menu();
	//initial_nzb();
	document.getElementById("select_lang_"+multi_INT).selected = true;
	get_Refresh_time();
	showcloud_rulelist();
	initial_general_status(generalSetting_data_tmp);
}
function get_Refresh_time(){
	/*var url = "ms_apply.cgi";
	var action_mode = "initial";
	var type = "General";
	url += "?action_mode=" + action_mode + "&download_type=" +type+ "&t=" +Math.random();
	$j.ajax({
			url:url,
			success:function(data){initial_Refresh_time(data)},
			error:function(XMLHttpRequest, textStatus, errorThrown){error_Refresh_time()}
			});*/
	document.getElementById("helpAddress").href = "http://"+ location.host +"/cloudui/help.asp";
	}

function initial_Refresh_time(data){
	var initial_array = new Array();
	eval("initial_array="+data);
	document.getElementById("helpAddress").href = "http://"+initial_array[10]+":8081/cloudui/help.asp";
	}

function checkPassword(){
	var x = true;
	//if(document.getElementById('Password').readOnly == false){
		var a = document.getElementById('cloud_username').value;
		var b = document.getElementById('cloud_password').value;
		var c = document.getElementById('cloud_password_x').value;
		var d = document.getElementById('cloud_type').value;
		var e = document.getElementById('cloud_url').value;
		var f = document.getElementById('cloud_dir').value;
		if(a == ""){
			x = false;
			Ext.MessageBox.alert("Notice","Please key in the username.");
			}
		else if(b=="" || c ==""){
			x = false;
			Ext.MessageBox.alert("Notice","Please key in the password.");
			}
		else if(b!=c){
			x = false
			Ext.MessageBox.alert("Notice","Please check the password.");
			}
		else if(d==2 && e == ""){
			x = false;
			Ext.MessageBox.alert("Notice","Please key in the url.");
			}
		else if(f == ""){
			x = false;
			Ext.MessageBox.alert("Notice","Please select Directory.");
			}

		//}
		if(x)
		addRow_Group(32);
	}

function clear_text(){
	document.getElementById('cloud_username').value="";
	document.getElementById('cloud_password').value="";
	document.getElementById('cloud_password_x').value="";
	document.getElementById('cloud_url').value="";
	document.getElementById('cloud_dir').value="";
	document.getElementById('cloud_rule').value="0";
	document.getElementById('cloud_type').value="0";
	document.getElementById('cloud_enable').value="1";
	document.getElementById('cloud_url_x').style.display= "none";
	document.getElementById('Modify_button').style.display= "none";
	document.getElementById('Cancel_button').style.display= "none";
	document.getElementById('Add_button').style.display= "";

}

function applyRule(v,s)
{
	var url = "cloud_apply.cgi";
		url += "?action_mode=" + v;
		if(v=="CLOUD_ADD"){
			url += "&cloud_username=" + document.getElementById('cloud_username').value;
			url += "&cloud_password=" + document.getElementById('cloud_password').value;
			url += "&cloud_type=" + document.getElementById('cloud_type').value;
			if(document.getElementById('cloud_url').value==""){
				url += "&cloud_url=none";
			}
			else{
				url += "&cloud_url=" + document.getElementById('cloud_url').value;
			}
			url += "&cloud_dir="  + document.getElementById('cloud_dir').value;
			url += "&cloud_rule=" + document.getElementById('cloud_rule').value;
			url += "&cloud_enable=" + document.getElementById('cloud_enable').value+"&t="+Math.random();
		}
		else if (v=="CLOUD_DEL")
		{
					if(del_type==0)
					{
					  	url += "&cloud_type=0";
					}
					else if(del_type==1)
					{
						url += "&cloud_type=1";
					}
					else if(del_type==2)
					{
						url += "&cloud_type=2";
					}
					else if(del_type==3)
					{
						url += "&cloud_type=3";
					}
			url += "&cloud_enable=1";
			url += "&cloud_del_rowid=" + s +"&t="+Math.random();
		}
		else if(v=="CLOUD_MODIFY"){
			url += "&cloud_username=" + document.getElementById('cloud_username').value;
			url += "&cloud_password=" + document.getElementById('cloud_password').value;
			url += "&cloud_type=" + document.getElementById('cloud_type').value;
			if(document.getElementById('cloud_url').value==""){
				url += "&cloud_url=none";
			}
			else{
				url += "&cloud_url=" + document.getElementById('cloud_url').value;
			}
			url += "&cloud_dir="  + document.getElementById('cloud_dir').value;
			url += "&cloud_rule=" + document.getElementById('cloud_rule').value;
			url += "&cloud_enable=" + document.getElementById('cloud_enable').value;
			url += "&cloud_del_rowid=" + s +"&t="+Math.random();
		}
		$j.get(url,function(data){clear_text();});
}

function showcloud_rulelist(){
	//alert(cloud_rulelist_array);
	var cloud_rulelist_row = cloud_rulelist_array.split('&#60');
	var code = "";

	code +='<table width="760px" border="1" align="left" cellpadding="4" cellspacing="0"  class="list_table" id="cloud_rulelist_table">';
	if(cloud_rulelist_row.length == 1)
		code +='<tr><td style="color:#FFCC00;" colspan="6">Please select media server path.</td></tr>';
	else{
		for(var i = 1; i < cloud_rulelist_row.length; i++){
			code +='<tr id="row'+i+'">';
			var cloud_rulelist_col = cloud_rulelist_row[i].split('&#62');
			/*var wid=[15, 15, 20, 25, 10];
				for(var j = 0; j < cloud_rulelist_col.length; j++){
					alert(cloud_rulelist_col[j]);
					code +='<td width="'+wid[j]+'%">'+ cloud_rulelist_col[j] +'</td>';
				}*/
			code +='<td width="15%">'+ cloud_rulelist_col[2] +'</td>';
			if(cloud_rulelist_col[0]==0){
				code +='<td width="15%">'+ "AsusWebStorage" +'</td>';
			}else if(cloud_rulelist_col[0]==1)
			{
				code +='<td width="15%">'+ "Box.net" +'</td>';
			}
			else if(cloud_rulelist_col[0]==2)
			{
				code +='<td width="15%">'+ "WebDAV" +'</td>';
			}
			else if(cloud_rulelist_col[0]==3)
			{
				code +='<td width="15%">'+ "SkyDrive" +'</td>';
			}
			code +='<td style="display:none;">'+ cloud_rulelist_col[3] +'</td>';
			if(cloud_rulelist_col[4]==0){
				code +='<td width="20%">'+ "Download + Upload" +'</td>';
				//code +='<td width="20%">'+'<select name="cloud_rule_1" id="cloud_rule_1" class="input_option"><option class="content_input_fd" value="0" selected="selected">Download + Upload</option><option class="content_input_fd" value="1" >Download Only</option><option class="content_input_fd" value="2" >Upload Only</option></select>'+'</td>';
			}else if(cloud_rulelist_col[4]==1)
			{
				code +='<td width="20%">'+ "Download Only" +'</td>';
				//code +='<td width="20%">'+'<select name="cloud_rule_1" id="cloud_rule_1" class="input_option"><option class="content_input_fd" value="0" >Download + Upload</option><option class="content_input_fd" value="1" selected="selected">Download Only</option><option class="content_input_fd" value="2" >Upload Only</option></select>'+'</td>';
			}
			else if(cloud_rulelist_col[4]==2)
			{
				code +='<td width="20%">'+ "Upload Only" +'</td>';
				//code +='<td width="20%">'+'<select name="cloud_rule_1" id="cloud_rule_1" class="input_option"><option class="content_input_fd" value="0" >Download + Upload</option><option class="content_input_fd" value="1" >Download Only</option><option class="content_input_fd" value="2" selected="selected">Upload Only</option></select>'+'</td>';
			}

			code +='<td width="25%">'+ cloud_rulelist_col[5] +'</td>';
			if(cloud_rulelist_col[1]==1){
				code +='<td width="10%">'+'<img  src="images/statusIcon/Initialing.gif">'+'</td>';
			}
			else
			{
				code +='<td width="10%">'+'<img  src="images/statusIcon/Stop.gif">'+'</td>';
			}

				code +='<td width="10%"><input class="edit_btn" onclick="edit_Row(this);" value="">';
				code +='<input class="remove_btn" onclick="del_Row(this);" value=""/></td></tr>';
		}
	}
  code +='</table>';
	$("cloud_rulelist_Block").innerHTML = code;
}

function addRow(obj, head){
	if(head == 1)
		cloud_rulelist_array += "&#60"
	else
		cloud_rulelist_array += "&#62"

	cloud_rulelist_array += obj.value;
	//obj.value = "";
}

function addRow_Group(upper){
	var rule_num = $('cloud_rulelist_table').rows.length;
	var item_num = $('cloud_rulelist_table').rows[0].cells.length;

	if(rule_num >= upper){
		alert("This table only allow " + upper + " items!");
		return false;
	}

		/*if(item_num >=2){
			for(i=0; i<rule_num; i++){
					if($('cloud_rulelist_table').rows[i].cells[1].innerHTML== "AsusWebStorage")
					{
					  	cloud_type_tmp = 0;
					}
					else if($('cloud_rulelist_table').rows[i].cells[1].innerHTML== "Box.net")
					{
						cloud_type_tmp = 1;
					}
					else if($('cloud_rulelist_table').rows[i].cells[1].innerHTML== "WebDAV")
					{
						cloud_type_tmp = 2;
					}
					else if($('cloud_rulelist_table').rows[i].cells[1].innerHTML== "SkyDrive")
					{
						cloud_type_tmp = 3;
					}

					if(document.getElementById('cloud_type').value ==  cloud_type_tmp &&  document.getElementById('cloud_username').value == $('cloud_rulelist_table').rows[i].cells[0].innerHTML){
						Ext.MessageBox.alert("Notice","User Exist!");
						return false;
					}

					if(document.getElementById('cloud_type').value ==  cloud_type_tmp  )
					{
						document.getElementById('cloud_enable').value=0;
					}
			}
		}*/
		if(item_num >= 2){
			for(i=0; i<rule_num; i++){

					if(document.getElementById('cloud_type').value==0 && $('cloud_rulelist_table').rows[i].cells[1].innerHTML== "AsusWebStorage")
					{
						Ext.MessageBox.alert("Notice","AsusWebStorage User Exist!");
						return false;
					}
					else if(document.getElementById('cloud_type').value==1 && $('cloud_rulelist_table').rows[i].cells[1].innerHTML== "Box.net")
					{
						Ext.MessageBox.alert("Notice","Box.net User Exist!");
						return false;
					}
					else if(document.getElementById('cloud_type').value==2 && $('cloud_rulelist_table').rows[i].cells[1].innerHTML== "WebDAV")
					{
						var cloud_rulelist_row_1 = cloud_rulelist_array.split('&#60');
						for(var i = 1; i < cloud_rulelist_row_1.length; i++){
							var cloud_rulelist_col_1 = cloud_rulelist_row_1[i].split('&#62');

							if(document.getElementById('cloud_url').value==cloud_rulelist_col_1[3]){
									Ext.MessageBox.alert("Notice","WebDAV User Exist!");
									return false;
							}

						}
					}
					else if(document.getElementById('cloud_type').value==3 && $('cloud_rulelist_table').rows[i].cells[1].innerHTML== "SkyDrive")
					{
						Ext.MessageBox.alert("Notice","SkyDrive User Exist!");
						return false;
					}
			}
		}
		addRow(document.getElementById('cloud_type'), 1);
		addRow(document.getElementById('cloud_enable'), 0);
		addRow(document.getElementById('cloud_username') ,0);

		if(document.getElementById('cloud_url').value==""){
			document.getElementById('cloud_url').value="none";
		}
		addRow(document.getElementById('cloud_url'), 0);
		addRow(document.getElementById('cloud_rule'), 0);
		addRow(document.getElementById('cloud_dir'), 0);



		showcloud_rulelist();
		applyRule("CLOUD_ADD",0);
		showLoading(2);
}

function getdata(){
	var url = "cloud_print.cgi";
	url += "?t=" +Math.random();
	$j.get(url,function(data){cloud_rulelist_array = data;});

}


function table_refresh(){
	if(cloud_rulelist_array == "")
		showcloud_rulelist();
}

function modify_cloud(){

	$('cloud_rulelist_table').rows[modify_rowIndex].cells[0].innerHTML=document.getElementById('cloud_username').value;
	$('cloud_rulelist_table').rows[modify_rowIndex].cells[2].innerHTML=document.getElementById('cloud_url').value;
	$('cloud_rulelist_table').rows[modify_rowIndex].cells[4].innerHTML=document.getElementById('cloud_dir').value;
	if(document.getElementById('cloud_rule').value=="0"){
		$('cloud_rulelist_table').rows[modify_rowIndex].cells[3].innerHTML="Download + Upload";
	}
	else if(document.getElementById('cloud_rule').value=="1"){
		$('cloud_rulelist_table').rows[modify_rowIndex].cells[3].innerHTML="Download Only";
	}
	else if(document.getElementById('cloud_rule').value=="2"){
		$('cloud_rulelist_table').rows[modify_rowIndex].cells[3].innerHTML="Upload Only";
	}

	applyRule("CLOUD_MODIFY",modify_rowIndex);
	showLoading(3);
}


function edit_Row(r){

	document.getElementById('Modify_button').style.display= "";
	document.getElementById('Cancel_button').style.display= "";
	document.getElementById('Add_button').style.display= "none";
	var i=r.parentNode.parentNode.rowIndex;
	modify_rowIndex = i;
	if($('cloud_rulelist_table').rows[i].cells[1].innerHTML=="AsusWebStorage")
	{
		document.getElementById('cloud_type')[0].selected = true;
		document.getElementById('cloud_url_x').style.display= "none";
	}
	else if($('cloud_rulelist_table').rows[i].cells[1].innerHTML=="Box.net")
	{
		document.getElementById('cloud_type')[1].selected = true;
		document.getElementById('cloud_url_x').style.display= "none";
	}
	else if($('cloud_rulelist_table').rows[i].cells[1].innerHTML=="WebDAV")
	{
		document.getElementById('cloud_type')[2].selected = true;
		document.getElementById('cloud_url_x').style.display= "";
	}
	else if($('cloud_rulelist_table').rows[i].cells[1].innerHTML=="SkyDrive")
	{
		document.getElementById('cloud_type')[3].selected = true;
		document.getElementById('cloud_url_x').style.display= "none";
	}
	document.getElementById('cloud_url').value = $('cloud_rulelist_table').rows[i].cells[2].innerHTML;
	document.getElementById('cloud_username').value = $('cloud_rulelist_table').rows[i].cells[0].innerHTML;
	document.getElementById('cloud_dir').value = $('cloud_rulelist_table').rows[i].cells[4].innerHTML;
	if($('cloud_rulelist_table').rows[i].cells[3].innerHTML=="Download + Upload")
	{
		document.getElementById('cloud_rule')[0].selected = true;
	}
	else if($('cloud_rulelist_table').rows[i].cells[3].innerHTML=="Download Only")
	{
		document.getElementById('cloud_rule')[1].selected = true;
	}
	else if($('cloud_rulelist_table').rows[i].cells[3].innerHTML=="Upload Only")
	{
		document.getElementById('cloud_rule')[2].selected = true;
	}

}

function del_Row(r){
if(confirm("Are you sure you want to delete?")){
  var i=r.parentNode.parentNode.rowIndex;

	if($('cloud_rulelist_table').rows[i].cells[1].innerHTML=="AsusWebStorage")
	{
		del_type=0;
	}
	else if($('cloud_rulelist_table').rows[i].cells[1].innerHTML=="Box.net")
	{
		del_type=1;
	}
	else if($('cloud_rulelist_table').rows[i].cells[1].innerHTML=="WebDAV")
	{
		del_type=2;
	}
	else if($('cloud_rulelist_table').rows[i].cells[1].innerHTML=="SkyDrive")
	{
		del_type=3;
	}

  $('cloud_rulelist_table').deleteRow(i);

  /*var cloud_rulelist_value = "";
	for(k=0; k<$('cloud_rulelist_table').rows.length; k++){
		for(j=0; j<$('cloud_rulelist_table').rows[k].cells.length-1; j++){
			if(j == 0)
				cloud_rulelist_value += "&#60";
			else
				cloud_rulelist_value += "&#62";

			cloud_rulelist_value += $('cloud_rulelist_table').rows[k].cells[j].innerHTML;

		}
	}*/



	applyRule("CLOUD_DEL",i);
	setTimeout('getdata();',3000);
	showLoading(3);
	//cloud_rulelist_array = cloud_rulelist_value;
	//if(cloud_rulelist_array == "")
	//	showcloud_rulelist();
	setTimeout('table_refresh();',4000);

}
}

function cloud_type_change(){

	if(document.getElementById('cloud_type').value==2)
	{
		document.getElementById('cloud_url_x').style.display= "";
	}
	else{
		document.getElementById('cloud_url_x').style.display= "none";
	}
}

</script>

</head>

<body onload="initial();" onunLoad="return unload_body();">
<div id="TopBanner"></div>
<div id="panel_add" class="panel" >
	<div id="tree"></div>
	<!--<div><input type="button" onclick="hidePanel();" /></div>-->
</div>
<div id="DM_mask_floder" class="mask_floder_bg"></div>
<div id="panel_addFloder" class="floder_panel">
<span style="margin-left:95px;"><b>Please key in the floder name</b></span><br /><br />
<span style="margin-left:8px;margin-right:8px;"><b>Floder Name:</b></span>
<input type="text" id="newFloder" class="input_15_table" value="" /><br /><br />
<input type="button" name="AddFloder" id="Apply_1" value="Apply" style="margin-left:100px;" onclick="AddFloderName();">
&nbsp;&nbsp;
<input type="button" name="Cancel_Floder_add" id="Cancel_1" value="Cancel" onClick="hide_AddFloder();">
</div>
<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="get" name="form" id="Setting" action="cloud_apply.cgi" target="hidden_frame" />
<input type="hidden" name="action_mode" value="CLOUD_APPLY" />


<table class="content" align="center" cellpadding="0" cellspacing="0">
	<tr>
		<td width="17">&nbsp;</td>

		<td valign="top" width="101">
		<!--<div  id="mainMenu"></div>
		<div  id="subMenu"></div>-->
		</td>

    <td valign="top">
		<!-- delete by alan <div id="tabMenu" class="submenuBlock"></div> -->
		<!--===================================Beginning of Main Content===========================================-->

        <div style="margin-top:-150px; padding-left:0px;">

<table width="98%" border="0" align="left" cellpadding="0" cellspacing="0">

	<tr>
		<td align="left" valign="top" >

		<table width="760px" border="0" cellpadding="5" cellspacing="0" class="FormTitle" id="FormTitle">
		<tbody>
			<tr>
		  		<td bgcolor="#4D595D">
		  		<div>&nbsp;</div>
		  		<div class="formfonttitle" id="multiSetting_30">Cloud Sync</div>
		  		<div style="margin-left:5px;margin-top:10px;margin-bottom:10px"><img src="/images/New_ui/export/line_export.png"></div>
		 		  <!-- delete by alan  <div class="formfontdesc"><#LANHostConfig_x_DDNSEnable_sectiondesc#></div>-->

			<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
			<tr><td colspan="2" id="multiSetting_31">Cloud Account</td></tr>
            <tr>
			<th class="hintstyle_download">Client Type</th>
			   	<td>
						<select name="cloud_type" id="cloud_type" class="input_option" onChange="cloud_type_change();">
							<option class="content_input_fd" value="0" >AsusWebStorage</option>
							<option class="content_input_fd" value="1" >Box.net</option>
							<option class="content_input_fd" value="2" >WebDAV</option>
							<option class="content_input_fd" value="3" >SkyDrive</option>
						</select>
			   	</td>
            <tr>
            	<th class="hintstyle_download" id="multiSetting_33">User Name</th>
            	<td><input id="cloud_username" type="text" class="input_25_table" name="cloud_username" /></td>
            </tr>
            <tr>
            	<th class="hintstyle_download" id="multiSetting_34">Password</th>
            	<td><input id="cloud_password" type="password" class="input_25_table" name="cloud_password" /></td>
            </tr>
            <tr>
            	<th class="hintstyle_download" id="multiSetting_35">Confirm Password</th>
            	<td><input id="cloud_password_x" name="cloud_password_x" type="password" class="input_25_table" /></td>
            </tr>
            <tr id="cloud_url_x" style="display:none;">
            	<th class="hintstyle_download" id="multiSetting_36">Url Address</th>
            	<td><input type="text" maxlength="256" class="input_32_table" id="cloud_url" name="cloud_url" onKeyPress="return is_string(this)"></td>
            </tr>
        <tr>
          <th class="hintstyle_download" id="multiSetting_37">Sync Dir</th>
          <td>
          <input type="text" id="cloud_dir" class="input_25_table" value="" name="cloud_dir" readonly="readonly"/>
          <input type="button" class="button_gen" id="multiSetting_6" value="Browse" onclick="showPanel();"/>
          </td>
        </tr>


         		<tr>
			<th class="hintstyle_download">Rule</th>
			   	<td>
						<select name="cloud_rule" id="cloud_rule" class="input_option" onChange="">
							<option class="content_input_fd" value="0" >Download + Upload</option>
							<option class="content_input_fd" value="1" >Download Only</option>
							<option class="content_input_fd" value="2" >Upload Only</option>
						</select>
			   	</td>
            <tr>
            <tr id="cloud_enable_x" style="display:none">
            	<th class="hintstyle_download" id="multiSetting_39">Enabled?</th>
            	<td><input type="text" maxlength="2" class="input_12_table" id="cloud_enable" name="cloud_enable" onKeyPress="return is_string(this)" value="1"></td>
            </tr>
		</table>
				<div class="apply_gen">
					<input id="Modify_button" class="button_gen" onclick="modify_cloud();" type="button" value="Modify" style="display:none;"/>
					<input id="Cancel_button" class="button_gen" onclick="clear_text();" type="button" value="Cancel" style="display:none;"/>
					<input id="Add_button" class="button_gen" onclick="checkPassword();" type="button" value="Add" />

				</div>

			  </td>
              </tr>
            </tbody>

            </table>
<table width="760px" border="1" align="left" cellpadding="4" cellspacing="0"  class="FormTable_table">
	  	  	<thead>
           		<tr>
            		<td colspan="7" id="cloud_rulelist">Cloud List</td>
          		</tr>
		  	</thead>

    	      <tr>
		<th class="hintstyle_download" width="15%">UserName</th>
		<th class="hintstyle_download" width="15%">Type</th>
		<th class="hintstyle_download" style="display:none;">url</th>
            	<th class="hintstyle_download" width="20%">Rule</th>
            	<th class="hintstyle_download" width="25%">Folder</th>
            	<th class="hintstyle_download" width="10%">Status</th>
            	<th class="hintstyle_download" width="10%">Edit</th>
  	        </tr>
		<!--<tr>
          	<td width="15%">user1</td>
		<td width="15%">AsusWebStorage</td>
            	<td width="20%">Download + Upload</td>
            	<td width="25%">//USB/DISK/SyncDir</td>
            	<td width="10%">Running</td>
            	<td width="10%"><input class="remove_btn" onclick="del_Row(this);" value=""/></td>
		</tr>-->
	  	<tr><td colspan="7"><div id="cloud_rulelist_Block"></div></td></tr>
</table>

		  </td>
</form>



        </tr>
      </table>
      </div>
		<!--===================================Ending of Main Content===========================================-->
	</td>

    <td width="10" align="center" valign="top">&nbsp;</td>

	</tr>

</table>

<div id="footer"></div>

</body>
</html>
