---
--- Draft of Netvim SQL schema just for rlm_sqlhpwippool
---

CREATE DATABASE `netvim` DEFAULT CHARACTER SET utf8 COLLATE utf8_general_ci;
USE netvim;

CREATE TABLE `gid_ip` (
  `gid` int(10) unsigned NOT NULL default '0' COMMENT 'Host group ID',
  `ip_start` bigint(20) unsigned NOT NULL default '0' COMMENT 'Beginning of IP range',
  `ip_stop` bigint(20) unsigned NOT NULL default '0' COMMENT 'End of IP range',
  KEY `gid` (`gid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Netvim: host groups to IP ranges relations';

CREATE TABLE `host_groups` (
  `gid` int(10) unsigned NOT NULL default '0' COMMENT 'Host group ID',
  `parent` int(10) unsigned default NULL COMMENT 'ID of parent group',
  `name` varchar(128) NOT NULL default '' COMMENT 'Host group UNIX name',
  PRIMARY KEY  (`gid`),
  UNIQUE KEY `group_name` (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Netvim: host groups';

CREATE TABLE `ids` (
  `id` int(10) unsigned NOT NULL auto_increment COMMENT 'The One True ID',
  `enabled` tinyint(1) NOT NULL default '1' COMMENT 'If 0, ignore the object',
  `modified` datetime NOT NULL default '0000-00-00 00:00:00' COMMENT 'Time when any of object properties were modified',
  `created` datetime NOT NULL default '0000-00-00 00:00:00' COMMENT 'Object creation date',
  `type` varchar(64) default NULL COMMENT 'Link to an ef action',
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Entity: the source of ID numbers';

CREATE TABLE `ip_pools` (
  `pid` int(10) unsigned NOT NULL default '0' COMMENT 'Named pool ID',
  `gid` int(10) unsigned NOT NULL default '0' COMMENT 'Host group ID',
  `pnid` int(10) unsigned NOT NULL default '0' COMMENT 'Pool name ID',
  `ip_start` bigint(20) unsigned NOT NULL default '0' COMMENT 'Beginning of IP range',
  `ip_stop` bigint(20) unsigned NOT NULL default '0' COMMENT 'End of IP range',
  `prio` int(11) NOT NULL default '0' COMMENT 'Pool priority',
  `weight` int(10) unsigned NOT NULL default '1' COMMENT 'Pool weight',
  `total` bigint(20) unsigned NOT NULL default '0' COMMENT 'Total number of IPs in pool',
  `free` bigint(20) unsigned NOT NULL default '0' COMMENT 'Number of free IPs in pool',
  PRIMARY KEY  (`pid`),
  KEY `gid` (`gid`,`pnid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Netvim: named IP pools assigned to given host group';

CREATE TABLE `ips` (
  `ip` bigint(20) unsigned NOT NULL default '0' COMMENT 'IP address',
  `pid` int(10) unsigned NOT NULL default '0' COMMENT 'Named pool ID',
  `rsv_since` datetime NOT NULL default '0000-00-00 00:00:00' COMMENT 'Time when IP was reserved',
  `rsv_by` varchar(64) default NULL COMMENT 'Who/what reserved IP',
  `rsv_until` datetime NOT NULL default '0000-00-00 00:00:00' COMMENT 'Reservation timeout',
  PRIMARY KEY  (`ip`),
  KEY `pid` (`pid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Netvim: states of single IP addresses';

CREATE TABLE `pool_names` (
  `pnid` int(10) unsigned NOT NULL default '0' COMMENT 'Named pool ID',
  `name` varchar(128) NOT NULL default '' COMMENT 'Pool UNIX name',
  PRIMARY KEY  (`pnid`),
  UNIQUE KEY `pool_name` (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Netvim: definitions of pool names';
