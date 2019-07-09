
CREATE TABLE  `ClientConfig` (
  `IdClientConfig` int(11) NOT NULL,
  `KeyId` varbinary(20) NOT NULL,
  `PublicKey` blob NOT NULL,
  `PrivateKey` blob NOT NULL,
  PRIMARY KEY  (`IdClientConfig`)
);


CREATE TABLE  `MediationServerConfig` (
  `IdMediationServerConfig` int(11) NOT NULL,
  `Address` varchar(200) NOT NULL,
  `KeyId` varbinary(20) NOT NULL,
  `PublicKey` blob NOT NULL,
  PRIMARY KEY  (`IdMediationServerConfig`)
);


CREATE TABLE  `Connection` (
  `IdConnection` int(11) NOT NULL auto_increment,
  `Active` tinyint(1) NOT NULL,
  `Alias` varchar(50) NOT NULL,
  `KeyId` varbinary(20) NOT NULL,
  `PublicKey` blob NOT NULL,
  `LocalSubnet` varchar(20),
  `RemoteSubnet` varchar(20),
  `Status` int(11) NOT NULL,
  PRIMARY KEY  (`IdConnection`),
  UNIQUE (`Alias`),
  UNIQUE (`KeyId`)
);
