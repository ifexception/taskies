PRAGMA foreign_keys=off;

CREATE TABLE IF NOT EXISTS tasks_new
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
    attribute_group_id INTEGER NULL,
    attended_meeting_id INTEGER NULL,

    FOREIGN KEY (project_id) REFERENCES projects(project_id),
    FOREIGN KEY (category_id) REFERENCES categories(category_id),
    FOREIGN KEY (workday_id) REFERENCES workdays(workday_id),
    FOREIGN KEY (attribute_group_id) REFERENCES attribute_groups(attribute_group_id),
    FOREIGN KEY (attended_meeting_id) REFERENCES attended_meetings(attended_meeting_id)
);

INSERT INTO tasks_new
(
    task_id,
    billable,
    unique_identifier,
    hours,
    minutes,
    description,
    date_created,
    date_modified,
    is_active,
    project_id,
    category_id,
    workday_id,
    attribute_group_id
)
    SELECT
        task_id,
        billable,
        unique_identifier,
        hours,
        minutes,
        description,
        date_created,
        date_modified,
        is_active,
        project_id,
        category_id,
        workday_id,
        attribute_group_id
    FROM tasks;

DROP TABLE tasks;

ALTER TABLE tasks_new RENAME TO tasks;

PRAGMA foreign_keys=on;
