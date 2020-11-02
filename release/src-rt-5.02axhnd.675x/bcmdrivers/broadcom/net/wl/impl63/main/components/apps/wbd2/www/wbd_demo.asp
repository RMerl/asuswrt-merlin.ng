<!DOCTYPE html>
<!--
 HTML part for WiFi Blanket Demo Tab

 Copyright (C) 2020, Broadcom. All Rights Reserved.

 Permission to use, copy, modify, and/or distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.

 THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

 <<Broadcom-WL-IPTag/Open:>>

 $Id: wbd_demo.asp 770293 2018-12-11 17:54:43Z $
-->

<html>
<head>
<meta charset="ISO-8859-1">
<link rel="stylesheet" href="wbd_demo.css"/>
<link rel="stylesheet" href="style.css"/>
<script src="jquery-2.0.3.min.js"></script>
<script src="wbd_demo.js"></script>
<title>Broadcom SmartMesh</title>
</head>
<body>
	<div id="main_div">
		<table class="logotable">
			<tr class="page_title">
				<td colspan="2" class="edge"><img border="0" src="blur_new.jpg" alt=""/></td>
			</tr>
			<tr>
				<td class="page_title"><img border="0" src="logo_new.gif" alt=""/></td>
				<td width="100%" valign="top">
					<br/>
					<span id="TextHeading"> <h1>Broadcom SmartMesh</h1> </span><br/>
				</td>
			</tr>
		</table>
		<div id="contentarea" class="btmbrdr">
			<div id="wbdcontent" class="maindiv">
				<div id="5gdiv" class="outerdivcommon">
					<h2 id="heading"> 5G Low </h2>
				</div>
				<div id="5gdivH" class="outerdivcommon">
					<h2 id="heading"> 5G High </h2>
				</div>
				<div id="2gdiv" class="outerdivcommon">
					<h2> 2.4G </h2>
				</div>
				<div id="logsdiv" class="outerdivcommon">
					<h2> Logs </h2>
					<div id="logsdivcontainer" class="commonbdr innerdivcommonforlogs">
						<div id="stamsgs" class="stamsgstyle txtstyle">
						</div>
					</div>
					<button id="clearlogs" type="button" class="logsbtn">Clear Logs</button>
				</div>
				<br style="clear:left"/>
			</div>
		</div>
	</div>

	<div id="templatesAdv" style="display:none">
	<table id="tableTemplate" class="tablestylecommon">
		<thead>
		<tr>
			<th style="width:9.5em">Client (MAC)</th>
			<th style="width:9.5em">RSSI</th>
		</tr>
		</thead>
		<tbody>
		</tbody>
	</table>
	</div>

</body>
</html>
