CREATE TABLE wormholes (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    source_map TEXT NOT NULL,
    source_x REAL NOT NULL,
    source_y REAL NOT NULL,
    source_yaw REAL NOT NULL,
    target_map TEXT NOT NULL,
    target_x REAL NOT NULL,
    target_y REAL NOT NULL,
    target_yaw REAL NOT NULL
);

INSERT INTO wormholes (source_map, source_x, source_y, source_yaw, target_map, target_x, target_y, target_yaw)
VALUES ('room1', 2.0, 0.0, 0.0, 'room2', 0.0, 0.0, 0.0);

INSERT INTO wormholes (source_map, source_x, source_y, source_yaw, target_map, target_x, target_y, target_yaw)
VALUES ('room2', 0.0, 0.0, 0.0, 'room1', 2.0, 0.0, 0.0);
