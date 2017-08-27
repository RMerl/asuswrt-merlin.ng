<!DOCTYPE html>
<!--
 HTML part for Channels sub Tab

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

 $Id: channels.asp 603186 2015-12-01 07:49:11Z $
-->

<html>
<head>
<meta charset="ISO-8859-1">
<link rel="stylesheet" href="visstyle.css">
<script src="jquery-2.0.3.min.js"></script>
<script src="jquery.flot.min.js"></script>
<script src="jquery.flot.orderBars.js"></script>
<script src="jquery.flot.axislabels.js"></script>
<script src="jquery.cookie.js"></script>
<script src="channels.js"></script>
<title>Visualization</title>
</head>
<body>
<!--
	<div id="headerplaceholder">
			<div id="topheader">
				<image id="logoplaceholder" src="../Images/broadcom.png">
				</image>
			</div>
	</div>
-->

	<div id="main_div">
		<div id="topnav" class="blackbrdrd">
			<div id="containerdiv">
				<ul class="custommenu">
				<li><a href="index.asp">Home</a></li>
					<li class="sitesurveytab">
						<a href="visindex.asp" class="selected">Site Survey</a>
						<ul>
							<li>
								<a href="visindex.asp">Networks</a>
							</li>
							<li>
								<a href="channels.asp">Channels</a>
							</li>
						</ul>
					</li>
					
					<li class="chnlcapacitytab"><a href="channelcapacity.asp">Channel Capacity</a></li>
					
					<li class="metricstab"><a href="metrics.asp">Metrics</a></li>
					
					<li><a href="configure.asp">Configure</a></li>
				</ul>
			</div>
		</div>
		
		<div id="contentarea" class="blkbrdr btmbrdr">
			<div class="tabheadings">
			<image src="tabheading.jpg" width="100%"/>
			<!--<p>Site Survey > Channels</p>-->
			</div>
			
			<div id="frequencybandselector">
				<form id="frequencybandform">
				<fieldset class="fontfamly cmnbrdr border ftalign">
				<input type="radio" name="frequencyband" id="24ghzband" checked="checked">2.4 GHZ</input>
				<input type="radio" name="frequencyband" id="5ghzband">5 GHZ</input>
				</fieldset>
				</form>
			</div>
			
			<div id="CongestiongraphPlaceHolder">
			</div>
			
		</div>
	</div>
</body>
</html>
