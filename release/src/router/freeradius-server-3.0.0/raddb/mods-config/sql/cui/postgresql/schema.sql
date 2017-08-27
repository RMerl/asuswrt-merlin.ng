CREATE TABLE cui (
  clientipaddress INET NOT NULL DEFAULT '0.0.0.0',
  callingstationid varchar(50) NOT NULL DEFAULT '',
  username varchar(64) NOT NULL DEFAULT '',
  cui varchar(32) NOT NULL DEFAULT '',
  creationdate TIMESTAMP with time zone NOT NULL default 'now()',
  lastaccounting TIMESTAMP with time zone NOT NULL default '-infinity'::timestamp,
  PRIMARY KEY  (username, clientipaddress, callingstationid)
);

CREATE RULE postauth_query AS ON INSERT TO cui
        WHERE EXISTS(SELECT 1 FROM cui WHERE (username, clientipaddress, callingstationid)=(NEW.username, NEW.clientipaddress, NEW.callingstationid))
        DO INSTEAD UPDATE cui SET lastaccounting ='-infinity'::timestamp with time zone, cui=NEW.cui WHERE (username, clientipaddress, callingstationid)=(NEW.username, NEW.clientipaddress, NEW.callingstationid);

