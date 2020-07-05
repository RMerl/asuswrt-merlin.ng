<!DOCTYPE html
	PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>

<head>
	<meta http-equiv="X-UA-Compatible" content="IE=Edge" />
	<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
	<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
	<meta HTTP-EQUIV="Expires" CONTENT="-1">
	<link rel="shortcut icon" href="images/favicon.png">
	<link rel="icon" href="images/favicon.png">
	<title>
		<#Web_Title#> - USB2JFFS
	</title>
	<link rel="stylesheet" type="text/css" href="index_style.css">
	<link rel="stylesheet" type="text/css" href="form_style.css">
	<link rel="stylesheet" type="text/css" href="/js/table/table.css">
	<script type="text/javascript" src="/state.js"></script>
	<script type="text/javascript" src="/general.js"></script>
	<script type="text/javascript" src="/popup.js"></script>
	<script type="text/javascript" src="/help.js"></script>
	<script type="text/javascript" src="/tmhist.js"></script>
	<script type="text/javascript" src="/tmmenu.js"></script>
	<script type="text/javascript" src="/client_function.js"></script>
	<script type="text/javascript" src="/js/jquery.js"></script>
	<script type="text/javascript" src="/switcherplugin/jquery.iphone-switch.js"></script>
	<script type="text/javascript" src="/js/table/table.js"></script>
	<style>
		.ks_btn {
			border: 1px solid #222;
			font-size:10pt;
			color: #fff;
			padding: 5px 5px 5px 5px;
			border-radius: 5px 5px 5px 5px;
			width:14%;
			vertical-align: middle;
			background: linear-gradient(to bottom, #003333  0%, #000000 100%);
		}
		.ks_btn:hover {
			border: 1px solid #222;
			font-size:10pt;
			color: #fff;
			padding: 5px 5px 5px 5px;
			border-radius: 5px 5px 5px 5px;
			width:14%;
			vertical-align: middle;
			background: linear-gradient(to bottom, #27c9c9  0%, #279fd9 100%);
		}
		.show-btn1, .show-btn2 {
			font-size:10pt;
			color: #fff;
			padding: 10px 3.75px;
			border-radius: 5px 5px 0px 0px;
			width:8.42%;
			border-left: 1px solid #67767d;
			border-top: 1px solid #67767d;
			border-right: 1px solid #67767d;
			border-bottom: none;
			background: #67767d;
		}
		.show-btn1:hover, .show-btn2:hover, .active {
			border: 1px solid #2f3a3e;
			background: #2f3a3e;
		}
		input[type=button]:focus {
			outline: none;
		}
	</style>
	<script>
		var mounted;
		var flag_total = 0;

		function applyRule() {
			showLoading();
			document.form.submit();
		}

		function actions(flag) {
			switch (flag) {
				case 1:
					FormActions("start_apply.htm", "toolscript", "usb2jffs.sh mount", "3");
					break;
				case 2:
					FormActions("start_apply.htm", "toolscript", "usb2jffs.sh unmount", "3");
					break;
				case 3:
					if(!confirm('注意：此操作会删除U盘里的.usb_jffs文件夹并卸载，其它文件不受影响！\n\n确定删除吗？')){
						return false;
					} else {
						FormActions("start_apply.htm", "toolscript", "usb2jffs.sh remove", "3");
					}
					break;
				case 4:
					FormActions("start_apply.htm", "toolscript", "usb2jffs.sh sync", "3");
					break;
				case 5:
					FormActions("start_apply.htm", "toolscript", "usb2jffs.sh setjob", "1");
					break;
				default:
			}
			applyRule();
		}

		function get_disks() {
			require(['/require/modules/diskList.js'], function (diskList) {
				var usb_path = [<% show_usb_path(); %>][0];
				var html = '';
				html += '<thead>'
				html += '<tr>'
				html += '<td colspan="8">磁盘分区列表</td>'
				html += '</tr>'
				html += '</thead>'
				html += '<tr>'
				html += '<th style="width:auto">端口</th>'
				html += '<th style="width:auto">名称</th>'
				html += '<th style="width:auto">大小</th>'
				html += '<th style="width:auto">已用</th>'
				html += '<th style="width:auto">权限</th>'
				html += '<th style="width:auto">格式</th>'
				html += '<th style="width:auto">挂载点</th>'
				html += '<th style="width:auto">路径</th>'
				html += '</tr>'
				if (usb_path == "") {
					E("_mount_status").innerHTML = "USB没有插入存储器";
					$('.bt1').attr("disabled", "disabled");
				} else {
					usbDevicesList = diskList.list();
					for (var i = 0; i < usbDevicesList.length; ++i) {
						for (var j = 0; j < usbDevicesList[i].partition.length; ++j) {
							//append options
							if (usbDevicesList[i].partition[j].status != "unmounted") {
								if (usbDevicesList[i].partition[j].format.indexOf("ext3") != -1
								 || usbDevicesList[i].partition[j].format.indexOf("ext4") != -1
								 || usbDevicesList[i].partition[j].format.indexOf("thfsplus") != -1)
								{
									flag_total = 1;
								}
							}
							var totalsize = ((usbDevicesList[i].partition[j].size) / 1000000).toFixed(2);
							var usedsize = ((usbDevicesList[i].partition[j].used) / 1000000).toFixed(2);
							var usedpercent = (usedsize / totalsize * 100).toFixed(2) + " %";
							var used = usedsize + " GB" + " (" + usedpercent + ")"
							html += '<tr>'
							html += '<td>' + usbDevicesList[i].usbPath + '</td>'
							html += '<td>' + usbDevicesList[i].deviceName + '</td>'
							html += '<td>' + totalsize + " GB" + '</td>'
							html += '<td>' + used + '</td>'
							html += '<td>' + usbDevicesList[i].partition[j].status + '</td>'
							html += '<td>' + usbDevicesList[i].partition[j].format + '</td>'
							html += '<td>' + usbDevicesList[i].partition[j].mountPoint + '</td>'
							html += '<td>' + '/tmp/mnt/' + usbDevicesList[i].partition[j].partName + '</td>'
							html += '</tr>'
						}
					}

					if (flag_total == 0) {
						E("_mount_status").innerHTML = "存储器格式不正确";
						$('.bt1').attr("disabled", "disabled");
					}
				}
				$('#disk_table').html(html);
			});
		}

		function select_tablet(w) {
			for (var i = 1; i <= 2; i++) {
				$('.show-btn' + i).removeClass('active');
				$('#tablet_' + i).hide();
			}
			$('.show-btn' + w).addClass('active');
			$('#tablet_' + w).show();
			if (document.form.usb2jffs_enable.value == 1) {
				if (mounted == 1 && w == 1) {
					$('.sbt0').hide();
					$('.sbt1').show();
				}
				if (mounted == 0 && w == 1) {
					$('.sbt0').show();
					$('.sbt1').hide();
				}
			} else {
				$('.tr1').hide();
			}
		}

		function show_status() {
			E("_jffs_status").innerHTML = mem_stats_arr[7];
			if (usb2jffs_mount == 1) {
				E("_mount_status").innerHTML = "已挂载";
			} else if (flag_total == 1) {
				E("_mount_status").innerHTML = "可挂载";
			}
		}

		function init() {
			show_menu();
			update_sysinfo();
			select_tablet(1);
		}

		function update_sysinfo(e) {
			$.ajax({
				url: '/ajax_sysinfo.asp',
				dataType: 'script',
				error: function (xhr) {
					setTimeout("update_sysinfo();", 1000);
				},
				success: function (response) {
					get_disks();
					show_status();
					setTimeout("update_sysinfo();", 3000);
				}
			});
		}
	</script>
</head>

<body onload="init();" onunLoad="return unload_body();">
	<div id="TopBanner"></div>
	<div id="Loading" class="popup_bg"></div>
	<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
	<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
		<input type="hidden" name="current_page" value="Tools_Usb2Jffs.asp" />
		<input type="hidden" name="next_page" value="" />
		<input type="hidden" name="group_id" value="" />
		<input type="hidden" name="modified" value="0" />
		<input type="hidden" name="action_mode" value="apply_new" />
		<input type="hidden" name="action_script" value="" />
		<input type="hidden" name="action_wait" value="3" />
		<input type="hidden" name="first_time" value="" />
		<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>" />
		<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>" />
		<input type="hidden" name="usb2jffs_enable" value="<% nvram_get("usb2jffs_enable"); %>">
		<input type="hidden" name="usb2jffs_rsync" value="<% nvram_get("usb2jffs_rsync"); %>">
		<input type="hidden" name="usb2jffs_sync_time" value="<% nvram_get("usb2jffs_sync_time"); %>">

		<table class="content" align="center" cellpadding="0" cellspacing="0">
			<tr>
				<td width="17">&nbsp;</td>
				<td valign="top" width="202">
					<div id="mainMenu"></div>
					<div id="subMenu"></div>
				</td>

				<td valign="top">
					<div id="tabMenu" class="submenuBlock"></div>
					<!--===================================Beginning of Main Content===========================================-->
					<table width="98%" border="0" align="left" cellpadding="0" cellspacing="0">
						<tr>
							<td align="left" valign="top">
								<table width="760px" border="0" cellpadding="4" cellspacing="0" class="FormTitle" id="FormTitle">
									<tr>
										<td bgcolor="#4D595D" valign="top">
											<div>&nbsp;</div>
											<div class="formfonttitle">
												<#Tools#> - USB2JFFS
											</div>
											<div style="margin:10px 0 10px 5px;" class="splitLine"></div>
											<div class="formfontdesc">
												轻松使用U盘挂载系统JFFS分区。
											</div>
											<div id="usb2jffs_disks_status" style="margin:10px 0px 0px 0px;">
												<table id="disk_table" width="100%" border="1" align="center" cellpadding="4"
													cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
													<thead>
														<tr>
															<td colspan="8">磁盘分区列表</td>
														</tr>
													</thead>
													<tr>
														<th style="width:auto">端口</th>
														<th style="width:auto">名称</th>
														<th style="width:auto">大小</th>
														<th style="width:auto">已用</th>
														<th style="width:auto">权限</th>
														<th style="width:auto">格式</th>
														<th style="width:auto">挂载点</th>
														<th style="width:auto">路径</th>
													</tr>
												</table>
											</div>
											<div id="usb2jffs_mount_status" style="margin:10px 0px 0px 0px;">
												<table id="status_pannel" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
													<thead>
														<tr>
															<td colspan="2">状态</td>
														</tr>
													</thead>
													<tr>
														<th><#JFFS#></th>
														<td id="_jffs_status"><% sysinfo("jffs.usage"); %></td>
													</tr>
													<tr>
														<th>挂载状态</th>
														<td><span id="_mount_status">正在检测......</span></td>
													</tr>
												</table>
											</div>

											<div class="apply_gen">
												<input style="cursor:pointer" type="button" class="button_gen" value="刷新" onclick="refreshpage();"/>
											</div>

											<div id="tablet_show">
												<table style="margin:10px 0px 0px 0px;border-collapse:collapse" width="100%" height="37px">
													<tr>
														<td cellpadding="0" cellspacing="0" style="padding:0" border="1" bordercolor="#222">
															<input id="show_btn1" class="show-btn1" style="cursor:pointer" type="button" value="操作" onclick="select_tablet(1);"/>
															<input id="show_btn2" class="show-btn2" style="cursor:pointer" type="button" value="帮助" onclick="select_tablet(2);"/>
														</td>
													</tr>
												</table>
											</div>

											<div id="tablet_1">
												<table id="usb2jffs_config" width="100%" border="0" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
													<tr>
														<th>启用USB2JFFS</th>
														<td>
															<div align="center" class="left" style="width:94px; float:left; cursor:pointer;" id="usb2jffs_enable"></div>
															<div class="iphone_switch_container" style="height:32px; width:74px; position: relative; overflow: hidden">
																<script type="text/javascript">
																	$('#usb2jffs_enable').iphoneSwitch('<% nvram_get("usb2jffs_enable"); %>',
																		function(){
																			document.form.usb2jffs_enable.value = 1;
																			applyRule();
																			$(".tr1").show();
																			select_tablet(1);
																		},
																		function(){
																			document.form.usb2jffs_enable.value = 0;
																			actions(2);
																			applyRule();
																			$(".tr1").hide();
																			select_tablet(1);
																		}
																	);
																</script>
															</div>
														</td>
													</tr>
													<tr id="usb2jffs_sync_tr" class="tr1">
														<th>定时同步（USB_JFFS → MTD_JFFS）</th>
														<td>
															每隔
															<select name="usb2jffs_sync_time" class="input_option" style="width:auto">
																<option value="0"
																	<% nvram_match("usb2jffs_sync_time", "0","selected"); %>>不同步
																</option>
																<option value="3600"
																	<% nvram_match("usb2jffs_sync_time", "3600","selected"); %>>1小时
																</option>
																<option value="7200"
																	<% nvram_match("usb2jffs_sync_time", "7200","selected"); %>>2小时
																</option>
																<option value="14400"
																	<% nvram_match("usb2jffs_sync_time", "14400","selected"); %>>4小时
																</option>
																<option value="21600"
																	<% nvram_match("usb2jffs_sync_time", "21600","selected"); %>>6小时
																</option>
																<option value="43200"
																	<% nvram_match("usb2jffs_sync_time", "43200","selected"); %>>12小时
																</option>
																<option value="86400"
																	<% nvram_match("usb2jffs_sync_time", "86400","selected"); %>>24小时
																</option>
															</select>
															<a type="button" class="ks_btn" style="cursor: pointer;margin-left:5px;border:none" onClick="actions(4);">手动同步</a>
														</td>
													</tr>
													<tr id="usb2jffs_rsync_tr" class="tr1">
														<th>卸载时同步</th>
														<td>
															<input type="radio" name="usb2jffs_rsync" class="input" value="1" <% nvram_match_x("", "usb2jffs_rsync", "1", "checked"); %>><#checkbox_Yes#>
															<input type="radio" name="usb2jffs_rsync" class="input" value="0" <% nvram_match_x("", "usb2jffs_rsync", "0", "checked"); %>><#checkbox_No#>
														</td>
													</tr>
													<tr id="operation_tr" class="tr1">
														<th>操作</th>
														<td>
															<input class="ks_btn bt1" type="button" style="cursor: pointer;margin-left:5px;border:none" onClick="actions(5);" value="保存设置">
															<input class="ks_btn bt1 sbt0" type="button" style="cursor: pointer;margin-left:5px;border:none" onClick="actions(1);" value="挂载">
															<input class="ks_btn bt1 sbt1" type="button" style="cursor: pointer;margin-left:5px;border:none" onClick="actions(2);" value="卸载">
															<input class="ks_btn bt1 sbt1" type="button" style="cursor: pointer;margin-left:5px;border:none" onClick="actions(3);" value="删除">
														</td>
													</tr>
												</table>
											</div>

											<div id="tablet_2" style="display: none;">
												<table style="margin:-1px 0px 0px 0px;" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
													<tr>
													<td>
														<ul>
														<span style="line-height:2em"><b>功能说明</b></span><br />
															&nbsp;&nbsp;&nbsp;&nbsp;1. 功能实现参考了：<a style="color:#17ffe4" href="https://koolshare.cn/thread-161017-1-1.html" title="" target="_blank">此koolshare论坛贴</a>，和软件中心的USB2JFFS插件。<br />
															&nbsp;&nbsp;&nbsp;&nbsp;2. 打开启用开关后每次开机及插入USB时会自动尝试挂载。<br />
															&nbsp;&nbsp;&nbsp;&nbsp;3. 卸载USB时会自动恢复原始的JFFS分区。<br />
														<span style="line-height:2em"><b>使用说明</b></span><br />
															&nbsp;&nbsp;&nbsp;&nbsp;1. 需要提前准备一个已经格式化成ext3/ext4/hfs格式的USB存储器。<br />
															&nbsp;&nbsp;&nbsp;&nbsp;2. USB存储器有多个分区时只使用第一个符合要求的分区（按系统识别顺序）。<br />
															&nbsp;&nbsp;&nbsp;&nbsp;3. 分区卷标不要使用中文，固件不能识别中文卷标。<br />
															&nbsp;&nbsp;&nbsp;&nbsp;4. 使用的U盘质量不能太差，要求读写速度不能太低，可长时间稳定运行，不然可能极度影响使用体验。<br />
															&nbsp;&nbsp;&nbsp;&nbsp;5. 建议挂载了USB JFFS后保持路由器长时间开机，不要频繁重启。<br />
															&nbsp;&nbsp;&nbsp;&nbsp;6. 不要与其它USB2JFFS插件同时启用，以免造成混乱。<br />
														<span style="line-height:2em"><b>操作说明</b></span><br />
															&nbsp;&nbsp;&nbsp;&nbsp;1. 挂载： 插入U盘，点击挂载，插件会在USB磁盘中创建.usb_jffs目录，并用此目录挂载到/jffs。<br />
															&nbsp;&nbsp;&nbsp;&nbsp;2. 卸载： 仅会移除/jffs与USB磁盘的挂载关系，并不会删除U盘和JFFS里的任何内容。<br />
															&nbsp;&nbsp;&nbsp;&nbsp;3. 删除： 不仅会移除/jffs与USB磁盘的挂载关系，还会删除USB磁盘里的.usb_jffs目录。<br />
															&nbsp;&nbsp;&nbsp;&nbsp;4. 同步： 点击同步，会将USB磁盘里.usb_jffs目录和路由器里的原始jffs挂载点分区进行同步。<br />
														<span style="line-height:2em"><b>注意事项</b></span><br />
															&nbsp;&nbsp;文件同步：<br />
															&nbsp;&nbsp;&nbsp;&nbsp;1. 首次成功挂载后，会复制原jffs目录下所有文件到USB的新jffs目录（即.usb_jffs文件夹）。<br />
															&nbsp;&nbsp;&nbsp;&nbsp;2. 因路由器系统相关文件写入/更新、软件中心插件安装/更新等操作，USB的jffs内的文件会比原jffs新。<br />
															&nbsp;&nbsp;&nbsp;&nbsp;3. 设定自动同步，或者手动点击同步按钮，会将新jffs下的文件和原jffs文件进行对比，并进行更新。<br />
															&nbsp;&nbsp;&nbsp;&nbsp;4. 点击同步按钮不宜太频繁，避免文件频繁更新导致原jffs出现问题。<br />
															&nbsp;&nbsp;&nbsp;&nbsp;5. 如果安装了很多插件导致jffs目录大小大于原jffs分区大小，同步功能将只同步系统相关文件，如证书文件等。<br />
															&nbsp;&nbsp;卸载USB存储器：<br />
															&nbsp;&nbsp;&nbsp;&nbsp;1. 请勿在已经挂载了USB型JFFS的状态下拔出USB存储器！<br />
															&nbsp;&nbsp;&nbsp;&nbsp;2. 如果需要，请先在本插件点击卸载按钮，卸载USB型JFFS，再使用系统自带的移除USB硬盘，最后拔掉USB存储器。<br />
														</ul>
													</td>
													</tr>
												</table>
											</div>
										</td>
									</tr>
								</table>
							</td>

						</tr>
					</table>
					<!--===================================Ending of Main Content===========================================-->
				</td>

				<td width="10" align="center" valign="top">&nbsp;</td>
			</tr>
		</table>
	</form>
	<div id="footer"></div>
</body>

</html>
