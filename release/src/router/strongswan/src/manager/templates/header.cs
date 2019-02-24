<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" 
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
  <head>
    <title><?cs var:title ?> - strongSwan Manager</title>
    <link rel="stylesheet" type="text/css" href="<?cs var:base ?>/static/style.css" />
    <script type="text/javascript" src="<?cs var:base ?>/static/jquery.js"></script>
    <script type="text/javascript" src="<?cs var:base ?>/static/script.js"></script>
  </head>
  <body>
  	<div class="fleft">
      <a href="<?cs var:base ?>/ikesa/list">
        <img class="fleft" src="<?cs var:base ?>/static/strongswan.png"/>
      </a>
      <h1>strongSwan Manager</h1>
  	  <h2><?cs var:title ?></h2>
    </div>
    <div class="menu">
      | <a href="<?cs var:base ?>/ikesa/list">IKE SAs</a>
      | <a href="<?cs var:base ?>/config/list">Config</a>
      | <a href="<?cs var:base ?>/gateway/list">Gateway</a>
      | <a href="<?cs var:base ?>/auth/logout">Logout</a>
    </div>
    <hr class="cleft"/>
    <div class="center">
      <div class="content">
