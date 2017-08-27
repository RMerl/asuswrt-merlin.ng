--
-- Table structure for table 'radippool'
--
CREATE TABLE  (
  id                    int(11) PRIMARY KEY,
  pool_name             varchar(30) NOT NULL,
  framedipaddress       varchar(15) NOT NULL default '',
  nasipaddress          varchar(15) NOT NULL default '',
  calledstationid       VARCHAR(30) NOT NULL,
  callingstationid      VARCHAR(30) NOT NULL,
  expiry_time           DATETIME NULL default NULL,
  username              varchar(64) NOT NULL default '',
  pool_key              varchar(30) NOT NULL
);

CREATE INDEX radippool_poolname_expire ON radippool(pool_name, expiry_time);
CREATE INDEX radippool_framedipaddress ON radippool(framedipaddress);
CREATE INDEX radippool_nasip_poolkey_ipaddress ON radippool(nasipaddress, pool_key, framedipaddress);
