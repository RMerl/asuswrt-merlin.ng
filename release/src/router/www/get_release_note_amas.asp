<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<script>
function reSize(){
	parent.document.getElementById('status_iframe').height=document.body.scrollHeight;	
}
</script>		
</head>		
<body onload="reSize();">
<textarea id="amas_relese_note" cols='63' rows='18' wrap='normal' readonly='readonly' style='width:98%; font-family:Courier New, Courier, mono; font-size:13px;color:#FFFFFF;background-color:rgb(43, 55, 59); word-wrap:normal; border:0px; display:none;'>
<%nvram_dump("release_note.txt","");%>
</textarea>
<textarea id="amas_relese_note_hint" cols='63' rows='18' wrap='normal' readonly='readonly' style='width:98%; font-family:Courier New, Courier, mono; font-size:13px;color:#FFFFFF;background-color:rgb(43, 55, 59); word-wrap:normal; border:0px; '>Loading release note, please wait…</textarea><!--untranslated-->
</body>
</html>
