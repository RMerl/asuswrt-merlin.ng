<?cs include:"templates/header.cs" ?>
<div>
<h3>strongSwan Mediation Service web frontend</h3>
<p>This web application builds the end user front end for a Mediation Service
as defined in the <i>
<a href="http://www.ietf.org/internet-drafts/draft-brunner-ikev2-mediation-00.txt">
IKEv2 Mediation Extension draft</a></i>.</p>
<h4>Mediation connection</h4>
<p>The authentication between Mediation Server and connecting clients is based
on RSA public keys. The identities used for IKEv2 are the public key identifier
of each clients key, encapsulated in a ID_KEY_ID identity.</p>
<p>The public key of this Mediation Server is:</p>
<pre>-----BEGIN PUBLIC KEY-----
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAzZRsIp99UrIdoctThOfc
r2Up92BTSlY1Xv1J6Hqcbx3dX/MDvX60nCPeA63Eh0VvQetfkpR73I/42+RD+NES
4NosmBRefE0c0Vzd0IV39NTz0KLh2jwIyUzYGXWHUZMeepckzEPXOhG44XaiaLTN
u/OZXLCXI6vJv8R3wl5xSkZhqEwHi+dATYmGvlXyBDfjprJ4o8yJrsCFlB8aGq+v
SyKuFG/kaE1VZ9wwZYoyCH0BuYUVBwyxZTMRy2EC+CqDxjjCp5mF27lgB1Lpy8Jy
AUpcVHtKtZEww6lIZYv/eUtvICz5WTn/pzsQUh8FwGDOyxX4WX7ZXXK55AXuMfG1
2QIDAQAB
-----END PUBLIC KEY-----</pre>
<p>The Mediation Server is reachable at <i>mediation.strongswan.org</i>.</p>
The mediation server allows connections from all registered peers.</p>
<h4>Mediated connections</h4>
<p>The authentication between mediated clients is done between clients, they
can use own keys or the same keys as defined for authentication of the
mediation connection.
<form action="<?cs var:base ?>/peer/list" method="get">
  <div class="right">
    <input type="submit" value="Back"/></td>
  </div>
</form>
</div>
<?cs include:"templates/footer.cs" ?>
