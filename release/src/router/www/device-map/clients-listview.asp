<!DOCTYPE html>
<html>
	<head>
		<link href="/form_style.css" rel="stylesheet" type="text/css" />
		<link href="/NM_style.css" rel="stylesheet" type="text/css" />
		<link href="/device-map/device-map.css" rel="stylesheet" type="text/css" />
		<link href="/css/business-white.css" rel="stylesheet" type="text/css" />
		<script type="text/javascript" src="/js/jquery.js"></script>
		<script type="text/javascript" src="/js/httpApi.js"></script>
		<script type="text/javascript" src="/help.js"></script>
		<script type="text/javascript" src="/validator.js"></script>
		<script language="JavaScript" type="text/javascript" src="/client_function.js"></script>
		<style>
			body{
				overflow-y: hidden !important;
			}
			#clientlist_viewlist_content{
				background: transparent;
			}
			.block_filter_name{
				text-align:center;
				padding-top:5px;
				color:#CCCCCC;
				font-size:14px
			} 
			.block_filter_pressed{
				background-color:#0d6efd !important;
				border-color:#0d6efd !important;
				border-width:1px;
				border-style:inset;
			}
			.block_filter_pressed:hover{
				color:#FFFFFF !important;
			} 
			.clientlist_viewlist{
				left: 0 !important;
				width: 100% !important;
			}
			.closeBtn{
				display: none;
			}
			#tr_all_title, #clientlist_all_list_Block{
				background-color: #FFF;
			}
		</style>
	</head>
	<body>
		<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
	</body>
	<script>
		pop_clientlist_listview();
		if(top.webWrapper) parent.$("#client_all_count").html(totalClientNum.online)
		if(top.webWrapper) parent.$("#wireless_count").html(totalClientNum.wireless)
		if(top.webWrapper) parent.$("#wired_count").html(totalClientNum.wired)
	</script>
</html>
