
CREATE TABLE users (
	username TEXT, password TEXT
);

CREATE TABLE gateways (
	name TEXT,
	port INTEGER,
	address text
);

CREATE TABLE user_gateway (
	user INTEGER,
	gateway INTEGER
);

INSERT INTO users VALUES(
	'strongSwan',
	'44092c0394d3661bd3c3587f43e48729836bdf6e' -- strongSwan
);

INSERT INTO gateways VALUES(
	'Local Unix',
	0,
	'/var/run/charon.xml'
);

INSERT INTO user_gateway VALUES(
	1,1
);
