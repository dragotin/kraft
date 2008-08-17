# message allow laternatives and demand positions for offers 
SELECT @item := docTypeID FROM DocTypes WHERE name="Offer";
INSERT INTO attributes (hostObject, hostId, name, value) VALUES ('DocType', @item, 'AllowAlternative', '1');
INSERT INTO attributes (hostObject, hostId, name, value) VALUES ('DocType', @item, 'AllowDemand', '1');

SELECT @item := docTypeID FROM DocTypes WHERE name="Angebot";
INSERT INTO attributes (hostObject, hostId, name, value) VALUES ('DocType', @item, 'AllowAlternative', '1');
INSERT INTO attributes (hostObject, hostId, name, value) VALUES ('DocType', @item, 'AllowDemand', '1');

# message Add a list value identification column to the attribute table
ALTER TABLE attributes ADD COLUMN valueIsList tinyint default 0 after value;

# ALTER TABLE attributes DROP PRIMARY KEY;
ALTER TABLE attributes ADD COLUMN id int not null  auto_increment primary key FIRST;
ALTER TABLE attributes ADD UNIQUE INDEX ( hostObject, hostId, name );

# message Create an attribute value table
CREATE TABLE attributeValues (
  id INT NOT NULL AUTO_INCREMENT,
  attributeId INT NOT NULL,
  value      VARCHAR(255),

  PRIMARY KEY( id ),
  INDEX( attributeId )
);

# message copy the attribute values over to the new attribute value table
INSERT INTO attributeValues (attributeId, value) SELECT id, value FROM attributes WHERE value is not null;

# message drop the attrib column
ALTER TABLE attributes DROP COLUMN value;

# message create a table to keep tag templates
CREATE TABLE `tagTemplates` (
  `tagTmplID` int(11) NOT NULL auto_increment,
  `sortkey` int(11) NOT NULL,
  `name` varchar(255) default NULL,
  `description` varchar(255) default NULL,
  `color` char(7) default NULL,
  PRIMARY KEY  (`tagTmplID`),
  KEY `sortkey` (`sortkey`)
);

INSERT INTO tagTemplates (sortkey, name, description, color) VALUES (3, 'discount', 'Marks items to give discount on', '#ff1c1c' );
INSERT INTO tagTemplates (sortkey, name, description, color) VALUES (1, 'material', 'Marks material', '#4e4e4e' );
INSERT INTO tagTemplates (sortkey, name, description, color) VALUES (2, 'work', 'Marks working hour items', '#ffbb39' );
INSERT INTO tagTemplates (sortkey, name, description, color) VALUES (4, 'plants', 'Marks plant items', '#26b913' );

