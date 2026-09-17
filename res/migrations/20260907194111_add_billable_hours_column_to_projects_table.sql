ALTER TABLE
    projects ADD COLUMN IF NOT EXISTS billable INT;

ALTER TABLE projects
    ADD COLUMN IF NOT EXISTS billable_hours INT;
