# message allow laternatives and demand positions for offers 
SELECT @item := docTypeID FROM DocTypes WHERE name="Offer";
INSERT IGNORE INTO attributes (hostObject, hostId, name, value) VALUES ('DocType', @item, 'AllowAlternative', '1');
INSERT IGNORE INTO attributes (hostObject, hostId, name, value) VALUES ('DocType', @item, 'AllowDemand', '1');

SELECT @item := docTypeID FROM DocTypes WHERE name="Angebot";
INSERT IGNORE INTO attributes (hostObject, hostId, name, value) VALUES ('DocType', @item, 'AllowAlternative', '1');
INSERT IGNORE INTO attributes (hostObject, hostId, name, value) VALUES ('DocType', @item, 'AllowDemand', '1');

# message Add a list value identification column to the attribute table
ALTER TABLE attributes ADD COLUMN valueIsList tinyint default 0 after value;

DROP TABLE IF EXISTS tmp_attrib;

CREATE TABLE tmp_attrib (
  id INT NOT NULL AUTO_INCREMENT,
  hostObject VARCHAR(64),
  hostId     INT,
  name       VARCHAR(64),
  value      MEDIUMTEXT,
  valueIsList TINYINT,

  PRIMARY KEY(id),
  UNIQUE INDEX( hostObject, hostId, name )
);

INSERT INTO tmp_attrib (hostObject, hostId, name, value, valueIsList) SELECT hostObject, hostId, name, value, 0 FROM attributes;


# message Create an attribute value table
CREATE TABLE IF NOT EXISTS attributeValues (
  id INT NOT NULL AUTO_INCREMENT,
  attributeId INT NOT NULL,
  value      VARCHAR(255),

  PRIMARY KEY( id ),
  INDEX( attributeId )
);

# message copy the attribute values over to the new attribute value table
INSERT INTO attributeValues (attributeId, value) SELECT id, value FROM tmp_attrib WHERE value is not null;

# message drop the attrib column
ALTER TABLE tmp_attrib DROP COLUMN value;

DROP TABLE IF EXISTS attribute_old;
RENAME TABLE attributes TO attribute_old, tmp_attrib TO attributes;

# message create a table to keep tag templates
CREATE TABLE IF NOT EXISTS `tagTemplates` (
  `tagTmplID` int(11) NOT NULL auto_increment,
  `sortkey` int(11) NOT NULL,
  `name` varchar(255) default NULL,
  `description` varchar(255) default NULL,
  `color` char(7) default NULL,
  PRIMARY KEY  (`tagTmplID`),
  KEY `sortkey` (`sortkey`)
);

INSERT IGNORE INTO tagTemplates (sortkey, name, description, color) VALUES (3, 'Discount', 'Marks items to give discount on', '#ff1c1c' );
INSERT IGNORE INTO tagTemplates (sortkey, name, description, color) VALUES (1, 'Material', 'Marks material', '#4e4e4e' );
INSERT IGNORE INTO tagTemplates (sortkey, name, description, color) VALUES (2, 'Work', 'Marks working hour items', '#ffbb39' );
INSERT IGNORE INTO tagTemplates (sortkey, name, description, color) VALUES (4, 'Plants', 'Marks plant items', '#26b913' );

