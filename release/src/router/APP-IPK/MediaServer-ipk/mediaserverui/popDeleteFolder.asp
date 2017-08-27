<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<!--<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>Del Folder</title>
<link rel="stylesheet" href="../form_style.css"  type="text/css">
<script type="text/javascript" src="../state.js"></script>
<script type="text/javascript" src="jquery.js"></script>-->
<style type="text/css">
.popTable {
	border:1px solid #666;	
}
.popTable thead td{
	background-image:url(images/title-bg-2.gif);
	border-bottom:1px solid #666;
}
	
.popTable thead span{
	font-family:Verdana;
	font-size:12px;
	font-weight:bold;
	color:#000000;
	float:left;
	margin:6px 5px 0px 5px;
	}
.popTable thead img{
	float:right;
	margin:0px 4px 0px 4px;
	cursor:pointer;
}

.popTable tbody{
	background-color:#FFF;
	text-align:left;
}
.popTable tbody th{
	font-family:Verdana;
	font-size:12px;
	text-align:right;
	padding-right:5px;
	height:30px;
	color: #000;
}
.popTable tbody td{
	color: #666666;
	font-family: Verdana, Arial, Helvetica, sans-serif;
	font-size:10pt;
	padding:4px;
}
.input_25_table{
	margin-left:2px;	
	padding-left:0.4em;
	height:23px;
	width:235px;
	line-height:23px \9;	/*IE*/
	font-size:12px;
	font-family: Lucida Console;
	background-image:url(images/New_ui/inputbg.png);
	border-width:0;
	color:#FFFFFF;
}
.button_gen{
	font-weight: bolder;
	text-shadow: 1px 1px 0px black;
  background: transparent url(images/New_ui/contentbt_normal.png) no-repeat scroll center top;
  _background: transparent url(images/New_ui/contentbt_normal_ie6.png) no-repeat scroll center top;
  border:0;
  color: #FFFFFF;
	height:33px;
	font-family:Verdana;
	font-size:12px;
  padding:0 .70em 0 .70em;  
 	width:122px;
  overflow:visible;
	cursor:pointer;
	outline: none; /* for Firefox */
 	hlbr:expression(this.onFocus=this.blur()); /* for IE */
}
</style>

<script type="text/javascript">
var FullName = parent.path_directory;
var FullNames = FullName.split("/");
var PoolDevice = FullNames[2];
var selectedFolder = FullNames[3];
var folderlist = parent.get_sharedfolder_in_pool(PoolDevice);
var delete_flag = 0;

function showtext(obj, str){
	if(obj)
		obj.innerHTML = str;
}

function $(){
	var elements = new Array();
	
	for(var i = 0; i < arguments.length; ++i){
		var element = arguments[i];
		if(typeof element == 'string')
			element = document.getElementById(element);
		
		if(arguments.length == 1)
			return element;
		
		elements.push(element);
	}
	
	return elements;
}

function showhtmlspace(ori_str){
	var str = "", head, tail_num;
	
	head = ori_str;
	while((tail_num = head.indexOf(" ")) >= 0){
		str += head.substring(0, tail_num);
		str += "&nbsp;";
		
		head = head.substr(tail_num+1, head.length-(tail_num+1));
	}
	str += head;
	
	return str;
}

function showhtmland(ori_str){
	var str = "", head, tail_num;
	
	head = ori_str;
	while((tail_num = head.indexOf("&")) >= 0){
		str += head.substring(0, tail_num);
		str += "&amp;";
		
		head = head.substr(tail_num+1, head.length-(tail_num+1));
	}
	str += head;
	
	return str;
}

function initial(){
	showtext($("selected_Folder"), showhtmlspace(showhtmland(selectedFolder)));
	document.deleteFolderForm.Cancel.focus();
	delete_flag = parent.get_layer_items_test(parent.document.aidiskForm.layer_order.value.substring(0,5));
	clickevent();	
}

function clickevent(){
if(navigator.userAgent.search("MSIE") == -1)
		window.addEventListener('keydown',keyDownHandler,false);
	else{	
		//window.attachEvent('onkeydown',keyDownHandler);
	}
	$("Submit").onclick = submit;
}
function submit(){
	$("pool").value = PoolDevice;
	$("folder").value = selectedFolder;
	var eric;
			
	if(delete_flag == 1){
		parent.FromObject = parent.document.aidiskForm.layer_order.value.substring(0,3);
		eric = parent.deletefolder(FullName);
		if(eric == "error")
			alert("Notice:Failed to delete the folder");
		else if(eric == "have no this folder")
			alert("Warning:Folder does not exist, please try again after the confirmation.");
		setTimeout(" ",2000);
		parent.get_layer_items(parent.document.aidiskForm.layer_order.value.substring(0,3));
		delete_flag = 0;
	}	
	else{
		parent.FromObject = parent.document.aidiskForm.layer_order.value.substring(0,5);
		eric = parent.deletefolder(FullName);
		if(eric == "error")
			alert("Notice:Failed to delete the folder");
		else if(eric == "have no this folder")
			alert("Warning:Folder does not exist, please try again after the confirmation.");
		setTimeout(" ",2000);
		parent.get_layer_items(parent.document.aidiskForm.layer_order.value.substring(0,5));				
	}
	parent.hidePop("apply");
	setTimeout(" ",2000);
}
function keyDownHandler(event){
	var keyPressed = event.keyCode ? event.keyCode : event.which;

	if(keyPressed == 13){   // Enter key
		submit();
	}	
	else if(keyPressed == 27){  // Escape key
		parent.hidePop("apply");
	}	
}

</script>
</head>
<body onLoad="initial();" onKeyPress="keyDownHandler(event);">
<form name="deleteFolderForm" target="hidden_frame">
<input type="hidden" name="pool" id="pool" value="">
<input type="hidden" name="folder" id="folder" value="">
  <table width="100%" class="popTable" border="0" align="center" cellpadding="0" cellspacing="0">
  <thead>
    <tr>
      <td><span style="color:#FFF" id="multi_27"></span><img src="images/button-close.gif" onClick="parent.hidePop('OverlayMask');"></td>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td height="70" valign="middle"><span id="multi_28"></span><span id="selected_Folder" style="color:#333333; "></span></td>
	</tr>

    <tr>
      <td height="30" align="right" bgcolor="#E6E6E6">
	  <input name="Submit" id="Submit" type="button" class="button_gen" value="Delete" onclick="">
	  <input name="Cancel" id="Cancel" type="button" class="button_gen" value="Cancel" onClick="parent.hidePop('OverlayMask');"></td>
    </tr>
	</tbody>
  </table>
</form>  
</body>
<script>
$("multi_27").innerHTML=parent.multiLanguage_array[parent.multi_INT][27];
$("multi_28").innerHTML=parent.multiLanguage_array[parent.multi_INT][28];
$("Submit").value=parent.multiLanguage_array[parent.multi_INT][29];
$("Cancel").value=parent.multiLanguage_array[parent.multi_INT][17];
</script>
</html>
