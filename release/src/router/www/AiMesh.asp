<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=Edge"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title><#Web_Title#> - AiMesh</title>
<link rel="stylesheet" type="text/css" href='./aimesh/aimesh_topology.css?v=<% nvram_char_to_ascii("", "extendno"); %>' />
<link rel="stylesheet" type="text/css" href="./index_style.css">
<link rel="stylesheet" type="text/css" href="./form_style.css">
<link rel="stylesheet" type="text/css" href="./device-map/device-map.css" />
<script type="text/javascript" src="./js/jquery.js"></script>
<script type="text/javascript" src="./calendar/jquery-ui.js"></script>
<script type="text/javascript" src="./js/jstree/jstree.min.js"></script>
<script type="text/javascript" src="./js/jstree/jstree_customize.js"></script>
<script type="text/javascript" src="./state.js"></script>
<script type="text/javascript" src="./js/httpApi.js"></script>
<script type="text/javascript" src="./client_function.js" defer></script>
<script type="text/javascript" src="./help.js" defer></script>
<script type="text/javascript" src="./form.js" defer></script>
<script>
var getUrlParameter = function getUrlParameter(param){
	var url_parm = window.location.search.substring(1);
	var parm_array = url_parm.split("&");
	var key_value;

	for(var i = 0; i < parm_array.length; i += 1){
		key_value = parm_array[i].split("=");
		if (key_value[0] == param) {
			return typeof key_value[1] == "undefined" ? "" : decodeURIComponent(key_value[1]);
		}
	}
	return "";
};
function getSafeTheme(paramName, allowedValues = ['WHITE', 'ROG', 'TUF', '']) {
	let rawValue = getUrlParameter(paramName);
	if (!rawValue) return "";
	let safePrefix = rawValue.toUpperCase().match(/^[A-Z0-9_]*/)[0];
	return allowedValues.includes(safePrefix) ? safePrefix : "";
}

var theme = getSafeTheme("current_theme").toUpperCase();
if (isSupport("UI4")) {
	$('link').filter("[href*='./aimesh/aimesh_topology.css']").after('<link rel="stylesheet" type="text/css" href="./aimesh/aimesh_topology_v4.css">');
} else if (theme == "WHITE") {
	$('link').filter("[href*='./aimesh/aimesh_topology.css']").after('<link rel="stylesheet" type="text/css" href="./aimesh/aimesh_topology_' + theme + '.css">');
}

if(isSupport("TS_UI"))
	$('link').last().after('<link rel="stylesheet" type="text/css" href="./css/difference.css">');

function initial(){
	show_menu();
	change_tab(1);
	let retryCount = 0;
	function loadAiMeshTopology() {
		$("#AiMesh_Topology").load(`${rootPath}/aimesh/aimesh_topology.html`, function(response, status, xhr){
			if(status === "error" && retryCount < 3){
				retryCount++;
				setTimeout(loadAiMeshTopology, 500);
				return;
			}
			else if(status === "error") {
				$("#AiMesh_Topology").html(`<div class='text_emphasize'><#vpn_ipsec_update_cert_fail#></div>`);
				return;
			}
			setTimeout(function(){
				$('link').filter("[href*='./aimesh/aimesh_topology.css']").after('<link rel="stylesheet" type="text/css" href="./aimesh/aimesh_system_settings.css">');
				if (isSupport("UI4")) {
					$('link').filter("[href='./aimesh/aimesh_system_settings.css']").after('<link rel="stylesheet" type="text/css" href="./aimesh/aimesh_system_settings_v4.css">');
				} else if(theme == "WHITE"){
					$('link').filter("[href='./aimesh/aimesh_system_settings.css']").after('<link rel="stylesheet" type="text/css" href="./aimesh/aimesh_system_settings_' + theme + '.css">');
				}
				document.head.insertAdjacentHTML('beforeend', '<link rel="stylesheet" type="text/css" href="./css/asus_faq.css">');
				$("#AiMesh_System_Settings").load("./aimesh/aimesh_system_settings.html");
				[
					'./general.js',
					'./popup.js',
					'./validator.js',
					'./disk_functions.js',
					'./form.js',
					'./switcherplugin/jquery.iphone-switch.js',
					'./js/collected_FAQ.js'
				].forEach(function(src) {
					loadScriptWithRetry(src, 3);
				});
			}, 100);
		});
	}
	loadAiMeshTopology();

	if(parent.webWrapper){
		const resizeObserver = new ResizeObserver(entries => {
			for (let entry of entries) {
				const height = entry.target.getBoundingClientRect().height;
				window.parent.postMessage({ type: 'resize', height: height }, '*');
			}
		});

		resizeObserver.observe(document.querySelector('.content'));
	}
}
function change_tab(_index){
	$(".aimesh_tab span").removeClass("clicked");
	$(".aimesh_tab span").eq(_index - 1).addClass("clicked");
	$(".aimesh_tab_content").hide();
	$(".aimesh_tab_content.idx" + _index + "").css("display", "block");
	if(_index.toString() === "2"){
		initial_system_settings();
	}

	if(top.webWrapper){
		if(!$(".info_left").is(":visible")){
			$(".info_right").hide();
			$(".info_left").show();
		}
	}
}
function loadScriptWithRetry(src, retryLimit = 3, callback) {
	let retries = 0;
	function tryLoad() {
		const script = document.createElement('script');
		script.src = src;
		script.type = 'text/javascript';
		script.onload = function() {
			if (typeof callback === "function") callback();
		};
		script.onerror = function() {
			retries++;
			if (retries < retryLimit) {
				tryLoad();
			}
		};
		document.head.appendChild(script);
	}
	tryLoad();
}
</script>
</head>

<body onload="initial();" onunLoad="return unload_body();" class="bg">
<div id="TopBanner"></div>


<div id="Loading" class="popup_bg"></div>
<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame" autocomplete="off">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="AiMesh.asp">
<input type="hidden" name="next_page" value="AiMesh.asp">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="action_wait" value="5">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">

<table class="content ui4-no-bg" align="center" cellpadding="0" cellspacing="0">
	<tr>
		<td width="17">&nbsp;</td>
		<!--=====Beginning of Main Menu=====-->
		<td valign="top" width="202">
			<div id="mainMenu"></div>
			<div id="subMenu"></div>
		</td>
		<td valign="top">
			<div id="tabMenu" class="submenuBlock"></div>
			<!--===================================Beginning of Main Content===========================================-->
			<table class="ui4-no-bg" width="98%" border="0" align="left" cellpadding="0" cellspacing="0">
				<tr>
					<td align="left" valign="top">
						<table width="760px" border="0" cellpadding="5" cellspacing="0" class="FormTitle ui4-no-bg" id="FormTitle" style="border-radius:3px;">
							<tbody>
								<tr>
									<td bgcolor="#4D595D" valign="top">
										<div>&nbsp;</div>
										<!--div class="formfonttitle">AiMesh</div>
										<div style="margin:5px;" class="splitLine"></div-->
										<div class="aimesh_tab">
											<span onclick="change_tab(1);"><#AiMesh_Topology#></span>
											<span onclick="change_tab(2);"><#AiMesh_System_Settings#></span>
											<span style="display:none;" onclick="change_tab(3);"><#AiMesh_Statistics#></span>
										</div>
										<div class="splitLine"></div>
										<div id="AiMesh_Topology" class="aimesh_tab_content idx1"></div>
										<div id="AiMesh_System_Settings" class="aimesh_tab_content idx2"></div>
									</td>
								</tr>
							</tbody>
						</table>
					</td>
				</tr>
			</table>
			<!--===================================End of Main Content===========================================-->
		</td>
		<td width="10" align="center" valign="top">&nbsp;</td>
	</tr>
</table>
</form>
<div id="footer"></div>
</body>
</html>
