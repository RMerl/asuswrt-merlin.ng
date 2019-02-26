/* IMV PTS SQLite database */

DROP TABLE IF EXISTS directories;
CREATE TABLE directories (
  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  path TEXT NOT NULL
);
DROP INDEX IF EXISTS directories_path;
CREATE INDEX directories_path ON directories (
  path
);

DROP TABLE IF EXISTS files;
CREATE TABLE files (
  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  dir INTEGER DEFAULT 0 REFERENCES directories(id),
  name TEXT NOT NULL
);
DROP INDEX IF EXISTS files_name;
CREATE INDEX files_name ON files (
  name
);

DROP TABLE IF EXISTS products;
CREATE TABLE products (
  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  name TEXT NOT NULL
);
DROP INDEX IF EXISTS products_name;
CREATE INDEX products_name ON products (
  name
);

DROP TABLE IF EXISTS algorithms;
CREATE TABLE algorithms (
  id INTEGER PRIMARY KEY,
  name VARCHAR(20) not NULL
);

DROP TABLE IF EXISTS file_hashes;
CREATE TABLE file_hashes (
  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  file INTEGER NOT NULL REFERENCES files(id),
  version INTEGER REFERENCES versions(id),
  device INTEGER REFERENCES devices(id),
  size INTEGER,
  algo INTEGER NOT NULL REFERENCES algorithms(id),
  hash VARCHAR(64) NOT NULL,
  mutable INTEGER DEFAULT 0
);
DROP INDEX IF EXISTS "file_hashes_idx";
CREATE INDEX "file_hashes_idx" ON "file_hashes" ("file", "version", "algo");

DROP TABLE IF EXISTS groups;
CREATE TABLE groups (
  id INTEGER NOT NULL PRIMARY KEY,
  name VARCHAR(50) NOT NULL UNIQUE,
  parent INTEGER
);

DROP TABLE IF EXISTS groups_members;
CREATE TABLE groups_members (
  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  group_id INTEGER NOT NULL REFERENCES groups(id),
  device_id INTEGER NOT NULL REFERENCES devices(id),
  UNIQUE (group_id, device_id)
);

DROP TABLE IF EXISTS groups_product_defaults;
CREATE TABLE groups_product_defaults (
  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  group_id INTEGER NOT NULL REFERENCES groups(id),
  product_id INTEGER NOT NULL REFERENCES products(id),
  UNIQUE (group_id, product_id)
);

DROP TABLE IF EXISTS policies;
CREATE TABLE policies (
  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  type INTEGER NOT NULL,
  name VARCHAR(100) NOT NULL UNIQUE,
  argument TEXT DEFAULT '' NOT NULL,
  rec_fail INTEGER NOT NULL,
  rec_noresult INTEGER NOT NULL,
  file INTEGER DEFAULT 0 REFERENCES files(id),
  dir INTEGER DEFAULT 0 REFERENCES directories(id)
);

DROP TABLE IF EXISTS enforcements;
CREATE TABLE enforcements (
  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  policy INTEGER NOT NULL REFERENCES policies(id),
  group_id INTEGER NOT NULL REFERENCES groups(id),
  rec_fail INTEGER,
  rec_noresult INTEGER,
  max_age INTEGER NOT NULL,
  UNIQUE (policy, group_id)
);

DROP TABLE IF EXISTS sessions;
CREATE TABLE sessions (
  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  time INTEGER NOT NULL,
  connection INTEGER NOT NULL,
  identity INTEGER DEFAULT 0 REFERENCES identities(id),
  device INTEGER DEFAULT 0 REFERENCES devices(id),
  product INTEGER DEFAULT 0 REFERENCES products(id),
  rec INTEGER DEFAULT 3
);

DROP TABLE IF EXISTS sessions_identities;
CREATE TABLE sessions_identities (
  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  session_id INTEGER NOT NULL REFERENCES sessions(id),
  identity_id INTEGER NOT NULL REFERENCES identities(id),
  UNIQUE (session_id, identity_id)
);

DROP TABLE IF EXISTS workitems;
CREATE TABLE workitems (
  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  session INTEGER NOT NULL REFERENCES sessions(id),
  enforcement INTEGER NOT NULL REFERENCES enforcements(id),
  type INTEGER NOT NULL,
  arg_str TEXT,
  arg_int INTEGER DEFAULT 0,
  rec_fail INTEGER NOT NULL,
  rec_noresult INTEGER NOT NULL,
  rec_final INTEGER,
  result TEXT
);
DROP INDEX IF EXISTS workitems_session;
CREATE INDEX workitems_sessions ON workitems (
  session
);

DROP TABLE IF EXISTS results;
CREATE TABLE results (
  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  session INTEGER NOT NULL REFERENCES measurements(id),
  policy INTEGER NOT NULL REFERENCES policies(id),
  rec INTEGER NOT NULL,
  result TEXT NOT NULL
);
DROP INDEX IF EXISTS results_session;
CREATE INDEX results_session ON results (
  session
);

DROP TABLE IF EXISTS components;
CREATE TABLE components (
  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  vendor_id INTEGER NOT NULL,
  name INTEGER NOT NULL,
  qualifier INTEGER DEFAULT 0,
  label TEXT NOT NULL
);

DROP TABLE IF EXISTS component_hashes;
CREATE TABLE component_hashes (
  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  component INTEGER NOT NULL REFERENCES components(id),
  key INTEGER NOT NULL REFERENCES devices(id),
  seq_no INTEGER NOT NULL,
  pcr INTEGER NOT NULL,
  algo INTEGER NOT NULL REFERENCES algorithms(id),
  hash BLOB NOT NULL
);

DROP TABLE IF EXISTS packages;
CREATE TABLE packages (
  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  name TEXT NOT NULL,
  blacklist INTEGER DEFAULT 0
);
DROP INDEX IF EXISTS packages_name;
CREATE INDEX packages_name ON packages (
  name
);

DROP TABLE IF EXISTS versions;
CREATE TABLE versions (
  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  product INTEGER NOT NULL REFERENCES products(id),
  package INTEGER NOT NULL REFERENCES packages(id),
  release TEXT,
  security INTEGER DEFAULT 0,
  blacklist INTEGER DEFAULT 0,
  time INTEGER DEFAULT 0
);
DROP INDEX IF EXISTS versions_release;
CREATE INDEX versions_release ON versions (
  release
);
DROP INDEX IF EXISTS versions_package_product;
CREATE INDEX versions_package_product ON versions (
  package, product
);

DROP TABLE IF EXISTS devices;
CREATE TABLE devices (
  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  description TEXT DEFAULT '',
  value TEXT NOT NULL,
  product INTEGER REFERENCES products(id),
  trusted INTEGER DEFAULT 0,
  created INTEGER,
  inactive INTEGER DEFAULT 0
);
DROP INDEX IF EXISTS devices_id;
CREATE INDEX devices_value ON devices (
  value
);

DROP TABLE IF EXISTS identities;
CREATE TABLE identities (
  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  type INTEGER NOT NULL,
  value BLOB NOT NULL,
  UNIQUE (type, value)
);

DROP TABLE IF EXISTS "swid_entities";
CREATE TABLE "swid_entities" (
  "id" INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  "name" VARCHAR(255) NOT NULL,
  "regid" VARCHAR(255) NOT NULL
);
DROP INDEX IF EXISTS "swid_entities_name";
DROP INDEX IF EXISTS "swid_entities_regid";
CREATE INDEX "swid_entities_name" ON "swid_entities" (
  "name"
);
CREATE INDEX "swid_entities_regid" ON "swid_entities" (
  "regid"
);

DROP TABLE IF EXISTS "swid_entityroles";
CREATE TABLE "swid_entityroles" (
  "id" INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  "tag_id" INTEGER NOT NULL REFERENCES "swid_tags" ("id"),
  "entity_id" INTEGER NOT NULL,
  "role" SMALLINT UNSIGNED NOT NULL
);
DROP INDEX if EXISTS "swid_entityroles_tag_id";
DROP INDEX IF EXISTS "swid_entityroles_tag_entity_id";
CREATE INDEX "swid_entityroles_tag_id" ON "swid_entityroles" (
  "tag_id"
);
CREATE INDEX "swid_entityroles_entity_id" ON "swid_entityroles" (
  "entity_id"
);

DROP TABLE IF EXISTS "swid_tags";
CREATE TABLE "swid_tags" (
  "id" INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  "version_id" INTEGER REFERENCES "versions" ("id"),
  "package_name" VARCHAR(255) NOT NULL,
  "version_str" VARCHAR(255) NOT NULL,
  "unique_id" VARCHAR(255) NOT NULL,
  "swid_xml" TEXT NOT NULL,
  "software_id" VARCHAR(255) NOT NULL
);
DROP INDEX IF EXISTS "swid_tags_software_id";
DROP INDEX if EXISTS "swid_tags_unique_id";
DROP INDEX IF EXISTS "swid_tags_version";
DROP INDEX IF EXISTS "swid_tags_package_name";

CREATE INDEX "swid_tags_software_id" ON "swid_tags" (
  "software_id"
);
CREATE INDEX "swid_tags_unique_id" ON "swid_tags" (
  "unique_id"
);
CREATE INDEX "swid_tags_version_id" ON "swid_tags" (
  "version_id"
);
CREATE INDEX "swid_tags_package_name" ON "swid_tags" (
  "package_name"
);

DROP TABLE IF EXISTS "swid_tags_files";
CREATE TABLE "swid_tags_files" (
  "id" INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  "tag_id" INTEGER NOT NULL,
  "file_id" INTEGER NOT NULL REFERENCES "files" ("id"),
  UNIQUE ("tag_id", "file_id")
);
DROP INDEX IF EXISTS "swid_tags_files_file_id";
DROP INDEX IF EXISTS "swid_tags_files_tag_id";
CREATE INDEX "swid_tags_files_file_id" ON "swid_tags_files" (
  "file_id"
);
CREATE INDEX "swid_tags_files_tag_id" ON "swid_tags_files" (
  "tag_id"
);

DROP TABLE IF EXISTS "swid_tags_sessions";
CREATE TABLE "swid_tags_sessions" (
  "id" INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  "tag_id" INTEGER NOT NULL,
  "session_id" INTEGER NOT NULL REFERENCES "sessions" ("id"),
  UNIQUE ("tag_id", "session_id")
);
DROP INDEX IF EXISTS "swid_tags_sessions_tag_id";
DROP INDEX IF EXISTS "swid_tags_sessions_session_id";
CREATE INDEX "swid_tags_sessions_tag_id" ON "swid_tags_sessions" (
  "tag_id"
);
CREATE INDEX "swid_tags_sessions_session_id" ON "swid_tags_sessions" (
"session_id"
);

DROP TABLE IF EXISTS "swid_tagstats";
CREATE TABLE "swid_tagstats" (
  "id" INTEGER NOT NULL PRIMARY KEY,
  "tag_id" INTEGER NOT NULL REFERENCES "swid_tags" ("id"),
  "device_id" INTEGER NOT NULL REFERENCES "devices" ("id"),
  "first_seen_id" INTEGER NOT NULL REFERENCES "sessions" ("id"),
  "last_seen_id" INTEGER NOT NULL REFERENCES "sessions" ("id"),
  "first_installed_id" INTEGER REFERENCES "swid_events" ("id"),
  "last_deleted_id" INTEGER REFERENCES "swid_events" ("id"),
  UNIQUE ("tag_id", "device_id")
);
CREATE INDEX "swid_tagstats_tag_id" ON "swid_tagstats" ("tag_id");
CREATE INDEX "swid_tagstats_device_id" ON "swid_tagstats" ("device_id");
CREATE INDEX "swid_tagstats_first_seen_id" ON "swid_tagstats" ("first_seen_id");
CREATE INDEX "swid_tagstats_last_seen_id" ON "swid_tagstats" ("last_seen_id");

DROP TABLE IF EXISTS "swid_events";
CREATE TABLE "swid_events" (
  "id" INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  "device" INTEGER REFERENCES "devices" ("id"),
  "epoch" INTEGER NOT NULL,
  "eid" INTEGER NOT NULL,
  "timestamp" CHAR(20) NOT NULL
);
DROP INDEX IF EXISTS "swid_events_device";
CREATE INDEX "swid_events_device" ON "swid_events" (
  "device"
);

DROP TABLE IF EXISTS "swid_tags_events";
CREATE TABLE "swid_tags_events" (
  "id" INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  "tag_id" INTEGER NOT NULL REFERENCES "swid_tags" ("id"),
  "event_id" INTEGER NOT NULL REFERENCES "swid_events" ("id"),
  "action" INTEGER NOT NULL,
  "record_id" INTEGER DEFAULT 0,
  "source_id" INTEGER DEFAULT 0
);
DROP INDEX IF EXISTS "swid_tags_events_event_id";
DROP INDEX IF EXISTS "swid_tags_events_tag_id";
CREATE INDEX "swid_tags_events_event_id" ON "swid_tags_events" (
  "event_id"
);
CREATE INDEX "swid_tags_events_tag_id" ON "swid_tags_events" (
  "tag_id"
);
