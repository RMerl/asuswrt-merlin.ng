/* SW Identifiers */

INSERT INTO sw_identifiers (
  name, package, version, source, installed
) VALUES (
	'strongswan.org__Debian_DEBIAN_VERSION-x86_64-libutempter0-1.1.5', 'libutempter0', '1.1.5', 1, 0
);

INSERT INTO sw_identifiers (
  name, package, version, source, installed
) VALUES (
  'strongswan.org__Debian_DEBIAN_VERSION-x86_64-libevent-2.0-5-2.0.20', 'libevent-2.0-5', '2.0.20', 1, 0
);

INSERT INTO sw_identifiers (
  name, package, version, source, installed
) VALUES (
  'strongswan.org__Debian_DEBIAN_VERSION-x86_64-tmux-2.2', 'tmux', '2.2', 1, 0
);

/* SW Events */

INSERT INTO sw_events (
  eid, sw_id, action
) VALUES (
  2, 1, 2
);

INSERT INTO sw_events (
  eid, sw_id, action
) VALUES (
  2, 2, 2
);

INSERT INTO sw_events (
  eid, sw_id, action
) VALUES (
  2, 3, 2
);
