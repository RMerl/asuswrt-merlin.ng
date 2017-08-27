/* Identities */

INSERT INTO identities (
  type, data
) VALUES ( /* moon.strongswan.org */
  2, X'6d6f6f6e2e7374726f6e677377616e2e6f7267'
 );

INSERT INTO identities (
  type, data
) VALUES ( /* carol@strongswan.org */
  3, X'6361726f6c407374726f6e677377616e2e6f7267'
 );

INSERT INTO identities (
  type, data
) VALUES ( /* dave@strongswan.org */
  3, X'64617665407374726f6e677377616e2e6f7267'
 );

INSERT INTO identities (
  type, data
) VALUES ( /* %any */
  0, '%any'
);

INSERT INTO identities (
  type, data
) VALUES ( /* subjkey of moon.strongswan.org */
  11, X'6a9c74d1f8897989f65a94e989f1fac3649d292e'
 );

INSERT INTO identities (
  type, data
) VALUES ( /* subjkey of carol@strongswan.org */
  11, X'1fa1a988d9648cb5a0a2546439b4f23d745d6e7c'
 );

INSERT INTO identities (
  type, data
) VALUES ( /* subjkey of dave@strongswan.org */
  11, X'ee7f38daeea1b81a41777f78f2674be8439d8e0e'
 );

/* Raw RSA public keys */

INSERT INTO certificates (
   type, keytype, data
) VALUES ( /* moon.strongswan.org */
  6, 1, X'30820122300d06092a864886f70d01010105000382010f003082010a0282010100ca2f633dd4bbba0586215b15a0312f73f533124f0b339b9ae13bb648b02b4c468e0f01e630fbef92197b7708f5dbffea7e496286966d75acf13bd5e4377a1821d82de102eadf9963b489041a0b0f9f76b79e2150aa39020e3fa52a677dbb879c986291e4f1542fe2f0494e9c5c954d4faa75a17aa7b56652f1b16efbdcb46697f7d0b7f520bc990205365938d2cd31f2beed30e761a56c02d9dc82f0cdefc9d43447b6a98f7628aed2ac127a4a9504838f66e7517e5e0b0672c8165474bce689f73a6fc6e3c72b2c45498ddbbc0b17b04915606fe94f256cc777c42c534560ffbbe5aacdd944cc8db4d2abaf8a294af55b03a6a01a54d78430ab78389753c2870203010001'
);

INSERT INTO certificates (
   type, keytype, data
) VALUES ( /* carol@strongswan.org */
  6, 1, X'30820122300d06092a864886f70d01010105000382010f003082010a0282010100d05d594f8117bc78972a3ec479ebe1400e53cf72410b93e6f74fa17cf1ea444fb23600bae92d81747e49a2e4407c3f6118033d22a3e67ce69a53907ffad646bfbc3b6abe0bdd9a5080a690dbd919a9a8e70d9694e319e93e5d9361eff9033ac53fc6cd6c95af574c62effbb72c03d41c3b696fc7aa4444483bbaabde555aef8bce0e9797108d11ecf462c66b37f7c2e812f6ab3280a8c05b207156f0e3a787e9c4638205e40ce466716bc35d8623bd99f3cda9c3dee5c8ac19852cff18c405049c7eae735dc393f5209c13946e4f51da030ad7bf31caf58a203eccea2fc79e71d46a06c5dba85d65397a0adfd4cb5a9517fd3dcf17af8ab7584293026b19ad510203010001'
);

INSERT INTO certificates (
   type, keytype, data
) VALUES ( /* dave@strongswan.org */
  6, 1, X'30820122300d06092a864886f70d01010105000382010f003082010a0282010100c007f2536f0558e68345d3ef017a175b73434e797f9b97448e720a5985aea76cc0503b8d63b4e239af95db83db0af65f8360e9d941912c121215643a0af32188fc520413e81645ff8e2e623c9362be1b57649530bf54fdad6563106105ace949d7de2895a4771c237090aaa2567bd7d9b08b2ad09f63f61bba87d7462046e89fa4570cb3c8e4322220a737af48c31cd0ec2140f3723b94742c4a14232e1d409f6b53c18aaa63e693fa5d3d06808e948db8273563d33dbd9ac44ecfd71e60426570885898b3e5538767eaf4ef713719e7fd89b32e4e3f60d972ef1617437d4dba14af4691fb8ec275a78552bae9d8aaf71c3d42aca793ef0ad09e1af353daab750203010001'
);

INSERT INTO certificate_identity (
  certificate, identity
) VALUES (
  1, 1
);

INSERT INTO certificate_identity (
  certificate, identity
) VALUES (
  1, 5
);

INSERT INTO certificate_identity (
  certificate, identity
) VALUES (
  2, 2
);

INSERT INTO certificate_identity (
  certificate, identity
) VALUES (
  2, 6
);

INSERT INTO certificate_identity (
  certificate, identity
) VALUES (
  3, 3
);

INSERT INTO certificate_identity (
  certificate, identity
) VALUES (
  3, 7
);

/* Private Keys */

INSERT INTO private_keys (
   type, data
) VALUES ( /* key of CN=moon.strongswan.org' */
  1, X'308204a30201000282010100ca2f633dd4bbba0586215b15a0312f73f533124f0b339b9ae13bb648b02b4c468e0f01e630fbef92197b7708f5dbffea7e496286966d75acf13bd5e4377a1821d82de102eadf9963b489041a0b0f9f76b79e2150aa39020e3fa52a677dbb879c986291e4f1542fe2f0494e9c5c954d4faa75a17aa7b56652f1b16efbdcb46697f7d0b7f520bc990205365938d2cd31f2beed30e761a56c02d9dc82f0cdefc9d43447b6a98f7628aed2ac127a4a9504838f66e7517e5e0b0672c8165474bce689f73a6fc6e3c72b2c45498ddbbc0b17b04915606fe94f256cc777c42c534560ffbbe5aacdd944cc8db4d2abaf8a294af55b03a6a01a54d78430ab78389753c287020301000102820100204507f5ea6a3bfa7db9fd2baa71af3d36b97c0699a71702d5480e83f37a35a65d2e10038975ec7ac90e67a54a785e9432abcbc9e7607913ad3cfb9a7d304381c35b2f3aa3fa242541bf4ca44b77a6dfefd69142aaa886a777890907938dc6cb3b971fea068a854a1747dc0020d6c38c1f8cbec530d747099e01cfd0eb1ceff2b077bd07aaef4989b75594614b16a778891a2e490369d2a9571ddf5cd165331638a8a3c96184a8259eb588caab3bbfab9c0f77b66c830ecf0f294dc1b67a5f36b75e3e095e247864f19ab212fdbf34e0925316ca13c342b4ba464ecf93d2a8e39eee24dd63dddd938101a9f4b8f0de90765e1c1fda5c62e161cc712794aeaea102818100f85d60a6990447926da1ab9db7f094a5d435b11f70c5fef9541a89e05898001190cfdc651b8a23ccbfe8e7bdacd225776f01699d06be5ae5abc4690fe99b81fd9f369e973437fbcba2efdbe1dc6f8389fb2be78e3847f4f05323b2c7b6b6a4c85ca0aa72642747434f4358f0baf10ab173f9c3f24e9674570179dde23c6c248d02818100d06693eb5c92b6d516f630b79b1b98ea3910cbc4c442a4779ce16f5b09825c858ea4dfcc4d33eeb3e4de971a7fa5d2a153e9a83e65f7527ca77b93efc257960eadd8ce5b57e590d9189e542652ae3677c623343a39c1d16dbef3069406eaa4913eeba06e0a3af3c8539dbd4be7d9caf3ccd654ae397ae7faa72ba823e4b0206302818100ef2bc4f249f28415ef7b3bafd33d5b7861e61e9e7f543c18d0340a4840288810625ab90ba8bc9b8305dffca27c75965cf049f4f1a157d862c9c987bf2a2075cacdf2a44049aa0bd16b23fea3ff4a67ea8d351774aea024b0f5ef2fb00134db749336a94d254369edd8bbab3f8f56a60c82f9a807844480de746e6e0cfa50cdd50281807b32d8e93fadc00612eff176e96c14270b1b41cb0dd6f3d17e5dcaedbf9e6041d844e1c4ae33303f0ae307e2f3693d2e8023d68124d863dc2b4aa3f70e25a7210066f5ff0be43b900bbcb5b47e165d3ecb544e70c96a29fbbdf17f870cdbb3f3e585782ef53f4a94b7d1bd715d1be49de20f26ba6462a3370b928470cba5cf4f028180324ffacf705e6746f741d24ff6aa0bb14aad55cba41eb7758e6cc0d51f40feac6b4a459ce374af424287f602b0614520079b436b8e90cde0ddff679304e9efdd74a2ffbfe6e4e1bd1236c360413f2d2656e00b3e3cb217567671bf73a722a222e5e85d109fe2c77caf5951f5b9f4171c744afa717fe7e9306488e6ab87341298'
);

INSERT INTO private_key_identity (
  private_key, identity
) VALUES (
  1, 1
);

INSERT INTO private_key_identity (
  private_key, identity
) VALUES (
  1, 5
);

/* Configurations */

INSERT INTO ike_configs (
  local, remote
) VALUES (
  'PH_IP_MOON', '0.0.0.0'
);

INSERT INTO peer_configs (
  name, ike_cfg, local_id, remote_id
) VALUES (
  'rw', 1, 1, 4
);

INSERT INTO child_configs (
  name, updown
) VALUES (
  'rw', 'ipsec _updown iptables'
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
  1, 1, 0
);

INSERT INTO child_config_traffic_selector (
	child_cfg, traffic_selector, kind
) VALUES (
  1, 2, 3
);

