/* Devices */

INSERT INTO devices (                  /*  1 */
  value, product, created
)
SELECT 'aabbccddeeff11223344556677889900', id, 1372330615
FROM products WHERE name = 'Debian DEBIAN_VERSION x86_64';

/* Groups Members */

INSERT INTO groups_members (
  group_id, device_id
) VALUES (
  5, 1
);

/* Identities */

INSERT INTO identities (
  type, value
) VALUES ( /* dave@strongswan.org */
  4, X'64617665407374726f6e677377616e2e6f7267'
);

/* Sessions */

INSERT INTO sessions (
  time, connection, identity, device, product, rec
)
SELECT NOW, 1, 1, 1, id, 0
FROM products WHERE name = 'Debian DEBIAN_VERSION x86_64';

/* Results */

INSERT INTO results (
  session, policy, rec, result
) VALUES (
  1, 1, 0, 'processed 355 packages: 0 not updated, 0 blacklisted, 4 ok, 351 not found'
);

/* Enforcements */

UPDATE enforcements SET
  rec_fail = 2, rec_noresult = 2
WHERE id = 3;
