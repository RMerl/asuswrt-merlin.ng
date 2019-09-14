DROP DATABASE IF EXISTS syslogtest;
CREATE DATABASE syslogtest;
\c syslogtest

CREATE TABLE systemevents (
	ID serial not null primary key,
	CustomerID bigint,
	ReceivedAt timestamp without time zone NULL,
	DeviceReportedTime timestamp without time zone NULL,
	Facility smallint NULL,
	Priority smallint NULL,
	FromHost varchar(60) NULL,
	Message text,
	NTSeverity int NULL,
	Importance int NULL,
	EventSource varchar(60),
	EventUser varchar(60) NULL,
	EventCategory int NULL,
	EventID int NULL,
	EventBinaryData text NULL,
	MaxAvailable int NULL,
	CurrUsage int NULL,
	MinUsage int NULL,
	MaxUsage int NULL,
	InfoUnitID int NULL ,
	SysLogTag varchar(60),
	EventLogType varchar(60),
	GenericFileName VarChar(60),
	SystemID int NULL
);
