CREATE TABLE IF NOT EXISTS attended_meetings
(
    attended_meeting_id INTEGER PRIMARY KEY NOT NULL,

    entry_id TEXT NOT NULL,
    subject TEXT NOT NULL,
    start TEXT NOT NULL,
    end TEXT NOT NULL,
    duration TEXT NOT NULL,
    location TEXT NOT NULL,

    date_created INTEGER NOT NULL DEFAULT (strftime('%s','now', 'localtime')),
    date_modified INTEGER NOT NULL DEFAULT (strftime('%s','now', 'localtime')),
    is_active INTEGER NOT NULL DEFAULT (1)
);
