/* Identities */

INSERT INTO identities (
  type, data
) VALUES ( /* fec0::1 */
  5 , X'fec00000000000000000000000000001'
 );

INSERT INTO identities (
  type, data
) VALUES ( /* fec0::10 */
  5 , X'fec00000000000000000000000000010'
 );

/* Shared Secrets */

INSERT INTO shared_secrets (
   type, data
) VALUES ( 
  1, X'16964066a10de938bdb2ab7864fe4459cab1'
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
  'PH_IP6_CAROL', 'PH_IP6_MOON'
);

INSERT INTO peer_configs (
  name, ike_cfg, local_id, remote_id, auth_method
) VALUES (
  'home', 1, 2, 1, 2
);

INSERT INTO child_configs (
  name, updown
) VALUES (
  'home', '/usr/local/libexec/ipsec/_updown iptables'
);

INSERT INTO peer_config_child_config (
  peer_cfg, child_cfg
) VALUES (
  1, 1
);

INSERT INTO traffic_selectors (
  type, start_addr, end_addr
) VALUES ( /* fec1::/16 */
  8, X'fec10000000000000000000000000000', X'fec1ffffffffffffffffffffffffffff'
);

INSERT INTO traffic_selectors (
  type
) VALUES ( /* dynamic/128 */
  8
);

INSERT INTO child_config_traffic_selector (
  child_cfg, traffic_selector, kind
) VALUES (
  1, 1, 1
);

INSERT INTO child_config_traffic_selector (
	child_cfg, traffic_selector, kind
) VALUES (
  1, 2, 2
);

