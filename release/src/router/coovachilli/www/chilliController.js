chilliController.onUpdate = updateUI ;
chilliController.onError  = handleError ;
chilliClock.onTick = function () { }

if (!window.queryObj) {
    window.queryObj = new Object();
    window.location.search.replace(new RegExp("([^?=&]+)(=([^&]*))?","g"), function($0,$1,$2,$3) { queryObj[$1] = $3; });
}

chilliController.queryObj = window.queryObj;

function ie_getElementsByTagName(str) {
  if (str=="*") return document.all;
  else return document.all.tags(str);
}

if (document.all) 
  document.getElementsByTagName = ie_getElementsByTagName;

function hidePage(page) { 
    var e = document.getElementById(page);
    if (e != null) e.style.display='none';
}

function showPage(page) { 
    var e = document.getElementById(page);
    if (e != null) e.style.display='inline';
}

function setElementValue(elem, val, forceHTML) {
    var e = document.getElementById(elem);
    if (e != null) {
	var node = e;
	if (!forceHTML && node.firstChild) {
	    node = node.firstChild;
	    node.nodeValue = val;
	} else {
	    node.innerHTML = val;
	}
    }
}

chilliClock.onChange = function ( newval ) {
    setElementValue("sessionTime", chilliController.formatTime(newval));
}
    
function updateUI (cmd ) {
    log ( "Update UI is called. chilliController.clientState = " + chilliController.clientState ) ; 
    
    clearTimeout ( delayTimer );

    if ( chilliController.redir ) {
	if (chilliController.redir.originalURL != null &&
	    chilliController.redir.originalURL != '') {
	    setElementValue('originalURL', '<a target="_blank" href="'+chilliController.redir.originalURL+
			    '">'+chilliController.redir.originalURL+'</a>', true);
	}
	if (chilliController.redir.redirectionURL != null &&
	    chilliController.redir.redirectionURL != '') {
	    setElementValue('redirectionURL', chilliController.redir.redirectionURL);
	}
    }

    if ( chilliController.message ) {
	setElementValue('logonMessage', chilliController.message);
	chilliController.message = null;
	chilliController.refresh();
    }

    if ( chilliController.location ) {
	setElementValue('locationName', chilliController.location.name);
	chilliController.location = null;
    }

    if ( chilliController.clientState == 0 ) {
        showLogonPage();
    }

    if ( chilliController.clientState == 1 ) {
        if ( chilliController.statusURL ) {
	   chilliController.statusWindow = window.open(chilliController.statusURL, "");
	} else {
	   showStatusPage();
        }
    }

    if (chilliController.redir.redirectionURL) {
	//chilliController.nextWindow = window.open(chilliController.redir.redirectionURL,'nextURL');
	window.location.href = chilliController.redir.redirectionURL;
	chilliController.redir.redirectionURL = null;
    }
    
    if ( chilliController.clientState == 2 ) showWaitPage();
}

function handleError( code ) {
    clearTimeout(delayTimer);
    //showErrorPage(code);
}

/* Action triggered when buttons are pressed */
function connect() {
    var username =  document.getElementById('username').value ;
    var password =  document.getElementById('password').value ;

    if (username == null || username == '')
	return setElementValue('logonMessage', 'Username is required');
    
    showWaitPage(1000);
    chilliController.logon( username , password ) ;
}

function disconnect() {
    if (confirm("Are you sure you want to disconnect now?")) {
	chilliClock.stop();
	showWaitPage(1000);
	chilliController.logoff();
    }
    return false;
}

/* User interface pages update */
function showLogonPage() {
    if (chilliController.openid) 
        showPage('openIDSelect');
    showPage("logonPage");
    hidePage("statusPage");
    hidePage("waitPage");
    hidePage("errorPage");
}

function showStatusPage() {
    hidePage("logonPage");
    showPage("statusPage");
    hidePage("waitPage");
    hidePage("errorPage");
    
    // Update message
    if ( chilliController.message ) { 
	setElementValue("statusMessage", chilliController.message);
    }
    
    // Update session
    setElementValue("sessionId",
		    chilliController.session.sessionId ?
		    chilliController.session.sessionId :
		    "Not available");

    setElementValue("startTime",
		    chilliController.session.startTime ?
		    chilliController.session.startTime :
		    "Not available");
    
    setElementValue("sessionTimeout",
		    chilliController.formatTime(chilliController.session.sessionTimeout, 'unlimited'));

    setElementValue("idleTimeout",
		    chilliController.formatTime(chilliController.session.idleTimeout, 'unlimited'));

    setElementValue("maxInputOctets",
		    chilliController.formatBytes(chilliController.session.maxInputOctets));
    setElementValue("maxOutputOctets",
		    chilliController.formatBytes(chilliController.session.maxOutputOctets));
    setElementValue("maxTotalOctets",
		    chilliController.formatBytes(chilliController.session.maxTotalOctets));

    // Update accounting
    setElementValue("sessionTime",
		    chilliController.formatTime(chilliController.accounting.sessionTime));
    
    setElementValue("idleTime",
		    chilliController.formatTime(chilliController.accounting.idleTime));
    
    setElementValue("inputOctets" , chilliController.formatBytes(chilliController.accounting.inputOctets));
    setElementValue("outputOctets", chilliController.formatBytes(chilliController.accounting.outputOctets));
    
    chilliClock.resync (chilliController.accounting.sessionTime);
}

function showOpenIDForm(e)
{
     var form = document.getElementById('openIDForm');
     var x = document.createElement('div');
     x.style.display = 'block';
     x.style.position = 'absolute';
     x.style.top = e.y - 25;
     x.style.left = e.x + 25;
     x.style.xIndex = 2;
     x.innerHTML = form.innerHTML;
     document.body.appendChild(x);
}

function openID() {
  var openIDSelect = document.getElementById('openIDSelect');

  openIDSelect.onclick = function(e) {
     if (!e) e = window.event;
     e.stopPropagation;
     showOpenIDForm(e);
  };

  var openIDForm = document.getElementById('openIDForm');

  openIDForm.onclick = function(e) {
    if (!e) e = window.event;
    e.stopPropagation;
  };

  document.onclick = closeOpenIDForm();
}

function closeOpenIDForm() {
  hidePage('openIDForm');
}

function showWaitPage(delay) {
    /* Wait for delay  */
    clearTimeout(delayTimer);	
    if (typeof(delay) == 'number' && (delay > 10)) {
	delayTimer= setTimeout('showWaitPage(0)' , delay);
	return;
    }
    
    /* show the waitPage */
    hidePage("logonPage");
    hidePage("statusPage");
    showPage("waitPage");
    hidePage("errorPage");
}

function showErrorPage( str )  {
    setTimeout('chilliController.refresh()', 15000);
    
    hidePage("logonPage");
    hidePage("statusPage");
    hidePage("waitPage");
    showPage("errorPage");
    setElementValue("errorMessage", str);
}

var chillijsWindowOnLoad = window.onload;
var delayTimer; // global reference to delayTimer
window.onload = function() {
    if (chillijsWindowOnLoad) 
	chillijsWindowOnLoad();

    var logonForm = document.getElementById('logonForm');

    var head = document.getElementsByTagName("head")[0];
    if (head == null) head = document.body;

    if (logonForm == null) {
        logonForm = document.getElementById('loginForm');
    }

    if (logonForm == null) {
        try {
            logonForm = document.createElement('div');
            logonForm.setAttribute('id', 'logonForm');
            logonForm.setAttribute('name', 'logonForm');
            var thisScript = document.getElementById('chillijs');
            if (thisScript != null) {
                thisScript.parentNode.insertBefore(logonForm, thisScript);
            } else {
                document.body.appendChild(logonForm);
            }
        } catch(exception) {
            document.body.innerHTML += "<div id='logonForm'></div>";
        }
        logonForm = document.getElementById('logonForm');
    }

    if (logonForm.innerHTML == '') {
	if (head != null) {
	    var script = document.createElement('script');
	    script.id = 'chilliform';
	    script.type = 'text/javascript';
	    script.src = 'http://'+chilliController.host+':'+chilliController.port+'/www/chilliform.chi';
	    head.appendChild(script);
	} else {
	    logonForm.innerHTML='Error loading generic login form';
	}
    }

    showWaitPage(); 
    setTimeout('chilliController.refresh()', 500);
}
