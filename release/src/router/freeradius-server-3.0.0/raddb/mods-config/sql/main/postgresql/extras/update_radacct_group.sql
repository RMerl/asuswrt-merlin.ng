/*
 * $Id$
 *
 * OPTIONAL Postgresql trigger for FreeRADIUS
 *
 * This trigger updates fills in the groupname field (which doesnt come in Accounting packets)
 * by querying the radusergroup table.
 * This makes it easier to do group summary reports, however note that it does add some extra
 * database load to 50% of your SQL accounting queries. If you dont care about group summary
 * reports then you dont need to install this.
 *
 */


CREATE OR REPLACE FUNCTION upd_radgroups() RETURNS trigger AS'

DECLARE
        v_groupname varchar;

BEGIN
        SELECT INTO v_groupname GroupName FROM radusergroup WHERE CalledStationId = NEW.CalledStationId AND UserName = NEW.UserName;
        IF FOUND THEN
                UPDATE radacct SET GroupName = v_groupname WHERE RadAcctId = NEW.RadAcctId;
        END IF;

        RETURN NEW;
END

'LANGUAGE plpgsql;


DROP TRIGGER upd_radgroups ON radacct;

CREATE TRIGGER upd_radgroups AFTER INSERT ON radacct
    FOR EACH ROW EXECUTE PROCEDURE upd_radgroups();


