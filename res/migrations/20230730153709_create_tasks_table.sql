CREATE TABLE IF NOT EXISTS tasks
(
    task_id INTEGER PRIMARY KEY NOT NULL,
    billable INTEGER NOT NULL,
    unique_identifier TEXT NULL,
    hours INTEGER NOT NULL,
    minutes INTEGER NOT NULL,
    description TEXT NOT NULL,
    date_created INTEGER NOT NULL DEFAULT (strftime('%s','now', 'localtime')),
    date_modified INTEGER NOT NULL DEFAULT (strftime('%s','now', 'localtime')),
    is_active INTEGER NOT NULL DEFAULT (1),

    project_id INTEGER NOT NULL,
    category_id INTEGER NOT NULL,
    workday_id INTEGER NOT NULL,

    FOREIGN KEY (project_id) REFERENCES projects(project_id),
    FOREIGN KEY (category_id) REFERENCES categories(category_id),
    FOREIGN KEY (workday_id) REFERENCES workdays(workday_id)
);
