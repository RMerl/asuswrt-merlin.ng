#
# WiMAX Table structure for table 'wimax',
# which replaces the "radpostauth" table.
#

CREATE TABLE wimax (
  id int(11) NOT NULL auto_increment,
  username varchar(64) NOT NULL default '',
  authdate timestamp NOT NULL,
  spi varchar(16) NOT NULL default '',
  mipkey varchar(400) NOT NULL default '',
  lifetime int(12) default NULL,
  PRIMARY KEY  (id),
  KEY username (username),
  KEY spi (spi)
) ;
