<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">

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
var PoolName = FullNames[2];
var folderlist = parent.get_sharedfolder_in_pool(PoolDevice);
var selectedFolder = FullNames[3];

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
	showtext($("selected_Pool"), PoolName);
	showtext($("selected_Folder"), showhtmlspace(showhtmland(selectedFolder)));
	document.modifyFolderForm.new_folder.focus();
	clickevent();
}

function clickevent(){
	if(navigator.userAgent.search("MSIE") == -1)
		document.getElementById('new_folder').addEventListener('keydown',keyDownHandler,false);		
	else
		document.getElementById('new_folder').attachEvent('onkeydown',keyDownHandler);
	
	$("Submit").onclick = submit;
}
function submit(){
	if(validForm()){
		$("pool").value = PoolDevice;
		$("folder").value = selectedFolder;
	
		var path = PoolDevice + "@" + selectedFolder +"@" + document.getElementById('new_folder').value;
		var eric = parent.modifyfolder(path);
		if(eric == "have no this folder")
			alert("Warning:Folder does not exist, please try again after the confirmation.");
		else if(eric == "new folder name exists")
			alert("Warning:New folder name alreadly exists in this volume.\nPlease try the action again and enter a different folder name!");
		else if(eric == "error")
			alert("Notice:Failed to rename the folder");
		parent.hidePop("apply");
		setTimeout(" ",2000);
		parent.FromObject = parent.document.aidiskForm.layer_order.value.substring(0,5);
		setTimeout(" ",2000);
		parent.get_layer_items(parent.document.aidiskForm.layer_order.value.substring(0,5));				
	}
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

function trim(val){
	val = val+'';
	for (var startIndex=0;startIndex<val.length && val.substring(startIndex,startIndex+1) == ' ';startIndex++);
	for (var endIndex=val.length-1; endIndex>startIndex && val.substring(endIndex,endIndex+1) == ' ';endIndex--);
	return val.substring(startIndex,endIndex+1);
}

function validForm(){
	$("new_folder").value = trim($("new_folder").value);
	if(selectedFolder == "asusware"){
		alert("System folder does not allow this operation!");
		$("new_folder").focus();
		return false;
	}
	if(selectedFolder == "Download2"){
		alert("This fodler is DownloadMaster's default downloading folder and dose not allow this operation!");
		$("new_folder").focus();
		return false;
	}
	
	// share name
	if($("new_folder").value.length == 0){
		alert(parent.multiLanguage_array[parent.multi_INT][23]);
		$("new_folder").focus();
		return false;
	}
	
	var re = new RegExp("[^a-zA-Z0-9 _-]+","gi");
	if(re.test($("new_folder").value)){
		alert(parent.multiLanguage_array[parent.multi_INT][24]);
		$("new_folder").focus();
		return false;
	}
	
	if(parent.checkDuplicateName($("new_folder").value, folderlist)){
		alert(parent.multiLanguage_array[parent.multi_INT][25]);
		$("new_folder").focus();
		return false;
	}
	
	if(trim($("new_folder").value).length > 12)
		if (!(confirm(parent.multiLanguage_array[parent.multi_INT][26])))
			return false;
	
	return true;
}

function NoSubmit(e){
    e = e || window.event;  
    var keynum = e.keyCode || e.which;
    if(keynum === 13){        
        return false;
    }
}
</script>
</head>

<body onLoad="initial();">
<form name="modifyFolderForm" target="hidden_frame">
<input type="hidden" name="pool" id="pool" value="">
<input type="hidden" name="folder" id="folder" value="">
	<table width="100%" class="popTable" border="0" align="center" cellpadding="0" cellspacing="0">
	<thead>
      <tr>
        <td colspan="2"><span id="multi_30" style="color:#FFF"></span><img src="images/button-close.gif" onClick="parent.hidePop('OverlayMask');"></td>
      </tr>
	</thead>	  
	<tbody>
      <tr>
        <td id="multi_31" colspan="2" height="30"></td>
      </tr>
      <tr>
        <th><span id="multi_32"></span>: </th>
        <td colspan="3"><span id="selected_Pool"></span></td>
	  </tr>
      <tr>
        <th><span id="multi_21_1"></span>: </th>
        <td colspan="3"><span id="selected_Folder"></span></td>
      </tr>
      <tr>
        <th id="multi_33"><span id="multi_33"></span>: </th>
        <td><input class="input_25_table" type="text" name="new_folder" id="new_folder" onkeypress="return NoSubmit(event)" ></td>
      </tr>
      <tr bgcolor="#E6E6E6">
        <th colspan="2" align="right"><input id="Submit" type="button" class="button_gen" value="Modify"></th>
      </tr>
	</tbody>	  
    </table>
</form>
</body>
<script>
$("multi_30").innerHTML=parent.multiLanguage_array[parent.multi_INT][30];
$("multi_31").innerHTML=parent.multiLanguage_array[parent.multi_INT][31];
$("multi_32").innerHTML=parent.multiLanguage_array[parent.multi_INT][32];
$("multi_21_1").innerHTML=parent.multiLanguage_array[parent.multi_INT][21];
$("multi_33").innerHTML=parent.multiLanguage_array[parent.multi_INT][33];
$("Submit").value=parent.multiLanguage_array[parent.multi_INT][34];
</script>
</html>
