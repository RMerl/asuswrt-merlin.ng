<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="X-UA-Compatible" content="IE=Edge">
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title><#Network_Tools#> - <#Network_Analysis#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="/js/table/table.css">
<style>
#ClientList_Block_PC{
	border:1px outset #999;
	background-color:#576D73;
	position:absolute;
	*margin-top:26px;	
	margin-left:2px;
	*margin-left:-353px;
	width:346px;
	text-align:left;	
	height:auto;
	overflow-y:auto;
	z-index:200;
	padding: 1px;
	display:none;
}
#ClientList_Block_PC div{
	background-color:#576D73;
	height:auto;
	*height:20px;
	line-height:20px;
	text-decoration:none;
	font-family: Lucida Console;
	padding-left:2px;
}

#ClientList_Block_PC a{
	background-color:#EFEFEF;
	color:#FFF;
	font-size:12px;
	font-family:Arial, Helvetica, sans-serif;
	text-decoration:none;	
}
#ClientList_Block_PC div:hover{
	background-color:#3366FF;
	color:#FFFFFF;
	cursor:default;
}	
</style>
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script language="JavaScript" type="text/javascript" src="/help.js"></script>
<script language="JavaScript" type="text/javascript" src="/validator.js"></script>
<script language="JavaScript" type="text/javascript" src="/js/jquery.js"></script>
<script type="text/javascript" language="JavaScript" src="/js/table/table.js"></script>
<script>
	//set table Struct
	var tableStruct = {
		data: [["-", "-", "-", "-"]],
		container: "tableContainer",
		capability: {
			clickEdit: true
		},
		header: [ 
			{
				"title" : "<#NetworkTools_target#>",
				"width" : "30%"
			},
			{
				"title" : "<#Average#>",
				"width" : "30%"
			},
			{
				"title" : "<#Loss#>",
				"width" : "20%"
			},
			{
				"title" : "<#Jitter#>",
				"width" : "20%"
			}
		],

		clickRawEditPanel: {
			inputs : [
				{
					"editMode" : "callBack",
					"callBackFun" : controlClickEvent,
					"className" : "Target",
					"styleList" :  {"cursor":"pointer"}
				},
				{
					"editMode" : "callBack",
					"callBackFun" : controlClickEvent,
					"styleList" :  {"cursor":"pointer"}
				},
				{
					"editMode" : "callBack",
					"callBackFun" : controlClickEvent,
					"styleList" :  {"cursor":"pointer"}
				},
				{
					"editMode" : "callBack",
					"callBackFun" : controlClickEvent,
					"styleList" : {"cursor":"pointer"}
				}
			]
		}
	};
function initial(){
	show_menu();
	showLANIPList();

	$("#tableContainer1").empty();
	tableApi.genTableAPI(tableStruct);
	hideCNT(1); // default select cmdMethod = 1
}

function controlClickEvent(_$this) {
	var idx = parseInt(_$this.closest("*[row_tr_idx]").attr( "row_tr_idx" ));
	var thisTarget = htmlEnDeCode.htmlDecode($("tr[row_tr_idx='" + idx + "']").find($(".Target")).find($(".static-text"))[0].innerHTML);
	if(thisTarget == "-")
		return false;

	$("#destIP").val(thisTarget);

	netoolApi.stopAll();
	netoolApi.render(thisTarget);

	targetData[thisTarget].isDoing = true;
	netoolApi.start({
		"type": $("#cmdMethod").val(),
		"target": thisTarget
	});
}

var targetData = {}

function initTargetData(){
	var retObj = {};
	retObj["points"] = [];
	retObj["sum"] = 0;
	retObj["avg"] = 0;
	retObj["pingMax"] = 0;
	retObj["pingMin"] = 9999;
	retObj["jitter"] = 0;
	retObj["loss"] = 0;
	retObj["max"] = 0;
	retObj["min"] = 9999;
	retObj["isDoing"] = true;
	return retObj;
}

var netoolApi = {
	start: function(obj){
//		$("#loadingIcon").fadeIn(500);
		$("#loadingIcon").hide();
		
		if(!targetData[obj.target]){
			targetData[obj.target] = new initTargetData();
		}

		$.getJSON("/netool.cgi", obj)
			.done(function(data){
				if(data.successful != "0") setTimeout(function(){
					netoolApi.check(obj, data.successful)
				}, 1000)
			})
			.fail(function(e){
				netoolApi.render(obj.target)
			});
	},

	check: function(obj, fileName){
		$.getJSON("/netool.cgi", {"type":0,"target":fileName})
			.done(function(data){
//				$("#loadingIcon").fadeOut(500);

				var thisTarget = targetData[obj.target];
				var pingVal = (data.result[0].ping !== "") ? parseFloat(data.result[0].ping) : 0;

				thisTarget.isDoing = (thisTarget.points.length > 240) ? false : thisTarget.isDoing;
				thisTarget.points.push(pingVal);
				thisTarget.pingMax = (thisTarget.pingMax > pingVal) ? thisTarget.pingMax : pingVal;
				thisTarget.pingMin = (thisTarget.pingMin < pingVal) ? thisTarget.pingMin : pingVal;
				thisTarget.sum += pingVal;
				thisTarget.avg = (thisTarget.sum/thisTarget.points.length).toFixed(3);
				thisTarget.jitter = Math.abs(thisTarget.pingMax - thisTarget.pingMin).toFixed(3);
				thisTarget.loss += (parseInt(data.result[0].loss) / 100);

				var gap = parseInt(thisTarget.jitter/4) + 2;
				thisTarget.min = parseInt(thisTarget.pingMin/gap)*gap;
				thisTarget.max = thisTarget.min + gap*4;

				if(thisTarget.isDoing){
					netoolApi.render(obj.target);
					netoolApi.start(obj);
				}
			})
			.fail(function(data){
				setTimeout(function(){
					netoolApi.check(obj, fileName);
				}, 500);
			});
	},

	render: function(target){
		var thisTarget = targetData[target];

		var toPosition = function(point){
			return (250-((point-thisTarget.min)/(thisTarget.max-thisTarget.min))*250);
		}

		// graph
		$(".yAxis")
			.each(function(id){
				$(this).html(thisTarget.min + (thisTarget.max-thisTarget.min)*id/4 + " ms")
			})

		$("#ping_graph")
			.attr("points", function(){
				return thisTarget.points
					.map(function(el, id){return ((id*3) + "," + toPosition(el));})
					.join(" ");
			});

		$("#ping_avg_graph")
			.attr("points", "0," + toPosition(thisTarget.avg) + " 730," + toPosition(thisTarget.avg));
		
		// table
		$("#tableContainer").empty();
		tableStruct.data = function() {
			var retTable = [];
			for(var i in targetData) {
				var eachRuleArray = new Array();
				eachRuleArray.push(i);
				eachRuleArray.push(targetData[i].avg + " ms");
				eachRuleArray.push(targetData[i].loss);
				eachRuleArray.push(targetData[i].jitter + " ms");
				retTable.push(eachRuleArray);
			}
			return retTable;
		}();
		tableApi.genTableAPI(tableStruct);
	},

	reset: function(obj){
		netoolApi.stopAll();
		targetData[obj.target] = new initTargetData();
		$("#ping_graph").attr("points", "0,250");
		$("#ping_avg_graph").attr("points", "0,250");
	},

	stopAll: function(){
		for(var i in targetData){
			targetData[i].isDoing = false;
		}
	},

	startText: function(obj){
		$("#loadingIcon").fadeIn(500);

		$.getJSON("/netool.cgi", obj)
			.done(function(data){
				$("#loadingIcon").show();
				$("#cmdBtn").hide();

				if(data.successful != "0") setTimeout(function(){
					netoolApi.checkText(data.successful)
				}, 1000)
			})
			.fail(function(e){
				netoolApi.showText('Fail to start')
			});
	},

	checkText: function(fileName){
		$.get("/netool.cgi", {"type":0, "target": fileName})
			.done(function(data){
				netoolApi.showText(data)

				if(data.search("XU6J03M6") == -1){
					setTimeout(function(){
						netoolApi.checkText(fileName);
					}, 500);				
				}
				else{
					$("#loadingIcon").hide();
					$("#cmdBtn").show();
				}
			})
			.fail(function(e){
				netoolApi.showText('Fail to get data')
			});
	},

	showText: function(content){
		$("#textarea").val(content.replace("XU6J03M6", "").replace('{"result":', "").replace('}', ""));
	}	
}

function hideCNT(_val){
	if(_val == "3"){
		document.getElementById("pingCNT_tr").style.display = "";
		document.getElementById("cmdDesc").innerHTML = "<#NetworkTools_Ping#>";
	}
	else if(_val == "4"){
		document.getElementById("pingCNT_tr").style.display = "none";
		document.getElementById("cmdDesc").innerHTML = "<#NetworkTools_tr#>";
	}
	else{
		document.getElementById("pingCNT_tr").style.display = "none";
		document.getElementById("cmdDesc").innerHTML = "<#NetworkTools_nslookup#>";
	}

	if(_val == 1){
		$("#logArea").fadeOut(300);
		setTimeout(function(){
			$("#graphArea").fadeIn(300);
		}, 300);
	}
	else{
		$("#graphArea").fadeOut(300);
		setTimeout(function(){
			$("#logArea").fadeIn(300);
		}, 300);
	}	
}

function showLANIPList(){
	var AppListArray = [
		["Google ", "www.google.com"], ["Facebook", "www.facebook.com"], ["Youtube", "www.youtube.com"], ["Yahoo", "www.yahoo.com"],
		["Baidu", "www.baidu.com"], ["Wikipedia", "www.wikipedia.org"], ["Windows Live", "www.live.com"], ["QQ", "www.qq.com"],
		["Twitter", "www.twitter.com"], ["Taobao", "www.taobao.com"], ["Blogspot", "www.blogspot.com"], 
		["Linkedin", "www.linkedin.com"], ["eBay", "www.ebay.com"], ["Bing", "www.bing.com"], 
		["Яндекс", "www.yandex.ru"], ["WordPress", "www.wordpress.com"], ["ВКонтакте", "www.vk.com"]
	];

	var code = "";
	for(var i = 0; i < AppListArray.length; i++){
		code += '<a><div onmouseover="over_var=1;" onmouseout="over_var=0;" onclick="setClientIP(\''+AppListArray[i][1]+'\');"><strong>'+AppListArray[i][0]+'</strong></div></a>';
	}
	code +='<!--[if lte IE 6.5]><iframe class="hackiframe2"></iframe><![endif]-->';	
	document.getElementById("ClientList_Block_PC").innerHTML = code;
}

function setClientIP(ipaddr){
	document.form.destIP.value = ipaddr;
	hideClients_Block();
	over_var = 0;
}

var over_var = 0;
var isMenuopen = 0;
function hideClients_Block(){
	document.getElementById("pull_arrow").src = "/images/arrow-down.gif";
	document.getElementById('ClientList_Block_PC').style.display='none';
	isMenuopen = 0;
}

function pullLANIPList(obj){
	if(isMenuopen == 0){		
		obj.src = "/images/arrow-top.gif"
		document.getElementById("ClientList_Block_PC").style.display = 'block';		
		document.form.destIP.focus();		
		isMenuopen = 1;
	}
	else{
		hideClients_Block();
	}
}

validator.targetDomainName = function($o){
	var str = $o.val();

	if(str == ""){
		$o.val("www.google.com");
	}

	if (!validator.string($o[0])){
		return false;
	}
		
	for(i=0;i<str.length;i++){
		c = str.charCodeAt(i);
		
		if (!validator.hostNameChar(c)){
			$("#alert_block").html("<#LANHostConfig_x_DDNS_alarm_13#> '" + str.charAt(i) +"' !").show();
			return false;
		}
	}
		
	$("#alert_block").hide();
	return true;
}
</script>
</head>
<body onload="initial();" class="bg">
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>
<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="POST" name="form" action="/apply.cgi" target="hidden_frame">
<input type="hidden" name="current_page" value="Main_Analysis_Content.asp">
<input type="hidden" name="next_page" value="Main_Analysis_Content.asp">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="action_wait" value="">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="SystemCmd" value="">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">

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
						<table width="760px" border="0" cellpadding="5" cellspacing="0" bordercolor="#6b8fa3"  class="FormTitle" id="FormTitle">		
							<tr>
								<td bgcolor="#4D595D" colspan="3" valign="top">
									<div>&nbsp;</div>
									<div class="formfonttitle"><#Network_Tools#> - <#Network_Analysis#></div>
									<div style="margin: 10px 0 10px 5px;" class="splitLine"></div>
									<div class="formfontdesc" id="cmdDesc"><#NetworkTools_Ping#></div>
									<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
										<tr>
											<th width="20%"><#NetworkTools_Method#></th>
											<td>
												<select id="cmdMethod" class="input_option" name="cmdMethod" onchange="hideCNT(this.value);">
													<option value="1" selected>Ping (<#dualwan_pingtime_detect_continuous#>)</option>
													<option value="3">Ping</option>
													<option value="4">Traceroute</option>
													<option value="5">Nslookup</option>
 												</select>
											</td>										
										</tr>
										<tr>
											<th width="20%"><#NetworkTools_target#></th>
											<td>
												<input type="text" class="input_32_table" id="destIP" name="destIP" maxlength="100" value="" placeholder="ex: www.google.com" autocorrect="off" autocapitalize="off">
												<img id="pull_arrow" height="14px;" src="/images/arrow-down.gif" style="position:absolute;*margin-left:-3px;*margin-top:1px;" onclick="pullLANIPList(this);" title="<#select_network_host#>" onmouseover="over_var=1;" onmouseout="over_var=0;">
												<div id="ClientList_Block_PC" class="ClientList_Block_PC"></div>
												<br/>
												<span id="alert_block" style="color:#FC0;display:none"></span>
											</td>
										</tr>
										<tr id="pingCNT_tr" style="display:none">
											<th width="20%"><#NetworkTools_Count#></th>
											<td>
												<input type="text" id="pingCNT" name="pingCNT" class="input_3_table" maxlength="2" value="" onKeyPress="return validator.isNumber(this, event);" placeholder="5" autocorrect="off" autocapitalize="off">
											</td>
										</tr>
									</table>

									<div class="apply_gen" style="height:40px">
										<input class="button_gen" id="cmdBtn" type="button" value="<#NetworkTools_Diagnose_btn#>">
										<script>
											$("#cmdBtn")
												.click(function(){
													if(!validator.targetDomainName($("#destIP"))){
														return false;
													}

													var targetObj = {
														"type": $("#cmdMethod").val(), 
														"target": $("#destIP").val(),
														"pcnt": $("#pingCNT").val()
													}

													if($("#cmdMethod").val() == 1){
														netoolApi.reset(targetObj);
														netoolApi.start(targetObj);
													}
													else{
														$("#textarea").val("");
														netoolApi.stopAll();
														netoolApi.startText(targetObj);											
													}
												})
										</script>
										<img id="loadingIcon" style="display:none;" src="/images/InternetScan.gif">
									</div>

									<div id="graphArea">
										<div id="svgContainer" style="margin:0px 11px 0px 11px;background-color:black;">
											<svg width="730px" height="250px">
												<g>
													<line stroke-width="1" stroke-opacity="1"   stroke="rgb(255,255,255)" x1="0" y1="0%"   x2="100%" y2="0%" />
													<line stroke-width="1" stroke-opacity="0.2" stroke="rgb(255,255,255)" x1="0" y1="25%"  x2="100%" y2="25%" />
													<line stroke-width="1" stroke-opacity="0.2" stroke="rgb(255,255,255)" x1="0" y1="50%"  x2="100%" y2="50%" />
													<line stroke-width="1" stroke-opacity="0.2" stroke="rgb(255,255,255)" x1="0" y1="75%"  x2="100%" y2="75%" />
													<line stroke-width="1" stroke-opacity="1"   stroke="rgb(255,255,255)" x1="0" y1="100%" x2="100%" y2="100%" />
												</g>							
												<g>
													<text class="yAxis" font-family="Verdana" fill="#FFFFFF" font-size="8" x="0" y="98%">0 ms</text>
													<text class="yAxis" font-family="Verdana" fill="#FFFFFF" font-size="8" x="0" y="78%">25 ms</text>
													<text class="yAxis" font-family="Verdana" fill="#FFFFFF" font-size="8" x="0" y="55%">50 ms</text>
													<text class="yAxis" font-family="Verdana" fill="#FFFFFF" font-size="8" x="0" y="28%">75 ms</text>
													<text class="yAxis" font-family="Verdana" fill="#FFFFFF" font-size="8" x="0" y="5%">100 ms</text>
												</g>							

												<polyline id="ping_avg_graph" style="fill:none;stroke:#FC0;stroke-width:1;" points="0,250"></polyline>
												<polyline id="ping_graph" style="fill:none;stroke:#00FE00;stroke-width:1;z-index:9999" points="0,250"></polyline>
											</svg>
										</div>
										
										<div id="tableContainer" style="width:730px;margin-left:10px"></div>							
									</div>

									<div style="margin-top:8px" id="logArea">
										<textarea cols="63" rows="27" wrap="off" readonly="readonly" id="textarea" class="textarea_ssh_table" style="width:99%;font-family:Courier New, Courier, mono; font-size:11px;">
											<% nvram_dump("syscmd.log","syscmd.sh"); %>
										</textarea>
										<script type="text/javascript">
										<!--[if !IE]>-->
											(function($){
												var textArea = document.getElementById('textarea');
												textArea.scrollTop = textArea.scrollHeight;
											})(jQuery);
										<!--<![endif]-->
										</script>
									</div>

								</td>
							</tr>
						</table>
					</td>
				</tr>
			</table>
			<!--===================================Ending of Main Content===========================================-->
		</td>
		<td width="10" align="center" valign="top"></td>
	</tr>
</table>
</form>

<div id="footer"></div>
</body>
</html>
