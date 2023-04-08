CREATE TABLE clients
(
    client_id INTEGER PRIMARY KEY NOT NULL,
    name TEXT NOT NULL,
    description TEXT NULL,
    date_created INTEGER NOT NULL DEFAULT (strftime('%s','now', 'localtime')),
    date_modified INTEGER NOT NULL DEFAULT (strftime('%s','now', 'localtime')),
    is_active INTEGER NOT NULL DEFAULT (1),

    employer_id INTEGER NOT NULL,

    FOREIGN KEY (employer_id) REFERENCES employers(employer_id)
);
