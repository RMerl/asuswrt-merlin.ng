insert into radcheck(username,attribute,op,value) values('test','Cleartext-Password',':=','test');
insert into radcheck(username,attribute,op,value) values('time1','Cleartext-Password',':=','time1');
insert into radcheck(username,attribute,op,value) values('ee','Cleartext-Password',':=','ee');
insert into radcheck(username,attribute,op,value) values('flow','Cleartext-Password',':=','flow');
insert into radcheck(username,attribute,op,value) values('t1','Cleartext-Password',':=','t1');
insert into radcheck(username,attribute,op,value) values('b1','Cleartext-Password',':=','b1');
insert into radcheck(username,attribute,op,value) values('f1','Cleartext-Password',':=','f1');

insert into radusergroup(username,groupname) values('test','user');
insert into radusergroup(username,groupname) values('t1','limit_time');
insert into radusergroup(username,groupname) values('b1','limit_bandwidth');
insert into radusergroup(username,groupname) values('f1','limit_flow');

insert into radgroupreply(groupname,attribute,op,value) values('user','Auth-Type',':=','Local');
insert into radgroupreply(groupname,attribute,op,value) values('user','Service-Type',':=','Framed-User');
insert into radgroupreply(groupname,attribute,op,value) values('user','Framed-IP-Address',':=','255.255.255.255');
insert into radgroupreply(groupname,attribute,op,value) values('user','Framed-IP-Netmask',':=','255.255.255.0');
insert into radgroupreply(groupname,attribute,op,value) values('limit_time','Session-Timeout',':=','100');
insert into radgroupreply(groupname,attribute,op,value) values('limit_bandwidth','ChilliSpot-Bandwidth-Max-Down',':=','100');
insert into radgroupreply(groupname,attribute,op,value) values('limit_flow','ChilliSpot-Max-Total-Octets',':=','10000000');

insert into radreply(username,attribute,op,value) values('time1','Session-Timeout',':=','100');
insert into radreply(username,attribute,op,value) values('ee','ChilliSpot-Bandwidth-Max-Down',':=','100');
insert into radreply(username,attribute,op,value) values('flow','ChilliSpot-Max-Total-Octets',':=','10000000');

insert into nas(id,nasname,shortname,secret) values(1,'10.0.10.0/24','chillispot','testing123');
