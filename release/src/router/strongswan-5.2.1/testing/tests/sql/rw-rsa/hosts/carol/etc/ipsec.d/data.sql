/* Identities */

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

INSERT INTO identities (
  type, data
) VALUES ( /* subjkey of carol@strongswan.org */
  11, X'1fa1a988d9648cb5a0a2546439b4f23d745d6e7c'
 );

INSERT INTO identities (
  type, data
) VALUES ( /* subjkey of moon.strongswan.org */
  11, X'6a9c74d1f8897989f65a94e989f1fac3649d292e'
 );

/* Certificates */

INSERT INTO certificates (
   type, keytype, data
) VALUES ( /* carol@strongswan.org */
  6, 1, X'30820122300d06092a864886f70d01010105000382010f003082010a0282010100d05d594f8117bc78972a3ec479ebe1400e53cf72410b93e6f74fa17cf1ea444fb23600bae92d81747e49a2e4407c3f6118033d22a3e67ce69a53907ffad646bfbc3b6abe0bdd9a5080a690dbd919a9a8e70d9694e319e93e5d9361eff9033ac53fc6cd6c95af574c62effbb72c03d41c3b696fc7aa4444483bbaabde555aef8bce0e9797108d11ecf462c66b37f7c2e812f6ab3280a8c05b207156f0e3a787e9c4638205e40ce466716bc35d8623bd99f3cda9c3dee5c8ac19852cff18c405049c7eae735dc393f5209c13946e4f51da030ad7bf31caf58a203eccea2fc79e71d46a06c5dba85d65397a0adfd4cb5a9517fd3dcf17af8ab7584293026b19ad510203010001'
);

INSERT INTO certificates (
   type, keytype, data
) VALUES ( /* moon.strongswan.org */
  6, 1, X'30820122300d06092a864886f70d01010105000382010f003082010a0282010100ca2f633dd4bbba0586215b15a0312f73f533124f0b339b9ae13bb648b02b4c468e0f01e630fbef92197b7708f5dbffea7e496286966d75acf13bd5e4377a1821d82de102eadf9963b489041a0b0f9f76b79e2150aa39020e3fa52a677dbb879c986291e4f1542fe2f0494e9c5c954d4faa75a17aa7b56652f1b16efbdcb46697f7d0b7f520bc990205365938d2cd31f2beed30e761a56c02d9dc82f0cdefc9d43447b6a98f7628aed2ac127a4a9504838f66e7517e5e0b0672c8165474bce689f73a6fc6e3c72b2c45498ddbbc0b17b04915606fe94f256cc777c42c534560ffbbe5aacdd944cc8db4d2abaf8a294af55b03a6a01a54d78430ab78389753c2870203010001'
);

INSERT INTO certificate_identity (
  certificate, identity
) VALUES (
  1, 1
);

INSERT INTO certificate_identity (
  certificate, identity
) VALUES (
  1, 3
);

INSERT INTO certificate_identity (
  certificate, identity
) VALUES (
  2, 2
);

INSERT INTO certificate_identity (
  certificate, identity
) VALUES (
  2, 4
);

/* Private Keys */

INSERT INTO private_keys (
   type, data
) VALUES ( /* key of carol@strongswan.org' */
  1, X'308204a40201000282010100d05d594f8117bc78972a3ec479ebe1400e53cf72410b93e6f74fa17cf1ea444fb23600bae92d81747e49a2e4407c3f6118033d22a3e67ce69a53907ffad646bfbc3b6abe0bdd9a5080a690dbd919a9a8e70d9694e319e93e5d9361eff9033ac53fc6cd6c95af574c62effbb72c03d41c3b696fc7aa4444483bbaabde555aef8bce0e9797108d11ecf462c66b37f7c2e812f6ab3280a8c05b207156f0e3a787e9c4638205e40ce466716bc35d8623bd99f3cda9c3dee5c8ac19852cff18c405049c7eae735dc393f5209c13946e4f51da030ad7bf31caf58a203eccea2fc79e71d46a06c5dba85d65397a0adfd4cb5a9517fd3dcf17af8ab7584293026b19ad51020301000102820101008a86f560a12fd9b2c6b5646395b757db116b7108c1ebe399f3a43275f213d0f4b743a5fa9e1a0dee1823f6b30f984b3ecf0b20330e8aa7515d1adf7fe6915d1a0e17e6a0911cf4823ef5fe6adb8c6f3a86da86d9579f1b6dc622bd4320e79fc411e1a72360e4e1023ce8d60df2aa5cf3a420361ba5bc9b34f6d8e578fc8067aef457ff2a5a91f7c7c8a4f242cc46ab1cff18d6d1a4d8b82b5791df1f18a04b8cb2da235c3a795900fa7ffbbcb4b1d079a504673794eeca5e75c8c16ad2558965453515f57180b8dc518710aa874cd3bcdeddf109d5b553c27a351437f9c1bcdf8940f3ff8632dd59f6c8e01107b4bbf49c6c5409b6e9ecb29eada333800fa5d102818100f5537045d067df4d786df67e1cfea01ce3ca6de3f06be48efd8b4dc48c1ec19a711003875ee202343c851c8f970d2160a2e4d1108da6437af6bccfae803f61fc450582a89aa0906cba65fc7033aa584c1a2a17e96b2f368951aedaa5261f6e1c14999cfa7f4c4091890eff281cb4e6f37209f4b6d5778607caeca0a15137746d02818100d96e379ba5b4ed604d84a7ed71678a1f23391b749a697289bd3bee5d17985af89264f5ec35d3e37b1e44d6fdafec8a78ae199e0bb680f59f9800bb894ba5b31ee964f3ddc142f361835cf49a1edb75330f44f33c26c56e0752ae3fe0df760838b6437b99a131cb544d64da996baf3bc3f5709cc75f134b048cdebbf8bed1a5f502818017e2d8e2a34909cff432d8b62cc289fa661f7695a3fa0d55f2cfd67195a704f8b19b7a8c7aab8cc563431ea5c87249d6abe595898411352ecac9557b4b1760c5fc3e6e1b567addb5086d17a8210fabeb34fcf6390eeb98e6c3e23f7da6f99671fb7b7d725264ad40be548c796e5a1ac6874afaddb6691dceb26e59b17b43e7fd02818100ac90cb559f089fb56510b9eebba8bf78c30dd9fa8b6ad15afc738a551c1af1688357d9cea5520c2374fc91dd3a38f4159fa7f945b68d576458d18c0605a1f72dbb734211680768fea5b1aaf87f31122d7a1af12976640f55848b836b482f778afb2d47f5c077b2b6afce31a8be4c8f949141d54c6eeaf309237ccb973a6b4dd902818025ec4178c7f7c017c46a93fbdb72e2e3b199ae33c9fa6834f81b75b481cd6756124c744c821b126b07ea9852dc58de40cc87f160209d8af3b4bac4862778d3e21cbfdfcdbbfaad8911500f43a43041b88eefb27c66adfffeb9c8add95d1a4ed05f0bd81a4eda79fd4a0885a4a3bad0dcddecf83401c19df721d6cc7163c4125e'
);

INSERT INTO private_key_identity (
  private_key, identity
) VALUES (
  1, 1
);

INSERT INTO private_key_identity (
  private_key, identity
) VALUES (
  1, 3
);

/* Configurations */

INSERT INTO ike_configs (
  local, remote
) VALUES (
  'PH_IP_CAROL', 'PH_IP_MOON'
);

INSERT INTO peer_configs (
  name, ike_cfg, local_id, remote_id
) VALUES (
  'home', 1, 1, 2 
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

