CREATE TABLE IF NOT EXISTS persistent_objects
(
    key TEXT PRIMARY KEY,
    value TEXT NOT NULL,

    UNIQUE (key) ON CONFLICT REPLACE
);
