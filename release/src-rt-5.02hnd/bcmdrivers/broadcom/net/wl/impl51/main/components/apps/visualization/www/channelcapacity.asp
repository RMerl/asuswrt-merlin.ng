<!DOCTYPE html>
<!--
 HTML part for Channel Statistics Tab

 Copyright (C) 2016, Broadcom. All Rights Reserved.
 
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

 $Id: channelcapacity.asp 604666 2015-12-08 06:20:01Z $
-->

<html>
<head>
<meta charset="ISO-8859-1">
<link rel="stylesheet" href="jquery-ui.min.css">
<link rel="stylesheet" href="visstyle.css">
<link rel="stylesheet" href="channelCapacity.css">
<script src="jquery-2.0.3.min.js"></script>
<script src="common.js"></script>
<script src="jquery.flot.min.js"></script>
<script src="jquery.flot.orderBars.js"></script>
<script src="jquery.flot.axislabels.js"></script>
<script src="jquery.flot.stack.js"></script>
<script src="jquery.cookie.js"></script>
<script src="jquery-ui.js"></script>
<script src="channelcapacity.js"></script>
<title>WiFi Radar</title>
</head>
<body>
	<div id="main_div">
		<script>commonTopNavBar()</script> <!-- Defined custom menu in common.js -->

		<table class="logotable">
			<tr>
				<td width="100%" valign="top">
					<br>
					<span id="TextHeading">Wireless Channel Statistics</span><br>
					<span id="TextHeadingDesc">Wireless channel statistics will show all wireless signal interferences nearby, and insight channel capacity as well.</span>
				</td>
			</tr>
		</table>
		<div id="contentarea" class="btmbrdr">
			<br>
			<div id="showerrmessage">
			</div>
			<div id="frequencybandselector">
				<form id="frequencybandform">
					<fieldset class="fontfamly cmnfilterclass">
						<select id="bandselectcntrl" class="selectbandctrl cmnctrlborder">
							<option>No Interfaces</option>
						</select>
					</fieldset>
				</form>
				<br/>
				<table id="channelcapacityhdrid" class="channelcapacityhdr fontfamly">
					<tbody><tbody>
				</table>
			</div>

			<div id="channelcapacitygraphcontent">
			</div>

		</div>
	</div>
	<div id="stainfopopup" title="Station Details">
		<div id="stainfopopdtls">
		</div>
	</div>
</body>
</html>
