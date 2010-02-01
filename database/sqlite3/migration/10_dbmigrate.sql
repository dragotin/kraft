-- 5_dbmigrate.sql
-- message Add a list value identification column to the attribute table
--ALTER TABLE attributes ADD COLUMN valueIsList tinyint default 0; -- AFTER value;

DROP TABLE IF EXISTS tmp_attrib;

CREATE TABLE tmp_attrib (
  id         INTEGER PRIMARY KEY ASC autoincrement,
  hostObject VARCHAR(64),
  hostId     INT,
  name       VARCHAR(64),
  value      MEDIUMTEXT,
  valueIsList TINYINT
);
-- CREATE UNIQUE INDEX tmpIndx_10 ON tmp_attrib( hostObject, hostId, name );

INSERT INTO tmp_attrib (hostObject, hostId, name, value, valueIsList) SELECT hostObject, hostId, name, value, 0 FROM attributes;


-- message Create an attribute value table
CREATE TABLE IF NOT EXISTS attributeValues (
  id          INTEGER PRIMARY KEY ASC autoincrement,
  attributeId INT NOT NULL,
  value      VARCHAR(255)
);
CREATE INDEX attribValueIndx_10 ON attributeValues( attributeId );

-- message copy the attribute values over to the new attribute value table
INSERT INTO attributeValues (attributeId, value) SELECT id, value FROM tmp_attrib WHERE value is not null;

-- message drop the attrib column
-- ALTER TABLE tmp_attrib DROP COLUMN value;

-- DROP TABLE attributes;
ALTER TABLE attributes RENAME TO attributes_unused;

CREATE TABLE attributes (
  id         INTEGER PRIMARY KEY ASC autoincrement,
  hostObject VARCHAR(64),
  hostId     INT,
  name       VARCHAR(64),
  valueIsList TINYINT,
  relationTable varchar(64) default NULL,
  relationIDColumn varchar(64) default NULL,
  relationStringColumn varchar(64) default NULL
);
CREATE UNIQUE INDEX attribIndx_10 ON attributes( hostObject, hostId, name );
INSERT INTO attributes (hostObject, hostId, name, valueIsList) SELECT hostObject, hostId, name, valueIsList FROM tmp_attrib;

-- message create a table to keep tag templates
CREATE TABLE IF NOT EXISTS tagTemplates (
  tagTmplID    INTEGER PRIMARY KEY ASC autoincrement,
  sortkey      int NOT NULL,
  name         varchar(255) default NULL,
  description  varchar(255) default NULL,
  color        char(7) default NULL
);


INSERT INTO tagTemplates (sortkey, name, description, color) VALUES (3, 'Discount', 'Marks items to give discount on', '#ff1c1c' );
INSERT INTO tagTemplates (sortkey, name, description, color) VALUES (1, 'Material', 'Marks material', '#4e4e4e' );
INSERT INTO tagTemplates (sortkey, name, description, color) VALUES (2, 'Work', 'Marks working hour items', '#ffbb39' );
INSERT INTO tagTemplates (sortkey, name, description, color) VALUES (4, 'Plants', 'Marks plant items', '#26b913' );


DROP TABLE IF EXISTS tmp_attrib;
DROP TABLE IF EXISTS attributes_unused;

