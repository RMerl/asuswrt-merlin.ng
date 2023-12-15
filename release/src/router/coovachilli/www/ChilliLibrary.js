/**
 *   ChilliLibrary.js
 *   V2.0
 *
 *   This Javascript library can be used to create HTML/JS browser
 *   based smart clients (BBSM) for the CoovaChilli access controller
 *   Coova Chilli rev 81 or higher is required
 *   
 *   This library creates four global objects :
 *
 *    - chilliController  Expose session/client state and 
 *                        connect()/disconnect() methods the to BBSM.
 *
 *    - chilliJSON        INTERNAL (should not be called from the BBSM).
 *                        Issues a command to the chilli daemon by adding a new <SCRIPT>
 *                        tag to the HTML DOM (this hack enables cross server requests). 
 *                        
 *    - chilliClock       Can be used by BBSMs to display a count down.
 *                        Will sync with chilliController for smooth UI display (not yet implemented)
 *
 *    - chilliLibrary     Expose API and library versions
 *
 *  For more information http://www.coova.org/CoovaChilli/JSON
 *
 *  TODO :
 *   - Fine tune level of debug messages
 *   - Define error code when invoking onError
 *   - Retry mechanism after a JSON request fails
 *   - Delay clock tick when there is already an ongoing request
 *   - Use a true JSON parser to validate what we received
 *   - Use idleTime and idleTimeout to re-schedule autofresh after
 *     a likely idle termination by chilli
 *   - check that the library can be compiled as a Flash swf library
 *     and used from Flash BBSMs with the same API.
 *
 *   Copyright (C) Y.Deltroo 2007
 *   Distributed under the BSD License
 *
 *   This file also contains third party code :
 *   - MD5, distributed under the BSD license
 *     http://pajhome.org.uk/crypt/md5
 *
 */

var chilliLibrary = { revision:'85' , apiVersion:'2.0' } ;


/**
 *   Global chilliController object
 *
 *   CONFIGUARION PROPERTIES
 *   -----------------------
 *    ident (String) 
 *      Hex encoded string (used for client side CHAP-Password calculations) 
 *		  
 *    interval (Number)
 *       Poll the gateway every interval, in seconds
 *
 *    host (String)
 *       IP address of the controller (String)
 *
 *    port (Number)
 *        UAM port to direct request to on the gateway
 *
 *    ssl (Boolean)
 *       Shall we use HTTP or HTTPS to communicate with the chilli controller 
 *
 *    uamService : String
 *        !!! EXPERIMENTAL FEATURE !!!
 *        URL to external uamService script (used for external MD5 calculation when portal/chilli trust is required)
 *        This remote script runs on a SSL enable web server, and knows UAM SECRET.
 *        The chilliController javascript object will send the password over SSL (and challenge for CHAP) 
 *        UAM SERVICE should reply with a JSON response containing
 *           - CHAP logon : CHAP-Password X0Red with UAM SECRET
 *           - PAP  logon : Password XORed with UAM SECRET
 *
 *   For more information http://www.coova.org/CoovaChilli/JSON
 *
 */

if (!chilliController || !chilliController.host)
var chilliController = { interval:30 , host:"1.0.0.1" , port:false , ident:'00' , ssl:false , uamService: false };

/* Define clientState numerical code constants  */
chilliController.stateCodes = { UNKNOWN:-1 , NOT_AUTH:0 , AUTH:1 , AUTH_PENDING:2 , AUTH_SPLASH:3 } ;

/* Initializing session and accounting members, objet properties */
chilliController.session     = {} ;
chilliController.accounting  = {} ;
chilliController.redir       = {} ;

chilliController.location   = { name: '' } ;
chilliController.challenge       = '' ;
chilliController.message         = '' ;
chilliController.clientState     = chilliController.stateCodes.UNKNOWN ;
chilliController.command         = '' ;
chilliController.autorefreshTimer = 0  ;

/* This method returns the root URL for commands */
chilliController.urlRoot = function () {
	var protocol = ( chilliController.ssl ) ? "https" : "http" ;
	var urlRoot = protocol + "://" + chilliController.host + (chilliController.port ? ":" + chilliController.port.toString() : "") + "/json/" ;
	return urlRoot;
};

/* Default event handlers */
chilliController.onUpdate = function ( cmd ) {
	log('>> Default onUpdate handler. <<\n>> You should write your own. <<\n>> cmd = ' + cmd + ' <<' );
};

chilliController.onError = function ( str ) {
	log ( '>> Default Error Handler<<\n>> You should write your own  <<\n>> ' + str + ' <<' );
};


chilliController.formatTime = function ( t , zeroReturn ) {

    if ( typeof(t) == 'undefined' ) {
	return "Not available";
    }

    t = parseInt ( t , 10 ) ;
    if ( (typeof (zeroReturn) !='undefined') && ( t === 0 ) ) {
	return zeroReturn;
    }

    var h = Math.floor( t/3600 ) ;
    var m = Math.floor( (t - 3600*h)/60 ) ;
    var s = t % 60  ;

    var s_str = s.toString();
    if (s < 10 ) { s_str = '0' + s_str;   }

    var m_str = m.toString();
    if (m < 10 ) { m_str= '0' + m_str;    }

    var h_str = h.toString();
    if (h < 10 ) { h_str= '0' + h_str;    }


    if      ( t < 60 )   { return s_str + 's' ; }
    else if ( t < 3600 ) { return m_str + 'm' + s_str + 's' ; }
    else                 { return h_str + 'h' + m_str + 'm' + s_str + 's'; }

};

chilliController.formatBytes = function ( b , zeroReturn ) {

    if ( typeof(b) == 'undefined' ) {
        b = 0;
    } else {
        b = parseInt ( b , 10 ) ;
    }

    if ( (typeof (zeroReturn) !='undefined') && ( b === 0 ) ) {
	return zeroReturn;
    }

    var kb = Math.round(b  / 10) / 100;
    if (kb < 1) return b  + ' Bytes';

    var mb = Math.round(kb / 10) / 100;
    if (mb < 1)  return kb + ' Kilobytes';

    var gb = Math.round(mb / 10) / 100;
    if (gb < 1)  return mb + ' Megabytes';

    return gb + ' Gigabytes';
};


/**
 *   Global chilliController object
 *
 *   PUBLIC METHODS
 *   --------------
 *     logon ( username, password ) :
 *           Attempt a CHAP logon with username/password
 *           issues a /logon command to chilli daemon
 *
 *     logon2 ( username, response ) :
 *           Attempt a CHAP logon with username/response
 *           issues a /logon command to chilli daemon
 *
 *     logoff () :
 *           Disconnect the current user by issuing a
 *           /logoff command to the chilli daemon
 *
 *     refresh () :
 *           Issues a /status command to chilli daemon to refresh 
 *           the local chilliController object state/session data
 *
 */
 
chilliController.logon = function ( username , password )  {

	if ( typeof(username) !== 'string') {
		chilliController.onError( 1 , "username missing (or incorrect type)" ) ;
	}
	
	if ( typeof(password) !== 'string') {
		chilliController.onError( 2 , "password missing (or incorrect type)" ) ;
	}

	log ( 'chilliController.logon( "' + username + '" , "' + password + ' " )' );

	chilliController.temp = { 'username': username , 'password': password };
	chilliController.command = 'logon';

	log ('chilliController.logon: asking for a new challenge ' );
	chilliJSON.onError        = chilliController.onError    ;
	chilliJSON.onJSONReady    = chilliController.logonStep2 ;
	chilliController.clientState = chilliController.AUTH_PENDING ; 
	chilliJSON.get( chilliController.urlRoot() + 'status'  ) ;
};

chilliController.logon2 = function ( username , response )  {

	if ( typeof(username) !== 'string') {
		chilliController.onError( 1 , "username missing (or incorrect type)" ) ;
	}
	
	if ( typeof(response) !== 'string') {
		chilliController.onError( 2 , "response missing (or incorrect type)" ) ;
	}

	log ( 'chilliController.logon2( "' + username + '" , "' + response + ' " )' );

	chilliController.temp = { 'username': username , 'response': response };
	chilliController.command = 'logon2';

	log ('chilliController.logon2: asking for a new challenge ' );
	chilliJSON.onError        = chilliController.onError    ;
	chilliJSON.onJSONReady    = chilliController.logonStep2 ;
	chilliController.clientState = chilliController.AUTH_PENDING ; 
	chilliJSON.get( chilliController.urlRoot() + 'status'  ) ;
};


/**
 *   Second part of the logon process invoked after
 *   the just requested challenge has been received
 */
chilliController.logonStep2 = function ( resp ) {

	log('Entering logonStep 2');

	if ( typeof (resp.challenge) != 'string' ) {
		log('logonStep2: cannot find a challenge. Aborting.');
		return chilliController.onError('Cannot get challenge');
	}

	if ( resp.clientSate === chilliController.stateCodes.AUTH ) {
		log('logonStep2: Already connected. Aborting.');
		return chilliController.onError('Already connected.');
	}

	var challenge = resp.challenge;

	var username = chilliController.temp.username ; 
	var password = chilliController.temp.password ;
	var response = chilliController.temp.response ;

	log ('chilliController.logonStep2: Got challenge = ' + challenge );

	if ( chilliController.uamService ) { /* MD5 CHAP will be calculated by uamService */

		log ('chilliController.logonStep2: Logon using uamService (external MD5 CHAP)');

		var c ;
		if ( chilliController.uamService.indexOf('?') === -1 ) { 
			c = '?' ;
		}
		else {
			c = '&' ;
		}

		// Build command URL
		var url = chilliController.uamService + c + 'username=' + escape(username) +'&password=' + escape(password) +'&challenge=' + challenge ;

		if (chilliController.queryObj && chilliController.queryObj['userurl'] ) {
		    url += '&userurl='+chilliController.queryObj['userurl'] ;
		}

		// Make uamService request
		chilliJSON.onError     = chilliController.onError     ;
		chilliJSON.onJSONReady = chilliController.logonStep3 ;

		chilliController.clientState = chilliController.AUTH_PENDING ; 
		chilliJSON.get( url ) ;
	}
	else {
		/* TODO: Should check if challenge has expired and possibly get a new one */
        	/*       OR always call status first to get a fresh challenge             */

	    if (!response || response == '') {
		/* Calculate MD5 CHAP at the client side */
		var myMD5 = new ChilliMD5();
		response = myMD5.chap ( chilliController.ident , password , challenge );
		log ( 'chilliController.logonStep2: Calculating CHAP-Password = ' + response );
	    }

		/* Prepare chilliJSON for logon request */
		chilliJSON.onError     = chilliController.onError     ;
		chilliJSON.onJSONReady = chilliController.processReply ;
		chilliController.clientState = chilliController.stateCodes.AUTH_PENDING ; 
	
		/* Build /logon command URL */
		var logonUrl = chilliController.urlRoot() + 'logon?username=' + escape(username) + '&response='  + response;
		if (chilliController.queryObj && chilliController.queryObj['userurl'] ) {
		    logonUrl += '&userurl='+chilliController.queryObj['userurl'] ;
		}
		chilliJSON.get ( logonUrl ) ;
	}

}; 

/**
 *   Third part of the logon process invoked after
 *   getting a uamService response
 */
chilliController.logonStep3 = function ( resp ) {
	log('Entering logonStep 3');

	var username = chilliController.temp.username ; 

	if ( typeof (resp.response) == 'string' ) {
		chilliJSON.onError     = chilliController.onError     ;
		chilliJSON.onJSONReady = chilliController.processReply ;
		chilliController.clientState = chilliController.stateCodes.AUTH_PENDING ; 
	
		/* Build /logon command URL */
		var logonUrl = chilliController.urlRoot() + 'logon?username=' + escape(username) + '&response='  + resp.response;
		if (chilliController.queryObj && chilliController.queryObj['userurl'] ) {
		    logonUrl += '&userurl='+chilliController.queryObj['userurl'] ;
		}
		chilliJSON.get ( logonUrl ) ;
	}
}

chilliController.refresh = function ( ) {

	if ( chilliController.autorefreshTimer ) {
		chilliController.command = 'autorefresh' ;
	}
	else {
		chilliController.command = 'refresh' ;
	}

	chilliJSON.onError     = chilliController.onError        ;
	chilliJSON.onJSONReady = chilliController.processReply   ;
	chilliJSON.get( chilliController.urlRoot() + 'status'  ) ;
};

chilliController.logoff = function () {

	chilliController.command  = 'logoff'                      ;
	chilliJSON.onError        = chilliController.onError      ;
	chilliJSON.onJSONReady    = chilliController.processReply ;
	chilliJSON.get( chilliController.urlRoot() + 'logoff' );
};

/* *
 *
 * This functions does some check/type processing on the JSON resp
 * and updates the corresponding chilliController members
 *
 */
chilliController.processReply = function ( resp ) {

	if ( typeof (resp.message)  == 'string' ) {

		/* The following trick will replace HTML entities with the corresponding
                 * character. This will not work in Flash (no innerHTML) 
                 */

		var fakediv = document.createElement('div');
		fakediv.innerHTML = resp.message ;
		chilliController.message = fakediv.innerHTML  ;
	}

	if ( typeof (resp.challenge) == 'string' ) {
		chilliController.challenge = resp.challenge ;
	}

	if ( typeof ( resp.location ) == 'object' ) {
		chilliController.location =  resp.location ;
	}

	if ( typeof ( resp.accounting ) == 'object' ) {
		chilliController.accounting = resp.accounting ;
	}

	if (  (typeof ( resp.redir ) == 'object') ) {
		chilliController.redir = resp.redir ;
	}		
	
	/* Update the session member only the first time after AUTH */
	if (  (typeof ( resp.session ) == 'object') &&
	      ( chilliController.session==null || (
	         ( chilliController.clientState !== chilliController.stateCodes.AUTH  )  &&
	         ( resp.clientState === chilliController.stateCodes.AUTH  )))) {

		chilliController.session = resp.session ;

		if ( resp.session.startTime ) {
			chilliController.session.startTime = new Date();
			chilliController.session.startTime.setTime(resp.session.startTime);
		}
	}

	/* Update clientState */
	if (  ( resp.clientState === chilliController.stateCodes.NOT_AUTH     ) ||
              ( resp.clientState === chilliController.stateCodes.AUTH         ) ||
              ( resp.clientState === chilliController.stateCodes.AUTH_SPLASH  ) ||
	      ( resp.clientState === chilliController.stateCodes.AUTH_PENDING ) ) {

		chilliController.clientState = resp.clientState ;
	}
	else {
		chilliController.onError("Unknown clientState found in JSON reply");
	}


	/* Launch or stop the autorefresh timer if required */
	if ( chilliController.clientState === chilliController.stateCodes.AUTH  ) {

             if ( !chilliController.autorefreshTimer ) {
			chilliController.autorefreshTimer = setInterval ('chilliController.refresh()' , 1000*chilliController.interval);
	     }
	} 
	else if ( chilliController.clientState  === chilliController.stateCodes.NOT_AUTH ) {
		clearInterval ( chilliController.autorefreshTimer ) ;
		 chilliController.autorefreshTimer = 0 ;
	}

	/* Lastly... call the event handler  */
	log ('chilliController.processReply: Calling onUpdate. clienState = ' + chilliController.clientState);
	chilliController.onUpdate( chilliController.command );
};



/** 
 *  chilliJSON object  
 *
 *  This private objet implements the cross domain hack
 *  If no answer is received before timeout, then an error is raised.
 *
 */

var chilliJSON = { timeout:25000 , timer:0 , node:0 , timestamp:0 };

chilliJSON.expired   = function () {

		if ( chilliJSON.node.text ) {
			log ('chilliJSON: reply content \n' + chilliJSON.node.text );
		}
		else {
			log ('chilliJSON: request timed out (or reply is not valid JS)');
		}

		clearInterval ( chilliJSON.timer ) ;
		chilliJSON.timer = 0 ;

		/* remove the <SCRIPT> tag node that we have created */
		if ( typeof (chilliJSON.node) !== 'number' ) {
			document.getElementsByTagName('head')[0].removeChild ( chilliJSON.node );
		}
		chilliJSON.node = 0;

		/* TODO: Implement some kind of retry mechanism here ... */

		chilliJSON.onError('JSON request timed out (or reply is not valid)');
};

chilliJSON.reply = function  ( raw ) {

		clearInterval ( chilliJSON.timer ) ;
		chilliJSON.timer = 0 ;

		var now = new Date()    ;
		var end = now.getTime() ;
		 
		if ( chilliJSON.timestamp ) {
			log ( 'chilliJSON: JSON reply received in ' + ( end - chilliJSON.timestamp ) + ' ms\n' + dumpObject(raw) );
		}

		if ( typeof (chilliJSON.node) !== 'number' ) {
			document.getElementsByTagName('head')[0].removeChild ( chilliJSON.node );
		}
		chilliJSON.node = 0;

         	/* TODO: We should parse raw JSON as an extra security measure */
				
		chilliJSON.onJSONReady( raw ) ;
} ;

chilliJSON.get = function ( gUrl ) {

		if ( typeof(gUrl) == "string" ) {
			chilliJSON.url = gUrl ;
		}
		else {
			log ( "chilliJSON:error:Incorrect url passed to chilliJSON.get():" + gUrl );
			chilliJSON.onError ( "Incorrect url passed to chilliJSON.get() " );
			return ;
		}

		if ( chilliJSON.timer ) {
			log('logon:   There is already a request running. Return without launching a new request.');
			return ;
		}

	
		var scriptElement  = document.createElement('script');
		scriptElement.type = 'text/javascript';

		var c ;
		if ( this.url.indexOf('?') === -1 ) { 
			c = '?' ;
		}
		else {
			c = '&' ;
		}

		scriptElement.src = chilliJSON.url + c + 'callback=chilliJSON.reply' ;
		scriptElement.src += '&'+Math.random(); // prevent caching in Safari
		
		/* Adding the node that will trigger the HTTP request to the DOM tree */
		chilliJSON.node = document.getElementsByTagName('head')[0].appendChild(scriptElement);

		/* Using interval instead of timeout to support Flash 5,6,7 */
		chilliJSON.timer     = setInterval ( 'chilliJSON.expired()' , chilliJSON.timeout ) ; 
		var now              = new Date();
		chilliJSON.timestamp = now.getTime() ;

		log ('chilliJSON: getting ' + chilliJSON.url + ' . Waiting for reply ...');

}; // end chilliJSON.get = function ( url )


/** 
 *  chilliClock object  
 *
 *  Can be used by BBSMs to display a count down.
 *
 *  Will sync with chilliController and modulate the delay to call onTick
 *  This will avoid ugly sequence of short updates in the IO
 *  (not yet implemented)
 *
 */

var chilliClock = { isStarted : 0 };

chilliClock.onTick = function () {
	log ("You should define your own onTick() handler on this clock object. Clock value = " + this.value );
};
 
chilliClock.increment = function () {

	chilliClock.value  =  chilliClock.value + 1 ;
	chilliClock.onTick( chilliClock.value ) ;
};

chilliClock.resync = function ( newval ) {
	clearInterval ( chilliClock.isStarted )    ;
	chilliClock.value     = parseInt( newval , 10 ) ;
	chilliClock.isStarted = setInterval ( 'chilliClock.increment()' , 1000 );
};

chilliClock.start = function ( newval ) {

	if ( typeof (newval) !== 'Number' ) {
		chilliClock.resync ( 0 ) ;
	}
	else {
		chilliClock.resync ( newval ) ;
	}
};

chilliClock.stop = function () {
	clearInterval ( chilliClock.isStarted )  ;
	chilliClock.isStarted = 0 ;
};


function getel(e) {
	if (document.getElementById) {
		return document.getElementById(e);
	} else if (document.all){
		return document.all[e];
	}
}

function log( msg , messageLevel ) {
	if (!chilliController.debug) return;
	if ( typeof(trace)=="function") {
		// ActionScript trace
		trace ( msg );
	}
	else if ( typeof(console)=="object") {
		// FireBug console
		console.debug ( msg );
	}

	if ( getel('debugarea') ) {
		var e = getel('debugarea') ;
		e.value = e.value + '\n' + msg;
		e.scrollTop = e.scrollHeight - e.clientHeight;
	}
}

/* Transform an object to a text representation */
function dumpObject ( obj ) {

	var str = '' ;

	for (var key in obj ) {
		str = str + "    " + key + " = " + obj[key] + "\n" ;
		if ( typeof ( obj[key] ) == "object" ) {
			for ( var key2 in obj[key] ) {
				str = str + "      " + key2 + " = "  + obj[key][key2] + "\n" ;
			}
		}
	}

	return str;
}

/*
 * A JavaScript implementation of the RSA Data Security, Inc. MD5 Message
 * Digest Algorithm, as defined in RFC 1321.
 * Version 2.1 Copyright (C) Paul Johnston 1999 - 2002.
 * Other contributors: Greg Holt, Andrew Kepert, Ydnar, Lostinet
 * Distributed under the BSD License
 * See http://pajhome.org.uk/crypt/md5 for more info.
 *
 * added by Y.DELTROO
 *   - new functions: chap(), hex2binl() and str2hex() 
 *   - modifications to comply with the jslint test, http://www.jslint.com/
 *
 * Copyright (c) 2007
 * Distributed under the BSD License
 *
 */


function ChilliMD5() {

	var hexcase = 0;  /* hex output format. 0 - lowercase; 1 - uppercase        */
	var b64pad  = ""; /* base-64 pad character. "=" for strict RFC compliance   */
	var chrsz   = 8;  /* bits per input character. 8 - ASCII; 16 - Unicode      */

	this.hex_md5 = function (s){
		return binl2hex(core_md5(str2binl(s), s.length * chrsz));
	};

	this.chap = function ( hex_ident , str_password , hex_chal ) {

		//  Convert everything to hex encoded strings
		var hex_password =  str2hex ( str_password );

		// concatenate hex encoded strings
		var hex   = hex_ident + hex_password + hex_chal;

		// Convert concatenated hex encoded string to its binary representation
		var bin   = hex2binl ( hex ) ;

		// Calculate MD5 on binary representation
		var md5 = core_md5( bin , hex.length * 4 ) ; 

		return binl2hex( md5 );
	};

	function core_md5(x, len) {
	  x[len >> 5] |= 0x80 << ((len) % 32);
	  x[(((len + 64) >>> 9) << 4) + 14] = len;

	  var a =  1732584193;
	  var b = -271733879;
	  var c = -1732584194;
	  var d =  271733878;

	  for(var i = 0; i < x.length; i += 16) {
		var olda = a;
		var oldb = b;
		var oldc = c;
		var oldd = d;

		a = md5_ff(a, b, c, d, x[i+ 0], 7 , -680876936);
		d = md5_ff(d, a, b, c, x[i+ 1], 12, -389564586);
		c = md5_ff(c, d, a, b, x[i+ 2], 17,  606105819);
		b = md5_ff(b, c, d, a, x[i+ 3], 22, -1044525330);
		a = md5_ff(a, b, c, d, x[i+ 4], 7 , -176418897);
		d = md5_ff(d, a, b, c, x[i+ 5], 12,  1200080426);
		c = md5_ff(c, d, a, b, x[i+ 6], 17, -1473231341);
		b = md5_ff(b, c, d, a, x[i+ 7], 22, -45705983);
		a = md5_ff(a, b, c, d, x[i+ 8], 7 ,  1770035416);
		d = md5_ff(d, a, b, c, x[i+ 9], 12, -1958414417);
		c = md5_ff(c, d, a, b, x[i+10], 17, -42063);
		b = md5_ff(b, c, d, a, x[i+11], 22, -1990404162);
		a = md5_ff(a, b, c, d, x[i+12], 7 ,  1804603682);
		d = md5_ff(d, a, b, c, x[i+13], 12, -40341101);
		c = md5_ff(c, d, a, b, x[i+14], 17, -1502002290);
		b = md5_ff(b, c, d, a, x[i+15], 22,  1236535329);

		a = md5_gg(a, b, c, d, x[i+ 1], 5 , -165796510);
		d = md5_gg(d, a, b, c, x[i+ 6], 9 , -1069501632);
		c = md5_gg(c, d, a, b, x[i+11], 14,  643717713);
		b = md5_gg(b, c, d, a, x[i+ 0], 20, -373897302);
		a = md5_gg(a, b, c, d, x[i+ 5], 5 , -701558691);
		d = md5_gg(d, a, b, c, x[i+10], 9 ,  38016083);
		c = md5_gg(c, d, a, b, x[i+15], 14, -660478335);
		b = md5_gg(b, c, d, a, x[i+ 4], 20, -405537848);
		a = md5_gg(a, b, c, d, x[i+ 9], 5 ,  568446438);
		d = md5_gg(d, a, b, c, x[i+14], 9 , -1019803690);
		c = md5_gg(c, d, a, b, x[i+ 3], 14, -187363961);
		b = md5_gg(b, c, d, a, x[i+ 8], 20,  1163531501);
		a = md5_gg(a, b, c, d, x[i+13], 5 , -1444681467);
		d = md5_gg(d, a, b, c, x[i+ 2], 9 , -51403784);
		c = md5_gg(c, d, a, b, x[i+ 7], 14,  1735328473);
		b = md5_gg(b, c, d, a, x[i+12], 20, -1926607734);

		a = md5_hh(a, b, c, d, x[i+ 5], 4 , -378558);
		d = md5_hh(d, a, b, c, x[i+ 8], 11, -2022574463);
		c = md5_hh(c, d, a, b, x[i+11], 16,  1839030562);
		b = md5_hh(b, c, d, a, x[i+14], 23, -35309556);
		a = md5_hh(a, b, c, d, x[i+ 1], 4 , -1530992060);
		d = md5_hh(d, a, b, c, x[i+ 4], 11,  1272893353);
		c = md5_hh(c, d, a, b, x[i+ 7], 16, -155497632);
		b = md5_hh(b, c, d, a, x[i+10], 23, -1094730640);
		a = md5_hh(a, b, c, d, x[i+13], 4 ,  681279174);
		d = md5_hh(d, a, b, c, x[i+ 0], 11, -358537222);
		c = md5_hh(c, d, a, b, x[i+ 3], 16, -722521979);
		b = md5_hh(b, c, d, a, x[i+ 6], 23,  76029189);
		a = md5_hh(a, b, c, d, x[i+ 9], 4 , -640364487);
		d = md5_hh(d, a, b, c, x[i+12], 11, -421815835);
		c = md5_hh(c, d, a, b, x[i+15], 16,  530742520);
		b = md5_hh(b, c, d, a, x[i+ 2], 23, -995338651);

		a = md5_ii(a, b, c, d, x[i+ 0], 6 , -198630844);
		d = md5_ii(d, a, b, c, x[i+ 7], 10,  1126891415);
		c = md5_ii(c, d, a, b, x[i+14], 15, -1416354905);
		b = md5_ii(b, c, d, a, x[i+ 5], 21, -57434055);
		a = md5_ii(a, b, c, d, x[i+12], 6 ,  1700485571);
		d = md5_ii(d, a, b, c, x[i+ 3], 10, -1894986606);
		c = md5_ii(c, d, a, b, x[i+10], 15, -1051523);
		b = md5_ii(b, c, d, a, x[i+ 1], 21, -2054922799);
		a = md5_ii(a, b, c, d, x[i+ 8], 6 ,  1873313359);
		d = md5_ii(d, a, b, c, x[i+15], 10, -30611744);
		c = md5_ii(c, d, a, b, x[i+ 6], 15, -1560198380);
		b = md5_ii(b, c, d, a, x[i+13], 21,  1309151649);
		a = md5_ii(a, b, c, d, x[i+ 4], 6 , -145523070);
		d = md5_ii(d, a, b, c, x[i+11], 10, -1120210379);
		c = md5_ii(c, d, a, b, x[i+ 2], 15,  718787259);
		b = md5_ii(b, c, d, a, x[i+ 9], 21, -343485551);

		a = safe_add(a, olda);
		b = safe_add(b, oldb);
		c = safe_add(c, oldc);
		d = safe_add(d, oldd);
	  }
	  return [ a, b, c, d ];

	}

	function md5_cmn(q, a, b, x, s, t) {
	  return safe_add(bit_rol(safe_add(safe_add(a, q), safe_add(x, t)), s),b);
	}

	function md5_ff(a, b, c, d, x, s, t) {
	  return md5_cmn((b & c) | ((~b) & d), a, b, x, s, t);
	}

	function md5_gg(a, b, c, d, x, s, t) {
	  return md5_cmn((b & d) | (c & (~d)), a, b, x, s, t);
	}

	function md5_hh(a, b, c, d, x, s, t) {
	  return md5_cmn(b ^ c ^ d, a, b, x, s, t);
	}

	function md5_ii(a, b, c, d, x, s, t) {
	  return md5_cmn(c ^ (b | (~d)), a, b, x, s, t);
	}

	function safe_add(x, y) {
	  var lsw = (x & 0xFFFF) + (y & 0xFFFF);
	  var msw = (x >> 16) + (y >> 16) + (lsw >> 16);
	  return (msw << 16) | (lsw & 0xFFFF);
	}
	function bit_rol(num, cnt) {
	  return (num << cnt) | (num >>> (32 - cnt));
	}

	function str2binl(str) {
	  var bin = [] ;
	  var mask = (1 << chrsz) - 1;
	  for (var i = 0; i < str.length * chrsz; i += chrsz) {
		bin[i>>5] |= (str.charCodeAt(i / chrsz) & mask) << (i%32);
	  }
	  return bin;
	}

	function binl2hex(binarray) {
	  var hex_tab = hexcase ? "0123456789ABCDEF" : "0123456789abcdef";
	  var str = "";
	  for (var i = 0; i < binarray.length * 4; i++) {
		str += hex_tab.charAt((binarray[i>>2] >> ((i%4)*8+4)) & 0xF) +
			   hex_tab.charAt((binarray[i>>2] >> ((i%4)*8  )) & 0xF);
	  }
	  return str;
	}

	function str2hex ( str ) {
		var hex_tab = hexcase ? "0123456789ABCDEF" : "0123456789abcdef";
		var hex = '';
		var val ;
		for ( var i=0 ; i<str.length ; i++) {
			/* TODO: adapt this if chrz=16   */
			val = str.charCodeAt(i);
			hex = hex + hex_tab.charAt( val/16 );
			hex = hex + hex_tab.charAt( val%16 );
		}
		return hex;
	}

	function hex2binl ( hex ) {
		/*  Clean-up hex encoded input string */
		hex = hex.toLowerCase() ;
		hex = hex.replace( / /g , "");

		var bin =[] ;

		/* Transfrom to array of integers (binary representation) */ 
		for ( i=0 ; i < hex.length*4   ; i=i+8 )  {
			octet =  parseInt( hex.substr( i/4 , 2) , 16) ;
			bin[i>>5] |= ( octet & 255 ) << (i%32);
		}
		return bin;
	}
	
} // end of ChilliMD5 constructor
