/* SQLit database for an Endpoint Collector */

DROP TABLE IF EXISTS "events";
CREATE TABLE "events" (
  "id" INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  "epoch" INTEGER NOT NULL,
  "timestamp" CHAR(20) NOT NULL
);

DROP TABLE IF EXISTS "sw_identifiers";
CREATE TABLE "sw_identifiers" (
  "id" INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  "name" VARCHAR(255) NOT NULL,
  "package" VARCHAR(255) NOT NULL,
  "version" VARCHAR(255) NOT NULL,
  "source" INTEGER DEFAULT 0,
  "installed" INTEGER DEFAULT 1,
  "tag" TEXT
  );
DROP INDEX IF EXISTS "sw_identifiers_name";
CREATE INDEX "sw_identifiers_name" ON "sw_identifiers" (
  "name"
);

DROP TABLE IF EXISTS "sw_events";
CREATE TABLE "sw_events" (
  "id" INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  "eid" INTEGER REFERENCES "events" ("id"),
  "sw_id" INTEGER NOT NULL REFERENCES "sw_identifiers" ("id"),
  "action" INTEGER NOT NULL
);
