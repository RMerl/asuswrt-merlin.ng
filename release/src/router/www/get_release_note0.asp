<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<script language="JavaScript" type="text/javascript" src="/js/jquery.js"></script>
<script>
function reSize(){
	parent.document.getElementById('status_iframe').height=document.body.scrollHeight;	
}
var content = "";
function GenContent(){
	var dead = 0;
	$.ajax({
		url: '/release_note0.asp',
		dataType: 'text',
		timeout: 1500,
		error: function(xhr){
			if(dead > 30){
				$("#noteArea0").html("Fail to grab release note");
			}
			else{
				dead++;
				setTimeout("GenContent();", 1000);
			}
		},

		success: function(resp){
			content = parent.htmlEnDeCode.htmlEncode(resp);
			if(content.length > 10){
				$("#noteArea0").html(content);
			}
			else{
				$("#noteArea0").html("Fail to grab release note");
			}
		}
	});
}
</script>		
</head>		
<body onload="GenContent();reSize();">
<textarea cols='63' rows='18' wrap='normal' id="noteArea0" readonly='readonly' style='width:98%; font-family:Courier New, Courier, mono; font-size:13px;color:#FFFFFF;background-color:rgb(43, 55, 59); word-wrap:normal; border:0px;'>
</textarea>
</body>
</html>
