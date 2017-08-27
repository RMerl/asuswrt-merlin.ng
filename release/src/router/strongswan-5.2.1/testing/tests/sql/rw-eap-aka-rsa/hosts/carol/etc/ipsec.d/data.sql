/* Identities */

INSERT INTO identities (
  type, data
) VALUES ( /* C=CH, O=Linux strongSwan, CN=strongSwan Root CA */
  9, X'3045310B300906035504061302434831193017060355040A13104C696E7578207374726F6E675377616E311B3019060355040313127374726F6E675377616E20526F6F74204341'
 );

INSERT INTO identities (
  type, data
) VALUES ( /* subjkey of 'C=CH, O=Linux strongSwan, CN=strongSwan Root CA' */
  11, X'5da7dd700651327ee7b66db3b5e5e060ea2e4def'
 );

INSERT INTO identities (
  type, data
) VALUES ( /* keyid of 'C=CH, O=Linux strongSwan, CN=strongSwan Root CA' */
  11, X'ae096b87b44886d3b820978623dabd0eae22ebbc'
 );

INSERT INTO identities (
  type, data
) VALUES ( /* carol@strongswan.org */
  3, X'6361726f6c407374726f6e677377616e2e6f7267'
 );

INSERT INTO identities (
  type, data
) VALUES ( /* moon.strongswan.org */
  2, X'6d6f6f6e2e7374726f6e677377616e2e6f7267'
 );

/* Certificates */

INSERT INTO certificates (
   type, keytype, data
) VALUES ( /* C=CH, O=Linux strongSwan, CN=strongSwan Root CA */
  1, 1, X'308203b8308202a0a003020102020100300d06092a864886f70d01010b05003045310b300906035504061302434831193017060355040a13104c696e7578207374726f6e675377616e311b3019060355040313127374726f6e675377616e20526f6f74204341301e170d3034303931303130303131385a170d3139303930373130303131385a3045310b300906035504061302434831193017060355040a13104c696e7578207374726f6e675377616e311b3019060355040313127374726f6e675377616e20526f6f7420434130820122300d06092a864886f70d01010105000382010f003082010a0282010100bff25f62ea3d566e58b3c87a49caf3ac61cfa96377734d842db3f8fd6ea023f7b0132e66265012317386729c6d7c427a8d9f167be138e8ebae2b12b95933baef36a315c3ddf224cee4bb9bd578135d0467382629621ff96b8d45f6e002e5083662dce181805c140b3f2ce93f83aee3c861cff610a39f0189cb3a3c7cb9bf7e2a09544e2170efaa18fdd4ff20fa94be176d7fecff821f68d17152041d9b46f0cfcfc1e4cf43de5d3f3a587763afe9267f53b11699b3264fc55c5189f5682871166cb98307950569641fa30ffb50de134fed2f973cef1a392827862bc4ddaa97bbb01442e293c41070d07224d4be47ae2753eb2bed4bc1da91c68ec780c4620f0f0203010001a381b23081af30120603551d130101ff040830060101ff020101300b0603551d0f040403020106301d0603551d0e041604145da7dd700651327ee7b66db3b5e5e060ea2e4def306d0603551d230466306480145da7dd700651327ee7b66db3b5e5e060ea2e4defa149a4473045310b300906035504061302434831193017060355040a13104c696e7578207374726f6e675377616e311b3019060355040313127374726f6e675377616e20526f6f74204341820100300d06092a864886f70d01010b0500038201010023929aa101b412d1f5a577532088f209b34798a72ed7bd6945d74beaa2b3a1768764ad7f8b0df8d97a1a3ed1102e92a5f107e3059dc2250be49d02261ca83a342e0e5de7d43c37744e3fcea3197720ca1184d4ef94e6beeb0d241746b0b92b7fb1004c08e88bf9eb4ce60f3e149466f3e9fc3f98bce449f448f9d465e52b59f0101e6203cfad0d89e23509fa043d4c12021e8f32be7db8b2edbada641d64aa1a04af64a2ee5b814a753dd76b30e3de04f3c6b61166e632f8364d51cf3730a9564a4d93b9227c28b09b0f5595d92a632f72fe509a129ca9ee54df2b0edc6c3d38564f10256efcd8be82b2ec64977e3a6f5ef098eaa7f00662a6cded16cb80637c'
);

INSERT INTO certificate_identity (
  certificate, identity
) VALUES (
  1, 1
);

INSERT INTO certificate_identity (
  certificate, identity
) VALUES (
  1, 2
);

INSERT INTO certificate_identity (
  certificate, identity
) VALUES (
  1, 3
);

/* Shared Secrets */

INSERT INTO shared_secrets (
  type, data
) VALUES ( 
  2, X'4172336574546e703031716c704f6762'
);

INSERT INTO shared_secret_identity (
  shared_secret, identity
) VALUES (
  1,  4 
);

INSERT INTO shared_secret_identity (
  shared_secret, identity
) VALUES (
  1, 5 
);

/* Configurations */

INSERT INTO ike_configs (
  local, remote
) VALUES (
  'PH_IP_CAROL', 'PH_IP_MOON'
);

INSERT INTO peer_configs (
  name, ike_cfg, local_id, remote_id, auth_method
) VALUES (
  'home', 1, 4, 5, 3
);

INSERT INTO child_configs (
  name, updown
) VALUES (
  'home', 'ipsec _updown iptables'
);

INSERT INTO peer_config_child_config (
  peer_cfg, child_cfg
) VALUES (
  1, 1
);

INSERT INTO traffic_selectors (
  type, start_addr, end_addr
) VALUES ( /* 10.1.0.0/16 */
  7, X'0a010000', X'0a01ffff'
);

INSERT INTO traffic_selectors (
  type
) VALUES ( /* dynamic/32 */
  7
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

