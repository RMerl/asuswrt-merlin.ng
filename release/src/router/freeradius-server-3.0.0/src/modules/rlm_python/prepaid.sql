# MySQL dump 8.13
#
# Host: localhost    Database: python
#--------------------------------------------------------
# Server version	3.23.36

#
# Table structure for table 'sessions'
#

CREATE TABLE sessions (
  username char(32) default NULL,
  seconds int(11) default NULL
) TYPE=MyISAM;

#
# Dumping data for table 'sessions'
#

INSERT INTO sessions VALUES ('map',10);
INSERT INTO sessions VALUES ('map',10);
INSERT INTO sessions VALUES ('map',10);
INSERT INTO sessions VALUES ('map',10);

#
# Table structure for table 'users'
#

CREATE TABLE users (
  username char(32) NOT NULL default '',
  passwd char(32) default NULL,
  maxseconds int(11) default NULL,
  PRIMARY KEY  (username)
) TYPE=MyISAM;

#
# Dumping data for table 'users'
#

INSERT INTO users VALUES ('map','abc',100);

