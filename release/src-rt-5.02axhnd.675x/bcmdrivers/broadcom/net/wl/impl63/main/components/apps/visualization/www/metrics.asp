<!DOCTYPE html>
<!--
 HTML part for Advanced Troubleshooting Tab

 Copyright (C) 2019, Broadcom. All Rights Reserved.

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

 $Id: metrics.asp 705872 2017-06-19 19:09:46Z $
-->

<html>
<head>
<meta charset="ISO-8859-1">

<link rel="stylesheet" href="visstyle.css">
<link rel="stylesheet" href="matrics.css">
<script src="jquery-2.0.3.min.js"></script>
<script src="common.js"></script>
<script src="jquery.flot.min.js"></script>
<script src="jquery.flot.axislabels.js"></script>
<script src="jquery.flot.orderBars.js"></script>
<script src="jquery.flot.time.min.js"></script>
<script src="jquery.flot.navigate.min.js"></script>
<script src="jquery.cookie.js"></script>
<script src="matrics.js"></script>
<script src="jquery.flot.pie.js"></script>
<script src="jquery.flot.stack.js"></script>
<title>WiFi Insight</title>
</head>
<body>
	<div id="main_div">
		<div id="page_header">
		<script>commonTopNavBar()</script> <!-- Defined custom menu in common.js -->
		</div>
		<table class="logotable">
			<tr  class="page_title">
				<td colspan="2" class="edge"><img border="0" src="blur_new.jpg" alt=""></td>
			</tr>
			<tr>
				<td  class="page_title"><img border="0" src="logo_new.gif" alt=""></td>
				<td width="100%" valign="top">
					<br>
					<span id="TextHeading">Advanced Troubleshooting</span><br>
					<span id="TextHeadingDesc">In this page you will see most of the counters like AMPDU(if available), Glitch, Chanim and Packet Queue Statistics</span>
				</td>
			</tr>
		</table>
		<div id="contentarea" class="blkbrdr btmbrdr">
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
			</div>
			<table>
				<tr>
					<td>
						<br/><br/>
						<table class = "ampdutable"></table>
					</td>
				</tr>
			</table>
			<br/><br/>
			<div id="countermatrics">
				<div id="counterstatsplace">
				</div>
				<div id="apstainfoplaceholder">
					<table class="ampdustatstable fontfamly cmngraphbox">
						<thead class="cmngraphboxbkgrnd-h1">
							<tr><th>Associated Station's</th></tr>
						</thead>
						<tbody>
							<tr>
								<td width="90%" align="center">
									<table class="apstamappingtbl fontfamly">
										<caption><b>Click on station's to see the Packet Queue Statistics</b><br><br></caption>
										<tbody>
											<tr>
												<td width=30% align="right">
													<table class="apstamapleftcol fontfamily">
														<tbody></tbody>
													</table>
												</td>
												<td class="sprator" align="left" width=5px>
													<div class="staseparator"></div>
												</td>
												<td width=40%>
													<table class="cmntable">
														<tr>
															<td align="center">
																<div class="boxdiv">
																	<font class="fontonwhite">
																	<table class="centerap">
																		<tbody>
																			<tr>
																				<td align="right">SSID</td>
																				<td> : </td>
																			</tr>
																			<tr>
																				<td align="right">BSSID</td>
																				<td> : </td>
																			</tr>
																			<tr>
																				<td align="right">Channel</td>
																				<td> : </td>
																			</tr>
																		</tbody>
																	</table>
																	</font>
																	<img src="ap-image.jpg" width="100px"/>
																</div>
																<div class="verticalline"></div>
																<div class="horizontalline"></div>
															</td>
														</tr>
													</table>
												</td>
												<td class="sprator" align="left" width=5px>
													<div class="staseparator"></div>
												</td>
												<td width=30% align="left">
													<table class="apstamaprightcol fontfamily">
														<tbody></tbody>
													</table>
												</td>
											</tr>
										</tbody>
									</table>
									<table id ="apstalisttable" class="apstainfolisttable fontfamly cmnbrdr">
										<thead class="cmngraphboxbkgrnd-h1">
											<tr>
											<th>SR.</th>
											<th>MAC</th>
											<th>RSSI [dBm]</th>
											<th>PHY Rate [Mbps]</th>
											</tr>
										</thead>
										<tbody></tbody>
									</table>
								</td>
							</tr>
						</tbody>
					</table>
				</div>

				<div id="advancedperstadetdiv">
				</div>
				<!--<table id="stasglitchgrphtbl" class="cmnstasstatsgrphtbl fontfamly cmngraphbox">
					<thead class="cmngraphboxbkgrnd-h1">
						<tr>
							<th>STA's Glitch Counters</th>
						</tr>
					</thead>
					<tbody></tbody>
				</table>
				<table id="packetqueuestatsgrphtbl" class="packetqueuetable fontfamly cmngraphbox">
					<thead class="cmngraphboxbkgrnd-h1">
						<tr>
							<th>Packet Queue Statistics</th>
						</tr>
					</thead>
					<tbody></tbody>
				</table>-->
			</div>

		</div>
	</div>
</body>
</html>
