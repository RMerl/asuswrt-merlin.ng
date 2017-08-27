<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=EmulateIE7"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png"><title>Media Server</title>
<link rel="stylesheet" type="text/css" href="index_style.css">
<link rel="stylesheet" type="text/css" href="form_style.css">
<script type="text/javascript" src="state.js"></script>
<script type="text/javascript" src="detect.js"></script>
<script type="text/javascript" src="popup.js"></script>
<script type="text/javascript" src="jquery.js"></script>
<script type="text/javascript" src="plugin/jquery.iphone-switch.js"></script>
<script type="text/javascript" src="multiLanguage.js"></script>
<script>
var data_temp;
var produc;
var FromObject = "0";
var lastClickedObj = 0;
var $j = jQuery.noConflict();
var multi_INT = 0;
var LOCAL_DOMAIN="";
var AUTOLOGOUT_MAX_MINUTE_TMP=0;

var url = "ms_apply.cgi";
var action_mode = "initial";
var selectedPoolOrder = -1;
var type = "General";
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


function initial_multi_INT_status(data){
	var array = new Array();
	eval("array="+data);
	data_temp = data;
	var lang = array[14];
	LOCAL_DOMAIN = array[28];
	AUTOLOGOUT_MAX_MINUTE_TMP=array[29];
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
	else if(lang == "IT")
	multi_INT = 16;
	else if(lang == "JP")
	multi_INT = 17;
	else if(lang == "UK")
	multi_INT = 18;
	else if(lang == "ES")
	multi_INT = 19;
	else if(lang == "HU")
	multi_INT = 20;
	else if(lang == "RO")
	multi_INT = 21;
	else
	multi_INT = 0;
	}

</script>
<style type="text/css">
.upnp_table{
	width:750px;
	padding:5px;
	padding-top:20px;
	margin-top:-17px;
	position:relative;
	background-color:#4d595d;
	align:left;
	-webkit-border-top-right-radius: 05px;
	-webkit-border-bottom-right-radius: 5px;
	-webkit-border-bottom-left-radius: 5px;
	-moz-border-radius-topright: 05px;
	-moz-border-radius-bottomright: 5px;
	-moz-border-radius-bottomleft: 5px;
	border-top-right-radius: 05px;
	border-bottom-right-radius: 5px;
	border-bottom-left-radius: 5px;
}
.vert_line{
	max-width:25px;
	width:19px;
	line-height:25px;
	vertical-align:top;
	/*margin-left:2px;
	padding-left:23px;	*/
}
.line_export{
	height:20px;
	width:736px;
}
.upnp_button_table{
	width:730px;
	background-color:#15191b;
	margin-top:15px;
	margin-right:5px;
}
.upnp_button_table th{
	width:300px;
	height:40px;
	text-align:left;
	background-color:#1f2d35;
	font:Arial, Helvetica, sans-serif;
	font-size:12px;
	padding-left:10px;
	color:#FFFFFF;
	background: url(images/general_th.gif) repeat;
}
.upnp_button_table td{
	width:436px;
	height:40px;
	background-color:#475a5f;
	font:Arial, Helvetica, sans-serif;
	font-size:12px;
	padding-left:5px;
	color:#FFFFFF;
}
.upnp_icon{
	background: url(images/New_ui/media_sever.jpg) no-repeat;
	width:736px;
	height:500px;
	margin-top:15px;
	margin-right:5px;
}
/* folder tree */
.mask_bg{
	position:absolute;
	margin:auto;
	top:0;
	left:0;
	width:100%;
	height:100%;
	z-index:100;
	/*background-color: #FFF;*/
	background:url(images/popup_bg2.gif);
	background-repeat: repeat;
	filter:progid:DXImageTransform.Microsoft.Alpha(opacity=60);
	-moz-opacity: 0.6;
	display:none;
	/*visibility:hidden;*/
	overflow:hidden;
}
.mask_floder_bg{
	position:absolute;
	margin:auto;
	top:0;
	left:0;
	width:100%;
	height:100%;
	z-index:300;
	/*background-color: #FFF;*/
	background:url(images/popup_bg2.gif);
	background-repeat: repeat;
	filter:progid:DXImageTransform.Microsoft.Alpha(opacity=60);
	-moz-opacity: 0.6;
	display:none;
	/*visibility:hidden;*/
	overflow:hidden;
}
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
.panel_folder{
	font-family:Courier ;
	width:500px;
	position:absolute;
	z-index:2000;
	display:none;
	background-image:url(images/Tree/bg_01.png);
	background-repeat:no-repeat;
}
.icon{
	width:40px;
	height:40px;
	margin-left:2px;
	float:left;
	text-decoration:none;
}
.icon a:link, .icon a:visited{
	cursor:pointer;
	display: block;
	width:40px;
	height:40px;
}

.folder_tree{
	font-size:10pt;
	margin:0px 0px 1px 30px;
	height:339px;
	overflow:auto;
	width:455px;
}

.folderClicked{
	color:#569AC7;
	/*font-weight:bolder;*/
	font-size:14px;
	cursor:text;
}
.lastfolderClicked{
	color:#FFFFFF;
	cursor:pointer;
}
.dlna_path_td{
 padding-left:15px;
 padding-right:15px;
 text-align:left;
}

#icon_home a:active{background:url(images/icon/home_click.png);}
#icon_home{background:url(images/icon/home_disable.png);}
#icon_home a:link, #icon_home a:visited, #icon_home a:hover{background:url(images/icon/home.png);}

</style>
<script>
var dms_status = "Idle"
var _layer_order = "";


function update_status(data){
	var dmsStatus_tmp = new Array();
	eval("dmsStatus_tmp="+data);
	if(dmsStatus_tmp[0] != "Idle")
		$("dmsStatus").innerHTML = "Scanning.."
	else
		$("dmsStatus").innerHTML = "Idle"
}

var updateStatusCounter = 0;
var AUTOLOGOUT_MAX_MINUTE = 0;
function Ajax_Get_MediaServer_Status()
{
	if(updateStatusCounter > parseInt(20 * AUTOLOGOUT_MAX_MINUTE))
	{
		location = "Logout.asp";
	}
	if(AUTOLOGOUT_MAX_MINUTE > 0) {
		updateStatusCounter++;
	}
	var url = "media_print.cgi" + "?action_mode=MEDIASERVER_GETSTATUS"+ "&BaseDir=" + $j("#PATH").attr("title") +"&t="+Math.random();
	$j.ajax({url: url,
			async: false,
			success: function(data){update_status(data);},
			error: function(XMLHttpRequest, textStatus, errorThrown){
			  if(XMLHttpRequest.status==598){
				if(XMLHttpRequest.responseText!=null){
					self.location = "http://"+ location.host + XMLHttpRequest.responseText + "/mediaserverui/mediaserver.asp";
				}
				else{
					self.location = "http://"+ location.host +"/Main_Login.asp";}
				}}
			});
}

function del_Row(r){
  var i=r.parentNode.parentNode.rowIndex;
  document.getElementById("dlna_path_table").deleteRow(i);

  var dms_dir_x_tmp = "";
  var dms_dir_type_x_tmp = "";
	for(var k=0; k<document.getElementById("dlna_path_table").rows.length; k++){
			dms_dir_x_tmp += "<";
			dms_dir_x_tmp += document.getElementById("dlna_path_table").rows[k].cells[0].title;
			dms_dir_type_x_tmp += "<";
			dms_dir_type_x_tmp += document.getElementById("dlna_path_table").rows[k].cells[1].innerHTML.indexOf("Audio")>=0? "A" :"";
			dms_dir_type_x_tmp += document.getElementById("dlna_path_table").rows[k].cells[1].innerHTML.indexOf("Image")>=0? "P" :"";
			dms_dir_type_x_tmp += document.getElementById("dlna_path_table").rows[k].cells[1].innerHTML.indexOf("Video")>=0? "V" :"";
	}
	dms_dir_x_array = dms_dir_x_tmp;
	dms_dir_type_x_array = dms_dir_type_x_tmp;

	if(dms_dir_x_array == "")
		show_dlna_path();
}

function addRow_Group(upper){
	var dms_dir_type_x_tmp = "";
	var rule_num = document.getElementById("dlna_path_table").rows.length;
	var item_num = document.getElementById("dlna_path_table").rows[0].cells.length;
	if(rule_num >= upper){
		alert("This table only allow " + upper + " items!");
		return false;
	}

	if(document.getElementById("PATH").value==""){
		alert(multiLanguage_array[multi_INT][16]);
		document.getElementById("PATH").focus();
		document.getElementById("PATH").select();
		return false;
	}else if(document.getElementById("PATH").value.indexOf("<") >= 0){
		alert("This string cannot contain:&nbsp; <");
		document.getElementById("PATH").focus();
		document.getElementById("PATH").select();
		return false;
	}
//Not allow to set shared UPNP path if shared content type is not set
	if(!document.getElementById("type_A_audio").checked &&
			!document.getElementById("type_P_image").checked &&
			!document.getElementById("type_V_vedio").checked){
				alert("Shared Content Type is not set yet.");
				return false;
	}
	else{
		dms_dir_type_x_tmp += document.getElementById("type_A_audio").checked? "A" : "";
		dms_dir_type_x_tmp += document.getElementById("type_P_image").checked? "P" : "";
		dms_dir_type_x_tmp += document.getElementById("type_V_vedio").checked? "V" : "";
	}

	//Viz check same rule  //match(path) is not accepted
		if(item_num >=2){
			for(i=0; i<rule_num; i++){
					if(document.getElementById("PATH").value.toLowerCase() == document.getElementById("dlna_path_table").rows[i].cells[0].title.toLowerCase()){
						alert("This entry has been in list.");
						document.getElementById("PATH").focus();
						document.getElementById("PATH").select();
						return false;
					}
			}
		}
	addRow_dir_x(document.getElementById("PATH"));
	addRow_dir_type_x(dms_dir_type_x_tmp);
	document.getElementById("PATH").value = "";
	document.getElementById("type_A_audio").checked = true;
	document.getElementById("type_P_image").checked = true;
	document.getElementById("type_V_vedio").checked = true;

	show_dlna_path();
}

function addRow_dir_x(obj){
	dms_dir_x_array += "<"
	dms_dir_x_array += obj.value;
}

function addRow_dir_type_x(v){
	dms_dir_type_x_array += "<"
	dms_dir_type_x_array += v;
}

function show_dlna_path(){
	var dms_dir_x_array_row = dms_dir_x_array.split('<');
	var dms_dir_type_x_array_row = dms_dir_type_x_array.split('<');
	var code = "";

	code +='<table width="98%" cellspacing="0" cellpadding="4" align="center" class="list_table" id="dlna_path_table">';
	if(dms_dir_x_array_row.length == 1)
		code +='<tr><td style="color:#FFCC00;" colspan="6">Please select media server path.</td></tr>';
	else{
		for(var i = 1; i < dms_dir_x_array_row.length; i++){
			var tmp_type = "";
			code +='<tr id="row'+i+'">';

			code +='<td width="45%" style="word-break:break-all;word-wrap:break-word;" class="dlna_path_td" title="'+dms_dir_x_array_row[i]+'">'+ dms_dir_x_array_row[i] +'</td>';
				tmp_type += dms_dir_type_x_array_row[i].indexOf("A")>=0? "Audio " : "";
				tmp_type += dms_dir_type_x_array_row[i].indexOf("P")>=0? "Image " : "";
				tmp_type += dms_dir_type_x_array_row[i].indexOf("V")>=0? "Video " : "";
			code +='<td width="40%" class="dlna_path_td">'+ tmp_type +'</td>';

			code +='<td width="15%">';
			code +='<input class="remove_btn" onclick="del_Row(this);" value=""/></td></tr>';
		}
	}

	code +='</table>';
	$("dlna_path_Block").innerHTML = code;
}

function dlna_display()
{
	if(dlna_enable == 1){
		document.getElementById("dms_friendly_name").parentNode.parentNode.parentNode.style.display = "";
		document.getElementById("dmsStatus").parentNode.parentNode.style.display = "";
		document.getElementById("dms_dir_manual_control").style.display = "";
		set_dms_dir(dms_dir_manual_value);
	}
	else{
		document.getElementById("dms_friendly_name").parentNode.parentNode.parentNode.style.display = "none";
		document.getElementById("dmsStatus").parentNode.parentNode.style.display = "none";
		document.getElementById("dlna_path_div").style.display = "none";
		document.getElementById("dms_dir_manual_control").style.display = "none";
	}
}

function daapd_display()
{
	if(daapd_enable == 1){
		document.getElementById("daapd_friendly_name").parentNode.parentNode.parentNode.style.display = "";
	}
	else{
		document.getElementById("daapd_friendly_name").parentNode.parentNode.parentNode.style.display = "none";
	}
}

function get_Refresh_time(){
	}
function initial(){
	show_menu();

	daapd_display();
	dlna_display();
	//alert("multi_INT = "+multi_INT);
	//document.getElementById("select_lang_"+multi_INT).selected = true;
	get_Refresh_time();
	AUTOLOGOUT_MAX_MINUTE=AUTOLOGOUT_MAX_MINUTE_TMP;
	var initial_array = new Array();
	eval("initial_array="+data_temp);
	produc = initial_array[15];
	$j("#router_model").html(produc);
	if(location.host.split(":")[0] != initial_array[10])
	{
		if(initial_array[12] == 1 && location.host.split(":")[0]!=LOCAL_DOMAIN){
			document.getElementById("homeAddress").href = "http://" + location.host.split(":")[0] + ":" + initial_array[11] + "/APP_Installation.asp";
		}
		else if(location.host.split(":")[0]==LOCAL_DOMAIN){
			document.getElementById("homeAddress").href = "http://" + location.host.split(":")[0] + "/APP_Installation.asp";
		}
		else if(initial_array[12] == 0){
			document.getElementById("icon_home").title = "Open \"Enable Web Access from WAN\" From Router!";
		}
	}
	else{
			document.getElementById("homeAddress").href = "http://" + location.host.split(":")[0] + "/APP_Installation.asp";
	}

	setInterval("Ajax_Get_MediaServer_Status();",5000);

	//if((calculate_height-3)*52 + 20 > 535)
	//	$("upnp_icon").style.height = (calculate_height-3)*52 + 20 + "px";
	//else
	//	$("upnp_icon").style.height = "500px";
}

// get folder
var dm_dir = new Array();
var Download_path = 'Download2';
var WH_INT=0,Floder_WH_INT=0,General_WH_INT=0;
var BASE_PATH;
var folderlist = new Array();

function cal_panel_block(){
	var blockmarginLeft;
	if (window.innerWidth)
		winWidth = window.innerWidth;
	else if ((document.body) && (document.body.clientWidth))
		winWidth = document.body.clientWidth;

	if (document.documentElement  && document.documentElement.clientHeight && document.documentElement.clientWidth){
		winWidth = document.documentElement.clientWidth;
	}

	if(winWidth >1050){
		winPadding = (winWidth-1050)/2;
		winWidth = 1105;
		blockmarginLeft= (winWidth*0.25)+winPadding;
	}
	else if(winWidth <=1050){
		blockmarginLeft= (winWidth)*0.25+document.body.scrollLeft;
	}

	$("folderTree_panel").style.marginLeft = blockmarginLeft+"px";
}

function get_disk_tree(){
	cal_panel_block();
	$j("#folderTree_panel").fadeIn(300);
	get_layer_items("0");
}

function showClickedObj(clickedObj){
	if(this.lastClickedObj != 0)
		this.lastClickedObj.className = "lastfolderClicked";  //this className set in AiDisk_style.css

	clickedObj.className = "folderClicked";
	this.lastClickedObj = clickedObj;
}

function get_layer(barcode){
	var tmp, layer;

	layer = 0;
	while(barcode.indexOf('_') != -1){
		barcode = barcode.substring(barcode.indexOf('_'), barcode.length);
		++layer;
		barcode = barcode.substring(1);
	}

	return layer;
}

function getDiskBarcode(src_barcode){
	var layer = get_layer(src_barcode);
	var str_len, tmp_str;

	if(layer < 1)
		return "";
	else if(layer == 1)
		return src_barcode;

	str_len = src_barcode.indexOf('_');
	tmp_str = src_barcode.substring(str_len+1);

	str_len += tmp_str.indexOf('_')+1;

	return src_barcode.substring(0, str_len);
}

function getDiskOrder(disk_barcode){
	return parseInt(disk_barcode.substring(disk_barcode.indexOf('_')+1));
}

function setSelectedDiskOrder(selectedId){
	this.selectedDiskBarcode = getDiskBarcode(selectedId.substring(1));
	this.selectedPoolBarcode = "";
	this.selectedFolderBarcode = "";

	this.selectedDiskOrder = getDiskOrder(this.selectedDiskBarcode);
	this.selectedPoolOrder = -1;
	this.selectedFolderOrder = -1;
}

function build_array(obj,layer){
	var path_temp ="/mnt";
	var layer2_path ="";
	var layer3_path ="";
	if(obj.id.length>6){
		if(layer ==3){
			//layer3_path = "/" + $(obj.id).innerHTML;
			layer3_path = "/" + $(obj.id).title;
			while(layer3_path.indexOf("&nbsp;") != -1)
				layer3_path = layer3_path.replace("&nbsp;"," ");
			while(layer3_path.indexOf("&amp;") != -1)
				layer3_path = layer3_path.replace("&amp;","&");

			if(obj.id.length >8)
				layer2_path = "/" + $(obj.id.substring(0,obj.id.length-3)).innerHTML;
			else
				layer2_path = "/" + $(obj.id.substring(0,obj.id.length-2)).innerHTML;

			while(layer2_path.indexOf("&nbsp;") != -1)
				layer2_path = layer2_path.replace("&nbsp;"," ");
			while(layer2_path.indexOf("&amp;") != -1)
				layer2_path = layer2_path.replace("&amp;","&");

			//layer2_path = layer2_path.replace(/\`/g,"spechar3spechar");
		}
	}
	if(obj.id.length>4 && obj.id.length<=6){
		if(layer ==2){
			//layer2_path = "/" + $(obj.id).innerHTML;
			layer2_path = "/" + $(obj.id).title;
			while(layer2_path.indexOf("&nbsp;") != -1)
				layer2_path = layer2_path.replace("&nbsp;"," ");
			while(layer2_path.indexOf("&amp;") != -1)
				layer2_path = layer2_path.replace("&amp;","&");

			//layer2_path = layer2_path.replace(/\`/g,"spechar3spechar");
		}
	}
	path_temp = path_temp + layer2_path +layer3_path;
	return path_temp;
}

function GetTree(layer_order, v){
	if(layer_order == "0"){
		this.FromObject = layer_order;
		$('d'+layer_order).innerHTML = '<span class="FdWait">. . . . . . . . . .</span>';
		setTimeout('get_layer_items("'+layer_order+'", "gettree")', 1);
		return;
	}

	if($('a'+layer_order).className == "FdRead"){
		$('a'+layer_order).className = "FdOpen";
		$('a'+layer_order).src = "images/Tree/vert_line_s"+v+"1.gif";
		this.FromObject = layer_order;
		$('e'+layer_order).innerHTML = '<img src="images/Tree/folder_wait.gif">';
		setTimeout('get_layer_items("'+layer_order+'", "gettree")', 1);
	}
	else if($('a'+layer_order).className == "FdOpen"){
		$('a'+layer_order).className = "FdClose";
		$('a'+layer_order).src = "images/Tree/vert_line_s"+v+"0.gif";
		$('e'+layer_order).style.position = "absolute";
		$('e'+layer_order).style.visibility = "hidden";
	}
	else if($('a'+layer_order).className == "FdClose"){
		$('a'+layer_order).className = "FdOpen";
		$('a'+layer_order).src = "images/Tree/vert_line_s"+v+"1.gif";
		$('e'+layer_order).style.position = "";
		$('e'+layer_order).style.visibility = "";
	}
	else
		alert("Error when show the folder-tree!");
}

function getPoolBarcode(src_barcode){
	var layer = get_layer(src_barcode);
	var str_len, tmp_str;

	if(layer < 2)
		return "";
	else if(layer == 2)
		return src_barcode;

	str_len = getDiskBarcode(src_barcode).length;
	tmp_str = src_barcode.substring(str_len+1);

	str_len += tmp_str.indexOf('_')+1;

	return src_barcode.substring(0, str_len);
}

function get_sharedfolder_in_pool(poolname){
	var url = "media_print.cgi" + "?action_mode="+ "getsharefolder=" + poolname + "&t=" +Math.random();//eric
	var lists = new Array();
	$j.ajax({url: url,
		async: false,
		success: function(data){eval("lists="+data);},
		error: function(XMLHttpRequest, textStatus, errorThrown){
		  if(XMLHttpRequest.status==598){
				if(XMLHttpRequest.responseText!=null){
					self.location = "http://"+ location.host + XMLHttpRequest.responseText + "/mediaserverui/mediaserver.asp";
				}
				else{
					self.location = "http://"+ location.host +"/Main_Login.asp";}
				}}
		});
	return lists;
}

function creatfolder(path){
	var url = "media_print.cgi" + "?action_mode="+ "createfolder=" + path + "&t=" +Math.random();
	var get_data;
	$j.ajax({url: url,
		async: false,
		success: function(data){get_data = data;},
		error: function(XMLHttpRequest, textStatus, errorThrown){
		  if(XMLHttpRequest.status==598){
				if(XMLHttpRequest.responseText!=null){
					self.location = "http://"+ location.host + XMLHttpRequest.responseText + "/mediaserverui/mediaserver.asp";
				}
				else{
					self.location = "http://"+ location.host +"/Main_Login.asp";}
				}}
		});
	return get_data;
}

function deletefolder(path){
	//alert("path="+path);
	path = path.replace(/\&/g,"spechar7spechar");
	path = path.replace(/\#/g,"spechar3spechar");
	path = path.replace(/\+/g,"spechar12spechar");
	path = path.replace(/\;/g,"spechar11spechar");
	//alert("encode="+encodeURI(path));
	var url = "media_print.cgi" + "?action_mode="+ "deletefolder=" + encodeURI(path) + "&t=" +Math.random();
	var get_data;
	$j.ajax({url: url,
		async: false,
		success: function(data){get_data = data;},
		error: function(XMLHttpRequest, textStatus, errorThrown){
		  if(XMLHttpRequest.status==598){
				if(XMLHttpRequest.responseText!=null){
					self.location = "http://"+ location.host + XMLHttpRequest.responseText +"/mediaserverui/mediaserver.asp";
				}
				else{
					self.location = "http://"+ location.host +"/Main_Login.asp";}
				}}
		});
	return get_data;
}

function modifyfolder(path){
	path = path.replace(/\&/g,"spechar7spechar");
	path = path.replace(/\#/g,"spechar3spechar");
	path = path.replace(/\+/g,"spechar12spechar");
	path = path.replace(/\;/g,"spechar11spechar");
	var url = "media_print.cgi" + "?action_mode="+ "modifyfolder=" + encodeURI(path) + "&t=" +Math.random();
	var get_data;
	$j.ajax({url: url,
		async: false,
		success: function(data){get_data = data;},
		error: function(XMLHttpRequest, textStatus, errorThrown){
		  if(XMLHttpRequest.status==598){
				if(XMLHttpRequest.responseText!=null){
					self.location = "http://"+ location.host + XMLHttpRequest.responseText +"/mediaserverui/mediaserver.asp";
				}
				else{
					self.location = "http://"+ location.host +"/Main_Login.asp";}
				}}
		});
	return get_data;
}

function getPoolOrder(pool_barcode){
	return parseInt(pool_barcode.substring(getDiskBarcode(pool_barcode).length+1));
}

function setSelectedPoolOrder(selectedId){
	this.selectedDiskBarcode = getDiskBarcode(selectedId.substring(1));
	this.selectedPoolBarcode = getPoolBarcode(selectedId.substring(1));
	this.selectedFolderBarcode = "";

	this.selectedDiskOrder = getDiskOrder(this.selectedDiskBarcode);
	this.selectedPoolOrder = this.selectedDiskOrder+getPoolOrder(this.selectedPoolBarcode);
	this.selectedFolderOrder = -1;
}
function getFolderBarcode(src_barcode){
	var layer = get_layer(src_barcode);
	var str_len, tmp_str;

	if(layer < 3)
		return "";
	else if(layer == 3)
		return src_barcode;

	str_len = getPoolBarcode(src_barcode).length;
	tmp_str = src_barcode.substring(str_len+1);

	str_len += tmp_str.indexOf('_')+1;

	return src_barcode.substring(0, str_len);
}

function getFolderOrder(folder_barcode){
	return parseInt(folder_barcode.substring(getPoolBarcode(folder_barcode).length+1));
}
function setSelectedFolderOrder(selectedId){
	this.selectedDiskBarcode = getDiskBarcode(selectedId.substring(1));
	this.selectedPoolBarcode = getPoolBarcode(selectedId.substring(1));
	this.selectedFolderBarcode = getFolderBarcode(selectedId.substring(1));

	this.selectedDiskOrder = getDiskOrder(this.selectedDiskBarcode);
	this.selectedPoolOrder = this.selectedDiskOrder+getPoolOrder(this.selectedPoolBarcode);
	this.selectedFolderOrder = 1+getFolderOrder(this.selectedFolderBarcode);
}

function disableCheckChangedStatus(){
	stopFlag = 1;
}

function popupWindow(w,u){

	disableCheckChangedStatus();

	winW_H();

	$(w).style.width = winW+"px";
	$(w).style.height = winH+"px";
	$(w).style.visibility = "visible";

	$('popupframe').src = u;
}

function getSelectedPoolOrder(){
	return this.selectedPoolOrder;
}

function GetFolderItem(selectedObj, haveSubTree){
	var barcode, layer = 0;
	showClickedObj(selectedObj);
	barcode = selectedObj.id.substring(1);
	layer = get_layer(barcode);

	if(layer == 0)
		alert("Machine: Wrong");
	else if(layer == 1){
		// chose Disk
		setSelectedDiskOrder(selectedObj.id);
		path_directory = build_array(selectedObj,layer);
		$('createFolderBtn').className = "createFolderBtn";
		$('deleteFolderBtn').className = "deleteFolderBtn";
		$('modifyFolderBtn').className = "modifyFolderBtn";

		$('createFolderBtn').onclick = function(){};
		$('deleteFolderBtn').onclick = function(){};
		$('modifyFolderBtn').onclick = function(){};
	}
	else if(layer == 2){
		// chose Partition
		setSelectedPoolOrder(selectedObj.id);
		path_directory = build_array(selectedObj,layer);
		$('createFolderBtn').className = "createFolderBtn_add";
		$('deleteFolderBtn').className = "deleteFolderBtn";
		$('modifyFolderBtn').className = "modifyFolderBtn";

		$('createFolderBtn').onclick = function(){popupWindow('OverlayMask','popCreateFolder.asp');};
		$('deleteFolderBtn').onclick = function(){};
		$('modifyFolderBtn').onclick = function(){};
		document.aidiskForm.layer_order.disabled = "disabled";
		document.aidiskForm.layer_order.value = barcode;
	}
	else if(layer == 3){
		// chose Shared-Folder
		setSelectedFolderOrder(selectedObj.id);
		path_directory = build_array(selectedObj,layer);
		$('createFolderBtn').className = "createFolderBtn";
		$('deleteFolderBtn').className = "deleteFolderBtn_add";
		$('modifyFolderBtn').className = "modifyFolderBtn_add";

		$('createFolderBtn').onclick = function(){};
		$('deleteFolderBtn').onclick = function(){popupWindow('OverlayMask','popDeleteFolder.asp');};
		$('modifyFolderBtn').onclick = function(){popupWindow('OverlayMask','popModifyFolder.asp');};
		document.aidiskForm.layer_order.disabled = "disabled";
		document.aidiskForm.layer_order.value = barcode;
	}

	if(haveSubTree)
		GetTree(barcode, 1);
}
function getSelectedFolderOrder(){
	return this.selectedFolderOrder;
}

function BuildTree(){
	var ItemText, ItemSub, ItemIcon;
	var vertline, isSubTree;
	var layer;
	var short_ItemText = "";
	var shown_ItemText = "";
	var ItemBarCode ="";
	var TempObject = "";
	for(var i = 0; i < this.Items.length; ++i){
		this.Items[i] = this.Items[i].split("#");
		var Item_size = 0;
		Item_size = this.Items[i].length;
		if(Item_size > 3){
			var temp_array = new Array(3);

			temp_array[2] = this.Items[i][Item_size-1];
			temp_array[1] = this.Items[i][Item_size-2];
			temp_array[0] = "";
			for(var j = 0; j < Item_size-2; ++j){
				if(j != 0)
					temp_array[0] += "#";
				temp_array[0] += this.Items[i][j];
			}
			this.Items[i] = temp_array;
		}
		ItemText = (this.Items[i][0]).replace(/^[\s]+/gi,"").replace(/[\s]+$/gi,"");
		ItemBarCode = this.FromObject+"_"+(this.Items[i][1]).replace(/^[\s]+/gi,"").replace(/[\s]+$/gi,"");
		ItemSub = parseInt((this.Items[i][2]).replace(/^[\s]+/gi,"").replace(/[\s]+$/gi,""));
		layer = get_layer(ItemBarCode.substring(1));
		if(layer == 3){
			if(ItemText.length > 21)
		 		short_ItemText = ItemText.substring(0,30)+"...";
		 	else
		 		short_ItemText = ItemText;
		}
		else
			short_ItemText = ItemText;

		shown_ItemText = showhtmlspace(short_ItemText);

		if(layer == 1)
			ItemIcon = 'disk';
		else if(layer == 2)
			ItemIcon = 'part';
		else
			ItemIcon = 'folders';

		SubClick = ' onclick="GetFolderItem(this, ';
		if(ItemSub <= 0){
			SubClick += '0);"';
			isSubTree = 'n';
		}
		else{
			SubClick += '1);"';
			isSubTree = 's';
		}

		if(i == this.Items.length-1){
			vertline = '';
			isSubTree += '1';
		}
		else{
			vertline = ' background="images/Tree/vert_line.gif"';
			isSubTree += '0';
		}

		if(layer == 2 && isSubTree == 'n1'){	// Uee to rebuild folder tree if disk without folder, Jieming add at 2012/08/29
			//document.aidiskForm.test_flag.value = 1;
		}
		TempObject +='<table class="tree_table" id="bug_test">';
		TempObject +='<tr>';
		// the line in the front.
		TempObject +='<td class="vert_line">';
		TempObject +='<img id="a'+ItemBarCode+'" onclick=\'$("d'+ItemBarCode+'").onclick();\' class="FdRead" src="images/Tree/vert_line_'+isSubTree+'0.gif">';
		TempObject +='</td>';

		if(layer == 3){
			TempObject +='<td>';
			TempObject +='<img id="c'+ItemBarCode+'" onclick=\'$("d'+ItemBarCode+'").onclick();\' src="images/New_ui/advancesetting/'+ItemIcon+'.png">';
			TempObject +='</td>';
			TempObject +='<td>';
			TempObject +='<span id="d'+ItemBarCode+'"'+SubClick+' title="'+ItemText+'">'+shown_ItemText+'</span>\n';
			TempObject +='</td>';
		}
		else if(layer == 2){
			TempObject +='<td>';
			TempObject +='<table class="tree_table">';
			TempObject +='<tr>';
			TempObject +='<td class="vert_line">';
			TempObject +='<img id="c'+ItemBarCode+'" onclick=\'$("d'+ItemBarCode+'").onclick();\' src="images/New_ui/advancesetting/'+ItemIcon+'.png">';
			TempObject +='</td>';
			TempObject +='<td class="FdText">';
			TempObject +='<span id="d'+ItemBarCode+'"'+SubClick+' title="'+ItemText+'">'+shown_ItemText+'</span>';
			TempObject +='</td>';
			TempObject +='<td></td>';
			TempObject +='</tr>';
			TempObject +='</table>';
			TempObject +='</td>';
			TempObject +='</tr>';
			TempObject +='<tr><td></td>';
			TempObject +='<td colspan=2><div id="e'+ItemBarCode+'" ></div></td>';
		}
		else{
			TempObject +='<td>';
			TempObject +='<table><tr><td>';
			TempObject +='<img id="c'+ItemBarCode+'" onclick=\'$("d'+ItemBarCode+'").onclick();\' src="images/New_ui/advancesetting/'+ItemIcon+'.png">';
			TempObject +='</td><td>';
			TempObject +='<span id="d'+ItemBarCode+'"'+SubClick+' title="'+ItemText+'">'+shown_ItemText+'</span>';
			TempObject +='</td></tr></table>';
			TempObject +='</td>';
			TempObject +='</tr>';
			TempObject +='<tr><td></td>';
			TempObject +='<td><div id="e'+ItemBarCode+'" ></div></td>';
		}

		TempObject +='</tr>';
	}
	TempObject +='</table>';
	$("e"+this.FromObject).innerHTML = TempObject;
}

function get_tree_items(treeitems){
	this.isLoading = 1;
	var array_temp = new Array();
	var array_temp_split = new Array();
	for(var j=0;j<treeitems.length;j++){
		//treeitems[j] : "Download2#22#0"
		array_temp_split[j] = treeitems[j].split("#");
		// Mipsel:asusware  Mipsbig:asusware.big  Armel:asusware.arm  // To hide folder 'asusware'
		if( array_temp_split[j][0].match(/^asusware$/)	|| array_temp_split[j][0].match(/^asusware.big$/) || array_temp_split[j][0].match(/^asusware.arm$/) ){
			continue;
		}

		//Specific folder 'Download2/Complete'
		if( array_temp_split[j][0].match(/^Download2$/) ){
			treeitems[j] = "Download2/Complete"+"#"+array_temp_split[j][1]+"#"+array_temp_split[j][2];
		}
		array_temp.push(treeitems[j]);
	}
	this.Items = array_temp;
	if(this.Items && this.Items.length >= 0){
		BuildTree();
	}
}


function get_layer(barcode){
	var tmp, layer;
	layer = 0;
	while(barcode.indexOf('_') != -1){
		barcode = barcode.substring(barcode.indexOf('_'), barcode.length);
		++layer;
		barcode = barcode.substring(1);
	}
	return layer;
}

function get_layer_items(layer_order){
	var url = "media_print.cgi" + "?action_mode="+ "layer_order=" + layer_order + "&t=" +Math.random();//eric
	$j.get(url,function(data){
				var treeitems = new Array();
				eval("treeitems="+data);
				get_tree_items(treeitems);});//eirc 11.9
}
function get_layer_items_test(layer_order_t){
	var url = "media_print.cgi" + "?action_mode="+ "layer_order=" + layer_order_t + "&t=" +Math.random();
	var flag;
	$j.ajax({url: url,
		async: false,
		success: function(data){var treeitems = new Array();eval("treeitems="+data);flag=treeitems.length;},
		error: function(XMLHttpRequest, textStatus, errorThrown){
		  if(XMLHttpRequest.status==598){
				if(XMLHttpRequest.responseText!=null){
					self.location = "http://"+ location.host + XMLHttpRequest.responseText + "/mediaserverui/mediaserver.asp";
				}
				else{
					self.location = "http://"+ location.host +"/Main_Login.asp";}
				}}
		});
	return flag;
}

function showPanel(){
	WH_INT = setInterval("getWH();",1000);
 	$j("#DM_mask").fadeIn(1000);
  $j("#panel_add").show(1000);
	create_tree();
}

function getWH(){
	var winWidth;
	var winHeight;
	winWidth = document.documentElement.scrollWidth;
	if(document.documentElement.clientHeight > document.documentElement.scrollHeight)
		winHeight = document.documentElement.clientHeight;
	else
		winHeight = document.documentElement.scrollHeight;
	$("DM_mask").style.width = winWidth+"px";
	$("DM_mask").style.height = winHeight+"px";
}

function getFloderWH(){
	var winWidth;
	var winHeight;
	winWidth = document.documentElement.scrollWidth;

	if(document.documentElement.clientHeight > document.documentElement.scrollHeight)
		winHeight = document.documentElement.clientHeight;
	else
		winHeight = document.documentElement.scrollHeight;

	$("DM_mask_floder").style.width = winWidth+"px";
	$("DM_mask_floder").style.height = winHeight+"px";
}

function show_AddFloder(){
	Floder_WH_INT = setInterval("getFloderWH();",1000);
	$j("#DM_mask_floder").fadeIn(1000);
	$j("#panel_addFloder").show(1000);
}

function hidePanel(){
	($j("#tree").children()).remove();
	clearInterval(WH_INT);
	$j("#DM_mask").fadeOut('fast');
	$j("#panel_add").hide('fast');
}

function create_tree(){
	var rootNode = new Ext.tree.TreeNode({ text:'/mnt', id:'0'});
	var rootNodechild = new Ext.tree.TreeNode({ text:'', id:'0t'});
	rootNode.appendChild(rootNodechild);
	var tree = new Ext.tree.TreePanel({
			tbar:[{text:"Ok",handler:function(){$j("#PATH").attr("value","/tmp"+Download_path);hidePanel();}},
				'->',{text:'X',handler:function(){hidePanel();}}
			],
			title:"Please select the desire folder",
				applyTo:'tree',
				root:rootNode,
				height:400,
				autoScroll:true
	});
	tree.on('expandnode',function(node){
		var allParentNodes = getAllParentNodes(node);
		var path='';

		for(var j=0; j<allParentNodes.length; j++){
			path = allParentNodes[j].text + '/' +path;
		}

		initial_dir(path,node);
	});
	tree.on('collapsenode',function(node){
		while(node.firstChild){
			node.removeChild(node.firstChild);
		}
		var childNode = new Ext.tree.TreeNode({ text:'', id:'0t'});
		node.appendChild(childNode);
	});
	tree.on('click',function(node){
		var allParentNodes = getAllParentNodes(node);
		var path='';
		for(var j=0; j<allParentNodes.length; j++){
		path = allParentNodes[j].text + '/' +path;
		}
		Download_path = path;
		path = BASE_PATH + '/' + path;
		var url = "ms_disk_info.cgi";
		var type = "General";
		url += "?action_mode=" +path+ "&t=" +Math.random();
		$j.get(url,function(data){initial_folderlist(data);});
	});
}

function initial_folderlist(data){
	eval("folderlist=["+data+"]");
}

function getAllParentNodes(node) {
	var parentNodes = [];
	var _nodeID = node.id;
	_layer_order = "0"

	for(i=1; i<_nodeID.length; i++)
		_layer_order += "_" + node.id[i]; // generating _layer_order for initial_dir()

	parentNodes.push(node);
	while (node.parentNode) {
		parentNodes = parentNodes.concat(node.parentNode);
		node = node.parentNode;
	}
	return parentNodes;
};

function initial_dir_status(data,node){
	dm_dir.length = 0;
	if(data == "/" || (data != null && data != "")){
		eval("dm_dir=[" + data +"]");
		while(node.lastChild &&(node.lastChild !=node.firstChild)) {
    			node.removeChild(node.lastChild);
		}
		for(var i=0; i<dm_dir.length; i++){
			var childNodeId = node.id +i;
			var childnode = new Ext.tree.TreeNode({id:childNodeId,text:dm_dir[i]});
			node.appendChild(childnode);
			var childnodeT = new Ext.tree.TreeNode({id:childNodeId+'t',text:''});
			childnode.appendChild(childnodeT);
		}
		node.removeChild(node.firstChild);
	}
	else{
		while(node.firstChild){
			node.removeChild(node.firstChild);
		}
	}
}

function initial_dir(path,node){
	var url = "ms_disk_info.cgi";
	var type = "General";
	url += "?action_mode=" +path+ "&t=" +Math.random();
	$j.get(url,function(data){initial_dir_status(data,node);});
	}
function validate_hostname(obj)
{
	var re = new RegExp("^[a-zA-Z0-9][a-zA-Z0-9\-\_]+$","gi");
	if(re.test(obj.value)){
		return "";
	}
	else
		return "The host name only accept alphanumeric characters, under line and dash symbol. The first character cannot be dash [-] or under line [_].";
}
function applyRule(v,s){
	if(daapd_enable == 1)
	{
		if(document.getElementById("daapd_friendly_name").value.length == 0)
		{
			showtext($("alert_msg1"),multiLanguage_array[multi_INT][16]);
			document.getElementById("daapd_friendly_name").focus();
			document.getElementById("daapd_friendly_name").select();
			return;
		}
		else
		{
			var alert_str1 = validate_hostname(document.getElementById("daapd_friendly_name"));
			if(alert_str1 != "")
			{
				showtext($("alert_msg1"),alert_str1);
				$("alert_msg1").style.display = "";
				document.getElementById("daapd_friendly_name").focus();
				document.getElementById("daapd_friendly_name").select();
				return;
			}
			else
				$("alert_msg1").style.display = "none";

			document.getElementById("daapd_friendly_name").value = trim(document.getElementById("daapd_friendly_name").value);
		}
	}
	if(dlna_enable == 1)
	{
		if(document.getElementById("dms_friendly_name").value.length == 0)
		{
			showtext($("alert_msg2"),multiLanguage_array[multi_INT][16]);
			document.getElementById("dms_friendly_name").focus();
			document.getElementById("dms_friendly_name").select();
			return;
		}
		else
		{
			var alert_str2 = validate_hostname(document.getElementById("dms_friendly_name"));
			if(alert_str2 != "")
			{
				showtext($("alert_msg2"),alert_str2);
				$("alert_msg2").style.display = "";
				document.getElementById("dms_friendly_name").focus();
				document.getElementById("dms_friendly_name").select();
				return;
			}
			else
				$("alert_msg2").style.display = "none";

			document.getElementById("dms_friendly_name").value = trim(document.getElementById("dms_friendly_name").value);
		}
	}

	var dms_dir_tmp_value = "";
	var dms_dir_type_tmp_value = "";

	if(dlna_enable == 1 && dms_dir_manual_value == 1)
	{
		var rule_num = document.getElementById("dlna_path_table").rows.length;
		var item_num = document.getElementById("dlna_path_table").rows[0].cells.length;

		//var dms_dir_manual_value = document.mediaserverForm.dms_dir_manual.value;
		if(item_num >1){
			for(i=0; i<rule_num; i++){
				dms_dir_tmp_value += "<";
				dms_dir_tmp_value += document.getElementById("dlna_path_table").rows[i].cells[0].title;
				var type_translate_tmp = "";
				dms_dir_type_tmp_value += "<";
				type_translate_tmp += document.getElementById("dlna_path_table").rows[i].cells[1].innerHTML.indexOf("Audio")>=0? "A":"";
				type_translate_tmp += document.getElementById("dlna_path_table").rows[i].cells[1].innerHTML.indexOf("Image")>=0? "P":"";
				type_translate_tmp += document.getElementById("dlna_path_table").rows[i].cells[1].innerHTML.indexOf("Video")>=0? "V":"";
				dms_dir_type_tmp_value += type_translate_tmp;
			}
			dms_dir_x_array = dms_dir_tmp_value;
			dms_dir_type_x_array = dms_dir_type_tmp_value;
		}
		else
		{
			dms_dir_x_array = "";
			dms_dir_type_x_array = "";
		}
	}
	else
	{
		dms_dir_x_array = "</tmp/mnt";
		dms_dir_type_x_array = "<APV";
	}


	/*if(dms_dir_x_array.length == 0)
	{
		alert("Have no available path for Media Server");
		return;
	}*/

	var dlna_enable_f = document.mediaserverForm.dms_enable.value;
	var dms_dir_manual_value_f = document.mediaserverForm.dms_dir_manual.value;
	var daapd_enable_f = document.mediaserverForm.daapd_enable.value;
	var dms_dir_x_array_f = document.mediaserverForm.mediaserver_path.value;
	var dms_dir_type_x_array_f = document.mediaserverForm.path_type.value;
	var dms_friendly_name_f = document.mediaserverForm.friendly_name.value;
	var daapd_friendly_name_f = document.mediaserverForm.itunes_name.value;
	var daapd_friendly_name_value = document.getElementById("daapd_friendly_name").value;
	var dms_friendly_name_value = document.getElementById("dms_friendly_name").value;

	if(dlna_enable_f == dlna_enable && dms_dir_manual_value == dms_dir_manual_value_f && daapd_enable == daapd_enable_f
		&& dms_dir_x_array == dms_dir_x_array_f && dms_dir_type_x_array == dms_dir_type_x_array_f &&
		dms_friendly_name_value == dms_friendly_name_f && daapd_friendly_name_value == daapd_friendly_name_f)
	{
		showLoading(1);
		return;
	}

		showLoading();
	dms_dir_x_array = dms_dir_x_array.replace(/\`/g,"spechar3spechar");

	var url = "media_apply.cgi";
		url += "?action_mode=" + v;
		if(v=="DLNA_SETTING"){
			url += "&dms_enable=" + dlna_enable + "&dms_dir_manual=" + dms_dir_manual_value + "&daapd_enable=" + daapd_enable + "&mediasever_path=" + encodeURIComponent(dms_dir_x_array) + "&path_type=" + dms_dir_type_x_array + "&friendly_name=" + encodeURIComponent(dms_friendly_name_value) + "&itunes_name=" + encodeURIComponent(daapd_friendly_name_value) +"&t="+Math.random();}
		//alert(url);
		$j.get(url,function(data){location.reload();});

	/*$j.ajax({url: url,
		async: true,
		success: function(data){location.reload();}
		});*/
}
function cancel_folderTree(){
	this.FromObject ="0";
	$j("#folderTree_panel").fadeOut(300);
}
function confirm_folderTree(){
	$('PATH').value = path_directory ;
	this.FromObject ="0";
	$j("#folderTree_panel").fadeOut(300);
}
function hidePop(flag){
	if(flag != "apply")
		enableCheckChangedStatus();

	$('popupframe').src = "";
	$('OverlayMask').style.visibility = "hidden";
}

function set_dms_dir(val){
	if(val == 1){	//manual path
		document.getElementById("dlna_path_div").style.display = "";
		dms_dir_manual_value = 1;
		show_dlna_path();
	}
	else{		//default path
		document.getElementById("dlna_path_div").style.display = "none";
		dms_dir_manual_value = 0;
	}
}

</script>
</head>

<body onload="initial();" onunload="unload_body();">
<div id="TopBanner"></div>
<div id="DM_mask" class="mask_bg"></div>
<div id="folderTree_panel" class="panel_folder" >

	<table><tr><td>
			<div class="machineName" style="width:200px;font-family:Microsoft JhengHei;font-size:12pt;font-weight:bolder; margin-top:15px;margin-left:30px;"><span id="router_model"></span></div>
		</td>
		<td>
			<div style="width:240px;margin-top:14px;margin-left:135px;">
				<table >
					<tr>
						<td><div id="createFolderBtn" class="createFolderBtn" title=""></div></td>
						<td><div id="deleteFolderBtn" class="deleteFolderBtn" title=""></div></td>
						<td><div id="modifyFolderBtn" class="modifyFolderBtn" title=""></div></td>
					</tr>
				</table>
			</div>
		</td></tr></table>
		<div id="e0" class="folder_tree"></div>
		<div style="background-image:url(images/Tree/bg_02.png);background-repeat:no-repeat;height:90px;">
		<input id="multi_17" class="button_gen" type="button" style="margin-left:27%;margin-top:18px;" onclick="cancel_folderTree();" value="">
		<input id="multi_1_1" class="button_gen" type="button"  onclick="confirm_folderTree();" value="">
	</div>
</div>

<div id="DM_mask_floder" class="mask_floder_bg"></div>
<!-- floder tree-->

<div id="Loading" class="popup_bg"></div>
<iframe name="hidden_frame" id="hidden_frame" width="0" height="0" frameborder="0" scrolling="no"></iframe>
<form method="post" name="mediaserverForm" action="media_apply.cgi" target="hidden_frame">
<input type="hidden" name="dms_enable" value="">
<input type="hidden" name="daapd_enable" value="">
<input type="hidden" name="mediaserver_path" value="">
<input type="hidden" name="path_type" value="">
<input type="hidden" name="friendly_name" value="">
<input type="hidden" name="itunes_name" value="">
<input type="hidden" name="dms_dir_manual" value="">
</form>

<form method="post" name="aidiskForm" action="" target="hidden_frame">
<input type="hidden" name="motion" id="motion" value="">
<input type="hidden" name="layer_order" id="layer_order" value="">

<table class="content" align="center" cellspacing="0">
  <tr>
	<td width="17">&nbsp;</td>

	<td valign="top" width="101">
	</td>

  <td valign="top">
<div class="upnp_table" style="margin-top:-150px; padding-left:0px;">
<table>
  <tr>
  	<td>
				<div style="width:730px">
					<table width="730px">
						<tr>
							<td align="left">
								<span class="formfonttitle"><b id="multi_0"></b></span>
							</td>
							<td align="right">
                        	<div class="icon" id="icon_home" title="Home" style="margin-right:-35px;"><a id="homeAddress"></a></div>
                        	</td>
						</tr>
					</table>
				</div>
				<div style="margin:5px;"><img src="images/New_ui/export/line_export.png"></div>

			<div class="formfontdesc" id="multi_4">Set up the iTunes and DLNA server.</div>
		</td>
  </tr>
  <tr>
	<td>
		<div>
		<table id="iTunes" width="98%" border="1" align="center" cellpadding="4" cellspacing="1" bordercolor="#6b8fa3" class="FormTable">
			<thead>
				<tr><td colspan="2">iTunes Server</td></tr>
			</thead>
			<tr>
			<th id="multi_2">Enable iTunes Server?</th>
			<td>
			<div class="left" style="width:94px; position:relative; left:3%;" id="radio_daapd_enable"></div>
			</td>
			</tr>
			<tr style="display:none">
				<th>iTunes Server Name</th>
				<td>
					<div><input id="daapd_friendly_name" name="daapd_friendly_name" type="text" style="margin-left:15px;" class="input_15_table" value=""><br/><div id="alert_msg1" style="color:#FC0;margin-left:10px;"></div></div>
				</td>
			</tr>
		</table>
   		</div>
		<div style="margin-top:10px;">
   		<table id="dlna" width="98%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
			<thead>
					<tr><td colspan="2" id="multi_0_1">Media Server</td></tr>
			</thead>
			<tr>
				<th id="multi_5">Enable DLNA Media Server</th>
			<td>
        			<div class="left" style="width:94px; position:relative; left:3%;" id="radio_dms_enable"></div>
			</td>
			</tr>
			<tr style="display:none">
				<th id="multi_6">Media Server Name</th>
					<td>
						<div><input id="dms_friendly_name" name="dms_friendly_name" type="text" style="margin-left:15px;" class="input_15_table" value=""><br/><div id="alert_msg2" style="color:#FC0;margin-left:10px;"></div></div>
					</td>
      			</tr>
			<tr style="display:none">
				<th id="multi_7">Media Server Status</th>
				<td><span id="dmsStatus" style="margin-left:15px">Idle</span>
				</td>
		    </tr>
            <tr id="dms_dir_manual_control" style="display:none">
				<th id="multi_8">Media Server Path Setting</th>
				<td>
                	<input type="radio" value="0" name="dms_dir_manual_x" class="input" onclick="set_dms_dir(0);"><span id="multi_9"></span>
                    <input type="radio" value="1" name="dms_dir_manual_x" class="input" onclick="set_dms_dir(1);"><span id="multi_10"></span>
				</td>
		    </tr>
		</table>
		</div>
		<div id="dlna_path_div" style="display:none">
		<table width="98%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable_table" style="margin-top:8px;">
			<thead>
				<tr>
				<td colspan="3" id="GWStatic"><span id="multi_10_0"></span>&nbsp;(<span id="multi_11"></span>&nbsp;10)</td>
				</tr>
			</thead>
			<tr>
		  	<th id="multi_12"></th>
        		<th id="multi_13"></th>
        		<th id="multi_14"></th>
			</tr>
			<tr>
		    	<td width="45%">
		    		<input id="PATH" type="text" class="input_30_table" title="" value="" onclick="get_disk_tree();" readonly="readonly"/" placeholder="Please select" >
			</td>
			<td width="40%">
            		<input type="checkbox" class="input" id="type_A_audio" name="type_A_audio" checked>&nbsp;Audio&nbsp;&nbsp;
								<input type="checkbox" class="input" id="type_P_image" name="type_P_image" checked>&nbsp;Image&nbsp;&nbsp;
								<input type="checkbox" class="input" id="type_V_vedio" name="type_V_vedio" checked>&nbsp;Video
            		</td>
			<td width="15%">
			<input type="button" class="add_btn" onClick="addRow_Group(10);" value="">
            		</td>
			</tr>
		</table>
			<div id="dlna_path_Block"></div>
		</div>
		<div class="apply_gen">
           		<input id="multi_1" type="button" class="button_gen" onclick="applyRule('DLNA_SETTING','')" value="Apply"/>
       		</div>
	</td>
  </tr>
</table>

<!--=====End of Main Content=====-->
		</td>

		<td width="20" align="center" valign="top"></td>
	</tr>
</table>
</div>
</form>

<div id="footer"></div>
<br>
<div id="OverlayMask" class="popup_bg">
	<div align="center">
	<iframe src="" frameborder="0" scrolling="no" id="popupframe" width="400" height="400" allowtransparency="true" style="margin-top:150px;"></iframe>
	</div>
<!--[if lte IE 6.5]><iframe class="hackiframe"></iframe><![endif]-->
</div>
</body>
<script>
var daapd_enable = "";
var dlna_enable = "";
//var friendly_name = "";
//var itunes_name = "";
var dms_dir_x_array = "";
var dms_dir_type_x_array = "";
var dms_dir_manual_value = "";
function  getsetting(data)
{
	var settings = new Array();
	eval("settings="+data);

	daapd_enable = settings[1];
	dlna_enable = settings[0];
	dms_dir_x_array = settings[2];
	dms_dir_type_x_array = settings[3];
	if(settings[6] == null || settings[6] == "")
	{
		document.mediaserverForm.dms_dir_manual.value = 0;
		dms_dir_manual_value = 0;
	}
	else
	{
		document.mediaserverForm.dms_dir_manual.value = settings[6];
		dms_dir_manual_value = settings[6];
	}
	document.mediaserverForm.dms_enable.value = settings[0];
	document.mediaserverForm.daapd_enable.value = settings[1];
	document.mediaserverForm.mediaserver_path.value = settings[2];
	document.mediaserverForm.path_type.value = settings[3];
	document.mediaserverForm.friendly_name.value = settings[4];
	document.mediaserverForm.itunes_name.value = settings[5];
	if(dms_dir_manual_value == 0)
		document.aidiskForm.dms_dir_manual_x[0].checked = "checked";
	else
		document.aidiskForm.dms_dir_manual_x[1].checked = "checked";
	document.getElementById("dms_friendly_name").value = settings[4];
	document.getElementById("daapd_friendly_name").value = settings[5];
	//$j("#PATH").attr("value",settings[2]);
	$j('#radio_dms_enable').iphoneSwitch(settings[0],
										 function() {
											dlna_enable = 1;
											document.getElementById("dms_friendly_name").parentNode.parentNode.parentNode.style.display = "";
											document.getElementById("dmsStatus").parentNode.parentNode.style.display = "";
                                            document.getElementById("dms_dir_manual_control").style.display = "";
											set_dms_dir(dms_dir_manual_value);
										 },
										 function() {
											dlna_enable = 0;
											document.getElementById("dms_friendly_name").parentNode.parentNode.parentNode.style.display = "none";
											document.getElementById("dmsStatus").parentNode.parentNode.style.display = "none";
											document.getElementById("dlna_path_div").style.display = "none";
                                            document.getElementById("dms_dir_manual_control").style.display = "none";
										 },
										 {
											switch_on_container_path: 'plugin/iphone_switch_container_off.png'
										 }
									);
	$j('#radio_daapd_enable').iphoneSwitch(settings[1],
										 function() {
											daapd_enable = 1;
											document.getElementById("daapd_friendly_name").parentNode.parentNode.parentNode.style.display = "";
										 },
										 function() {
											daapd_enable = 0;
											document.getElementById("daapd_friendly_name").parentNode.parentNode.parentNode.style.display = "none";
										 },
										 {
											switch_on_container_path: 'plugin/iphone_switch_container_off.png'
										 }
									);
}
var url1 = "media_print.cgi";
url1 += "?action_mode=" + "MEDIASERVER_GETCONFIG" + "&t=" +Math.random();
$j.ajax({url: url1,
		async: false,
		success: function(data){getsetting(data);},
		error: function(XMLHttpRequest, textStatus, errorThrown){
		  if(XMLHttpRequest.status==598){
				if(XMLHttpRequest.responseText!=null){
					self.location = "http://"+ location.host + XMLHttpRequest.responseText + "/mediaserverui/mediaserver.asp";
				}
				else{
					self.location = "http://"+ location.host +"/Main_Login.asp";}
				}}
		});

//multilanguage settings
$j("#multi_0").html(multiLanguage_array[multi_INT][0]);
$j("#multi_0_1").html(multiLanguage_array[multi_INT][0]);
$j("#multi_1").attr("value",multiLanguage_array[multi_INT][1]);
$j("#multi_1_1").attr("value",multiLanguage_array[multi_INT][1]);
$j("#multi_2").html(multiLanguage_array[multi_INT][2]);
$j("#multi_4").html(multiLanguage_array[multi_INT][4]);
$j("#multi_5").html(multiLanguage_array[multi_INT][5]);
$j("#multi_6").html(multiLanguage_array[multi_INT][6]);
$j("#multi_7").html(multiLanguage_array[multi_INT][7]);
$j("#multi_8").html(multiLanguage_array[multi_INT][8]);
$j("#multi_9").html(multiLanguage_array[multi_INT][9]);
$j("#multi_10").html(multiLanguage_array[multi_INT][10]);
$j("#multi_10_0").html(multiLanguage_array[multi_INT][10]);
$j("#multi_11").html(multiLanguage_array[multi_INT][11]);
$j("#multi_12").html(multiLanguage_array[multi_INT][12]);
$j("#multi_13").html(multiLanguage_array[multi_INT][13]);
$j("#multi_14").html(multiLanguage_array[multi_INT][14]);
$j("#PATH").attr("placeholder",multiLanguage_array[multi_INT][15]);
$j("#multi_17").attr("value",multiLanguage_array[multi_INT][17]);
</script>
</html>
