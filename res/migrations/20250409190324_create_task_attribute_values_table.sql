CREATE TABLE IF NOT EXISTS task_attribute_values
(
    task_attribute_value_id INTEGER PRIMARY KEY NOT NULL,

    text_value TEXT NOT NULL,
    boolean_value INTEGER NOT NULL,
    numeric_value INTEGER NOT NULL,

    date_created INTEGER NOT NULL DEFAULT (strftime('%s','now', 'localtime')),
    date_modified INTEGER NOT NULL DEFAULT (strftime('%s','now', 'localtime')),
    is_active INTEGER NOT NULL DEFAULT (1),

    task_id INT NOT NULL,
    attribute_id INT NOT NULL,
    attribute_type_id INT NOT NULL,

    FOREIGN KEY (task_id) REFERENCES tasks(task_id),
    FOREIGN KEY (attribute_id) REFERENCES attributes(attribute_id),
    FOREIGN KEY (attribute_type_id) REFERENCES attribute_types(attribute_type_id)
);
