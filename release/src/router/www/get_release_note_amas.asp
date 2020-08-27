<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<script language="JavaScript" type="text/javascript" src="/js/jquery.js"></script>
<script>
var flag_show = '<% get_parameter("flag"); %>';
function reSize(){
	if(flag_show==1)
		GenContent();
	parent.document.getElementById('status_iframe').height=document.body.scrollHeight;
}
var content = "";
function GenContent(){
	var dead = 0;
	$.ajax({
		url: '/release_note_amas.asp',
		dataType: 'text',
		timeout: 1500,
		error: function(xhr){
			if(dead > 30){
				$("#amas_release_note").html("Fail to grab release note");
			}
			else{
				dead++;
				setTimeout("GenContent();", 1000);
			}
		},
		
		success: function(resp){
			content = parent.htmlEnDeCode.htmlEncode(resp);
			if(content.length > 10){
				$("#amas_release_note_hint").css("display", "none");
				$("#amas_release_note")
					.html(content)
					.css("display", "");
			}
			else{
				$("#amas_release_note").html("Fail to grab release note");
			}
		}
	});
}
</script>		
</head>		
<body onload="reSize();">
<textarea id="amas_release_note" cols='63' rows='18' wrap='normal' readonly='readonly' style='width:98%; font-family:Courier New, Courier, mono; font-size:13px;color:#FFFFFF;background-color:rgb(43, 55, 59); word-wrap:normal; border:0px; display:none;'>
</textarea>
<textarea id="amas_release_note_hint" cols='63' rows='18' wrap='normal' readonly='readonly' style='width:98%; font-family:Courier New, Courier, mono; font-size:13px;color:#FFFFFF;background-color:rgb(43, 55, 59); word-wrap:normal; border:0px; '>Loading release note, please wait…</textarea><!--untranslated-->
</body>
</html>
