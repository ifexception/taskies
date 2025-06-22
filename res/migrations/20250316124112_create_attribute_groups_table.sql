CREATE TABLE IF NOT EXISTS attribute_groups
(
    attribute_group_id INTEGER PRIMARY KEY NOT NULL,

    name TEXT NOT NULL UNIQUE,
    description TEXT NULL,
    is_static INTEGER NOT NULL,
    is_default INTEGER NOT NULL DEFAULT (0),

    date_created INTEGER NOT NULL DEFAULT (strftime('%s','now', 'localtime')),
    date_modified INTEGER NOT NULL DEFAULT (strftime('%s','now', 'localtime')),
    is_active INTEGER NOT NULL DEFAULT (1)
);
