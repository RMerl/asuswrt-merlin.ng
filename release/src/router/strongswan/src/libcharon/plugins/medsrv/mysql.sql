
CREATE TABLE IF NOT EXISTS `peer` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `user` int(10) unsigned NOT NULL,
  `alias` varchar(30) NOT NULL,
  `keyid` varbinary(20) NOT NULL,
  `public_key` blob,
  PRIMARY KEY  (`id`),
  UNIQUE KEY (`user`,`alias`),
  UNIQUE KEY (`keyid`),
  KEY (`user`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE IF NOT EXISTS `user` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `login` varchar(30) NOT NULL,
  `password` varbinary(20) NOT NULL,
  PRIMARY KEY  (`id`),
  UNIQUE KEY (`login`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

