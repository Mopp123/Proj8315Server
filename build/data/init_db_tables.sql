
-- Create default tables
-- Users
CREATE TABLE users (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    name VARCHAR(32) UNIQUE NOT NULL,
    password VARCHAR(32),
    logged_in BOOLEAN DEFAULT FALSE,
    -- NOTE: tile coords in db are recorded as coordinates where user logged out / disconnected
    tile_x INTEGER DEFAULT 0,
    tile_z INTEGER DEFAULT 0
);

-- Factions
CREATE TABLE factions (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    user_id UUID,
    name VARCHAR(32) UNIQUE NOT NULL,
    deployments INTEGER ARRAY[16],
    CONSTRAINT fk_user FOREIGN KEY(user_id) REFERENCES users(id) ON DELETE CASCADE
);

-- Objects (obj infos/templates)
CREATE TABLE objects (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    name VARCHAR(32) UNIQUE NOT NULL,
    description VARCHAR(64),
    stats_speed INTEGER DEFAULT 0,

    -- Object type specific actions (doesn't include common actions)
    -- NOTE: Char len needs to be same as: OBJECT_DATA_STRLEN_ACTION_NAME
    -- NOTE: Arr len needs to be same as: TILE_STATE_MAX_action + 1
    action_slot VARCHAR(16) ARRAY[8] DEFAULT '{}'
);

INSERT INTO objects(
    name,
    description,
    stats_speed
) VALUES(
    'Empty',
    'Nothingness...',
    0
);

INSERT INTO objects(
    name,
    description,
    stats_speed,
    action_slot
)
VALUES
(
    'Tree1',
    'Testing tree object here...',
    0,
    '{}'
),
(
    'Movement Test',
    'For testing movement stuff',
    2,
    '{}'
),
(
    'Ship Test',
    'For testing ship landing',
    2,
    '{Deploy}'
);