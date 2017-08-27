<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<!--<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>Add New Folder</title>
<link rel="stylesheet" href="../form_style.css"  type="text/css">
<script type="text/javascript" src="../state.js"></script>-->
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
var PoolDevice = FullName.substring(5);
var PoolName = FullName.substring(5);
var folderlist = parent.get_sharedfolder_in_pool(PoolDevice);

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

function showtext(obj, str){
	if(obj)
		obj.innerHTML = str;
}

function initial(){
	showtext($("poolName"), PoolName);
	document.createFolderForm.folder.focus();
	clickevent();
}

function clickevent(){
	if(navigator.userAgent.search("MSIE") == -1)
		document.getElementById('folder').addEventListener('keydown',keyDownHandler,false);
	else
		document.getElementById('folder').attachEvent('onkeydown',keyDownHandler);
	
	$("Submit").onclick = submit;
}
function submit(){
	if(validForm()){
				document.createFolderForm.account.disabled = 1;
					
				document.createFolderForm.pool.value = PoolDevice;

				var path = document.getElementById('pool').value + "/" +document.getElementById('folder').value;
				var eric = parent.creatfolder(path);
				if(eric == "folder exists")
					alert("Notice:The folder name alreadly exists in this volume.\nPlease try the action again and enter a different folder name!");
				else if(eric == "error")
					alert("Notice:Failed to create folder");	
				parent.hidePop("apply");
				setTimeout(" ",2000);				
				parent.FromObject = parent.document.aidiskForm.layer_order.value;
				parent.get_layer_items(parent.document.aidiskForm.layer_order.value);				
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
	$("folder").value = trim($("folder").value);

	// share name
	if($("folder").value.length == 0){
		alert(parent.multiLanguage_array[parent.multi_INT][23]);
		$("folder").focus();
		return false;
	}
	
	var re = new RegExp("[^a-zA-Z0-9 _-]+", "gi");
	if(re.test($("folder").value)){
		alert(parent.multiLanguage_array[parent.multi_INT][24]);
		$("folder").focus();
		return false;
	}
	if(parent.checkDuplicateName($("folder").value, folderlist)){
		alert(parent.multiLanguage_array[parent.multi_INT][25]);
		$("folder").focus();
		return false;
	}
	
	if(trim($("folder").value).length > 12)
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
	
<form name="createFolderForm" target="hidden_frame">
<input type="hidden" name="account" id="account">
<input type="hidden" name="pool" id="pool">
	<table width="100%" class="popTable" border="0" align="center" cellpadding="0" cellspacing="0">
	<thead>
    <tr>
      <td colspan="2"><span style="color:#FFF" id="multi_18"></span><span style="color:#FFF" id="multi_19">in</span><span style="color:#FFF" id="poolName"></span><img src="images/button-close.gif" onClick="parent.hidePop('OverlayMask');"></td>
    </tr>
	</thead>
	<tbody>
    <tr align="center">
      <td height="50" colspan="2" id="multi_20"></td>
    </tr>
    <tr>
      <th width="100"><span id="multi_21"></span>: </th>
      <td height="50"><input class="input_25_table" type="text" name="folder" id="folder" style="width:220px;" onkeypress="return NoSubmit(event)"></td>
    </tr>
    <tr bgcolor="#E6E6E6">
      <th colspan="2"><input id="Submit" type="button" class="button_gen" value=""></th>
    </tr>
  </tbody>	
  </table>
</form>
</body>
<script>
$("multi_18").innerHTML=parent.multiLanguage_array[parent.multi_INT][18];
$("multi_19").innerHTML=parent.multiLanguage_array[parent.multi_INT][19];
$("multi_20").innerHTML=parent.multiLanguage_array[parent.multi_INT][20];
$("multi_21").innerHTML=parent.multiLanguage_array[parent.multi_INT][21];
$("Submit").value=parent.multiLanguage_array[parent.multi_INT][22];
</script>
</html>
