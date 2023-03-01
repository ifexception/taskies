CREATE TABLE employers
(
    employer_id INTEGER PRIMARY KEY NOT NULL,
    name TEXT NOT NULL,
    description TEXT NULL,
    date_created INTEGER NOT NULL DEFAULT (strftime('%s','now', 'localtime')),
    date_modified INTEGER NULL,
    is_active INTEGER NOT NULL DEFAULT (1)
);
