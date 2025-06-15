CREATE TABLE IF NOT EXISTS attributes
(
    attribute_id INTEGER PRIMARY KEY NOT NULL,
    name TEXT NOT NULL,
    description TEXT NULL,
    is_required INT NOT NULL,
    date_created INTEGER NOT NULL DEFAULT (strftime('%s','now', 'localtime')),
    date_modified INTEGER NOT NULL DEFAULT (strftime('%s','now', 'localtime')),
    is_active INTEGER NOT NULL DEFAULT (1),

    attribute_group_id INT NOT NULL,
    attribute_type_id INT NOT NULL,

    FOREIGN KEY (attribute_group_id) REFERENCES attribute_groups(attribute_group_id),
    FOREIGN KEY (attribute_type_id) REFERENCES attribute_types(attribute_type_id)
);
