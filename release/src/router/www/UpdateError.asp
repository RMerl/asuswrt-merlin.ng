<html>
<head>
<title>ASUS Wireless Router Web Manager</title>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
</head>
<body>
<script>
	var upgrade_fw_status = '<% nvram_get("upgrade_fw_status"); %>';
	parent.cancel_dr_advise();
	parent.document.form.update.disabled = true;
	parent.document.form.file.disabled = true;
	parent.document.form.upload.disabled = true;
	if(upgrade_fw_status == 6){
		//alert("<#FIRM_trx_valid_fail_desc#>");
		 parent.confirm_asus({
			title: "Invalid Firmware Upload",
			contentA: "<#FIRM_trx_valid_fail_desc#><br>",
			left_button: "",
			left_button_callback: function(){},
			left_button_args: {},
			right_button: "<#CTL_ok#>",
			right_button_callback: function(){parent.confirm_cancel();parent.location.reload();},
			right_button_args: {},
			iframe: "",
			margin: "100px 0px 0px 25px",
			note_display_flag: 1
                
		});
		
	}
	else{		
		//alert("<#FIRM_fail_desc#>");
		parent.confirm_asus({
			title: "Invalid Firmware Upload",
			contentA: "<#FIRM_fail_desc#><br><#FW_desc1#><br>",
			left_button: "",
			left_button_callback: function(){},
			left_button_args: {},
			right_button: "<#CTL_ok#>",
			right_button_callback: function(){parent.confirm_cancel();parent.location.reload();},
			right_button_args: {},
			iframe: "",
			margin: "100px 0px 0px 25px",
			note_display_flag: 1
		
		});
	}
	
</script>
</body>
</html>

