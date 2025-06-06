CREATE TABLE IF NOT EXISTS static_attribute_values
(
    static_attribute_value_id INTEGER PRIMARY KEY NOT NULL,

    text_value TEXT NULL,
    boolean_value INTEGER NULL,
    numeric_value INTEGER NULL,

    date_created INTEGER NOT NULL DEFAULT (strftime('%s','now', 'localtime')),
    date_modified INTEGER NOT NULL DEFAULT (strftime('%s','now', 'localtime')),
    is_active INTEGER NOT NULL DEFAULT (1),

    attribute_group_id INT NOT NULL,
    attribute_id INT NOT NULL,

    FOREIGN KEY (attribute_group_id) REFERENCES attribute_groups(attribute_group_id),
    FOREIGN KEY (attribute_id) REFERENCES attributes(attribute_id)
);
