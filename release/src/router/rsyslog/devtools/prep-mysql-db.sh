#!/bin/bash
# this script prepares a mysql instance for use by the rsyslog testbench

mysql -u root -e "CREATE USER 'rsyslog'@'localhost' IDENTIFIED BY 'testbench';"
mysql -u root -e "GRANT ALL PRIVILEGES ON * . * TO 'rsyslog'@'localhost'; FLUSH PRIVILEGES;"
mysql -u root -e "CREATE DATABASE Syslog; GRANT ALL ON Syslog.* to 'rsyslog'@'localhost' identified by 'testbench';"
mysql -u root -e "USE Syslog; CREATE TABLE SystemEvents (ID int unsigned not null auto_increment primary key, CustomerID bigint,ReceivedAt datetime NULL,DeviceReportedTime datetime NULL,Facility smallint NULL,Priority smallint NULL,FromHost varchar(60) NULL,Message text,NTSeverity int NULL,Importance int NULL,EventSource varchar(60),EventUser varchar(60) NULL,EventCategory int NULL,EventID int NULL,EventBinaryData text NULL,MaxAvailable int NULL,CurrUsage int NULL,MinUsage int NULL,MaxUsage int NULL,InfoUnitID int NULL,SysLogTag varchar(60),EventLogType varchar(60),GenericFileName VarChar(60),SystemID int NULL); CREATE TABLE SystemEventsProperties (ID int unsigned not null auto_increment primary key,SystemEventID int NULL,ParamName varchar(255) NULL,ParamValue text NULL);"
