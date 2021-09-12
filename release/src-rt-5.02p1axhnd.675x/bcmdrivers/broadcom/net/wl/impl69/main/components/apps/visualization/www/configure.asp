<!DOCTYPE html>
<!--
 HTML part for Configure Tab

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

 $Id: configure.asp 705872 2017-06-19 19:09:46Z $
-->

<html>
<head>
<meta charset="ISO-8859-1"/>
<link rel="stylesheet" href="visstyle.css"/>
<link rel="stylesheet" href="configure.css"/>
<script src="jquery-2.0.3.min.js"></script>
<script src="common.js"></script>
<script src="jquery.cookie.js"></script>
<script src="configure.js"></script>
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
					<span id="TextHeading">Configure</span><br>
					<span id="TextHeadingDesc">In this page you will be able to configure the WiFi Insight system</span>
				</td>
			</tr>
		</table>
		<div id="contentarea" class="btmbrdr">
			<br>
			<div id="configformdiv">
				<form class="configfrm">
					<fieldset class="fontfamly cmnbrdr border cmnipt cmtxtclr">
						<legend>Sample Interval</legend>
						<input type="radio" class="smplinterval cmncheckboxclass" name="smplinterval" value="5" checked="checked">5 Second
						<input type="radio" class="smplinterval cmncheckboxclass" name="smplinterval" value="10">10 Second
						<input type="radio" class="smplinterval cmncheckboxclass" name="smplinterval" value="15">15 Second
						<input type="radio" class="smplinterval cmncheckboxclass" name="smplinterval" value="20">20 Second
					</fieldset>

					<fieldset class="fontfamly cmnbrdr border cmnipt cmtxtclr">
						<legend>Start/Stop Data Collection</legend>
						<button id="startbutton" value="Start" type="button" class="cmnbtnclass" style="margin-left:15px;">Start Data Collection</button>
						<br/><br/>
						<input class="cmncheckboxclass" id="autostartid" type="checkbox" name="autostart" value="0">Start collecting data every
						<br/><br/>
						<div class="autostartdivclass">
							<input class="cmncheckboxclass" id="weekdaycheckboxid" type="checkbox" value="Sunday">Sunday
							<input class="cmncheckboxclass" id="weekdaycheckboxid" type="checkbox" value="Monday">Monday
							<input class="cmncheckboxclass" id="weekdaycheckboxid" type="checkbox" value="Tuesday">Tuesday
							<input class="cmncheckboxclass" id="weekdaycheckboxid" type="checkbox" value="Wednesday">Wednesday
							<input class="cmncheckboxclass" id="weekdaycheckboxid" type="checkbox" value="Thursday">Thursday
							<input class="cmncheckboxclass" id="weekdaycheckboxid" type="checkbox" value="Friday">Friday
							<input class="cmncheckboxclass" id="weekdaycheckboxid" type="checkbox" value="Saturday">Saturday
							<br/><br/>
							<label>From</label><input id="fromtimeentry" type="text" name="usr_time" readonly>
							<label>To</label><input id="totimeentry" type="text" name="usr_time" readonly>
							<!-- For selecting Time -->
							<div class="timeselectdiv">
								<div class="timeselectclosebtn" id="timeselectclosebtnid" align="center">X</div>
								<table class="timeselecttbl">
									<tr>
										<td align="center"><button id="hourplusid" value="Plus" type="button" class="timeselectbtn">+</button></td>
										<td align="center"><button id="minuteplusid" value="Plus" type="button" class="timeselectbtn">+</button></td>
										<td align="center"><button id="ampmplusid" value="Plus" type="button" class="timeselectbtn">+</button></td>
									</tr>
									<tr>
										<td align="center"><span class="hourentry">00</span></td>
										<td align="center"><span class="minuteentry">00</span></td>
										<td align="center"><span class="ampmentry">AM</span></td>
									</tr>
									<tr>
										<td align="center"><button id="hourminusid" value="Plus" type="button" class="timeselectbtn">-</button></td>
										<td align="center"><button id="minuteminusid" value="Plus" type="button" class="timeselectbtn">-</button></td>
										<td align="center"><button id="ampmminusid" value="Plus" type="button" class="timeselectbtn">-</button></td>
									</tr>
								</table>
							</div>
						</div>
					</fieldset>

					<fieldset class="fontfamly cmnbrdr border cmnipt cmtxtclr">
						<legend>Database Size</legend>
						<label>Database Size</label>
						<input id="dbsizeinmb" type="text" class="dbsize" name="dbsize"  onkeyup="this.value=this.value.replace(/[^\d]/,'')"/>
						<label>MB</label>
						<br/>
						<label id="dbwarningtxtid" class="warningtxtcolor"></label>
						<br/><br/>
						<label>Once Database size reaches maximum limit</label>
						<input type="radio" name="overwritedb" value="overwritedata" id="overwritedataid">Overwrite Older Data
						<input type="radio" name="overwritedb" value="stopdcoll" id="stopdcollid">Stop Datacollection
					</fieldset>

					<fieldset id="countervals" class="fontfamly cmnbrdr border dspblk cmnipt cmtxtclr">
						<legend>Counters</legend>
						<fieldset id="fst"></fieldset>
						<fieldset id="scnd"></fieldset>
					</fieldset>

					<fieldset id="smbtn" class="zerobrdr">
						<button id="submitbutton" type="button" class="cmnbtnclass">Submit</button>
					</fieldset>

					<fieldset id="idmcpusupport" class="fontfamly cmnbrdr border cmnipt cmtxtclr">
						<legend>Remote Debugging</legend>
						<input class="cmncheckboxclass" id="remoteenablechkid" type="checkbox" name="remoteenable" value="0">Enable Remote Debugging
						<button id="startremotedebug" value="Start" type="button" class="cmnbtnclass" style="margin-left:15px;">Apply Remote Settings</button>
						<div class="progressbarclass"></div>
						<div class="remoteipdiv"><br/><br/><label>Server IP Address</label>
							<input id="remoteipid" type="text" class="remoteip" name="remoteip"/>
						</div>
					</fieldset>
				</form>
				<br/>
				<form method="link" action="visdata.db" class="configfrm">
					<fieldset class="fontfamly cmnbrdr border cmnipt cmtxtclr">
						<legend>Export Database</legend>
						<label>Download Database File</label>
						<input id="exportdbbtn" type="submit" value="Save Database to File" class="cmnbtnclass"/>
					</fieldset>
				</form>
			</div>
		</div>
	</div>
</body>
</html>
