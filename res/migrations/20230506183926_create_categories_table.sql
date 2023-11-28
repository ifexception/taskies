CREATE TABLE IF NOT EXISTS categories
(
    category_id INTEGER PRIMARY KEY NOT NULL,
    name TEXT NOT NULL UNIQUE,
    color INTEGER NOT NULL,
    billable INTEGER NOT NULL DEFAULT (0),
    description TEXT NULL,
    date_created INTEGER NOT NULL DEFAULT (strftime('%s','now', 'localtime')),
    date_modified INTEGER NOT NULL DEFAULT (strftime('%s','now', 'localtime')),
    is_active INTEGER NOT NULL DEFAULT (1),

    project_id INTEGER NULL,

    FOREIGN KEY (project_id) REFERENCES projects(project_id)
);
