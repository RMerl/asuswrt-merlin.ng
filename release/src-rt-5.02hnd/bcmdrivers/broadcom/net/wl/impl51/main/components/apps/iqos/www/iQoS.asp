<!--
 HTML part for iQoS Device Summary page

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

 $Id: iQoS.asp, v 1.0 2015-02-12 18:43:43 $
-->

<!DOCTYPE html>
<html>
<head>
	<meta charset="ISO-8859-1">
	<link rel="stylesheet" href="iqosstyle.css">
	<link rel="stylesheet" href="iQoS.css">
	<script src="jquery-2.0.3.min.js"></script>
	<script src="jquery.flot.min.js"></script>
	<script src="jquery.flot.axislabels.js"></script>
	<script src="jquery.flot.time.min.js"></script>
	<script src="iqoscommon.js"></script>
	<script src="iQoS.js"></script>
	<title>iQoS: Device Summary</title>
</head>
<body>

	<div id="main_div">
		<div id="topnav">
			<ul class="iqosmenu">
				<li><a href="index.asp">Home</a></li>
				<li><a href="iQoSNetworkSummary.asp">Network Summary</a></li>
				<li><a href="iQoS.asp">Device Summary</a></li>
				<li><a href="iQoSSettings.asp">Settings</a></li>
			</ul>
		</div>

		<table class="toplogo">
			<tr>
				<td colspan="3" class="topbg"><img border="0" src="blur_new.jpg" alt=""> </td>
			</tr>
			<tr>
				<td width=45% valign=top><img border="0" src="logo_new.gif" alt=""> </td>
				<td width=45% align=right class="textheading">Select Device<select id="devlist" class="devlist"></select> </td>
				<td width=45% align=left class="Images"> <img border="0" width=100px height=100px src="computer.jpg"> </td>
			</tr>
		</table>

		<div id="contentarea" class="blkbrdr">

			<!--DeviceSummaryTable Starts(1st Table)-->
			<div id="DeviceSummaryTable">
			<p>
				<div>
					<div class="iQoSSummaryHeading">
						<table class="deviceselectedtbl"><tr><td>Summary : </td></tr></table>
					</div>

					<div class="iQoSSystemDetails">
						<table class="systemdetailstable">
							<tr> <td>IP Address:</td> </tr>
							<tr> <td>OS: </td> </tr>
							<tr> <td>Last Internet Access: </td> </tr>
						</table>
					</div>
					<div id="iQoSDataTransferDetails" class="iQoSDataTransferDetails iQoSSystemDetails">
						<table class= "datatransferdetails">
							<tr> <td>Current Downstream Bandwidth Used: </td> </tr>
							<tr> <td>Current Upstream Bandwidth Used:</td> </tr>
							<tr> <td>Bandwidth Used in Network (Downstream + Upstream): </td> </tr>
							<tr> <td>Total data Downloaded in last Session(5 Seconds): </td> </tr>
							<tr> <td>Total data Uploaded in last Session(5 seconds): </td> </tr>
						</table>
					</div>
				</div>
			</p>
			</div>
			<!--DeviceSummaryTable Ends(1st Table)-->

			<!--DeviceApplicationTable Starts(2nd Table)-->
			<div id="DeviceApplicationTable">
				<table class="appheading">
					<tr>
						<td width=50%>
							<div class="iQoSSummaryHeading">
								<table class="appselectedtbl" align="left"><tr><td>Application : </td></tr></table>
							</div>
						</td>
						<td width=50%>
							<div class="AppBandwidthHeading">
								<table class="appselectedtbl1" align="left"><tr><td>Downstream Bandwidth Usage by Applications (Mbps)</td></tr></table>
							</div>
						</td>
					</tr>
					<tr>
						<td width=50%>
							<div class="DeviceImages">
								<image src="computer.jpg" width="50" height="50"/>
							</div>
						</td>
						<td width=50%>
							<table class=bandwidthxaxis>
								<tr width=100%> <td class="hlines"></td> </tr>
								<tr width=100%> <td class="bandwidthyaxis"></td></tr>
							</table>
						</td>
					</tr>
				</table>
				<div class="vertical-line"> </div>

				<!--Applications Stats Data-->
				<table class="applicationlist"></table>
			</div>
			<!--DeviceApplicationTable Ends(2nd Table)-->

			<!--Link Rate, Bandwidth Usage Category Table Starts(3rd Table)-->
			<div id = "LinkBandwidthTable">
				<div>
					<table width=100%>
						<tr>
							<td width=50%>
								<div class="LinkRateHeading">
									<table class="appselectedtbl1">
										<tr><td>Bandwidth Used</td></tr>
									</table>
								</div>
							</td>
							<td width=30%>
								<div class="BandwidthCategoryHeading">
									<table class="appselectedtbl2">
										<tr><td>Bandwidth Usage by Categories (Mbps)</td></tr>
									</table>
								</div>
							</td>
							<td width=20%>
								<div class="selectcategory">
									<button id="selectcategorybtn" class="cmnbtnclass" type="button">Select Category &#8681;</button>
									<table class="selectcategorytbl">
										<tr><td><input id="selectcategorychk" type="checkbox" value="Select">Select</input></td></tr>
									</table>
								</div>
							</td>
						</tr>
					</table>
				</div>
				<div>
					<table width=100%>
						<tr>
							<td><div id="LinkRateGraphContent"></div></td>
							<td><div id="BandwidthBarGraph"></div></td>
						</tr>
					</table>
				</div>

			</div>
			<!--Link Rate, Bandwidth Usage Category Table Ends(3rd Table)-->
		</div>
	</div>
</body>
</html>
