<!DOCTYPE html>
<html>
<head>
	<meta charset="utf-8">
	<meta http-equiv="X-UA-Compatible" content="IE=11;IE=Edge"/>
	<meta http-equiv="Pragma" CONTENT="no-cache">
	<meta http-equiv="Expires" CONTENT="-1">
	<meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
	<link rel="shortcut icon" href="images/favicon.png">
	<title><#menu1#> - Light Effect</title><!-- untranslated -->
	<link rel="stylesheet" href="../form_style.css">
	<link rel="stylesheet" href="../NM_style.css">
	<link rel="stylesheet" href="../css/networkMap.css">
	<script src="../js/jquery.js"></script>
	<script src="../js/httpApi.js"></script>
	<script src="../state.js"></script>
	<script src="../js/device.js"></script>
</head>
<style type="text/css">
.light_effect_iframe {
	width: 320px;
	height: 490px;
	margin-left: -8px;
}
</style>
<body>
<script>
function switchTab(id){
	var obj = {
		'wireless_tab': 'router.asp',
		'status_tab': 'router_status.asp',
		'light_effect_tab': 'router_light_effect.asp'
	}
	var path = window.location.pathname.split('/').pop();
	var targetPath = obj[id];
	if(targetPath == path){return false;}

	location.href = targetPath;
}
</script>
	<div class="main-block">
		<div class="display-flex flex-a-center">
			<div id="wireless_tab" class="tab-block " onclick="switchTab(this.id)"><#menu5_1#></div>
			<div id="status_tab" class="tab-block" onclick="switchTab(this.id)"><#Status_Str#></div>
			<div id="light_effect_tab" class="tab-block tab-click" onclick="switchTab(this.id)">Aura RGB</div><!-- untranslated -->
		</div>
		<iframe id="light_effect_iframe" class="light_effect_iframe" frameborder="0"></iframe>
		<script>
			$("#light_effect_iframe")
				.attr("src", "/light_effect/light_effect.html")
				.load(function(){
					if(isSupport("rog") || isSupport("tuf"))
						$("#light_effect_iframe").css("background-color", "initial");
					else
						$("#light_effect_iframe").css("background-color", "#273342");
				})
				.css("height", function(){
					const support_night_mode = (()=>{
						return ((based_modelid == "GT-BE98" || based_modelid == "GT-BE98_PRO" || based_modelid == "GT-BE96" || based_modelid == "GT-BE19000") ? true : false);
					})();
					return (support_night_mode ? "530px" : $(this).css("height"));
				})
		</script>
	</div>
</body>
</html>
