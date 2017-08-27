<!--
 HTML part for iQoS Network Summary page

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

 $Id: iQoSNetworkSummary.asp, v 1.0 2015-02-12 18:43:43 $
-->

<!DOCTYPE html>
<html>
<head>
<meta charset="ISO-8859-1">
<link rel="stylesheet" href="iqosstyle.css">
<link rel="stylesheet" href="iQoSNetworkSummary.css">
<script src="jquery-2.0.3.min.js"></script>
<script src="jquery.flot.min.js"></script>
<script src="jquery.flot.time.min.js"></script>
<script src="jquery.flot.pie.js"></script>
<script src="jquery.flot.tooltip.js"></script>
<script src="iqoscommon.js"></script>
<script src="iQoSNetworkSummary.js"></script>

<title>iQoS: Network Summary</title>
</head>
<body>
	<div id="main_div">
		<div id="topnav" class="blackbrdrd">
			<ul class="iqosmenu">
				<li><a href="index.asp">Home</a></li>
				<li><a href="iQoSNetworkSummary.asp">Network Summary</a></li>
				<li><a href="iQoS.asp" onclick="return isValideState()">Device Summary</a></li>
				<li><a href="iQoSSettings.asp" onclick="return isValideState()">Settings</a></li>
			</ul>
		</div>

		<table class="toplogo">
			<tr>
				<td colspan="3" class="topbg"><img border="0" src="blur_new.jpg" alt=""> </td>
			</tr>
			<tr>
				<td width=45% valign=top><img border="0" src="logo_new.gif" alt=""> </td>
			</tr>
		</table>

		<div id="contentarea" class="blkbrdr">
			<!-- QuickSettings Starts.....-->
				<table id="iQoSQuickSettings"  class="ntwrksummarygroupbx" border="0" cellpadding="2" cellspacing="10" >
					<tr>
						<td>
							<table class="quicksettingstbl cmnheadingclass" border="0" cellpadding="0" cellspacing="0">
								<tr>
									<td width="50%" align="left">Quick Settings</td>
								</tr>
							</table>
						</td>
					</tr>
					<tr>
						<td>
							<table class="EnableiQoSCheck" border="0" cellpadding="0" cellspacing="0" style="height:90%;width:50%; margin-top:0px;">
								<tr>
									<td >
										<table border="0" cellpadding="0" cellspacing="0">
											<tr><td>
												<input id= "checkenableiqos" type="checkbox" name="enablecheck" value="enableiqos">Enable IQoS
											</td></tr>
											<tr><td>
												<input id= "checkautobandwidth" type="checkbox" name="autoenablecheck" value="autoenableiqos">
												<label id="auto_bw_lbl" for="checkautobandwidth">Determine my Bandwidth Automatically</label>
											</td></tr>
											<tr>
												<td>
													<table id = "bandwidthenabledid" class= "bandwidthenabled" border="0" cellpadding="0" cellspacing="10" style="height:90%;width:100%;">
														<tr>
															<td>Download Speed</td>
															<td><input id="downloadspeedtext" type="text" name="download" value="0" style="width: 30%; height: 15px;" disabled>  Mbps </td>
														</tr>
														<tr>
															<td> Upload Speed </td>
															<td><input id="uploadspeedtext" type="text" name="upload" value="0" style="width: 30%; height: 15px;" disabled>  Mbps </td>
														</tr>
													</table>
												</td>
											</tr>
										</table>
									</td>
									<td>
										<table border="0" cellpadding="0" cellspacing="5">
											<tr><td>
												<input id = "testbandwidthbtn" type="submit" class="cmnbtnclass" style="height:30px; width:100%;" value=" Test Bandwidth ">
											</td></tr>
											<tr><td>
												<div class="cmnprogressbar" style="width:100%; margin:0px" >  </div>
											</td></tr>
											<tr><td>
												<br/>
												<input id = "quicksubmitbtn" type="submit" class="cmnbtnclass" style="height:30px; width:100%;" value="Submit">
											</td></tr>
											<tr><td>
												<input id="quickresetbtn" type="reset" class="cmnbtnclass" style="height:30px; width:100%;" value="Reset">
											</td></tr>
										</table>
									</td>
								</tr>
							</table>
					</td></tr>
				</table>
			<!-- QuickSettings Ends.....-->

			<!-- NetworkLayout Starts.....-->
			<table class="NetworkLayout ntwrksummarygroupbx" border="0" cellpadding="2" cellspacing="10">
				<tr>
					<td>
						<table class="networklayoutheading cmnheadingclass" border="0" cellpadding="0" cellspacing="0">
							<tr>
								<td align="left">Network Layout</td>
							</tr>
						</table>
					</td>
				</tr>
				<tr>
					<td>
						<table cellspacing="0" border="0" width= 95% class="networklayoutmappingtbl">
							<tbody>
								<tr>
									<td width=35%>
										<table class="networklayoutmapleftcol" border="0" cellpadding="0" cellspacing="0">
											<tbody></tbody>
										</table>
									</td>
									<td align="center" width=1%>
										<table border="0" cellpadding="0" cellspacing="0">
											<tr>
												<td class="separator" align="left">
													<div class="networklayoutseperator cmnlineclass cmnverticalline"></div>
												</td>
												<td>
													<table align="center" border="0" cellpadding="0" cellspacing="0">
														<tr><td align="center"><img src="www.png" width="100px" hspace="20"/></td></tr>
														<tr><td align="center"><div class="centerverticalline cmnlineclass cmnverticalline"></div></td></tr>
														<tr><td align="center"><img src="router.png" width="100px" hspace="20"/></td></tr>
														<tr><td align="center"><div class="centerverticalline cmnlineclass cmnverticalline" style="height:30px;"></div></td></tr>
														<tr><td><div class="cmnlineclass cmnhorizontalline" style="width:100%; background:grey;"></div></td></tr>
													</table>
												</td>
												<td class="separator">
													<div class="networklayoutseperator cmnlineclass cmnverticalline"></div>
												</td>
											</tr>
										</table>
									</td>
									<td width=35%>
										<table class="networklayoutmaprightcol" border="0" cellpadding="0" cellspacing="0">
											<tbody></tbody>
										</table>
									</td>
									<td width=10% valign="bottom">
									</td>
								</tr>
							</tbody>
						</table>
					</td>
				</tr>
			</table>
			<!-- NetworkLayout Ends.....-->

			<!-- Bandwidth Usage Starts.....-->
			<table class="BandwidthUsage ntwrksummarygroupbx" border="0" cellpadding="2" cellspacing="10">
				<tr>
					<td>
						<table class="bandiwdthusageheading cmnheadingclass" border="0" cellpadding="0" cellspacing="0">
							<tr>
								<td align="left">Bandwidth Usage</td>
							</tr>
						</table>
					</td>
				</tr>
				<tr>
					<td>
						<table class="bandwidthpiechart" border="0" cellpadding="0" cellspacing="5" style="width:100%;">
							<tr>
								<td>
									<div class = "downband cmnheadingclass">Down Stream</div><br>
								</td>
								<td>
									<div class = "upband cmnheadingclass">Up Stream</div><br>
								</td>
							</tr>
							<tr>
								<td width="50%">
									<div id="downstreampiechart"  class="bwpielegend"></div>
									<div id="downstreampiechartlegend" class="bwpielegend-class"></div>
								</td>
								<td width="50%">
									<div id="upstreampiechart" class="bwpielegend"></div>
									<div id="upstreampiechartlegend" class="bwpielegend-class"></div>
								</td>
							</tr>
						</table>
					</td>
				</tr>
			</table>

		<!-- Bandwidth Usage Ends.....-->
		</div>
	</div>
</body>
</html>
