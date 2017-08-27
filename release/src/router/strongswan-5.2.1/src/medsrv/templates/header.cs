<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/2002/REC-xhtml1-20020801/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
  <head>
    <title>strongSwan Mediation Service</title>
    <link rel="stylesheet" type="text/css" href="<?cs var:base ?>/static/style.css"/>
    <link rel="icon" href="<?cs var:base ?>/static/favicon.ico" type="image/x-icon" />
    <script type="text/javascript" src="<?cs var:base ?>/static/mootools.js"></script>
    <script type="text/javascript" src="<?cs var:base ?>/static/script.js"></script>
  </head>
  <body>
  	<div class="fleft">
      <a href="<?cs var:base ?>/peer/list">
        <img class="fleft" src="<?cs var:base ?>/static/strongswan.png"/>
      </a>
      <h1>Mediation Service</h1>
    </div>
    <div class="menu">
	     <?cs if:?login ?>
	      Logged in as <i><?cs var:login ?></i> 
	      | <a href="<?cs var:base ?>/user/edit">Edit</a>
	      | <a href="<?cs var:base ?>/user/logout">Logout</a>
	      | <a href="<?cs var:base ?>/user/help">Help</a>
	     <?cs else ?>
	      | <a href="<?cs var:base ?>/user/help">Help</a>
	      | <a href="<?cs var:base ?>/user/login">Login</a>
	      | <a href="<?cs var:base ?>/user/add">Register</a>
	     <?cs /if ?>
    </div>
    <hr class="cleft"/>
    <div class="center">
      <div class="content">
