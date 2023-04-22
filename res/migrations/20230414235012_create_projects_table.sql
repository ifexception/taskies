CREATE TABLE IF NOT EXISTS projects
(
    project_id INTEGER PRIMARY KEY NOT NULL,
    name TEXT NOT NULL UNIQUE,
    display_name TEXT NOT NULL,
    is_default INTEGER NOT NULL DEFAULT (0),
    description TEXT NULL,
    date_created INTEGER NOT NULL DEFAULT (strftime('%s','now', 'localtime')),
    date_modified INTEGER NOT NULL DEFAULT (strftime('%s','now', 'localtime')),
    is_active INTEGER NOT NULL DEFAULT (1),

    employer_id INTEGER NOT NULL,
    client_id INTEGER NULL,

    FOREIGN KEY (employer_id) REFERENCES employers(employer_id),
    FOREIGN KEY (client_id) REFERENCES clients(client_id)
);
