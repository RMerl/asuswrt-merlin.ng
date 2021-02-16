<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="X-UA-Compatible" content="IE=Edge">
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>KoolShare - 软件中心</title>
<link rel="stylesheet" type="text/css" href="index_style.css">
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="/res/softcenter.css">
<link rel="stylesheet" type="text/css" href="/res/layer/theme/default/layer.css">
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/help.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script language="JavaScript" type="text/javascript" src="/client_function.js"></script>
<script language="JavaScript" type="text/javascript" src="/validator.js"></script>
<script type="text/javascript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/switcherplugin/jquery.iphone-switch.js"></script>
<script type="text/javascript" src="/res/softcenter.js"></script>
<script type="text/javascript" src="/form.js"></script>
<style>
.cloud_main_radius_left {
	-webkit-border-radius: 10px 0 0 10px;
	-moz-border-radius: 10px 0 0 10px;
	border-radius: 10px 0 0 10px;
}
.cloud_main_radius_right {
	-webkit-border-radius: 0 10px 10px 0;
	-moz-border-radius: 0 10px 10px 0;
	border-radius: 0 10px 10px 0;
}
.cloud_main_radius {
	-webkit-border-radius: 10px;
	-moz-border-radius: 10px;
	border-radius: 10px;
}
/* 软件中心icon新样式 by acelan */
 dl, dt, dd {
	padding:0;
	margin:0;
}
input[type=button]:focus {
	outline: none;
}
.icon {
	float:left;
	position:relative;
	margin: 10px 0px 30px 0px;
}
.icon-title {
	line-height: 3em;
	text-align:center;
}
.icon-pic {
	margin: 10px 30px 0px 30px;
}
.icon-pic img {
	border:0;
	width: 60px;
	height: 60px;
	margin:2px;
}
.icon-desc {
	position: absolute;
	left: 0;
	top: 0;
	height: 105%;
	visibility: hidden;
	font-size:0;
	width: 119px;
	border-radius: 8px;
	font-size: 16px;
	opacity: 0;
	background-color:#000;
	margin:5px;
	text-overflow:ellipsis;
	transition: opacity .5s ease-in;
}
.icon-desc .text {
	font-size: 12px;
	line-height: 1.4em;
	display: block;
	height: 100%;
	padding: 10px;
	box-sizing: border-box;
}
.icon:hover .icon-desc {
	opacity: .8;
	visibility: visible;
}
.icon-desc .opt {
	position: absolute;
	bottom: 0;
	height: 18px;
	width: 100%;
}
.install-status-0 .icon-desc .opt {
	height: 100%;
}
.icon-desc .install-btn, .icon-desc .uninstall-btn, .icon-desc .update-btn {
	background: #fff;
	color:#333;
	cursor:pointer;
	text-align: center;
	font-size: 13px;
	padding-bottom: 5px;
	margin-left: 10px;
	margin-right: 10px;
	display: block;
	width: 100%;
	height: 18px;
	border-radius: 0px 0px 5px 5px;
	border: 0px;
	position: absolute;
	bottom: 0;
	left: -10px;
}
.icon-desc .uninstall-btn {
	display: none;
}
.icon-desc .update-btn {
	display: none;
	border-radius: 0px 0px 0px 5px;
	width:60%;
	border-right: 1px solid #000;
}
.install-status-1 .uninstall-btn {
	display: block;
}
.install-status-1 .install-btn {
	display: none;
}
.update-btn {
	display: none;
}
.install-status-1 .update-btn {
	display: none;
}
.install-status-4 .uninstall-btn {
	display: block;
}
.install-status-4 .install-btn {
	display: none;
}
.install-status-4 .update-btn {
	display: none;
}
.install-status-2 .uninstall-btn {
	display: block;
	width: 40%;
	border-radius: 0px 0px 5px 0px;
	right: -10px;
	left: auto;
	border-left: 1px solid #000;
}
.install-status-2 .install-btn {
	display: none;
}
.install-status-2 .update-btn {
	display: block;
}
.install-status-1 {
	display: none;
}
.install-status-2 {
	display: none;
}
.install-status-0 {
	display: block;
}
.install-status-4 {
	display: none;
}
.install-view .install-status-1 {
	display: block;
}
.install-view .install-status-2 {
	display: block;
}
.install-view .install-status-0 {
	display: none;
}
.install-view .install-status-4 {
	display: block;
}
.cloud_main_radius h2 {
	border-bottom:1px #AAA dashed;
	height:32px;
	margin-top:15px;
	margin-bottom:15px;
}
.cloud_main_radius h3, .cloud_main_radius h4 {
	font-size:12px;
	color:#FC0;
	font-weight:normal;
	font-style: normal;
	margin-top:12px;
	margin-bottom:12px;
}
.cloud_main_radius h5 {
	color:#FFF;
	font-weight:normal;
	font-style: normal;
	margin-top:9px;
	margin-bottom:9x;
}
.content_status {
	position: absolute;
	-webkit-border-radius: 5px;
	-moz-border-radius: 5px;
	border-radius:10px;
	z-index: 100;
	/*background-color:#2B373B;*/
	/*margin-left: -215px;*/
	top: 0px;
	width:980px;
	return height:auto;
	box-shadow: 3px 3px 10px #000;
	background: rgba(0,0,0,0.90);
	visibility:hidden;
	margin-left:0px;
	width:728px;
}
.popup_bar_bg_ks{
	position:fixed;	
	margin: auto;
	top: 0;
	left: 0;
	width:100%;
	height:100%;
	z-index:99;
	/*background-color: #444F53;*/
	filter:alpha(opacity=90);  /*IE5、IE5.5、IE6、IE7*/
	background-repeat: repeat;
	visibility:hidden;
	overflow:hidden;
	/*background: url(/images/New_ui/login_bg.png);*/
	background:rgba(68, 79, 83, 0.94) none repeat scroll 0 0 !important;
	background-position: 0 0;
	background-size: cover;
	opacity: .94;
}
.user_title{
	text-align:center;
	font-size:18px;
	color:#99FF00;
	padding:10px;
	font-weight:bold;
}
#log_content{
	width: 98%;
	padding-left: 13px;
	padding-right: 33px;
	border: 0px solid #222;
	font-family:'Lucida Console';
	font-size: 11px;
	background: transparent;
	color: #FFFFFF;
	outline: none;
	overflow-x: hidden;
}
#softcenter_log_title i{
	color: #FC0;
	font-style: normal;
}
</style>
<script>
var db_softcenter_ = {};
var TIMEOUT_SECONDS = 18;
var softInfo = null;
var count_down;
var	refresh_flag;
var syncRemoteSuccess = 0; //判断是否进入页面后已经成功进行远端同步
var currState = {
	"installing": false,
	"lastChangeTick": 0,
	"lastStatus": "-1",
	"module": ""
};

String.prototype.myReplace = function(f, e){
    var reg = new RegExp(f, "g"); 
    return this.replace(reg, e); 
}

function checkField(o, f, d) {
	if (typeof o[f] == "undefined") {
		o[f] = d;
	}
	return o[f];
}
function appPostScript(moduleInfo, script) {
	var data = {};
	data["softcenter_home_url"] = "https://armsoft.ddnsto.com";
	data["softcenter_installing_todo"] = moduleInfo.name;
	data["softcenter_installing_title"] = moduleInfo.title;
	if (script == "ks_app_install.sh") {
		data["softcenter_installing_tar_url"] = moduleInfo.tar_url;
		data["softcenter_installing_md5"] = moduleInfo.md5;
		data["softcenter_installing_version"] = moduleInfo.version;
		//Update title for this module
		data[moduleInfo.name + "_title"] = moduleInfo.title;
		action = "ks_app_install";
	} else if (script == "ks_app_remove.sh") {
		action = "ks_app_remove";
	}
	var id = parseInt(Math.random() * 100000000);
	var postData = {
		"id": id,
		"method": script,
		"params": [action],
		"fields": data
	};
	$.ajax({
		type: "POST",
		url: "/_api/",
		data: JSON.stringify(postData),
		dataType: "json",
		success: function(response) {
			count_down = 5;
			refresh_flag = 1;
			get_log();
		}
	});
}
function appInstallModule(moduleInfo) {
	appPostScript(moduleInfo, "ks_app_install.sh");
}
function appUninstallModule(moduleInfo) {
	if (!window.confirm('确定卸载吗')) {
		return;
	}
	appPostScript(moduleInfo, "ks_app_remove.sh");
}

function initInstallStatus(){
	console.log("sadog!");
	$.ajax({
		url: '/_temp/soft_install_log.txt',
		type: 'GET',
		cache: false,
		dataType: 'text',
		success: function(response) {
			//获取一次日志，如果日志存在，且没有"XU6J03M6"字符串，则说明有插件正在安装，那么弹出日志显示页面
			if (response.search("XU6J03M6") == -1) {
				count_down = 5;
				refresh_flag = 1;
				get_log();
			}
		}
	});
}

function toggleAppPanel(showInstall) {
		$('.show-install-btn').removeClass('active');
		$('.show-uninstall-btn').removeClass('active');
		$(showInstall ? '.show-install-btn' : '.show-uninstall-btn').addClass('active');
		$('#IconContainer')[showInstall ? 'addClass' : 'removeClass']('install-view');
	}
	/**
	 * 渲染apps，安装和未安装按照class hook进行显示隐藏，存在同一个面板中
	 */
function renderView(apps) {
	// set apps to global variable of softInfo
	softInfo = apps;
	//console.log(softInfo);
	//简单模板函数
	function _format(source, opts) {
			var source = source.valueOf(),
				data = Array.prototype.slice.call(arguments, 1),
				toString = Object.prototype.toString;
			if (data.length) {
				data = data.length == 1 ?
					(opts !== null && (/\[object Array\]|\[object Object\]/.test(toString.call(opts))) ? opts : data) : data;
				return source.replace(/#\{(.+?)\}/g, function(match, key) {
					var replacer = data[key];
					// chrome 下 typeof /a/ == 'function'
					if ('[object Function]' == toString.call(replacer)) {
						replacer = replacer(key);
					}
					return ('undefined' == typeof replacer ? '' : replacer);
				});
			}
			return source;
		}
		//app 模板
	var tpl = ['',
		'<dl class="icon install-status-#{install}" data-name="#{name}">',
		'<dd class="icon-pic">',
		//当图标娶不到的时候，使用默认图标，如果已经是默认图标且娶不到，就狗带了，不管
		'<img src="#{icon}" onerror="this.src.indexOf(\'icon-default.png\')===-1 && (this.src=\'/res/icon-default.png\');" alt="图标出走了～"/>',
		'<img class="update-btn" style="position: absolute;width:20px;height:20px;margin-top:-66px;margin-left:44px;" src="/res/upgrade.png"',
		'</dd>',
		'<dt class="icon-title">#{title}</dt>',
		'<dd class="icon-desc">',
		'<a class="text" href="/#{home_url}" #{target}>',
		'#{description}',
		'</a>',
		'<div class="opt">',
		'<a type="button" class="install-btn" data-name="#{name}">安装</a>',
		'<a type="button" class="update-btn" data-name="#{name}">更新</a>',
		'<a type="button" class="uninstall-btn" data-name="#{name}">卸载</a>',
		'</div>',
		'</dd>',
		'</dl>'
	].join('');
	var installCount = 0;
	var uninstallCount = 0;
	var html = $.map(apps, function(app, name) {
		parseInt(app.install, 10) ? installCount++ : uninstallCount++;
		return _format(tpl, app);
	});
	$('#IconContainer').html(html.join(''));
	//更新安装数
	$('.show-install-btn').val('已安装(' + installCount + ')');
	$('.show-uninstall-btn').val('未安装(' + uninstallCount + ')');
}
function getRemoteData() {
	var remoteURL = db_softcenter_["softcenter_home_url"] + '/softcenter/app.json.js';
	return $.ajax({
		url: remoteURL,
		method: 'GET',
		dataType: 'jsonp',
		timeout: 3500
	});
}
function softceterInitData(data) {
	var remoteData = data;
	$("#spnOnlineVersion").html("<em>" + remoteData.version + "</em>");
	if (remoteData.version != db_softcenter_["softcenter_version"]) {
		$("#updateBtn").show();
		$("#updateBtn").click(function() {
			var moduleInfo = {
				"name": "softcenter",
				"title": "软件中心",
				"md5": remoteData.md5,
				"tar_url": remoteData.tar_url,
				"version": remoteData.version
			};
			appPostScript(moduleInfo, "ks_app_install.sh");
		});
	}
}
function init(cb) {
		//设置默认值
		function _setDefault(source, defaults) {
				$.map(defaults, function(value, key) {
					if (!source[key]) {
						source[key] = value;
					}
				});
			}
		//把本地数据平面化转换成以app为对象
		function _formatLocalData(localData) {
				var result = {};
				$.map(db_softcenter_, function(item, key) {
					key = key.split('_');
					if ('module' === key[1]) {
						var name = key[2];
						var prop = key.slice(3).join("_");
						if (!result[name]) {
							result[name] = {};
							result[name].name = name;
						}
						if (prop) {
							result[name][prop] = item;
						}
					}
				});
				for (var field in result) {
					if (!result[field].install){
						delete(result[field]);
					}
				}
				//console.log(result)
				return result;
			}
		//将本地和远程进行一次对比合并
		var usb_apps = ["aria2", "swap", "usb2jffs", "easyexplorer", "linkease"];
		var ro_model = '<% nvram_get("odmpid"); %>' || '<% nvram_get("productid"); %>';
		function _mergeData(remoteData) {
			var result = {};
			var localData = _formatLocalData(db_softcenter_);
			$.each(remoteData, function(i, app) {
				var name = app.name;
				var oldApp = localData[name] || {};
				var install = (parseInt(oldApp.install, 10) === 1 && app.version !== oldApp.version) ? 2 : oldApp.install || "0";
				if(ro_model == "ZenWiFi_XD4" || ro_model == "RT-AX56U_V2"){
					var usb_app = usb_apps.includes(name);
					if(usb_app == false){
						result[name] = $.extend(oldApp, app);
						result[name].install = install;
					}
				}else{
					result[name] = $.extend(oldApp, app);
					result[name].install = install;
				}
			});
			$.map(localData, function(app, name) {
				if (!result[name]) {
					result[name] = app;
				}
			});
			//设置默认值和设置icon的路径
			$.map(result, function(item, name) {
				_setDefault(item, {
					home_url: "Module_" + name + ".asp",
					title: name.capitalizeFirstLetter(),
					tar_url: "{0}/{0}.tar.gz".format(name),
					install: "0",
					description: "暂无",
					new_version: false
				});
				// icon 规则:
				// 如果已安装的插件,那图标必定在 /koolshare/res 目录, 通过 /res/icon-{name}.png 请求路径得到图标
				// 如果是未安装的插件,则必定在 https://armsoft.ddnsto.com/{name}/{name}/icon-{name}.png
				// TODO 如果因为一些错误导致没有图标, 有可能显示一张默认图标吗?
				item.icon = parseInt(item.install, 10) !== 0 ? ('/res/icon-' + item.name + '.png') : ('https://armsoft.ddnsto.com' + new Array(3).join('/softcenter') + '/res/icon-' + item.name + '.png');
			});
			return result;
		};
		if (syncRemoteSuccess) {
			cb();
			return;
		} else {
			renderView(_mergeData({}));
			cb();
			getRemoteData()
				.done(function(remoteData) {
					//远端更新成功
					syncRemoteSuccess = 1;
					softceterInitData(remoteData);
					remoteData = remoteData.apps || [];
					renderView(_mergeData(remoteData));
					cb();
				})
				.fail(function() {
					//如果没有更新成功，比如没网络，就用空数据merge本地
				//renderView(_mergeData({}));
				//cb();
				$("#spnOnlineVersion").html("<i>获取在线版本失败！请尝试重新刷新本页面，或者检查你的网络设置！</i>")
				});
		}
		notice_show();
	}
	//初始化整个界面展现，包括安装未安装的获取
	//当初始化过程获取软件列表失败时候，用本地的模块进行渲染
	//只要一次获取成功，以后不在重新获取，知道页面刷新重入
$(function() {
	//梅林要求用这个函数来显示左测菜单
	show_menu(menu_hook);
	//pop_111();
	$.ajax({
		type: "GET",
		url: "/_api/soft",
		dataType: "json",
		async: false,
		cache: false,
		success: function(response) {
			db_softcenter_ = response.result[0];
			db_softcenter_["softcenter_home_url"] = "https://armsoft.ddnsto.com";
			if (!db_softcenter_["softcenter_version"]) {
				db_softcenter_["softcenter_version"] = "0.0";
			}
			$("#spnCurrVersion").html("<em>" + db_softcenter_["softcenter_version"] + "</em>");
			var EXT = '<% nvram_get("extendno"); %>';
			if (EXT.indexOf('koolshare') != -1){
				console.log("正在使用koolshare官改固件！");
			}else{
				console.log("正在使用koolshare梅林改版固件！");
				var jff2_scripts="<% nvram_get("jffs2_scripts"); %>";
				if(jff2_scripts != 1){
					$('#software_center_message').html('<h1><font color="#FF9900">错误！</font></h1><h2>软件中心不可用！</h2><h2>因为你没有开启Enable JFFS custom scripts and configs选项！</h2><h2>请前往【系统管理】-<a href="Advanced_System_Content.asp"><u><em>【系统设置】</em></u></a>开启此选项再使用软件中心！</h2><h2>如果你是官改固件，请尝试重置路由器以重新初始化软件中心！</h2><h2>需要更多帮助，请前往<a href="https://koolshare.cn"><em><u>https://koolshare.cn</u></em></a>koolshare论坛交流反馈</h2>')
					return false;
				}
			}
			initInstallStatus();
			init(function() {
				toggleAppPanel(1);
				//一刷新界面是否就正在插件在安装.
			});
			//挂接tab切换安装状态事件
			$('.show-install-btn').click(function() {
				init(function() {
					toggleAppPanel(1);
				});
			});
			$('.show-uninstall-btn').click(function() {
				init(function() {
					toggleAppPanel(0);
				});
			});
			//挂接安装或者卸载事件
			$('#IconContainer').on('click', '.install-btn', function() {
				var name = $(this).data('name');
				console.log('install', name);
				appInstallModule(softInfo[name]);
			});
			$('#IconContainer').on('click', '.uninstall-btn', function() {
				var name = $(this).data('name');
				console.log('uninstall', name);
				appUninstallModule(softInfo[name]);
			});
			$('#IconContainer').on('click', '.update-btn', function() {
				var name = $(this).data('name');
				console.log('update', name);
				appInstallModule(softInfo[name]);
			});
			//保留日志显示窗口
			$(".popup_bar_bg_ks").click(
				function() {
					count_down = -1;
				});
			//窗口大小调整
			$(window).resize(function(){
				if($('.content_status').css("visibility") == "visible"){
					document.scrollingElement.scrollTop = 0;
					var page_h = window.innerHeight || document.documentElement.clientHeight || document.body.clientHeight;
					var page_w = window.innerWidth || document.documentElement.clientWidth || document.body.clientWidth;
					var log_h = E("softcenter_log_pannel").clientHeight;
					var log_w = E("softcenter_log_pannel").clientWidth;
					var log_h_offset = (page_h - log_h) / 2;
					var log_w_offset = (page_w - log_w) / 2 + 90;
					$('#softcenter_log_pannel').offset({top: log_h_offset, left: log_w_offset});
				}
			});
			//挂接按钮功能
			$("#clean_log").click(
				function() {
					manipulate_softcenter_log("clean_log");
				});
			$("#download_log").click(
				function() {
					manipulate_softcenter_log("download_log");
				});
			$("#close_log").click(
				function() {
					close_softcenter_log();
				});
		}
	});
});
function menu_hook() {
	tabtitle[tabtitle.length - 1] = new Array("", "软件中心", "离线安装");
	tablink[tablink.length - 1] = new Array("", "Module_Softcenter.asp", "Module_Softsetting.asp");
}
function notice_show() {
	$.ajax({
		url: 'https://armsoft.ddnsto.com/softcenter/push_message.json.js',
		type: 'GET',
		dataType: 'jsonp',
		success: function(res) {
			$("#push_titile").html(res.title);
			$("#push_content1").html(res.content1);
			if (res.content2) {
				E("push_content2_li").style.display = "";
				$("#push_content2").html(res.content2);
			}
			if (res.content3) {
				E("push_content3_li").style.display = "";
				$("#push_content3").html(res.content3);
			}
			if (res.content4) {
				E("push_content4_li").style.display = "";
				$("#push_content4").html(res.content4);
			}
		}
	});
}
function count_down_close() {
	if (count_down == "0") {
		close_softcenter_log();
	}
	if (count_down < 0) {
		E("close_log").value = "关闭日志"
		return false;
	}
	E("close_log").value = "自动关闭（" + count_down + "）"
		--count_down;
	setTimeout("count_down_close();", 1000);
}
function close_softcenter_log() {
	E("softcenter_shade_pannel").style.visibility = "hidden";
	E("softcenter_log_pannel").style.visibility = "hidden";
	E("download_log").style.visibility = "hidden";
	E("close_log").style.visibility = "hidden";
	E("clean_log").style.visibility = "hidden";
	if (refresh_flag == "1"){
		refreshpage();
	}
	$(".show-install-btn").trigger("click");
}
function get_log(flag) {
	if (flag){
		E("download_log").style.visibility = "visible";
		E("close_log").style.visibility = "visible";
		E("clean_log").style.visibility = "visible";
		E("log_content").rows = "32";
		var LOG_FILE = '/_temp/soft_install_log_backup.txt';
	}else{
		E("download_log").style.visibility = "hidden";
		E("close_log").style.visibility = "hidden";
		E("clean_log").style.visibility = "hidden";
		E("log_content").rows = "26";
		var LOG_FILE = '/_temp/soft_install_log.txt';
	}
	document.scrollingElement.scrollTop = 0;
	var page_h = window.innerHeight || document.documentElement.clientHeight || document.body.clientHeight;
	var page_w = window.innerWidth || document.documentElement.clientWidth || document.body.clientWidth;
	var log_h = E("softcenter_log_pannel").clientHeight;
	var log_w = E("softcenter_log_pannel").clientWidth;
	var log_h_offset = (page_h - log_h) / 2;
	var log_w_offset = (page_w - log_w) / 2 + 90;
	$('#softcenter_log_pannel').offset({top: log_h_offset, left: log_w_offset});

	E("softcenter_shade_pannel").style.visibility = "visible";
	E("softcenter_log_pannel").style.visibility = "visible";
	
	$.ajax({
		url: LOG_FILE,
		type: 'GET',
		dataType: 'html',
		async: false,
		cache: false,
		success: function(response) {
			if (flag){
				E("softcenter_log_title").innerHTML = "<i>&nbsp;&nbsp;&nbsp;&nbsp;软件中心插件安装/卸载日志记录。</i>";
			}else{
				E("softcenter_log_title").innerHTML = "<i>&nbsp;&nbsp;&nbsp;&nbsp;插件安装/卸载过程中请勿刷新此页面！</i>";
			}
			if (response.search("XU6J03M6") != -1) {
				E("log_content").value = response.myReplace("XU6J03M6", " ");
				E("log_content").scrollTop = E("log_content").scrollHeight;
				E("close_log").style.visibility = "visible";
				if (flag){
					count_down = -1;
				}else{
					count_down = 5;
				}
				count_down_close();
				return true;
			}
			setTimeout("get_log(" + flag + ");", 300);
			E("log_content").value = response.myReplace("XU6J03M6", " ");
			E("log_content").scrollTop = E("log_content").scrollHeight;
		},
		error: function(xhr) {
			E("softcenter_log_title").innerHTML = "<i>&nbsp;&nbsp;&nbsp;&nbsp;暂无软件中心日志信息</i>";
			E("log_content").value = "日志文件为空，请关闭本窗口！";
			E("close_log").style.visibility = "visible";
			return false;
		}
	});
}
function manipulate_softcenter_log(arg) {
	var id = parseInt(Math.random() * 100000000);
	var postData = {"id": id, "method": "ks_app_install.sh", "params":[arg], "fields": "" };
	$.ajax({
		type: "POST",
		url: "/_api/",
		async: false,
		cache: false,
		data: JSON.stringify(postData),
		dataType: "json",
		success: function(response){
			if(response.result == id){
				if(arg == "download_log"){
					var b = document.createElement('A')
					b.href = "_root/files/softcenter_log.txt"
					b.download = 'softcenter_log.txt'
					document.body.appendChild(b);
					b.click();
					document.body.removeChild(b);
				}
				if(arg == "clean_log"){
					E("log_content").value = "日志已清除，请关闭本窗口！"
				}
			}
		},
		error: function(xhr) {
			alert("日志下载失败！");
			return false;
		}
	});
}
function ks_online() {
	require(['/res/layer/layer.js'], function(layer) {
		layer.open({
			type: 1,
			title: false,
			closeBtn: false,
			area: '860px;',
			shade: 0.8,
			shadeClose: 0,
			scrollbar: false,
			id: 'LAY_layuipro',
			btn: ['关闭窗口', '参加【Ks Online】计划'],
			btnAlign: 'c',
			moveType: 1,
			content: '<div style="padding: 50px; line-height: 22px; background-color: #393D49; color: #fff; font-weight: 300;">\
				<b>软件中心【Ks Online】计划</b><br><br>\
				<p>\
				向大家介绍一下我们最近做的一个项目，叫：Ks Online<br>\
				<br>\
				参加Ks Online计划后，会有一个叫ks_online.sh的脚本发送你路由器的型号，固件版本等信息到一个在线的数据库服务器。<br>\
				ks_online.sh脚本提交到服务器的信息是100%匿名的，并且只会被用于数据统计用途。<br>\
				<b>ks_online.sh脚本在任何情况下都不会搜集或发送你的隐私或者个人数据（比如：MAC地址，IP地址等信息）!</b><br>\
				ks_online.sh脚本是完全开放的，并且是纯shell脚本，其安装在固件的/koolshare/scripts目录下，所有人都可以自由的查看自己路由器中该脚本将要搜集和发送到数据库的信息。<br>\
				<\p>\
				<br>\
				<p>\
				ks_online.sh脚本搜集的数据可以在此<a style="color:#e7bd16" target="_blank" href="https://ksonline.ddnsto.com"><u>Ks Online 统计页面</u></a>查看到。<br>\
				该信息可以在一定程度上帮助你选择最佳的路由器版本，或者帮助需要购买路由器的朋友了解到当前最流行的型号。<br>\
				因为你可以直观的看到使用人数最多的固件版本、路由器型号，也可以根据开机时间、路由器温度等信息找到你认为稳定的型号。<br>\
				如果你对上传并共享你的路由器相关信息感到不满，你可以随时在此页面关闭【Ks Online】功能。<br>\
				当然你也可以在任何时候重新开启【Ks Online】。<br>\
				<\p>\
				<br>\
				<p>\
				以下是ks_online.sh脚本将会搜集并发送的全部信息：<br>\
				<\p>\
				<ul>\
				<li>LAN MAC地址的md5校验值 - 用以提供路由器的唯一识别码, 例如: fab3201b83137711623c97504bd8b51c</li>\
				<li>路由器型号, 例如: RT-AX86U</li>\
				<li>固件版本号, 例如: 384_9262_koolshare</li>\
				<li>国家或地区, 例如: 上海市</li>\
				<li>路由开机时间, 例如: 3天</li>\
				<li>路由CPU温度, 例如: 69℃</li>\
				</ul>\
				对【Ks Online】计划有任何建议或者问题，可以前往<a style="color:#e7bd16" target="_blank" href="https://ksonline.ddnsto.com"><u>【Ks Online】计划讨论贴</u></a>反馈~<br><br>\
				我们的征途是星辰大海 ^_^</div>'
		});
	});
}
</script>
</head>
<body>
	<div id="TopBanner"></div>
	<div id="Loading" class="popup_bg"></div>
	<div id="softcenter_shade_pannel" class="popup_bar_bg_ks">
		<!-- this is the popup area for install/uninstall log status -->
		<div id="softcenter_log_pannel" class="content_status">
			<div class="user_title">软件中心 - 日志记录</div>
			<div style="margin-left:15px" id="softcenter_log_title"></div>
			<div style="margin: 10px 10px 10px 10px;width:98%;text-align:center;overflow:hidden;">
				<textarea cols="63" rows="25" wrap="on" readonly="readonly" id="log_content" autocomplete="off" autocorrect="off" autocapitalize="off" spellcheck="false"></textarea>
			</div>
			<div style="margin-top:5px;padding-bottom:10px;width:100%;text-align:center;">
				<input class="button_gen" type="button" style="visibility:hidden;min-width:88px;" id="download_log" value="下载日志">
				<input class="button_gen" type="button" style="visibility:hidden;min-width:88px;margin-left: 10px;" id="close_log" value="关闭日志">
				<input class="button_gen" type="button" style="visibility:hidden;min-width:88px;margin-left: 10px;" id="clean_log" value="清空日志">
			</div>
		</div>
	</div>
	<table class="content" align="center" cellpadding="0" cellspacing="0">
		<tr>
			<td width="17">&nbsp;</td>
			<td valign="top" width="202">
				<div id="mainMenu"></div>
				<div id="subMenu"></div>
			</td>
			<td valign="top">
				<div id="tabMenu" class="submenuBlock"></div>
					<table width="98%" border="0" align="left" cellpadding="0" cellspacing="0">
						<tr>
							<td align="left" valign="top">
								<div>
									<table width="760px" border="0" cellpadding="5" cellspacing="0" bordercolor="#6b8fa3" class="FormTitle" id="FormTitle">
										<tr>
											<td id="main_td" bgcolor="#4D595D" colspan="3" valign="top">
												<div>&nbsp;</div>
												<div id="title_name" class="formfonttitle" style="float: left;"></div>
												<script type="text/javascript">
													var MODEL = '<% nvram_get("odmpid"); %>' || '<% nvram_get("productid"); %>';
													$("#title_name").html("Software Center " + MODEL)
												</script>
												<div align="right" style="padding-right:10px;">
													<a type="button" class="ks_btn" href="javascript:void(0);" onclick="get_log(1)">查看日志</a>
													<!--<a type="button" class="ks_btn" style="margin-left: 5px;" href="javascript:void(0);" onclick="ks_online()">ks online</a>-->
												</div>
												<div style="margin:10px 0 10px 5px;" class="splitLine"></div>
													<table id="ks_info" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
													</table>
													<table id="ks_pannel" width="100%" height="150px" style="border-collapse:collapse;">
														<tr bgcolor="#444f53">
															<td colspan="5" bgcolor="#444f53" class="cloud_main_radius">
																<div style="width:95%;font-style:italic;font-size:14px;">
																	<table width="100%">
																		<tr>
																			<td>
																				<ul style="padding-left:25px;">
																					<h2 id="push_titile"><em>软件中心&nbsp;-&nbsp;by&nbsp;koolshare</em></h2>
																					<li>
																						<h4 id="push_content1" ><font color='#1E90FF'>交流反馈:&nbsp;&nbsp;</font><a href='https://github.com/koolshare/armsoft' target='_blank'><em>1.软件中心GitHub项目</em></a>&nbsp;&nbsp;&nbsp;&nbsp;<a href='https://t.me/xbchat' target='_blank'><em>2.加入telegram群</em></a>&nbsp;&nbsp;&nbsp;&nbsp;<a href='http://shang.qq.com/wpa/qunwpa?idkey=f475468129ba8019245425559b5df5bdad7d7201ac7780417dd0218bbb4e1322' target='_blank'><em>3.加入QQ群</em></a>&nbsp;&nbsp;&nbsp;&nbsp;<a href='http://koolshare.cn/forum-98-1.html' target='_blank'><em>4.Koolshare论坛插件版块</em></a></h4>
																					</li>
																					<li id="push_content2_li" style="display: none;">
																						<h4 id="push_content2"></h4>
																					</li>
																					<li id="push_content3_li" style="display: none;">
																						<h4 id="push_content3"></h4>
																					</li>
																					<li id="push_content4_li" style="display: none;">
																						<h4 id="push_content4"></h4>
																					</li>
																					<li>
																						<h5>当前版本：<span id="spnCurrVersion"></span>&nbsp;&nbsp;&nbsp;&nbsp;在线版本：<span id="spnOnlineVersion"></span>
																						<input type="button" id="updateBtn" value="更新" style="display:none" /></h5>
																					</li>
																				</ul>
																			</td>
																		</tr>
																	</table>
																</div>
															</td>
														</tr>
														<tr height="10px">
															<td colspan="3"></td>
														</tr>
														<tr height="10px">
															<td colspan="3"></td>
														</tr>
														<tr width="235px">
															<td colspan="4" cellpadding="0" cellspacing="0" style="padding:0">
																<input class="show-install-btn" type="button" value="已安装"/>
																<input class="show-uninstall-btn" type="button" value="未安装"/>
															</td>
														</tr>
														<tr bgcolor="#444f53" width="235px">
															<td colspan="4" id="IconContainer">
																<div id="software_center_message" style="text-align:center; line-height: 4em;">更新中...</div>
															</td>
														</tr>
														<tr height="10px">
															<td colspan="3"></td>
														</tr>
													</table>
												<div id="ks_logo" class="KoolshareBottom">
													论坛技术支持: <a href="https://koolshare.cn" target="_blank"> <i><u>https://koolshare.cn</u></i></a><br />
													GitHub: <a href="https://github.com/koolshare/armsoft" target="_blank"><i><u>https://github.com/koolshare</u></i></a><br />
													Shell & Web by: <a href="mailto:sadoneli@gmail.com"><i>sadoneli</i></a>, <i>Xiaobao</i>
												</div>
											</td>
										</tr>
								</table>
							</div>
						</td>
					</tr>
				</table>
			</td>
			<td width="10" align="center" valign="top"></td>
		</tr>
	</table>
<div id="footer"></div>
</body>
</html>
