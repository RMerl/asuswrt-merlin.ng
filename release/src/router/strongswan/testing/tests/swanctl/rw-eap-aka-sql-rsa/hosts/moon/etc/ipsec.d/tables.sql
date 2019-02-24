DROP TABLE IF EXISTS quintuplets;
CREATE TABLE quintuplets (
    id TEXT NOT NULL,
    used INTEGER NOT NULL,
    rand BLOB NOT NULL,
    autn BLOB NOT NULL,
    ck BLOB NOT NULL,
    ik BLOB NOT NULL,
    res BLOB NOT NULL
);
