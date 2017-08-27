CREATE TABLE radippool (
        id                      INT PRIMARY KEY,
        pool_name               VARCHAR(30) NOT NULL,
        framedipaddress         VARCHAR(30) NOT NULL,
        nasipaddress            VARCHAR(30) NOT NULL,
        pool_key                INT NOT NULL,
        CalledStationId         VARCHAR(64),
        CallingStationId        VARCHAR(64) NOT NULL,
        expiry_time             timestamp(0) NOT NULL,
        username                VARCHAR(100)
);

CREATE INDEX radippool_poolname_ipaadr ON radippool (pool_name, framedipaddress);
CREATE INDEX radippool_poolname_expire ON radippool (pool_name, expiry_time);
CREATE INDEX radippool_nasipaddr_key ON radippool (nasipaddress, pool_key);
CREATE INDEX radippool_nasipaddr_calling ON radippool (nasipaddress, callingstationid);

CREATE SEQUENCE radippool_seq START WITH 1 INCREMENT BY 1;

CREATE OR REPLACE TRIGGER radippool_serialnumber
        BEFORE INSERT OR UPDATE OF id ON radippool
        FOR EACH ROW
        BEGIN
                if ( :new.id = 0 or :new.id is null ) then
                        SELECT radippool_seq.nextval into :new.id from dual;
                end if;
        END;
/
