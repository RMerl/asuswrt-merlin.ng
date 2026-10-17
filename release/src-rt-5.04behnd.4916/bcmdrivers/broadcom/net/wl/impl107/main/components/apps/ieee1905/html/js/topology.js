$(function() {
    $(".wlan-tab li a").button();
    $("#graph").width($("#graph").width() - $('.wlan-tab').height() - parseInt($('body').css('margin-right')));
    $("#graph").height($("#graph").height() - parseInt($('body').css('margin-bottom')));

    var nodeTypes = {
        '1': {color: '#87DEFF', size: 13, borderColor: '#307070' },
        '2': { color: '#FFD700', size: 9, borderColor: '#806020' },
        '3': { color: '#88E08E', size: 7, borderColor: '#58915C' },
    };
    var edgeTypes = { 
        '1': { color: '#806020' , size: 8, weight: 3 },
        '2': { color: '#909090', size: 8, weight: 3 },
    }

    function connect() {
        var intervalID = null;
        // connect websocket
        var ws = $.websocket(get_appropriate_ws_uri(), "1905_protocol",
            function() {  // onopen
                $("#output").text("");
                ws.send("sessionKey="+ sessionKey);
                ws.send("1905init");
                intervalID = setInterval(function() { ws.send("1905delta"); }, 2000);
            }, function(e) {  // onclose
                var r = e.originalEvent ? e.originalEvent.reason : e.reason;
                var c = e.originalEvent ? e.originalEvent.code : e.code;
                var u = ws.url.substr(5);
                if (u[0] == "/") u = u.substr(1);
                if (u[u.length - 1] == "/") u = u.slice(0, -1);
                //alert( "Closing.." + document.URL + " (" + get_appropriate_ws_uri() + ") reason: " + e.code + ".");
                $("#output").text("Closing.." + document.URL + " (" + get_appropriate_ws_uri() + ") reason: " + e.code + ".");
                if (intervalID) clearInterval(intervalID);
            }, function(e) {  // onerror
                // Ack!!!  The failure reason is not given here (apperently some security reason)
                $("#output").text("Websocket error.");
            }, function(m) { //onmessage
                $("#output").text("");
                var s = $.evalJSON(m.originalEvent.data);
                if (s.an) {
                    for (var id in s.an) {
                        var n = s.an[id];
                        if (!n.x) n.x = Math.random();
                        if (!n.y) n.y = Math.random();
                        if (n.Type && nodeTypes[n.Type])
                            n = $.extend(n, nodeTypes[n.Type]);
                        if (!n.color) n.color = '#' + (0x1000000 + (Math.random()) * 0xffffff).toString(16).substr(1, 6);
                        try {
                            sig.addNode(id, n);
                        } catch (ex) { return; }
                    }
                }
                if (s.ae) {
                    for (var id in s.ae) {
                        var n = s.ae[id];
                        if (n.EdgeType && edgeTypes[n.EdgeType])
                            n = $.extend(n, edgeTypes[n.EdgeType]);
                        try {
                            sig.addEdge(id, n.source, n.target, n);
                        } catch (ex) {
                            var nn = [];
                            sig.iterNodes(function(t) { if (t.id == n.source || t.id == n.target) nn.push(t); });
                            if (nn.length == 1 && nn[0].attr.Type == '3')
                                sig.dropNode(nn[0].id);
                            return;
                        }
                    }
                }
                if (s.dn) {
                    for (var id in s.dn)
                        sig.dropNode(id);
                }
                if (s.de) {
                    for (var id in s.de)
                        sig.dropEdge(id);
                }
                if (s.cn) {
                    for (var id in s.cn)
                        sig.iterNodes(function(n) {
                            for (var k in s.cn[id])
                                n[k] = s.cn[id][k];
                        }, [id]);
                }
                if (s.ce) {
                    for (var id in s.ce)
                        sig.iterEdges(function(n) {
                            for (var k in s.ce[id])
                                n[k] = s.ce[id][k];
                        }, [id]);
                }

                if (sig.getNodesCount() > 0) {
                    sig.startForceAtlas2(s.ae || s.an ? 200 : null);
                }
                else {
                    sig.stopForceAtlas2();
                }
            });
        return ws;
    };

    var ws = connect();

    var sig = sigma.init(document.getElementById('graph'));


    var isOnNode=false;
    var isOnTip=false;
    var hideTipTimer;
    var isTipHidden;  //note: there's a bug in qtip where if you hide it when its
                      // already hidden, then next few show's will not work.  Keeping
                      // track manually, so as not to hide it to much.
    const tipTimerTimeout = 600;
    

    sig.drawingProperties({
        edgeLabels: true,
        borderSize: 4,
        //nodeHoverColor: 'default',
        //defaultNodeHoverColor: '#cc0000',
        nodeBorderColor: 'default',
        defaultNodeBorderColor: '#505050',
        labelSize: 'fixed',
        defaultLabelSize: 16, // for fixed display only
        defaultLabelColor: '#404040',
    }).graphProperties({
        minNodeSize: 1,
        maxNodeSize: 75,
        minEdgeSize: 1,
        maxEdgeSize: 10,
    }).mouseProperties({
        minRatio: .5,
        maxRatio: 3
    });

     var tip = $('#graph').qtip({
        id: 'node',
        content: {
            text: '',
            button: true,
        },
        position: {
            at: 'top left',  // Position at top left of graph
        },
        style: {
            classes: 'qtip-rounded qtip-shadow qtip-cluetip',
        },
        show: {
            event: false,
        },
        hide: {
            event: false
        },
        events: {
            hide: function(event, api) {
                // why is this line here???
                sig.startForceAtlas2();
                isTipHidden=true;
            },
            show: function(event, api) {
                 isTipHidden=false;
            },
            mouseleave: function(event,api) {
                isOnTip=false;
                if (isOnNode == false) {
                    hideTipTimer = setTimeout( function(){if (!isTipHidden) tip.hide();}, tipTimerTimeout); 
                }
                
            },
            mouseenter: function(event,api) {
                isOnTip=true;
                clearTimeout(hideTipTimer);
            }
        }
    }).qtip('api');

    sig.bind('downnodes', function(e) {
        if (e.ctrlKey) {
            var n = sig.getNodes(e.content)[0];
            if (n.attr.deviceFriendlyIp) {
                window.open( location.protocol+"//"+n.attr.deviceFriendlyIp );
            }
        }
    });

     sig.bind('overnodes', function(e) {
        //sig.stopForceAtlas2();
        clearTimeout(hideTipTimer);
        isOnNode = true;
        var n = sig.getNodes(e.content)[0];
        tip.set('content.title', n.label || 'Unnamed Node');
        if (n.attr.info)
            tip.set('content.text', n.attr.info);
        else {
            var info = "";
            sig.iterEdges(function(edge) {
                if (n.id == edge.source) {
                    info += "Connect to " + edge.target + "<br/>";
                }
                else if (n.id == edge.target) {
                    info += "Connect from " + edge.source + "<br/>";
                }
            });
            if (n.attr.deviceFriendlyIp) {
                info += "<a href=\"" + location.protocol + "//" + n.attr.deviceFriendlyIp + "\" target=\"_blank\">"+location.protocol + "//" + n.attr.deviceFriendlyIp+"</a>";
           }
            tip.set('content.text', info.length > 0 ? info : '(no info)');
        }
        tip.show();
        var x = n.displayX + 16;
        var y = n.displayY + 16;
        if (x + $('#qtip-node').width() > $('#graph').width()) x -= $('#qtip-node').width() + 16;
        if (y + $('#qtip-node').height() > $('#graph').height()) y -= $('#qtip-node').height() + 16;
        tip.set('position.adjust.x', x);
        tip.set('position.adjust.y', y);
    });

    sig.bind('outnodes', function(e) { 
        isOnNode=false;
        if (isOnTip == false) {
            hideTipTimer = setTimeout( function(){if (!isTipHidden) tip.hide();}, tipTimerTimeout); 
        }
    });

    sig.bind('dragnode', function(e) { clearTimeout(hideTipTimer); if (!isTipHidden) tip.hide(); });

    sig.bind('downgraph', function(e) {
        if (!e.content) {
            sig.startForceAtlas2();
        }
    });

    $('#zoomIn').button().click(function() { sig.zoom(true); });
    $('#zoomOut').button().click(function() { sig.zoom(false); });
    $('#refresh').button().click(function() { location.reload(true); });
    $('#toggleNeighbors').button().click(function() { ws.send("toggleNeighbors"); });
    
});
