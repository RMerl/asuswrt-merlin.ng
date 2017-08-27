CREATE OR REPLACE FUNCTION msqlippool(user varchar2, pool varchar2)
RETURN varchar2 IS

	PRAGMA AUTONOMOUS_TRANSACTION;
	ip_temp varchar2(20);
BEGIN

    -- If the user's pool is dynamic, get an ipaddress (oldest one) from the corresponding pool

    if pool = 'Dynamic' then
	select framedipaddress into ip_temp from (select framedipaddress from radippool where expiry_time < current_timestamp and pool_name = pool ORDER BY expiry_time) where rownum = 1;
	return (ip_temp);

    -- Else, then get the static ipaddress for that user from the corresponding pool

    else
	select framedipaddress into ip_temp from radippool where username = user and pool_name = pool;
	return (ip_temp);
    end if;

exception

 -- This block is executed if there's no free ipaddresses or no static ip assigned to the user

 when NO_DATA_FOUND then
	if pool = 'Dynamic' then
		return(''); -- so sqlippool can log it on radius.log
	end if;

	-- Else, grabs a free IP from the static pool and saves it in radippool so the user will always get the same IP the next time

	select framedipaddress into ip_temp from (select framedipaddress from radippool where expiry_time < current_timestamp and username is null and pool_name = pool) where rownum = 1;
	UPDATE radippool SET username = user where framedipaddress = ip_temp;
	commit;
	return (ip_temp);

 when others
  then return('Oracle Exception');

END;
/
