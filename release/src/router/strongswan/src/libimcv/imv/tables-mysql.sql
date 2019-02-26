
DROP TABLE IF EXISTS `directories`;
CREATE TABLE `directories` (
  `id` INTEGER NOT NULL PRIMARY KEY AUTO_INCREMENT,
  `path` VARCHAR(2048) NOT NULL
);

DROP TABLE IF EXISTS `files`;
CREATE TABLE `files` (
  `id` INTEGER NOT NULL PRIMARY KEY AUTO_INCREMENT,
  `dir` INTEGER DEFAULT 0 REFERENCES `directories`(`id`),
  `name` VARCHAR(512) NOT NULL
);

DROP TABLE IF EXISTS `products`;
CREATE TABLE `products` (
  `id` INTEGER NOT NULL PRIMARY KEY AUTO_INCREMENT,
  `name` VARCHAR(128) NOT NULL
);

DROP TABLE IF EXISTS `algorithms`;
CREATE TABLE `algorithms` (
  `id` INTEGER PRIMARY KEY,
  `name` VARCHAR(20) NOT NULL
);

DROP TABLE IF EXISTS `file_hashes`;
CREATE TABLE `file_hashes` (
  `id` INTEGER NOT NULL PRIMARY KEY AUTO_INCREMENT,
  `file` INTEGER NOT NULL REFERENCES `files`(`id`),
  `product` INTEGER NOT NULL REFERENCES `products`(`id`),
  `device` INTEGER DEFAULT 0,
  `key` INTEGER DEFAULT 0 REFERENCES `keys`(id),
  `algo` INTEGER NOT NULL REFERENCES `algorithms`(`id`),
  `hash` VARBINARY(64) NOT NULL
);

DROP TABLE IF EXISTS `keys`;
CREATE TABLE `keys` (
  `id` INTEGER NOT NULL PRIMARY KEY AUTO_INCREMENT,
  `keyid` VARBINARY(128) NOT NULL,
  `owner` VARCHAR(128) NOT NULL
);

DROP TABLE IF EXISTS `groups`;
CREATE TABLE `groups` (
  `id` INTEGER NOT NULL PRIMARY KEY AUTO_INCREMENT,
  `name` VARCHAR(50) NOT NULL UNIQUE,
  `parent` INTEGER
);

DROP TABLE IF EXISTS `groups_members`;
CREATE TABLE `groups_members` (
  `id` INTEGER NOT NULL PRIMARY KEY AUTO_INCREMENT,
  `group_id` INTEGER NOT NULL REFERENCES `groups`(`id`),
  `device_id` INTEGER NOT NULL REFERENCES `devices`(`id`),
  UNIQUE (`group_id`, `device_id`)
);

DROP TABLE IF EXISTS `groups_product_defaults`;
CREATE TABLE `groups_product_defaults` (
  `id` INTEGER NOT NULL PRIMARY KEY AUTO_INCREMENT,
  `group_id` INTEGER NOT NULL REFERENCES `groups`(`id`),
  `product_id` INTEGER NOT NULL REFERENCES `products`(`id`),
  UNIQUE (`group_id`, `product_id`)
);

DROP TABLE IF EXISTS `policies`;
CREATE TABLE `policies` (
  `id` INTEGER NOT NULL PRIMARY KEY AUTO_INCREMENT,
  `type` INTEGER NOT NULL,
  `name` VARCHAR(100) NOT NULL UNIQUE,
  `argument` VARCHAR(100) DEFAULT '' NOT NULL,
  `rec_fail` INTEGER NOT NULL,
  `rec_noresult` INTEGER NOT NULL,
  `file` INTEGER DEFAULT 0 REFERENCES `files`(`id`),
  `dir` INTEGER DEFAULT 0 REFERENCES `directories`(`id`)
);

DROP TABLE IF EXISTS `enforcements`;
CREATE TABLE `enforcements` (
  `id` INTEGER NOT NULL PRIMARY KEY AUTO_INCREMENT,
  `policy` INTEGER NOT NULL REFERENCES `policies`(`id`),
  `group_id` INTEGER NOT NULL REFERENCES `groups`(`id`),
  `rec_fail` INTEGER,
  `rec_noresult` INTEGER,
  `max_age` INTEGER NOT NULL,
  UNIQUE (`policy`, `group_id`)
);

DROP TABLE IF EXISTS `sessions`;
CREATE TABLE `sessions` (
  `id` INTEGER NOT NULL PRIMARY KEY AUTO_INCREMENT,
  `time` INTEGER NOT NULL,
  `connection` INTEGER NOT NULL,
  `identity` INTEGER DEFAULT 0 REFERENCES `identities`(`id`),
  `device` INTEGER DEFAULT 0 REFERENCES `devices`(`id`),
  `product` INTEGER DEFAULT 0 REFERENCES `products`(`id`),
  `rec` INTEGER DEFAULT 3
);

DROP TABLE IF EXISTS `sessions_identities`;
CREATE TABLE `sessions_identities` (
  `id` INTEGER NOT NULL PRIMARY KEY AUTO_INCREMENT,
  `session_id` INTEGER NOT NULL REFERENCES `sessions`(`id`),
  `identity_id` INTEGER NOT NULL REFERENCES `identities`(`id`),
  UNIQUE (`session_id`, `identity_id`)
);

DROP TABLE IF EXISTS `workitems`;
CREATE TABLE `workitems` (
  `id` INTEGER NOT NULL PRIMARY KEY AUTO_INCREMENT,
  `session` INTEGER NOT NULL REFERENCES `sessions`(`id`),
  `enforcement` INTEGER NOT NULL REFERENCES `enforcements`(`id`),
  `type` INTEGER NOT NULL,
  `arg_str` VARCHAR(128),
  `arg_int` INTEGER DEFAULT 0,
  `rec_fail` INTEGER NOT NULL,
  `rec_noresult` INTEGER NOT NULL,
  `rec_final` INTEGER,
  `result` VARCHAR(128)
);

DROP TABLE IF EXISTS `results`;
CREATE TABLE `results` (
  `id` INTEGER NOT NULL PRIMARY KEY AUTO_INCREMENT,
  `session` INTEGER NOT NULL REFERENCES `measurements`(`id`),
  `policy` INTEGER NOT NULL REFERENCES `policies`(`id`),
  `rec` INTEGER NOT NULL,
  `result` TEXT NOT NULL
);

DROP TABLE IF EXISTS `components`;
CREATE TABLE `components` (
  `id` INTEGER NOT NULL PRIMARY KEY AUTO_INCREMENT,
  `vendor_id` INTEGER NOT NULL,
  `name` INTEGER NOT NULL,
  `qualifier` INTEGER DEFAULT 0
);

DROP TABLE IF EXISTS `key_component`;
CREATE TABLE `key_component` (
  `key` INTEGER NOT NULL,
  `component` INTEGER NOT NULL,
  `depth` INTEGER DEFAULT 0,
  `seq_no` INTEGER DEFAULT 0,
  PRIMARY KEY (`key`, `component`)
);

DROP TABLE IF EXISTS `component_hashes`;
CREATE TABLE `component_hashes` (
  `component` INTEGER NOT NULL,
  `key` INTEGER NOT NULL,
  `seq_no` INTEGER NOT NULL,
  `pcr` INTEGER NOT NULL,
  `algo` INTEGER NOT NULL,
  `hash` VARBINARY(32) NOT NULL,
  PRIMARY KEY(`component`, `key`, `seq_no`, `algo`)
);

DROP TABLE IF EXISTS `packages`;
CREATE TABLE `packages` (
  `id` INTEGER NOT NULL PRIMARY KEY AUTO_INCREMENT,
  `name` VARCHAR(128) NOT NULL,
  `blacklist` INTEGER DEFAULT 0
);

DROP TABLE IF EXISTS versions;
CREATE TABLE versions (
  `id` INTEGER NOT NULL PRIMARY KEY AUTO_INCREMENT,
  `package` INTEGER NOT NULL REFERENCES packages(id),
  `product` INTEGER NOT NULL REFERENCES products(id),
  `release` VARCHAR(32) NOT NULL,
  `security` INTEGER DEFAULT 0,
  `blacklist` INTEGER DEFAULT 0,
  `time` INTEGER DEFAULT 0
);

DROP TABLE IF EXISTS `devices`;
CREATE TABLE `devices` (
  `id` INTEGER NOT NULL PRIMARY KEY AUTO_INCREMENT,
  `description` VARCHAR(100) DEFAULT "",
  `value` VARCHAR(256) NOT NULL,
  `product` INTEGER REFERENCES `products`(`id`),
  `trusted` INTEGER DEFAULT 0,
  `created` INTEGER,
  `inactive` INTEGER DEFAULT 0
);

DROP TABLE IF EXISTS `identities`;
CREATE TABLE `identities` (
  `id` INTEGER NOT NULL PRIMARY KEY AUTO_INCREMENT,
  `type` INTEGER NOT NULL,
  `value` VARBINARY(128) NOT NULL,
  UNIQUE (type, value)
);

DROP TABLE IF EXISTS `regids`;
CREATE TABLE `regids` (
  `id` INTEGER NOT NULL PRIMARY KEY AUTO_INCREMENT,
  `name` TEXT NOT NULL
);

DROP TABLE IF EXISTS `tags`;
CREATE TABLE `tags` (
  `id` INTEGER NOT NULL PRIMARY KEY AUTO_INCREMENT,
  `regid` INTEGER NOT NULL REFERENCES `regids`(`id`),
  `unique_sw_id` VARCHAR(64) NOT NULL,
  `value` VARCHAR(128)
);
