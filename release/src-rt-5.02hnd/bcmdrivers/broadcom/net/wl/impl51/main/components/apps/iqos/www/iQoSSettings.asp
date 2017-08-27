<!--
 HTML part for iQoS Settings page

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

 $Id: iQoSSettings.asp,v 1.0 2015-02-12 18:43:43 $
-->

<!DOCTYPE html>
<html>
<head>
<meta charset="ISO-8859-1">
<link rel="stylesheet" href="iqosstyle.css">
<link rel="stylesheet" href="iQoSSettings.css">
<script src="jquery-2.0.3.min.js"></script>
<script src="jquery-ui.js"></script>
<script src="jquery.flot.min.js"></script>
<script src="jquery.flot.tooltip.js"></script>
<script src="jquery.min.js"></script>
<script src="jquery-ui.min.js"></script>
<script src="iqoscommon.js"></script>
<script src="iQoSSettings.js"></script>

<title>iQoS: Settings</title>
</head>
<body>
	<div id="main_div">
		<div id="topnav" class="blackbrdrd">
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
			</tr>
		</table>

		<div id="contentarea" class="blkbrdr">
			<table id = "PriorityTable" border="0" cellpadding="2" cellspacing="10">
				<tr><td>
					<div class="deviceprioritytext">Drag and Drop Devices to change Priority</div>
				</td></tr>
			</table>
			<table id = "PriorityTableSummary">
				<tr>
					<td class = "devicepriorityhdg">Application Category Priority</td>
					<td class = "devicepriorityhdg">Device Type Priority</td>
					<td class = "devicepriorityhdg">My Device Priority</td>
				</tr>
				<tr>
					<td>
						<!--Application Type Priority Starts...-->
						<div id="maincointainerdiv" class="prioritybox">
							<div id="dvApp">
								<p class="Prioritytag">Priority 0 (Highest)</p>
								<ul class="priorityappclass" id="sortableulappid0" Priority="0"></ul>
							</div>
							<div id="dvApp">
								<p class="Prioritytag">Priority 1</p>
								<ul class="priorityappclass" id="sortableulappid1" Priority="1"></ul>
							</div>
							<div id="dvApp">
								<p class="Prioritytag">Priority 2</p>
								<ul class="priorityappclass" id="sortableulappid2" Priority="2"></ul>
							</div>
							<div id="dvApp">
								<p class="Prioritytag">Priority 3</p>
								<ul class="priorityappclass" id="sortableulappid3" Priority="3"></ul>
							</div>
							<div id="dvApp">
								<p class="Prioritytag">Priority 4</p>
								<ul class="priorityappclass" id="sortableulappid4" Priority="4"></ul>
							</div>
							<div id="dvApp">
								<p class="Prioritytag">Priority 5</p>
								<ul class="priorityappclass" id="sortableulappid5" Priority="5"></ul>
							</div>
							<div id="dvApp">
								<p class="Prioritytag">Priority 6</p>
								<ul class="priorityappclass" id="sortableulappid6" Priority="6"></ul>
							</div>
							<div id="dvApp">
								<p class="Prioritytag">Priority 7 (Lowest)</p>
								<ul class="priorityappclass" id="sortableulappid7" Priority="7"></ul>
							</div>
						</div>
						<!--Application Type Priority Ends!!-->
					</td>
					<td>
						<!--Device Type Priority Starts...-->
						<div id="maincointainerdiv1" class="prioritybox">
							<div id="dvDest">
								<p class="Prioritytag">Priority 0 (Highest)</p>
								<ul class="priorityclass" id="sortableulid0" Priority="0"></ul>
							</div>
							<div id="dvDest">
								<p class="Prioritytag">Priority 1</p>
								<ul class="priorityclass" id="sortableulid1" Priority="1"></ul>
							</div>
							<div id="dvDest">
								<p class="Prioritytag">Priority 2</p>
								<ul class="priorityclass" id="sortableulid2" Priority="2"></ul>
							</div>
							<div id="dvDest">
								<p class="Prioritytag">Priority 3</p>
								<ul class="priorityclass" id="sortableulid3" Priority="3"></ul>
							</div>
							<div id="dvDest">
								<p class="Prioritytag">Priority 4</p>
								<ul class="priorityclass" id="sortableulid4" Priority="4"></ul>
							</div>
							<div id="dvDest">
								<p class="Prioritytag">Priority 5</p>
								<ul class="priorityclass" id="sortableulid5" Priority="5"></ul>
							</div>
							<div id="dvDest">
								<p class="Prioritytag">Priority 6</p>
								<ul class="priorityclass" id="sortableulid6" Priority="6"></ul>
							</div>
							<div id="dvDest">
								<p class="Prioritytag">Priority 7 (Lowest)</p>
								<ul class="priorityclass" id="sortableulid7" Priority="7"></ul>
							</div>
						</div>
						<!--Device Type Priority Ends!!-->
					</td>
					<td>
						<!--Mac Priority Start...-->
						<div id="maincointainerdiv2" class="prioritybox">
							<div id="dvMac">
								<p class="Prioritytag">Priority 0 (Highest)</p>
								<ul class="prioritymacclass" id="sortableulmacid0" Priority="0"></ul>
							</div>
							<div id="dvMac">
								<p class="Prioritytag">Priority 1</p>
								<ul class="prioritymacclass" id="sortableulmacid1" Priority="1"></ul>
							</div>
							<div id="dvMac">
								<p class="Prioritytag">Priority 2</p>
								<ul class="prioritymacclass" id="sortableulmacid2" Priority="2"></ul>
							</div>
							<div id="dvMac">
								<p class="Prioritytag">Priority 3</p>
								<ul class="prioritymacclass" id="sortableulmacid3" Priority="3"></ul>
							</div>
							<div id="dvMac">
								<p class="Prioritytag">Priority 4</p>
								<ul class="prioritymacclass" id="sortableulmacid4" Priority="4"></ul>
							</div>
							<div id="dvMac">
								<p class="Prioritytag">Priority 5</p>
								<ul class="prioritymacclass" id="sortableulmacid5" Priority="5"></ul>
							</div>
							<div id="dvMac">
								<p class="Prioritytag">Priority 6</p>
								<ul class="prioritymacclass" id="sortableulmacid6" Priority="6"></ul>
							</div>
							<div id="dvMac">
								<p class="Prioritytag">Priority 7 (Lowest)</p>
								<ul class="prioritymacclass" id="sortableulmacid7" Priority="7"></ul>
							</div>
						</div>
						<!--Mac Priority Ends!!-->
					</td>
				</tr>
			</table>
			<table id = "buttons">
				<tr>
					<td width=30% />
					<td width=30%><div class="cmnprogressbar">  </div></td>
					<td width=30% />
				</tr>
				<tr>
					<td width=30% />

					<td width=30%>
						<input id = "submitbutton" class = "cmnbtnclass btnwidth" type="submit" value="Submit"/>
						<input id = "cancelbutton" class = "cmnbtnclass btnwidth" type="reset" value="Cancel"/>
						<input id = "restorebutton" class = "cmnbtnclass btnwidth" type="submit" value="Restore Defaults"/>
					</td>
					<td width=30% />
				</tr>
			</table>
		</div>

	</div>

</body>
</html>
