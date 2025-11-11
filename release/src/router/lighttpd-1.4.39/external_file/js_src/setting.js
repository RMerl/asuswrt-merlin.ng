var m = new lang();
var g_storage = new myStorage();
// var server_url = "http://oauth.asus.com/aicloud/";
// var g_bLoginFacebook = false;
// var g_bLoginGoogle = false;
var g_key_file = null;
var g_crt_file = null;
var g_intermediate_crt_file = null;
// var g_create_account_dialog = null;
// var g_account_info_dialog = null;
// var g_invite_token = "";

$("document").ready(function () {
	document.oncontextmenu = function () { return false; };

	var loc_lan = String(window.navigator.userLanguage || window.navigator.language).toLowerCase();
	var lan = (g_storage.get('lan') == undefined) ? loc_lan : g_storage.get('lan');
	m.setLanguage(lan);

	var init_page = (getUrlVar("p") == undefined) ? 0 : getUrlVar("p");
	var only_show_init_page = (getUrlVar("s") == undefined) ? 0 : getUrlVar("s");

	// 預設顯示第一個 Tab
	var _showTabIndex = init_page;
	$('ul.tabs li').eq(_showTabIndex).addClass('active');
	$('.tab_content').hide().eq(_showTabIndex).show();

	if (only_show_init_page) {
		$('ul.tabs li').each(function (i) {
			if (i != _showTabIndex)
				$('ul.tabs li').eq(i).hide();
		});
	}

	// 當 li 頁籤被點擊時...
	// 若要改成滑鼠移到 li 頁籤就切換時, 把 click 改成 mouseover
	$('ul.tabs li').click(function () {
		// 找出 li 中的超連結 href(#id)
		var $this = $(this),
			_clickTab = $this.find('a').attr('href');
		// 把目前點擊到的 li 頁籤加上 .active
		// 並把兄弟元素中有 .active 的都移除 class
		$this.addClass('active').siblings('.active').removeClass('active');
		// 淡入相對應的內容並隱藏兄弟元素
		$(_clickTab).stop(false, true).fadeIn().siblings().hide();

		//if(_clickTab=="#tab3")
		//	getLatestVersion();

		return false;
	}).find('a').focus(function () {
		this.blur();
	});

	$("button#btn_rescan").click(function () {
		$("button#btn_rescan").attr("disabled", true);
		$("button#btn_rescan").text("Wait");
		parent.doRescanSamba();
	});

	$("button#btn_gen_crt").click(function () {
		var client = new davlib.DavClient();
		client.initialize();

		var keylen = 2048;
		var caname = "AiCloud";
		var email = "";
		var country = "TW";
		var state = "Taiwan";
		var ln = "Hsinchu";
		var orag = "ASUS";
		var ounit = "RD";

		var cn = g_storage.get('request_host_url');
		if (cn.indexOf("://") != -1) {
			cn = cn.substr(cn.indexOf("://") + 3);
		}

		var msg = m.getString("msg_gen_crt_confirm");
		msg = msg.replace("%s", cn);

		if (confirm(msg)) {
			$("#crt_creating").show();

			client.GENROOTCERTIFICATE("/", keylen, caname, email, country, state, ln, orag, ounit, cn, function (error, content, statusstring) {
				if (error == 200) {
					$("#crt_creating").hide();
					alert(m.getString("msg_gen_crt_complete"));

					client.APPLYAPP("/", "apply", "", "restart_webdav", function (error, statusstring, content) {
						if (error == 200) {
							setTimeout(function () {
								parent.location.reload(false);
							}, 6000);
						}

						client = null;
					});
				}
			});
		}
	});

	$("button#btn_import_crt").click(function () {

		$("#import_crt").show();
		$("#import_crt").css("left", g_mouse_x);

		if (g_mouse_y + $("#filelink").height() > $("body").height())
			$("#import_crt").css("top", $("body").height() - $("#filelink").height());
		else
			$("#import_crt").css("top", g_mouse_y);
	});

	$("button#btn_export_crt").click(function () {
		window.open("/aicloud.crt");
	});

	// $("button#btn_create_account").click(function(){
	// 	g_create_account_dialog.data("action", "new").dialog("open");
	// });	

	// $("button#btn_create_invite").click(function(){
	// 	g_create_account_dialog.data("action", "invite").dialog("open");
	// });

	$('li#rescan a').text(m.getString('title_rescan'));
	$('li#sharelink a').text(m.getString('title_sharelink'));
	$('li#version a').text(m.getString('title_version'));
	$('li#settings a').text(m.getString('title_crt'));
	$('li#account a').text(m.getString('title_account'));
	$('button#ok').text(m.getString('btn_ok'));
	$('button#cancel').text(m.getString('btn_close'));
	$('#btn_rescan').text(m.getString('btn_rescan'));
	$('span#desc_rescan').text(m.getString('title_desc_rescan'));
	$('span#aicloud_desc_version').text("AiCloud " + m.getString('title_version'));
	$('span#aicloud_version').text(g_storage.get('aicloud_version'));
	$('span#smartsync_desc_version').text("SmartSync " + m.getString('title_version'));
	$('span#smartsync_version').text(g_storage.get('smartsync_version'));

	$('span#desc_version').text("FW " + m.getString('title_version'));
	$('span#version').text(g_storage.get('router_version'));

	$("p.desc_share_link").text(m.getString('title_copy_string'));

	$("#label_gen_crt").text(m.getString("title_gen_crt"));
	$("#btn_gen_crt").text(m.getString("title_gen_crt"));
	$("#btn_import_crt").text(m.getString("title_import_crt"));
	$("#desc_gen_crt").text(m.getString("title_desc_gen_crt"));
	$("#label_server_crt").text(m.getString("title_server_crt"));
	$("#label_crt_type").text(m.getString("title_crt_type"));
	$("#label_self_crt_type").text(m.getString("title_self_crt_type"));
	$("#label_crt_to").text(m.getString("title_crt_to"));
	$("#label_crt_from").text(m.getString("title_crt_from"));
	$("#label_crt_end_date").text(m.getString("title_crt_end_date"));
	$("#btn_export_crt").text(m.getString("title_export_crt"));
	$("#label_https_crt_cn").text(m.getString('https_crt_cn'));
	$("#crt_creating").text(m.getString('msg_gen_crt_creating'));

	$("#desc_import_crt").text(m.getString('desc_import_crt'));
	$("#label_select_private_key").text(m.getString('title_select_private_key'));
	$("#label_select_certificate").text(m.getString('title_select_certificate'));
	$("#label_select_intermediate_certificate").text(m.getString('title_select_intermediate_certificate'));
	$("#import").text(m.getString('title_import_crt'));

	if (_showTabIndex == 1)
		refreshShareLinkList();
	else if (_showTabIndex == 3)
		refreshCertificateInfo();

	$(".abgne_tab").css("height", $(window).height() - 80);

	$("#btn_select_key").change(function (evt) {

		if (evt.target.files.length != 1) {
			alert("Please select a key file!");
			return;
		}

		g_key_file = evt.target.files[0];
	});

	$("#btn_select_crt").change(function (evt) {
		if (evt.target.files.length != 1) {
			alert("Please select a key file!");
			return;
		}

		g_crt_file = evt.target.files[0];
	});

	$("#btn_select_intermediate_crt").change(function (evt) {
		if (evt.target.files.length != 1) {
			return;
		}

		g_intermediate_crt_file = evt.target.files[0];
	});
	
});

function onFacebookLogin(token, uid) {
}

function facebook_login() {
	
}

function onGoogleLogin(token, uid) {

}

function google_login() {
	
}

function refreshShareLinkList() {
	var webdav_mode = g_storage.get('webdav_mode');
	var ddns_host_name = g_storage.get('ddns_host_name');

	var hostName = "";
	var cur_host_name = g_storage.get('request_host_url');
	if (cur_host_name.indexOf("://") != -1) {
		cur_host_name = cur_host_name.substr(cur_host_name.indexOf("://") + 3);
	}

	if (!isPrivateIP(cur_host_name))
		hostName = cur_host_name;
	else
		hostName = (ddns_host_name == "") ? cur_host_name : ddns_host_name;

	if (hostName.indexOf(":") != -1)
		hostName = hostName.substring(0, hostName.indexOf(":"));

	if (webdav_mode == 0) //- Only enable http
		hostName = "http://" + hostName + ":" + g_storage.get("http_port");
	else {
		hostName = "https://" + hostName;

		if (g_storage.get("https_port") != "443")
			hostName += ":" + g_storage.get("https_port");
	}

	var client = new davlib.DavClient();
	client.initialize();
	client.GSLL("/", function (error, content, statusstring) {
		if (error == 200) {
			var data = parseXml(statusstring);

			$("#tab2").empty();

			var table_html = "<table id='sharelink' width='100%' border='0' style='table-layout:fixed'>";
			table_html += "<thead><tr>";
			table_html += "<th scope='col' class='check' style='width:5%'>";
			table_html += "<input type='checkbox' id='select_all' name='select_all' class='select_all'>";
			table_html += "</th>";

			table_html += "<th scope='col' class='filename' style='width:20%'>" + m.getString('table_filename') + "</th>";
			table_html += "<th scope='col' class='createtime' style='width:25%'>" + m.getString('table_createtime') + "</th>";
			table_html += "<th scope='col' class='expiretime' style='width:25%'>" + m.getString('table_expiretime') + "</th>";
			table_html += "<th scope='col' class='lefttime' style='width:15%'>" + m.getString('table_lefttime') + "</th>";
			table_html += "<th scope='col' class='remove' style='width:10%'>" + m.getString('func_delete') + "</th>";
			table_html += "</tr></thead>";

			table_html += "<tbody id='ntb'>";

			var encode_filename = parseInt($(data).find('encode_filename').text());

			var i = 0;
			$(data).find('sharelink').each(function () {

				try {
					var filename = "";
					var filetitle = "";

					if (encode_filename == 1) {
						filename = $(this).attr("filename");
						filetitle = decodeURIComponent(filename);
					}
					else {
						filetitle = $(this).attr("filename");
						filename = encodeURIComponent($(this).attr("filename"));
					}

					var url = hostName + "/" + $(this).attr("url") + "/" + filename;
					var createtime = $(this).attr("createtime");
					var expiretime = $(this).attr("expiretime");
					var lefttime = parseFloat($(this).attr("lefttime"));
					var hour = parseInt(lefttime / 3600);
					var minute = parseInt(lefttime % 3600 / 60);

					table_html += "<tr nid='" + i + "' class='even'>";

					table_html += "<td fid='check' align='center'><input type='checkbox' id='check_del' name='check_del' class='check_del' link=\"" + $(this).attr("url") + "\"></td>";

					table_html += "<td fid='filename' align='center'><div style='overflow:hidden;padding:5px;white-space:normal;word-break:break-all;text-align:left;'>";
					table_html += "<a class='share_link_url' uhref=\"" + url + "\" href='#' title=\"" + filetitle + "\">" + filetitle + "</a>";
					table_html += "</div></td>";
					table_html += "<td fid='createtime' align='center'>" + createtime + "</td>";
					if (expiretime == 0) {
						table_html += "<td fid='expiretime' align='center'>" + m.getString('title_unlimited') + "</td>";
						table_html += "<td fid='lefttime' align='center'>" + m.getString('title_unlimited') + "</td>";
					}
					else {
						table_html += "<td fid='expiretime' align='center'>" + expiretime + "</td>";
						table_html += "<td fid='lefttime' align='center'>" + hour + " hours " + minute + " mins" + "</td>";
					}
					table_html += "<td fid='remove' align='center'><a>";
					table_html += "<div class='dellink' title='remove' link=\"" + $(this).attr("url") + "\" style='cursor:pointer'></div>";
					table_html += "</a></td>";

					table_html += "</tr>";
				}
				catch (err) {
					//Handle errors here				  	
				}

				i++;
			});

			table_html += "</tbody>";
			table_html += "</table>";

			table_html += "<div class='delcheck_block'>";
			table_html += "<span>刪除選取連結</span>";
			table_html += "</div>";

			$("#tab2").html(encodeSafeString(table_html));

			$("div.delcheck_block").css("visibility", "hidden");

			$("a.share_link_url").click(function () {
				$("#filelink").css("display", "block");
				$("#filelink").css("left", g_mouse_x);

				if (g_mouse_y + $("#filelink").height() > $("body").height())
					$("#filelink").css("top", $("body").height() - $("#filelink").height());
				else
					$("#filelink").css("top", g_mouse_y);

				$("#resourcefile").attr("value", $(this).attr("uhref"));
				$("#resourcefile").focus();
				$("#resourcefile").select();
			});

			$(".dellink").click(function () {

				var r = confirm(m.getString('msg_confirm_delete_sharelink'));

				if (r == true) {
					client.REMOVESL("/", $(this).attr("link"), function (error, content, statusstring) {
						if (error == 200) {
							refreshShareLinkList();
						}
					});
				}
			});

			$(".check_del").change(function () {

				var del_count = 0;

				$("input:checkbox.check_del").each(function () {
					if ($(this).prop("checked")) {
						del_count++;
					}
				});

				if (del_count <= 0) {

					var newTop = g_mouse_y + 10;
					var newLeft = 0;
					$("div.delcheck_block").animate({
						top: newTop,
						left: newLeft
					},
						'fast',
						function () {
							$("div.delcheck_block").css("visibility", "hidden");
						});
				}
				else {

					$("div.delcheck_block").css("visibility", "");

					var newTop = g_mouse_y + 10;
					var newLeft = g_mouse_x + 10;
					$("div.delcheck_block").animate({
						top: newTop,
						left: newLeft
					}, 'fast');
				}
			});

			$(".delcheck_block").click(function () {

				var r = confirm(m.getString('msg_confirm_delete_sharelink'));

				if (r == true) {

					$("div.delcheck_block").css("visibility", "hidden");

					$("input:checkbox.check_del").each(function () {
						if ($(this).prop("checked")) {

							client.REMOVESL("/", $(this).attr("link"), function (error, content, statusstring) {
								if (error == 200) {
									refreshShareLinkList();
								}
							});
						}
					});
				}
			});

			$("input.select_all").change(function () {
				if ($(this).prop("checked")) {
					$('input:checkbox.check_del').prop('checked', true);
					$("div.delcheck_block").css("visibility", "");

					var newTop = g_mouse_y + 10;
					var newLeft = g_mouse_x + 10;
					$("div.delcheck_block").animate({
						top: newTop,
						left: newLeft
					}, 'fast');
				}
				else {
					$('input:checkbox.check_del').prop('checked', false);

					var newTop = g_mouse_y + 10;
					var newLeft = 0;
					$("div.delcheck_block").animate({
						top: newTop,
						left: newLeft
					},
						'fast',
						function () {
							$("div.delcheck_block").css("visibility", "hidden");
						});
				}
			});
		}
	});
}

function refreshCertificateInfo() {

	var userpermission = (g_storage.get('userpermission') == undefined) ? "" : g_storage.get('userpermission');
	if (userpermission == "admin") {
		$("#field_import_crt").show();
	}
	else {
		$("#field_import_crt").hide();
	}

	var client = new davlib.DavClient();
	client.initialize();
	client.GETX509CERTINFO("/", function (error, content, statusstring) {
		if (error == 200) {

			var data = parseXml(statusstring);

			var issuer = $(data).find('issuer').text();
			var subject = $(data).find('subject').text();
			var crt_start_date = $(data).find('crt_start_date').text();
			var crt_end_date = $(data).find('crt_end_date').text();

			if (subject != "") {
				subject = subject.substring(subject.indexOf("CN=") + 3, subject.length);

				if (subject.indexOf("/") != -1) {
					subject = subject.substring(0, subject.indexOf("/"));
				}
			}

			if (issuer != "") {
				issuer = issuer.substring(issuer.indexOf("CN=") + 3, issuer.length);

				if (issuer.indexOf("/") != -1) {
					issuer = issuer.substring(0, issuer.indexOf("/"));
				}
			}

			//crt_end_date = "160607080028Z";
			if (crt_end_date != "") {

				var mydate = new Date(2000 + parseInt(crt_end_date.substring(0, 2)),
					parseInt(crt_end_date.substring(2, 4)),
					parseInt(crt_end_date.substring(4, 6)));

				crt_end_date = mydate.getFullYear() + "-" + mydate.getMonth() + "-" + mydate.getDate();

				//alert(crt_end_date.substring(0, 2)+", "+crt_end_date.substring(2, 4) + ", " + crt_end_date.substring(4, 6));
				//alert(mydate.toISOString().slice(0, 10).replace(/-/g, '-'));
				//alert(mydate.getFullYear() + " - " + mydate.getMonth() + " - " + mydate.getDate());
			}

			if (issuer == subject) {
				$("#label_self_crt_type").text(m.getString("title_self_crt_type"));
			}
			else {
				$("#label_self_crt_type").text(m.getString("title_third_crt_type"));
			}

			$("#label_https_crt_cn").text(subject);
			$("#label_https_crt_issuer").text(issuer);
			$("#label_https_crt_end_date").text(crt_end_date);
			//alert(content + ", " + statusstring);

			if (issuer == "") {
				$("#btn_export_crt").attr("disabled", true);
			}
			else {
				$("#btn_export_crt").attr("disabled", false);
			}
		}

		client = null;
	});
}

function doOK(e) {
	parent.closeJqmWindow();
};

function doCancel(e) {
	parent.closeJqmWindow();
};

function onCloseShareLink() {
	$("#filelink").css("display", "none");
}

function onCloseImportCrt() {
	$('#import_crt').hide();
}

function read_file(the_file, onReadCompleteHandler) {
	if (the_file == null)
		return;

	var start = 0;
	var stop = the_file.size - 1;
	var bSliceAsBinary = 0;
	var reader = new FileReader();
	reader.onloadend = function (evt) {
		if (evt.target.readyState == FileReader.DONE) { // DONE == 2
			if (onReadCompleteHandler)
				onReadCompleteHandler(evt, bSliceAsBinary);
		}
	};

	if (the_file.webkitSlice) {
		var blob = the_file.webkitSlice(start, stop + 1);
		//reader.readAsArrayBuffer(blob);
		reader.readAsText(blob);
		bSliceAsBinary = 1;
	}
	else if (the_file.mozSlice) {
		var blob = the_file.mozSlice(start, stop + 1);
		reader.readAsBinaryString(blob);
		bSliceAsBinary = 0;
	}
	else {
		var blob = the_file.slice(start, stop + 1);
		//reader.readAsArrayBuffer(blob);
		reader.readAsText(blob);
		bSliceAsBinary = 1;
	}
}

function onDoImportCrt() {

	if (g_key_file == null) {
		alert("Please select a key file.");
		return;
	}

	if (g_crt_file == null) {
		alert("Please select a crt file.");
		return;
	}

	var key_pem = "";
	var crt_pem = "";
	var intermediate_crt_pem = "";
	var set_pem_content = function () {

		if (g_key_file != null && key_pem == "")
			return;

		if (g_crt_file != null && crt_pem == "")
			return;

		if (g_intermediate_crt_file != null && intermediate_crt_pem == "")
			return;

		var client = new davlib.DavClient();
		client.initialize();

		client.SETROOTCERTIFICATE('/', key_pem, crt_pem, intermediate_crt_pem, function (context, status, statusstring) {
			if (context == "200") {
				$('#import_crt').hide();

				alert(m.getString("msg_import_crt_complete"));

				client.APPLYAPP("/", "apply", "", "restart_webdav", function (error, statusstring, content) {
					if (error == 200) {
						setTimeout(function () {
							parent.location.reload(false);
						}, 6000);
					}

					client = null;
				});
			}
			else {
				alert("Fail to import certificate file! Maybe the imported file is invalid, please check and try again.");
			}
		}, null);
	}

	read_file(g_key_file, function (evt, bSliceAsBinary) {
		//alert("done: " +　evt.target.result.byteLength + ", " + bSliceAsBinary + ", " + evt.target.result);
		var content = evt.target.result;
		/*
		if(content.indexOf("-----BEGIN RSA PRIVATE KEY-----")!=0){
			alert("Invalid key file format!");
			return;
		}
		*/
		key_pem = content;

		set_pem_content();
	});

	read_file(g_crt_file, function (evt, bSliceAsBinary) {
		var content = evt.target.result;
		/*
		if(content.indexOf("-----BEGIN CERTIFICATE-----")!=0){
			alert("Invalid cert file format!");
			return;
		}
		*/
		crt_pem = content;

		set_pem_content();
	});

	read_file(g_intermediate_crt_file, function (evt, bSliceAsBinary) {
		var content = evt.target.result;
		/*
		if(content.indexOf("-----BEGIN CERTIFICATE-----")!=0){
			alert("Invalid intermediate cert file format!");
			return;
		}
		*/
		intermediate_crt_pem = content;

		set_pem_content();
	});
}

// Temporary variables to hold mouse x-y pos.s
var g_mouse_x = 0
var g_mouse_y = 0

// Set-up to use getMouseXY function onMouseMove
document.onmousemove = getMouseXY;

//- function to retrieve mouse x-y pos.s
function getMouseXY(e) {
	if (document.all) {
		// grab the x-y pos.s if browser is IE
		g_mouse_x = event.clientX + document.body.scrollLeft + document.documentElement.scrollLeft;
		g_mouse_y = event.clientY + document.body.scrollTop + document.documentElement.scrollTop;
	}
	else {
		// grab the x-y pos.s if browser is NS
		g_mouse_x = e.pageX;
		g_mouse_y = e.pageY;
	}

	if (g_mouse_x < 0) { g_mouse_x = 0 }
	if (g_mouse_y < 0) { g_mouse_y = 0 }
	return true
}