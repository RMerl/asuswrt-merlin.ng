/*
 * Jquery implementation for Channels Sub Tab
 *
 * $ Copyright Open Broadcom Corporation $
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: channels.js 682079 2017-01-31 10:30:10Z spalanga $
 */

var timers = [];
var congessioninterfarencegraph;
var timerval = unescape($.cookie('timerinterval'));
var timerdata = [];
if(timerval.length > 0){
	timerdata = timerval.split(',');
}
timerdata[0] = Number(timerdata[0]);
/*****
* Function which will get called when DOM for the page gets loaded.START
*****/
$(document).ready(function(){
	$('#24ghzband').prop('checked',true);
	for(i=0;i<timers.length;i++){
		clearTimeout(timers[i]);
	}
	timers = [];
	
	var errorelem = '<div id="errormsgid" class="errormsg">Please enable the graph in configure page</div>';
	
	var shared = unescape($.cookie('channelscntr'));
	var array = [];
	if(shared.length > 0){
		array = shared.split(',');
	}
	if(array.length > 0){
		index = 0;
		drawBarGraph(array,index);
	}else if($("#errormsgid").length == 0){
		$("#frequencybandselector").append(errorelem);
	}
		
	
	$("#5ghzband").click(function(event){
		for(i=0;i<timers.length;i++){
			clearTimeout(timers[i]);
		}
		timers = [];
		if(array.length > 0)
		drawcongessiongraphfor5frequencyband(array,0);
		else if($("#errormsgid").length == 0)
		$("#frequencybandselector").append(errorelem);
		
	});
	
	$("#24ghzband").click(function(event){
		for(i=0;i<timers.length;i++){
			clearTimeout(timers[i]);
		}
		timers = [];
		if(array.length > 0)
		drawcongessiongraphfor24frequencyband(array,0);
		else if($("#errormsgid").length == 0)
		$("#frequencybandselector").append(errorelem);
	});
});

function drawBarGraph(arr,index){
	var array = [];
	$.ajax({
		type:"GET",
		async:false,
		url:window.location.protocol + '//' + window.location.host +'/json.cgi',		//url:"http://localhost/congession.php",					//
		data:{Req:arr[index],DutMac:'1235',FreqBand:'2.4'},
		success:function(result){
		array = result;		//array = JSON.parse(result);				//
		drawCongessionAndInterferenceGraph(array,2.4);
		}
	});
	timers.push(setTimeout(function(){drawBarGraph(arr,index);},timerdata[0])); 
}
/*****
* Function which will get called when DOM for the page gets loaded.END
*****/

/*****
* Function For drawing the congession and interference Graph.START
*****/
function drawCongessionAndInterferenceGraph(array,fltr){
	var xaxixLabels = [];
	var dataset = [];
	var dataindex = [];
	var k = 0;
	for(i = 0; i < array.XValue.length ; i++){
	if(fltr == 2.4){
		if (array.XValue[i] > 14)
			continue;
	}else if(fltr == 5){
		if(array.XValue[i] < 14)
			continue;
	}
		dataindex.push(i);	
		xaxixLabels.push([k+1,array.XValue[i]]);
		k++;
	}

	for(i = 0; i < array.YValue.length; i++){ 
		var tempy = [];
		for(j = 0; j < dataindex.length; j++){
			tempy.push([j+1,array.YValue[i][dataindex[j]]]);
		}
		dataset.push({label:array.BarHeading[i],hoverable:true,data:tempy});
	}
	
	var options = {
		series:{
			bars:{
				order:1,
				show:true,
				barWidth: 0.1,
				align: 'center',
				horizontal:false,
				fill:1
			}
			
		},
		colors:["#27801F","#5C48EF"],
		xaxis:{
			axisLabel:array.XAxis,
			show:true,
			tickLength:1,
			ticks:xaxixLabels,
			autoscaleMargin:.10
		},
		yaxis:{
			axisLabel:array.YAxis,
			min:0,
			max:100,
			show:true,
			tickLength:20
		},
		grid: { hoverable: true, clickable: true },
		legend: {
            labelBoxBorderColor: "none",
            position: "right"
        }
	};		  
	congessioninterfarencegraph = $.plot($("#CongestiongraphPlaceHolder"),dataset,options);
}
/*****
* Function For drawing the congession and interference Graph.END
*****/


/*****
* Function For showing the tooltips for the bar graph.START
*****/
$(function(){
	$("#CongestiongraphPlaceHolder").bind("plothover",function(event,pos,item){
		$("#tooltipdiv").remove();
		if(item){
			var tooltip = item.series.data[item.dataIndex][1];
			
			 $('<div class="tooltip border fontfamly" id="tooltipdiv">' + tooltip + '</div>')
                .css({
                    top: (item.pageY - 30) + 'px',
                    left: (item.pageX + 10) + 'px'})
                .appendTo("body").fadeIn(200);
				
            showTooltip(item.pageX, item.pageY, tooltip);
		}
	});
});
/*****
* Function For showing the tooltips for the bar graph.END
*****/

/*****
* Function For Modifying the graph axis for different frequency bands.START
*****/
function drawcongessiongraphfor24frequencyband(arr,indx){
	var array=[];
	var congessionarray = [];
	var xaxisticks = [];
	var dataindex = [];
	var k = 0;
	$.get(window.location.protocol + '//' + window.location.host +'/json.cgi',			//"http://localhost/congession.php",				//
		{Req:arr[indx], DutMac:'1235',FreqBand:'2.4'},
		function(result,status){
		array = result;			//array = JSON.parse(result);						//
		drawCongessionAndInterferenceGraph(array,2.4);
		timers.push(setTimeout(function(){drawcongessiongraphfor24frequencyband(arr,indx);},timerdata[0]));
	});
}

function drawcongessiongraphfor5frequencyband(arr,indx){
	var array=[];
	var congessionarray = [];
	var xaxisticks = [];
	var dataindex = [];
	var k = 0;
	$.get(window.location.protocol + '//' + window.location.host +'/json.cgi',			//"http://localhost/congession.php",			//
		{Req:arr[indx], DutMac:'1235',FreqBand:'5'},
		function(result,status){
		array = result;			//array = JSON.parse(result);				//
		drawCongessionAndInterferenceGraph(array,5);
		timers.push(setTimeout(function(){drawcongessiongraphfor5frequencyband(arr,indx);},timerdata[0]));
	});
}

/*****
* Function For Modifying the graph axis for different frequency bands.START
*****/



