CREATE TABLE IF NOT EXISTS workdays
(
    workday_id INTEGER PRIMARY KEY NOT NULL,
    date TEXT NOT NULL UNIQUE,
    date_created INTEGER NOT NULL DEFAULT (strftime('%s','now', 'localtime'))
);
