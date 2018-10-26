#!/usr/bin/perl

use strict;
use warnings;

# the following 3 perl modules need to be installed
# before running this script
use Net::Telnet;
use HTTP::Request::Common;
use LWP::UserAgent;

# USAGE:
# perl psi_example.pl [modem lan ip address - optional - the default is 192.168.1.1]

# 1) Download config file via http
# 2) Change the WLAN ssid in the config file
# 3) Upload the config file via http
# 4) Run the psictl command via telnet (it is assumed that CFE parameter has been modified appropriately)
# 5) Perform restoredefault
# 6) Download config file via http
# 7) Verify that WLAN ssid is still the one as set in step 2

print "The script assumes that the image has been built with\n";
print "appropriate features and that the Backup PSI option has\n";
print "been enabled in CFE\n\n";
    

my $modem_ip = $ARGV[0] || "192.168.1.1";

if ( !skip_quicksetup( $modem_ip ) ) {
    die "Modem not accessible at $modem_ip";
}
print "HTTP available at $modem_ip\n";


# 1) Download config file via http
my $filename = "original_config.conf";
if ( !http_download_config ( $modem_ip, $filename ) ) {
    die "Unable to download configuration file $filename from modem";
}
print "1) Configuration file $filename downloaded\n";


# 2) Change the WLAN ssid in the config file
# edit the file - in this example simply change the WLAN SSID
my $config;
if ( open( FO, "<$filename" ) ) {
    local $/;
    $config = <FO>;
    close(FO);
} else {
    die "Couldn't open $filename";
}
        
$filename = "modified_config.conf";
if ( open( NEW_FO, ">$filename" ) ) {
    $config =~ s/<SSID>(.*?)<\/SSID>/<SSID>NEWBrcmAP0<\/SSID>/;
    $config =~ s/<WlSsid>(.*?)<\/WlSsid>/<WlSsid>NEWBrcmAP0<\/WlSsid>/;
    print NEW_FO $config;
    close(NEW_FO);
} else {
    die "Couldn't open $filename";
}
print "2) SSID updated to NEWBrcmAP0 in the configuration file $filename\n";


# 3) Upload the config file via http
if ( !http_upload_config( $modem_ip, $filename ) ) {
    die "Failed to upload $filename";
}
print "3) Waiting for modem to reboot. This might take upto 120s.\n";
# wait for telnet to connect upto 120s
wait_for_modem( $modem_ip, "120" );
sleep 20; # let other drivers and modules come up before trying telnet

# 4) Run the psictl command via telnet (it is assumed that CFE parameter has been modified appropriately)
my $t = new Net::Telnet( Host => $modem_ip, Prompt => '/(?: >|#)/' );
if ( $t->login( Name => "admin", Password => "admin" ) ) {
    $t->cmd("sh");
    sleep 1;
    $t->cmd("psictl copyprimary2backup");
    # unfortunately the output of the command goes to console
    print "4) Configuration backed-up\n";
    

# 5) Perform restoredefault
    print "5) Performaing a 'restoredefault'\n";
    $t->cmd("exit");
    $t->cmd("restoredefault");
} # no else require because if login fails the Net::Telnet will cause the script to 'die'
print "Waiting for modem to reboot. This might take upto 120s.\n";
wait_for_modem( $modem_ip, "120" );


# 6) Download config file via http
if ( !skip_quicksetup( $modem_ip ) ) {
    die "Modem not accessible at $modem_ip after uploading $filename";
}
$filename = "new_default_config.conf";
if ( !http_download_config ( $modem_ip, $filename ) ) {
    die "Unable to download configuration file $filename from modem";
}
print "6) Configuration $filename downloaded\n";

  
# 7) Verify that WLAN ssid is still the one as set in step 2
if ( open( FO, "<$filename" ) ) {
    local $/;
    $config = <FO>;
    close(FO);
    
    if ( $config =~ /NEWBrcmAP0/ ) {
        print "7) Verified that the SSID is still NEWBrcmAP0 even after 'restoredefault'\n";
    } else {
        die "ERROR: SSID is not NEWBrcmAP0! Example failed!";
    }
}

print "\nExample Complete\n";

#
#
# check to see it http service (port 80) is available
sub skip_quicksetup {
    my ($ip) = @_;

    sleep 2;

    my $ua      = LWP::UserAgent->new( timeout => 60 );
    $ua->credentials( "$ip:80", "Broadband Router", "admin", "admin" );
    my $rsp     = $ua->request( GET "http://" . $ip );
    my $rsp_str = $rsp->as_string;
    
    if ( $rsp_str =~ /frame src='quicksetup.html'/ ) {
        $rsp     = $ua->request( GET "http://" . $ip . "/quicksetup.html");
        $rsp_str = $rsp->as_string;
        
        if ( $rsp_str =~ /var sessionKey='(\d+)';/) {
            my $session_key = $1;
            # now do a get to skip the quick setup
            $rsp = $ua->request( GET "http://" . $ip . "/quicksetup.cmd?action=skip&sessionKey=$session_key");
        } else {
            print "Error: Unable to skip quicksetup!\n";
            return 0;
        }
    }
    
    return 1;
}

#
#
# download the config file from the modem
sub http_download_config {
    my ( $ip, $filename ) = @_;
    
    $filename = (defined $filename) ? $filename : "backupsettings.conf";

    sleep 5; 

    # access the main page in order to get a session ID
    my $ua = LWP::UserAgent->new( timeout => 60 );
    $ua->credentials( "$ip:80", "Broadband Router", "admin", "admin" );
    my $rsp     = $ua->request( GET "http://" . $ip );
    
    # download the configuration file
    my $res = $ua->get("http://$ip/backupsettings.conf");
    if ( !( $res->is_success ) ) {
        return 0;
    }
    open( FO, ">$filename" );
    print FO $res->content();
    close(FO);

    return 1;
}

#
#
# upload the config file to the modem
sub http_upload_config {
    my ( $ip, $config ) = @_;

    # make sure the image exists before trying upload
    # the file should exist locally
    if ( !( -e $config ) ) {
        print "$config does not exist\n";
        return 0;
    }
    my $url = "http://$ip/uploadsettings.cgi";

    my $ua = LWP::UserAgent->new( timeout => 60 );
    
    my $req = POST $url,
      Referer      => "http://$ip/updatesettings.html",
      Content_Type => 'multipart/form-data',
      Content      => [ filename => [$config] ];
    $req->authorization_basic("admin", "admin");
    
    my $response = $ua->request($req);

    my $http_status = $response->code();
    my $rst         = 0;
    if ( $response->is_success() ) {
        print "$config upload successful - modem should be rebooting\n";
        $rst = 1;
    }
    else {
        $response = $ua->request($req);
        print "Configuration upload failed [HTTP status $http_status]\n";
    }

    return $rst;
}

sub wait_for_modem {
    my ( $ip, $timeout ) = @_;
    
    my $start_timer = time();
    sleep 10; # let modem start rebooting
    Net::Telnet->new( Host => $ip, Timeout => $timeout, Errmode => 'return' );
    die "Modem didnot boot back up after reboot" if ( time() - $start_timer > $timeout );
}
