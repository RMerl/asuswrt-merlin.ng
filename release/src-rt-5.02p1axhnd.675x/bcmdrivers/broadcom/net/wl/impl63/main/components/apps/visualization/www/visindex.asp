<!DOCTYPE html>
<!--
 HTML part for Site Survey Tab

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

 $Id: visindex.asp 705872 2017-06-19 19:09:46Z $
-->

<html>
<head>
<meta charset="ISO-8859-1">
<link rel="stylesheet" href="visstyle.css"/>
<link rel="stylesheet" href="tablestyle.css"/>
<script src="jquery-2.0.3.min.js"></script>
<script src="common.js"></script>
<script src="jquery.cookie.js"></script>
<!--<script src="jquery.flot.min.js"></script>-->
<script src="jquery.flot.js"></script>
<script src="curvedLines.js"></script>
<script src="jquery.flot.navigate.min.js"></script>
<script src="jquery.flot.axislabels.js"></script>
<script src="jquery.tablesorter.min.js"></script>
<script src="highlightseries.js"></script>
<script src="main.js"></script>
<title>WiFi Insight</title>
</head>
<body>
	<div id="main_div">
		<div id="page_header">
		<script>commonTopNavBar()</script> <!-- Defined custom menu in common.js -->
		</div>
		<table class="logotable">
			<tr class="page_title">
				<td colspan="2" class="edge"><img border="0" src="blur_new.jpg" alt=""></td>
			</tr>
			<tr>
				<td class="page_title"><img border="0" src="logo_new.gif" alt=""></td>
				<td width="100%" valign="top">
					<br>
					<span id="TextHeading">Site Survey</span><br>
					<span id="TextHeadingDesc">In this page you will see all the AP's around.</span>
				</td>
			</tr>
		</table>
		<div id="contentarea" class="btmbrdr">
			<br>
			<div id="showerrmessage"></div>
			<div id="frequencybandselector">
				<form id="frequencybandform" class="formcls">
					<fieldset id="radioselectors" class="fontfamly cmnfilterclass">
						<select id="bandselectcntrl" class="selectbandctrl cmnctrlborder">
							<option>No Interfaces</option>
						</select>
						<select id="chnlselectcntrl" class="selectctrl cmnctrlborder">
							<option>Select Channel</option>
						</select>

						<select id="bndwthselectcntrl" class="selectctrl cmnctrlborder">
						<option>Select Bandwidth</option>
						<option>20</option>
						<option>40</option>
						<option>80</option>
						</select>
					</fieldset>
					<button id="scanbutton" type="button" class="cmnbtnclass">Scan</button>
					<div class="progressbarclass"></div>
				</form>
				<div id="errmsg"></div>
				<div id="clearflt"></div>
			</div>
			<div id="rssigraphplaceholder1">
				<table id ="sitesurveygraphtableid" class="sitesurveygraphtable fontfamly cmngraphbox" style="margin-bottom:10px;">
					<thead class="cmngraphboxbkgrnd-h1">
						<tr>
							<th>AP's Around</th>
						</tr>
					</thead>
					<tbody>
						<tr>
							<td>
								<div id='rssigraphplaceholder' style="margin-left:10px;width:98%;height:480px; padding:0px; position:relative;"></div>
							</td>
						</tr>
					</tbody>
				</table>
			</div>
			<div id="aplistplaceholder">
				<table id="aplisttableid" class="aplisttable fontfamly cmnbrdr">
					<thead class="cmngraphboxbkgrnd-h1">
						<tr>
							<th><span>Network Name</span></th>
							<th><span>Network Address</span></th>
							<th><span>Signal [dBm]</span></th>
							<th width="4%"><span>SNR <br/>[dB]</span></th>
							<th width="4%"><span>Bandwidth<br/>[MHz]</span></th>
							<th width="4%"><span>Center<br/>Channel</span></th>
							<th width="4%"><span>Control<br/>Channel</span></th>
							<th width="10%"><span>Max PHY<br/>Rate [Mbps]</span></th>
							<th><span>802.11</span></th>
							<th><span>Security</span></th>
						<tr>
					</thead>
					<tbody></tbody>
				</table>
			</div>
		</div>
	</div>
</body>
</html>
