/* Identities */

INSERT INTO identities (
  type, data
) VALUES ( /* moon.strongswan.org */
  2, X'6d6f6f6e2e7374726f6e677377616e2e6f7267'
 );

INSERT INTO identities (
  type, data
) VALUES ( /* sun.strongswan.org */
  2, X'73756e2e7374726f6e677377616e2e6f7267'
 );

INSERT INTO identities (
  type, data
) VALUES ( /* %any */
  0, '%any'
 );

/* Shared Secrets */

INSERT INTO shared_secrets (
   type, data
) VALUES ( 
  1, X'bfe364c58f4b2d9bf08f8a820b6a3f806ad60c5d9ddb58cb'
);

INSERT INTO shared_secret_identity (
  shared_secret, identity
) VALUES (
  1, 1
);

INSERT INTO shared_secret_identity (
  shared_secret, identity
) VALUES (
  1, 2
);

/* Configurations */

INSERT INTO ike_configs (
  local, remote
) VALUES (
  'PH_IP_MOON', 'PH_IP_SUN'
);

INSERT INTO peer_configs (
  name, ike_cfg, local_id, remote_id, auth_method, mobike
) VALUES (
  'net-net', 1, 1, 2, 2, 0
);

INSERT INTO child_configs (
  name, updown
) VALUES (
  'net-net', 'ipsec _updown iptables'
);

INSERT INTO peer_config_child_config (
  peer_cfg, child_cfg
) VALUES (
  1, 1
);

INSERT INTO traffic_selectors (
  type, start_addr, end_addr
) VALUES (
  7, X'0a010000', X'0a01ffff'
);

INSERT INTO traffic_selectors (
  type, start_addr, end_addr
) VALUES (
  7, X'0a020000', X'0a02ffff'
);

INSERT INTO child_config_traffic_selector (
  child_cfg, traffic_selector, kind
) VALUES (
  1, 1, 0
);

INSERT INTO child_config_traffic_selector (
	child_cfg, traffic_selector, kind
) VALUES (
  1, 2, 1
);

