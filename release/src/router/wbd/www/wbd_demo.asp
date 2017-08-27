<!DOCTYPE html>
<!--
 HTML part for WiFi Blanket Demo Tab

 Broadcom Proprietary and Confidential. Copyright (C) 2016,
 All Rights Reserved.
 
 This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 the contents of this file may not be disclosed to third parties, copied
 or duplicated in any form, in whole or in part, without the prior
 written permission of Broadcom.


 <<Broadcom-WL-IPTag/Open:>>

 $Id: wbd_demo.asp 636113 2016-05-06 12:02:04Z $
-->

<html>
<head>
<meta charset="ISO-8859-1">
<link rel="stylesheet" href="wbd_demo.css"/>
<script src="jquery-2.0.3.min.js"></script>
<script src="wbd_demo.js"></script>
<title>WiFi Blanket</title>
</head>
<body>
	<div id="main_div">
		<table class="logotable">
			<tr>
				<td colspan="2" class="edge"><img border="0" src="blur_new.jpg" alt=""/></td>
			</tr>
			<tr>
				<td><img border="0" src="logo_new.gif" alt=""/></td>
				<td width="100%" valign="top">
					<br/>
					<span id="TextHeading">Wifi Blanket</span><br/>
					<span id="TextHeadingDesc">In this page we will demonstrate
					Wifi Blanket</span>
				</td>
			</tr>
		</table>
		<div id="contentarea" class="btmbrdr">
			<div id="wbdcontent" class="maindiv" align="center">
				<div id="5gdiv" class="outerdivcommon outerdivforbothbands">
					<h2 id="heading"> 5G Blanket </h2>
				</div>
				<div id="2gdiv" class="outerdivcommon outerdivforbothbands">
					<h2> 2G Blanket </h2>
				</div>
				<div id="logsdiv" class="outerdivcommon outerdivforbothlogs">
					<h2> Logs </h2>
					<div id="logsdivcontainer" class="commonbdr innerdivcommonforlogs outerdivforbothlogs">
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
			<th style="width:45%">Client (MAC)</th>
			<th style="width:40%">RSSI</th>
			<th></th>
		</tr>
		</thead>
		<tbody>
		</tbody>
	</table>
	</div>

</body>
</html>
