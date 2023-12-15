#!/usr/bin/perl

# chilli - coova.org. A Wireless LAN Access Point Controller
# Copyright (C) 2003, 2004 Mondru AB.
# Copyright (C) 2006-2008 David Bird <david@coova.com>
#
# The contents of this file may be used under the terms of the GNU
# General Public License Version 2, provided that the above copyright
# notice and this permission notice is included in all copies or
# substantial portions of the software.

# This code is horrible -- it came that way, and remains that way. A
# real captive portal is demonstrated here: http://drupal.org/project/hotspot

# Redirects from CoovaChilli daemon:
#
# Redirection when not yet or already authenticated
#   notyet:  CoovaChilli daemon redirects to login page.
#   already: CoovaChilli daemon redirects to success status page.
#
# Response to login:
#   already: Attempt to login when already logged in.
#   failed:  Login failed
#   success: Login succeded
#
# logoff:  Response to a logout

# Shared secret used to encrypt challenge with. Prevents dictionary attacks.
# You should change this to your own shared secret.
$uamsecret = "uamsecret";

# Uncomment the following line if you want to use ordinary user-password (PAP)
# for radius authentication. 
# $userpassword=1;

$loginpath = "/cgi-bin/hotspotlogin.cgi";
$debug = 1;

# To use MSCHAPv2 Authentication with, 
# then uncomment the next two lines. 
#$ntresponse = 1;
#$chilli_response = '/usr/local/sbin/chilli_response';

# start program

use Digest::MD5  qw(md5 md5_hex md5_base64);

# Make sure that the form parameters are clean
$OK_CHARS='-a-zA-Z0-9_.@&=%!';
$_ = $input = <STDIN>;
s/[^$OK_CHARS]/_/go;
$input = $_;

# Make sure that the get query parameters are clean
$OK_CHARS='-a-zA-Z0-9_.@&=%!';
$_ = $query=$ENV{QUERY_STRING};
s/[^$OK_CHARS]/_/go;
$query = $_;


# If she did not use https tell her that it was wrong.
if (!$debug && !($ENV{HTTPS} =~ /^on$/)) {
    print "Content-type: text/html\n\n
<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">
<html>
<head>
  <title>CoovaChilli Login Failed</title>
  <meta http-equiv=\"Cache-control\" content=\"no-cache\">
  <meta http-equiv=\"Pragma\" content=\"no-cache\">
</head>
<body bgColor = '#c0d8f4'>
  <h1 style=\"text-align: center;\">CoovaChilli Login Failed</h1>
  <center>
    Login must use encrypted connection.
  </center>
</body>
<!--
<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<WISPAccessGatewayParam 
  xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"
  xsi:noNamespaceSchemaLocation=\"http://www.acmewisp.com/WISPAccessGatewayParam.xsd\">
<AuthenticationReply>
<MessageType>120</MessageType>
<ResponseCode>102</ResponseCode>
<ReplyMessage>Login must use encrypted connection</ReplyMessage>
</AuthenticationReply> 
</WISPAccessGatewayParam>
-->
</html>
";
    exit(0);
}


#Read form parameters which we care about
@array = split('&',$input);
foreach $var ( @array )
{
    @array2 = split('=',$var);
    if ($array2[0] =~ /^username$/i) { $username = $array2[1]; }
    if ($array2[0] =~ /^password$/i) { $password = $array2[1]; }
    if ($array2[0] =~ /^challenge$/) { $challenge = $array2[1]; }
    if ($array2[0] =~ /^button$/) { $button = $array2[1]; }
    if ($array2[0] =~ /^logout$/) { $logout = $array2[1]; }
    if ($array2[0] =~ /^prelogin$/) { $prelogin = $array2[1]; }
    if ($array2[0] =~ /^res$/) { $res = $array2[1]; }
    if ($array2[0] =~ /^uamip$/) { $uamip = $array2[1]; }
    if ($array2[0] =~ /^uamport$/) { $uamport = $array2[1]; }
    if ($array2[0] =~ /^userurl$/)   { $userurl = $array2[1]; }
    if ($array2[0] =~ /^timeleft$/)  { $timeleft = $array2[1]; }
    if ($array2[0] =~ /^redirurl$/)  { $redirurl = $array2[1]; }
}

#Read query parameters which we care about
@array = split('&',$query);
foreach $var ( @array )
{
    @array2 = split('=',$var);
    if ($array2[0] =~ /^username$/i) { $username = $array2[1]; }
    if ($array2[0] =~ /^password$/i) { $password = $array2[1]; }
    if ($array2[0] =~ /^res$/)       { $res = $array2[1]; }
    if ($array2[0] =~ /^challenge$/) { $challenge = $array2[1]; }
    if ($array2[0] =~ /^uamip$/)     { $uamip = $array2[1]; }
    if ($array2[0] =~ /^uamport$/)   { $uamport = $array2[1]; }
    if ($array2[0] =~ /^reply$/)     { $reply = $array2[1]; }
    if ($array2[0] =~ /^userurl$/)   { $userurl = $array2[1]; }
    if ($array2[0] =~ /^timeleft$/)  { $timeleft = $array2[1]; }
    if ($array2[0] =~ /^redirurl$/)  { $redirurl = $array2[1]; }
}


$reply =~ s/\+/ /g;
$reply =~s/%([a-fA-F0-9][a-fA-F0-9])/pack("C", hex($1))/seg;

$userurldecode = $userurl;
$userurldecode =~ s/\+/ /g;
$userurldecode =~s/%([a-fA-F0-9][a-fA-F0-9])/pack("C", hex($1))/seg;

$redirurldecode = $redirurl;
$redirurldecode =~ s/\+/ /g;
$redirurldecode =~s/%([a-fA-F0-9][a-fA-F0-9])/pack("C", hex($1))/seg;

$password =~ s/\+/ /g;
$password =~ s/%([a-fA-F0-9][a-fA-F0-9])/pack("C", hex($1))/seg;

# If attempt to login

if ($button =~ /^Login$/ || ($res eq "wispr" && $username ne "")) {

    print "Content-type: text/html\n\n";

    $hexchal  = pack "H*", $challenge;

    if (defined $uamsecret) {
	$newchal  = md5($hexchal, $uamsecret);
    }
    else {
	$newchal  = $hexchal;
    }

    if ($ntresponse == 1) {
	# Encode plain text into NT-Password 

	$response = `$chilli_response -nt "$challenge" "$uamsecret" "$username" "$password"`;

	$logonUrl = "http://$uamip:$uamport/logon?username=$username&ntresponse=$response";

    } elsif ($userpassword == 1) {
	# Encode plain text password with challenge 
	# (which may or may not be uamsecret encoded)

	# If challange isn't long enough, repeat it until it is
	while (length($newchal) < length($password)){
		$newchal .= $newchal;
        }

	$pappassword = unpack "H*", substr($password ^ $newchal, 0, length($password));

	$logonUrl = "http://$uamip:$uamport/logon?username=$username&password=$pappassword";

    } else {
	# Generate a CHAP response with the password and the
	# challenge (which may have been uamsecret encoded)

	$response = md5_hex("\0", $password, $newchal);

	$logonUrl = "http://$uamip:$uamport/logon?username=$username&response=$response&userurl=$userurl";
    }

#sleep 5;
print "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">
<html>
<head>
  <title>CoovaChilli Login</title>
  <meta http-equiv=\"Cache-control\" content=\"no-cache\">
  <meta http-equiv=\"Pragma\" content=\"no-cache\">
  <meta http-equiv=\"refresh\" content=\"0;url=$logonUrl\">
</head>
<body bgColor = '#c0d8f4'>
<h1 style=\"text-align: center;\">Logging in to CoovaChilli</h1>
  <center>
    Please wait......
  </center>
</body>
<!--
<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<WISPAccessGatewayParam 
  xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"
  xsi:noNamespaceSchemaLocation=\"http://www.acmewisp.com/WISPAccessGatewayParam.xsd\">
<AuthenticationReply>
<MessageType>120</MessageType>
<ResponseCode>201</ResponseCode>
<LoginResultsURL>$logonUrl</LoginResultsURL>
</AuthenticationReply> 
</WISPAccessGatewayParam>
-->
</html>
";
    exit(0);
}


# Default: It was not a form request
$result = 0;

# If login successful
if ($res =~ /^success$/) { 
    $result = 1;
}

# If login failed 
if ($res =~ /^failed$/) { 
    $result = 2;
}

# If logout successful
if ($res =~ /^logoff$/) { 
    $result = 3;
}

# If tried to login while already logged in
if ($res =~ /^already$/) { 
    $result = 4;
}

# If not logged in yet
if ($res =~ /^notyet$/) { 
    $result = 5;
}

# If login from smart client
if ($res =~ /^wispr$/) { 
    $result = 6;
}

# If requested a logging in pop up window
if ($res =~ /^popup1$/) { 
    $result = 11;
}

# If requested a success pop up window
if ($res =~ /^popup2$/) { 
    $result = 12;
}

# If requested a logout pop up window
if ($res =~ /^popup3$/) { 
    $result = 13;
}


# Otherwise it was not a form request
# Send out an error message
if ($result == 0) {
    print "Content-type: text/html\n\n
<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">
<html>
<head>
  <title>CoovaChilli Login Failed</title>
  <meta http-equiv=\"Cache-control\" content=\"no-cache\">
  <meta http-equiv=\"Pragma\" content=\"no-cache\">
</head>
<body bgColor = '#c0d8f4'>
  <h1 style=\"text-align: center;\">CoovaChilli Login Failed</h1>
  <center>
    Login must be performed through CoovaChilli daemon.
  </center>
</body>
</html>
";
    exit(0);
}

#Generate the output
if ($result != 6) {
    print "Content-type: text/html\n\n
<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">
<html>
<head>
  <title>CoovaChilli Login</title>
  <meta http-equiv=\"Cache-control\" content=\"no-cache\">
  <meta http-equiv=\"Pragma\" content=\"no-cache\">
  <SCRIPT LANGUAGE=\"JavaScript\">
    var blur = 0;
    var starttime = new Date();
    var startclock = starttime.getTime();
    var mytimeleft = 0;

    function doTime() {
      window.setTimeout( \"doTime()\", 1000 );
      t = new Date();
      time = Math.round((t.getTime() - starttime.getTime())/1000);
      if (mytimeleft) {
        time = mytimeleft - time;
        if (time <= 0) {
          window.location = \"$loginpath?res=popup3&uamip=$uamip&uamport=$uamport\";
        }
      }
      if (time < 0) time = 0;
      hours = (time - (time % 3600)) / 3600;
      time = time - (hours * 3600);
      mins = (time - (time % 60)) / 60;
      secs = time - (mins * 60);
      if (hours < 10) hours = \"0\" + hours;
      if (mins < 10) mins = \"0\" + mins;
      if (secs < 10) secs = \"0\" + secs;
      title = \"Online time: \" + hours + \":\" + mins + \":\" + secs;
      if (mytimeleft) {
        title = \"Remaining time: \" + hours + \":\" + mins + \":\" + secs;
      }
      if(document.all || document.getElementById){
         document.title = title;
      }
      else {   
        self.status = title;
      }
    }

    function popUp(URL) {
      if (self.name != \"chillispot_popup\") {
        chillispot_popup = window.open(URL, 'chillispot_popup', 'toolbar=0,scrollbars=0,location=0,statusbar=0,menubar=0,resizable=0,width=500,height=375');
      }
    }

    function doOnLoad(result, URL, userurl, redirurl, timeleft) {
      if (timeleft) {
        mytimeleft = timeleft;
      }
      if ((result == 1) && (self.name == \"chillispot_popup\")) {
        doTime();
      }
      if ((result == 1) && (self.name != \"chillispot_popup\")) {
        chillispot_popup = window.open(URL, 'chillispot_popup', 'toolbar=0,scrollbars=0,location=0,statusbar=0,menubar=0,resizable=0,width=500,height=375');
      }
      if ((result == 2) || result == 5) {
        document.form1.UserName.focus()
      }
      if ((result == 2) && (self.name != \"chillispot_popup\")) {
        chillispot_popup = window.open('', 'chillispot_popup', 'toolbar=0,scrollbars=0,location=0,statusbar=0,menubar=0,resizable=0,width=400,height=200');
        chillispot_popup.close();
      }
      if ((result == 12) && (self.name == \"chillispot_popup\")) {
        doTime();
        if (redirurl) {
          opener.location = redirurl;
        }
        else if (opener.home) {
          opener.home();
        }
        else {
          opener.location = \"about:home\";
        }
        self.focus();
        blur = 0;
      }
      if ((result == 13) && (self.name == \"chillispot_popup\")) {
        self.focus();
        blur = 1;
      }
    }

    function doOnBlur(result) {
      if ((result == 12) && (self.name == \"chillispot_popup\")) {
        if (blur == 0) {
          blur = 1;
          self.focus();
        }
      }
    }
  </script>
</head>
<body onLoad=\"javascript:doOnLoad($result, '$loginpath?res=popup2&uamip=$uamip&uamport=$uamport&userurl=$userurl&redirurl=$redirurl&timeleft=$timeleft','$userurldecode', '$redirurldecode', '$timeleft')\" onBlur = \"javascript:doOnBlur($result)\" bgColor = '#c0d8f4'>";
}

#      if (!window.opener) {
#        document.bgColor = '#c0d8f4';
#      }

#print "THE INPUT: $input";
#foreach $key (sort (keys %ENV)) {
#	print $key, ' = ', $ENV{$key}, "<br>\n";
#}

if ($result == 2) {
    print "
  <h1 style=\"text-align: center;\">CoovaChilli Login Failed</h1>";
    if ($reply) {
	print "<center> $reply </BR></BR></center>";
    }
}

if ($result == 5) {
    print "
  <h1 style=\"text-align: center;\">CoovaChilli Login</h1>";
}

if ($result == 2 || $result == 5) {
  print "
  <form name=\"form1\" method=\"post\" action=\"$loginpath\">
  <INPUT TYPE=\"hidden\" NAME=\"challenge\" VALUE=\"$challenge\">
  <INPUT TYPE=\"hidden\" NAME=\"uamip\" VALUE=\"$uamip\">
  <INPUT TYPE=\"hidden\" NAME=\"uamport\" VALUE=\"$uamport\">
  <INPUT TYPE=\"hidden\" NAME=\"userurl\" VALUE=\"$userurl\">
  <center>
  <table border=\"0\" cellpadding=\"5\" cellspacing=\"0\" style=\"width: 217px;\">
    <tbody>
      <tr>
        <td align=\"right\">Username:</td>
        <td><input STYLE=\"font-family: Arial\" type=\"text\" name=\"UserName\" size=\"20\" maxlength=\"128\"></td>
      </tr>
      <tr>
        <td align=\"right\">Password:</td>
        <td><input STYLE=\"font-family: Arial\" type=\"password\" name=\"Password\" size=\"20\" maxlength=\"128\"></td>
      </tr>
      <tr>
        <td align=\"center\" colspan=\"2\" height=\"23\"><input type=\"submit\" name=\"button\" value=\"Login\" onClick=\"javascript:popUp('$loginpath?res=popup1&uamip=$uamip&uamport=$uamport')\"></td> 
      </tr>
    </tbody>
  </table>
  </center>
  </form>
</body>
</html>";
}

if ($result == 1) {
  print "
  <h1 style=\"text-align: center;\">Logged in to CoovaChilli</h1>";

  if ($reply) { 
      print "<center> $reply </BR></BR></center>";
  }

  print "
  <center>
    <a href=\"http://$uamip:$uamport/logoff\">Logout</a>
  </center>
</body>
</html>";
}

if (($result == 4) || ($result == 12)) {
  print "
  <h1 style=\"text-align: center;\">Logged in to CoovaChilli</h1>
  <center>
    <a href=\"http://$uamip:$uamport/logoff\">Logout</a>
  </center>
</body>
</html>";
}


if ($result == 11) {
  print "<h1 style=\"text-align: center;\">Logging in to CoovaChilli</h1>";
  print "
  <center>
    Please wait......
  </center>
</body>
</html>";
}


if (($result == 3) || ($result == 13)) {
    print "
  <h1 style=\"text-align: center;\">Logged out from CoovaChilli</h1>
  <center>
    <a href=\"http://$uamip:$uamport/prelogin\">Login</a>
  </center>
</body>
</html>";
}


exit(0);

