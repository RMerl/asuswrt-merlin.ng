<!DOCTYPE html>
<!--
 HTML part for Zero Wait DFS Demo Tab

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

 $Id: zerowaitdfs.asp 603186 2015-12-01 07:49:11Z $
-->

<html>
<head>
<meta charset="ISO-8859-1">
<link rel="stylesheet" href="visstyle.css">
<link rel="stylesheet" href="zerowaitdfs.css">
<script src="jquery-2.0.3.min.js"></script>
<script src="common.js"></script>
<script src="jquery.flot.min.js"></script>
<script src="jquery.flot.orderBars.js"></script>
<script src="jquery.flot.axislabels.js"></script>
<script src="jquery.flot.stack.js"></script>
<script src="jquery.cookie.js"></script>
<script src="zerowaitdfs.js"></script>
<title>WiFi Insight</title>
</head>
<body>
	<div id="main_div">
		<script>zeroWaitDFSTopNavBar()</script> <!-- Defined custom menu in common.js -->

		<table class="logotable">
			<tr>
				<td colspan="2" class="edge"><img border="0" src="blur_new.jpg" alt=""></td>
			</tr>
			<tr>
				<td><img border="0" src="logo_new.gif" alt=""></td>
				<td width="100%" valign="top">
					<br>
					<span id="TextHeading">Zero Wait DFS</span><br>
					<span id="TextHeadingDesc">In this page we will demonstrate that there is no
						interruption of video when moving from non DFS to DFS channel and no
						disassociation of client when moving to 3X3</span>
				</td>
			</tr>
		</table>
		<div id="contentarea" class="btmbrdr">
			<br>
			<div id="showerrmessage">
			</div>

			<div id="zerowaitdfscontent" class="fontfamly cmtxtclr" align="center">
				<table class="zwdcontenttbl" align="center">
					<tbody>
						<tr>
							<td width="35%">
								<div id="actionbtns">
									<button id="switchtodfsbtn" type="button" class="cmnbtnclass cmnzwdbtnclass">Move to DFS Channel</button>
									<select id="dfschannelselectctrl" class="cmnzwdselectclass cmnctrlborder">
										<option>Select Channel</option>
									</select>
									<button id="triggerradarbtn" type="button" class="cmnbtnclass cmnzwdbtnclass">Simulate Radar</button>
									<select id="coreselectctrl" class="cmnzwdselectclass cmnctrlborder">
										<option value="0">Select Core</option>
										<option value="2">Primary Core</option>
										<option value="3">Scan Core</option>
									</select>
								</div>
							</td>
							<td width="1%">
								<div class="apdescverline">
								</div>
							</td>
							<td width="49%">
								<div id="apstatusdiv">
									<div class="phymodeheadclass" align="center">
										RADIO PHY MODE
									</div>
									<div class="primarycoredivclass">
										<div class="apstatusheader cmngraphboxbkgrnd-h1">
											<div class="divheading">Primary Core</div>
										</div>
										<table class="primarycoretbl cmndesctableclass">
											<tbody>
												<tr>
													<td align="right">Channel</td>
													<td> : </td>
												</tr>
												<tr>
													<td align="right">Tx / RX Chanins</td>
													<td> : </td>
												</tr>
												<tr>
													<td align="right">Phy Rate</td>
													<td> : </td>
												</tr>
												<tr>
													<td align="right">NRate</td>
													<td> : </td>
												</tr>
											</tbody>
										</table>
									</div>
									<div class="scancoredivclass">
										<div class="apstatusheader cmngraphboxbkgrnd-h1">
											<div class="divheading">Scan Core</div>
										</div>
										<table class="scancoretbl cmndesctableclass">
											<tbody>
												<tr>
													<td align="right">Channel</td>
													<td> : </td>
												</tr>
												<tr>
													<td align="right">Tx / RX Chanins</td>
													<td> : </td>
												</tr>
												<tr>
													<td align="right">Time Elapsed</td>
													<td> : </td>
												</tr>
											</tbody>
										</table>
									</div>
								</div>
							</td>
						</tr>
					</tbody>
				</table>
			</div>

		</div>
	</div>
</body>
</html>
